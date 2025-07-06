#!/bin/sh

{
for i in /proc/device-tree/soc/oplus,ufcs_charge/silicon_p_770/ufcs_charge_oplus_strategy/*; do
    [ -d "$i" ] || continue
    for j in "$i"/*; do
        echo "$(realpath $j | sed 's#/sys/firmware/devicetree/base##g')"
    done
done

} >m.txt
sed -i '/name/d' m.txt
