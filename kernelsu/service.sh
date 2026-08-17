#!/system/bin/sh

MODDIR=${0%/*}
KMOD="$MODDIR/kmod/K100PM_CHG_RUNTIME.ko"
MODNAME=k100pm_chg_runtime
DEFAULT_PROFILE="$MODDIR/profile.sh"
USER_PROFILE=/data/adb/k100pm_chg_runtime.conf
TAG=K100PM_CHG_RUNTIME

[ -r "$DEFAULT_PROFILE" ] && . "$DEFAULT_PROFILE"
[ -r "$USER_PROFILE" ] && . "$USER_PROFILE"

[ -d "/sys/module/$MODNAME" ] && exit 0
[ -f "$KMOD" ] || exit 1

attempt=0
while [ "$attempt" -lt 90 ]; do
	if [ -d /sys/module/mca_platform_buckchg_class ] && \
	   [ -d /sys/module/mca_strategy_quickchg ] && \
	   [ -d /sys/module/mca_strategy_fg_comp ]; then
		if insmod "$KMOD" \
			armed="$K100PM_ARMED" \
			boost="$K100PM_BOOST" \
			pps="$K100PM_PPS" \
			thermal="$K100PM_THERMAL" \
			temperature_spoof="$K100PM_TEMPERATURE_SPOOF" \
			cutoff="$K100PM_CUTOFF" \
			observe="$K100PM_OBSERVE" \
			temperature_ceiling="$K100PM_TEMPERATURE_CEILING" \
			cutoff_fw_mv="$K100PM_CUTOFF_FW_MV" \
			cutoff_delay_mv="$K100PM_CUTOFF_DELAY_MV" \
			cutoff_sw_mv="$K100PM_CUTOFF_SW_MV"; then
			log -t "$TAG" "runtime controls loaded"
			exit 0
		fi
		log -t "$TAG" "insmod failed"
		exit 1
	fi
	attempt=$((attempt + 1))
	sleep 1
done

log -t "$TAG" "vendor charging modules not ready"
exit 1
