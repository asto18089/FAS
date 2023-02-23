# !/system/bin/sh
MODDIR=${0%/*}

wait_until_login() {
    # in case of /data encryption is disabled
    while [ "$(getprop sys.boot_completed)" != "1" ]; do
        sleep 1
    done
    # no need to start before the user unlocks the screen
    local test_file="/sdcard/Android/.LOGIN_PERMISSION_TEST"
    true > "$test_file"
    while [ ! -f "$test_file" ]; do
        true > "$test_file"
        sleep 1
    done
    rm "$test_file"
}
wait_until_login

chmod a+x $MODDIR/FAS

nohup $MODDIR/FAS >/dev/null 2>&1 &