#!/system/bin/sh

injectprop(){
    /system/bin/injectprop "$1" "$2";
}

IFS=$"\n"
part="
.
.bootimage.
.odm_dlkm.
.odm.
.oem.
.product.
.system_ext.
.system.
.vendor_dlkm.
.vendor."

# override props
# comment out "exit 0" line if you want to override props
exit 0

# example
for n in $part; do
    injectprop ro${n}fingerprint xiaomi/lavender/lavender:10/QKQ1.190910.002/V12.5.3.0.QFGCNXM:user/release-keys
done