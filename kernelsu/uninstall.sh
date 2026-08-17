#!/system/bin/sh

if [ -d /sys/module/k100pm_chg_runtime ]; then
	rmmod k100pm_chg_runtime 2>/dev/null || true
fi
