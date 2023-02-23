SKIPUNZIP=0
MODDIR=${0%/*}
alias sh='/system/bin/sh'
echo "----------------------------------------------------"

echo -e "\nPlease wait…"
echo "请等待…"

# no uperf
if [ $(pidof uperf) != "" ]; then
    echo "Uperf detected, please remove."
    echo "检测到uperf，请移除"abort
fi

# permission
chmod a+x $MODPATH/FAS

# start on install
killall FAS > /dev/null 2>&1

echo "----------------------------------------------------"

$MODPATH/FAS &