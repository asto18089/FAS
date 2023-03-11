# !/system/bin/sh
MODDIR=${0%/*}

wait_until_login() {
    while [ "$(getprop sys.boot_completed)" != "1" ]; do
        sleep 1
    done
    while [ ! -d "/sdcard/Android" ]; do
        sleep 1
    done
}
wait_until_login

chmod a+x $MODDIR/DFAS

nohup $MODDIR/DFAS >/dev/null 2>&1 &
[[ ! -f "/data/powercfg.sh" ]] && cp -f "$MODDIR/powercfg.sh" "/data/powercfg.sh"
[[ ! -f "/data/powercfg.json" ]] && cp -f "$MODDIR/powercfg.json" "/data/powercfg.json"