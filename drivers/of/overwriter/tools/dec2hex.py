#!/usr/bin/env python3
import re, sys, fileinput

# 匹配十进制整数，排除 0x 开头
dec_re = re.compile(r'\b(?<!0x)(?<!\w)\d+(?!\w)\b')

def repl(m):
    return hex(int(m.group(0)))

def main():
    file = sys.argv[1]
    # 先读全文件，再写回（fileinput 不支持原地覆盖模式）
    with open(file, 'r', encoding='utf-8') as f:
        data = f.read()

    new_data = dec_re.sub(repl, data)

    with open(file, 'w', encoding='utf-8') as f:
        f.write(new_data)

if __name__ == '__main__':
    main()
