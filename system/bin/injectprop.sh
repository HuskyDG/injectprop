#!/system/bin/sh

##########
# CONFIG #
##########

# Set to inject_props=1 if you want to apply props bellow
inject_props=0

# Set to killgms=1 if you want to force close google services process
killgms=0

# configure information
BUILD_FINGERPRINT=OnePlus/OnePlus7Pro_EEA/OnePlus7Pro:10/QKQ1.190716.003/1910071200:user/release-keys
BUILD_DESCRIPTION=OnePlus7Pro-user 10 QKQ1.190716.003 1910071200 release-keys
BUILD_SECURITY_PATCH_DATE=2019-09-05
BUILD_VERSION_RELEASE=10
BUILD_VERSION_RELEASE_OR_CODENAME=10
MANUFACTURER_NAME=OnePlus
PRODUCT_NAME=OnePlus7Pro
BUILD_TAGS=release-keys
BUILD_TYPE=user
DEBUGGABLE=0



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

exec &>/dev/injectprop.log

if [ "$inject_props" == 1 ]; then
# early props
  for n in $part; do
    injectprop ro${n}build.fingerprint "$BUILD_FINGERPRINT"
    injectprop ro${n}build.tags "$BUILD_TAGS"
    injectprop ro${n}build.type "$BUILD_TYPE"
    injectprop ro.product${n}name "$PRODUCT_NAME"
    injectprop ro.debuggable "$DEBUGGABLE"
    injectprop ro.build.description "$BUILD_DESCRIPTION"
    injectprop ro.build.version.security_patch "$BUILD_SECURITY_PATCH_DATE"
  done

# some props should be set after boot_completed
  while [ "$(getprop sys.boot_completed)" != 1 ]; do
    sleep 1
  done
  for n in $part; do
    injectprop ro.product${n}manufacturer "$MANUFACTURER_NAME"
    injectprop ro${n}build.version.release "$BUILD_VERSION_RELEASE"
    injectprop ro${n}build.version.release_or_codename "$BUILD_VERSION_RELEASE_OR_CODENAME"
  done
fi

if [ "$killgms" == 1 ]; then
  killall -9 com.google.android.gms
  killall -9 com.android.vending
fi
