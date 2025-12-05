// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) KBUILD_MODNAME ":%s:%d: " fmt, __func__, __LINE__

#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/swap.h>
#include <linux/rcupdate.h>
#include <linux/profile.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/swap.h>
#include <linux/fs.h>
#include <linux/cpuset.h>
#include <linux/vmpressure.h>
#include <linux/circ_buf.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/sched.h>
struct task_struct *find_task_mm(struct task_struct *p);

#define PFA_LMK_WAITING		8	/* LMK is waiting */
#define PFA_LMK_PROTECT		9	/* LMK protect task */

TASK_PFA_TEST(LMK_WAITING, lmk_waiting)
TASK_PFA_SET(LMK_WAITING, lmk_waiting)

TASK_PFA_TEST(LMK_PROTECT, lmk_protect)
TASK_PFA_SET(LMK_PROTECT, lmk_protect)
TASK_PFA_CLEAR(LMK_PROTECT, lmk_protect)

#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			pr_info(x);			\
	} while (0)

static u32 lowmem_debug_level = 1;

static int lowmem_whitelist[128] = {
	0,
	1,
};
static int lowmem_whitelist_size = 2;

static short lowmem_adj[6] = {
	0,
	1,
	6,
	12,
};
static int lowmem_adj_size = 4;

static int lowmem_minfree[6] = {
	3 * 512,	/* 1536 * 4k = 6M */
	2 * 1024,	/* 2048 * 4k = 8M */
	4 * 1024,	/* 4096 * 4k = 16M */
	16 * 1024,	/* 16384 * 4k = 64M */
};
static int lowmem_minfree_size = 4;

#ifdef CONFIG_XRING_LMK_MULIT_KILL
static int lowmem_multi_kill;
static int lowmem_multi_count;
static int lowmem_timeout_inter = 1;
#endif

static unsigned long lowmem_deathpending_timeout;

static void set_task_protect(void) {
	int i;

	for (i = 0; i < lowmem_whitelist_size; ++i) {
		struct task_struct *task_protect = NULL;

		task_protect = get_pid_task(find_vpid(lowmem_whitelist[i]), PIDTYPE_PID);
		if (task_protect) {
			task_set_lmk_protect(task_protect);
			put_task_struct(task_protect);
			lowmem_print(2, "set %s (%d) protect, task usage:%u\n",
				task_protect->comm, task_protect->pid, refcount_read(&task_protect->usage));
		}
	}
}

static void clear_task_protect(void) {
	int i;

	for (i = 0; i < lowmem_whitelist_size; ++i) {
		struct task_struct *task_protect = NULL;

		task_protect = get_pid_task(find_vpid(lowmem_whitelist[i]), PIDTYPE_PID);
		if (task_protect) {
			task_clear_lmk_protect(task_protect);
			put_task_struct(task_protect);
			lowmem_print(2, "clear %s (%d) protect, task usage:%u\n",
				task_protect->comm, task_protect->pid, refcount_read(&task_protect->usage));
		}
	}
}

static unsigned long lowmem_count(struct shrinker *s,
				  struct shrink_control *sc)
{

	return global_node_page_state(NR_ACTIVE_ANON) +
		global_node_page_state(NR_ACTIVE_FILE) +
		global_node_page_state(NR_INACTIVE_ANON) +
		global_node_page_state(NR_INACTIVE_FILE);
}

