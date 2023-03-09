if [[ "$mode" = "init" ]]; then 
touch "/storage/emulated/0/Android/FAS/target_temp"
elif [[ "$scene" = "standby" ]]; then 
echo 80 > "/storage/emulated/0/Android/FAS/target_temp"
elif [[ "$mode" = "powersave" ]]; then 
echo 65 > "/storage/emulated/0/Android/FAS/target_temp"
elif [[ "$mode" = "balance" ]]; then 
echo 75 > "/storage/emulated/0/Android/FAS/target_temp"
elif [[ "$mode" = "performance" ]]; then 
echo 94 > "/storage/emulated/0/Android/FAS/target_temp"
elif [[ "$mode" = "fast" ]]; then 
echo 85 > "/storage/emulated/0/Android/FAS/target_temp"
elif [[ "$mode" = "pedestal" ]]; then 
echo 114 > "/storage/emulated/0/Android/FAS/target_temp"
fi