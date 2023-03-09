SKIPUNZIP=0
MODDIR=${0%/*}
alias sh='/system/bin/sh'
echo "----------------------------------------------------"

echo -e "\nPlease wait…"
echo "请等待…"

# no uperf
if [ $(pidof uperf) != "" ]; then
    echo "Uperf detected, please remove."
    echo "检测到uperf, 请移除"abort
fi

mkdir -p /storage/emulated/0/Android/FAS

# permission
chmod a+x $MODPATH/DFAS
# start on install
killall DFAS > /dev/null 2>&1
killall FAS
[[ -d /data/adb/modules/FAS || -d /data/adb/modules_update/FAS ]] && rm -rf /data/adb/modules/FAS && rm -rf /data/adb/modules_update/FAS

echo "----------------------------------------------------"
nohup $MODPATH/DFAS >/dev/null 2>&1 &
# scene
echo 75 > "/storage/emulated/0/Android/FAS/target_temp"
cp -f "$MODPATH/powercfg.sh" "/data/powercfg.sh"
cp -f "$MODPATH/powercfg.json" "/data/powercfg.json"