static unsigned long lowmem_scan(struct shrinker *s, struct shrink_control *sc)
{
	struct task_struct *tsk;
	struct task_struct *selected = NULL;
	unsigned long rem = 0;
	int tasksize;
	int i;
	short min_score_adj = OOM_SCORE_ADJ_MAX + 1;
	int minfree = 0;
	int times = 0;
	int selected_tasksize = 0;
	short selected_oom_score_adj;
	int array_size = ARRAY_SIZE(lowmem_adj);

	int other_free = global_zone_page_state(NR_FREE_PAGES);
	int other_file = global_node_page_state(NR_FILE_PAGES) -
				global_node_page_state(NR_SHMEM) -
				total_swapcache_pages();

	if (lowmem_adj_size < array_size)
		array_size = lowmem_adj_size;
	if (lowmem_minfree_size < array_size)
		array_size = lowmem_minfree_size;

	for (i = 0; i < array_size; i++) {
		minfree = lowmem_minfree[i];
		if (other_free < minfree && other_file < minfree) {
			lowmem_print(3, "lowmem_adj judge with file page\n");
			min_score_adj = lowmem_adj[i];
			break;
		}
	}

	lowmem_print(3, "%s %lu, %x, ofree %d %d, ma %hd\n",
				__func__, sc->nr_to_scan, sc->gfp_mask, other_free,
				other_file, min_score_adj);

	if (min_score_adj == OOM_SCORE_ADJ_MAX + 1) {
		lowmem_print(5, "%s %lu, %x, min_score_adj: %d return 0\n",
				__func__, sc->nr_to_scan, sc->gfp_mask, min_score_adj);
		return 0;
	}

	selected_oom_score_adj = min_score_adj;

	rcu_read_lock();
	set_task_protect();

#ifdef CONFIG_XRING_LMK_MULIT_KILL
kill_selected:
#endif
	for_each_process(tsk) {
		struct task_struct *p;
		short oom_score_adj;

		if (tsk->flags & PF_KTHREAD)
			continue;

		p = find_task_mm(tsk);
		if (!p)
			continue;

		oom_score_adj = p->signal->oom_score_adj;
		if (oom_score_adj < min_score_adj) {
			task_unlock(p);
			continue;
		}

		if (p->__state & TASK_UNINTERRUPTIBLE) {
			lowmem_print(2, "%s filter D state process: %d (%s) state:0x%x\n",
					__func__, p->pid, p->comm, p->__state);
			task_unlock(p);
			continue;
		}

		if (task_lmk_protect(p)) {
			task_unlock(p);
			continue;
		}

		if (task_lmk_waiting(p)) {
			if (time_before_eq(jiffies, lowmem_deathpending_timeout)) {
				task_unlock(p);
				rcu_read_unlock();
				clear_task_protect();
				return SHRINK_STOP;
			}
#ifdef CONFIG_XRING_LMK_MULIT_KILL
			if (lowmem_multi_kill) {
				task_unlock(p);
				continue;
			}
#endif
		}

		tasksize = get_mm_rss(p->mm);
		task_unlock(p);
		if (tasksize <= 0)
			continue;

		if (selected) {
			if (oom_score_adj < selected_oom_score_adj)
				continue;
			if (oom_score_adj == selected_oom_score_adj &&
				tasksize <= selected_tasksize)
				continue;
		}
		selected = p;
		selected_tasksize = tasksize;
		selected_oom_score_adj = oom_score_adj;
		lowmem_print(2, "select '%s' (%d), adj %hd, size %d, to kill\n",
				p->comm, p->pid, oom_score_adj, tasksize);
	}

	if (selected) {
		task_lock(selected);
		send_sig(SIGKILL, selected, 0);
		if (selected->mm)
			task_set_lmk_waiting(selected);
		task_unlock(selected);
		lowmem_print(1, "Killing %s (%d) at times: %d, adj %hd, to free %ldkB\n"
				"   on behalf of %s (%d) because cache %ldkB is below limit %ldkB\n"
				"   for oom_score_adj %hd Free memory is %ldkB above reserved.\n",
				selected->comm, selected->pid, times,
				selected_oom_score_adj,
				selected_tasksize * (long)(PAGE_SIZE / 1024),
				current->comm, current->pid,
				other_file * (long)(PAGE_SIZE / 1024),
				minfree * (long)(PAGE_SIZE / 1024),
				min_score_adj,
				other_free * (long)(PAGE_SIZE / 1024));

#ifdef CONFIG_XRING_LMK_MULIT_KILL
		/*lint !e647*/
		lowmem_deathpending_timeout = jiffies + lowmem_timeout_inter * HZ;
#else
		lowmem_deathpending_timeout = jiffies + HZ;
#endif

		rem += selected_tasksize;
	}

	lowmem_print(4, "%s %lu, %x, return %lu\n",
			__func__, sc->nr_to_scan, sc->gfp_mask, rem);

#ifdef CONFIG_XRING_LMK_MULIT_KILL
	if (selected) {
		times++;
		if (times < lowmem_multi_count) {
			selected = NULL;
			goto kill_selected;
		}
	}
#endif
	clear_task_protect();

	rcu_read_unlock();

	return rem;
}

static struct shrinker lowmem_shrinker = {
	.scan_objects = lowmem_scan,
	.count_objects = lowmem_count,
	.seeks = DEFAULT_SEEKS * 16,
};

static int __init lowmem_init(void)
{
	register_shrinker(&lowmem_shrinker, "xring-lmk");
	return 0;
}

static void __exit lowmem_exit(void)
{
	unregister_shrinker(&lowmem_shrinker);
}

module_param_named(cost, lowmem_shrinker.seeks, int, 0644);
module_param_array_named(adj, lowmem_adj, short, &lowmem_adj_size, 0644);
module_param_array_named(minfree, lowmem_minfree, uint, &lowmem_minfree_size, 0644);
module_param_named(debug_level, lowmem_debug_level, uint, 0644);
module_param_array_named(whitelist, lowmem_whitelist, uint, &lowmem_whitelist_size, 0644);
#ifdef CONFIG_XRING_LMK_MULIT_KILL
module_param_named(multi_kill, lowmem_multi_kill, int, 0644);
module_param_named(multi_count, lowmem_multi_count, int, 0644);
module_param_named(timeout_inter, lowmem_timeout_inter, int, 0644);
#endif

module_init(lowmem_init);
module_exit(lowmem_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring Xring LMK");
MODULE_LICENSE("GPL v2");
