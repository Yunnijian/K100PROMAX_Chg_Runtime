#!/bin/sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
module_dir=$project_root/kernelsu
module_file=$project_root/out/K100PM_CHG_RUNTIME.ko
output_dir=${K100PM_KSU_OUTPUT_DIR:-$project_root/dist}
output_file=$output_dir/K100PM_CHG_RUNTIME-KernelSU.zip
stage_dir=$(mktemp -d "${TMPDIR:-/tmp}/k100pm-ksu.XXXXXX")

cleanup() {
	rm -rf "$stage_dir"
}
trap cleanup EXIT HUP INT TERM

[ -f "$module_file" ] || {
	echo "error: build out/K100PM_CHG_RUNTIME.ko before packaging" >&2
	exit 1
}
[ -f "$module_dir/module.prop" ] || {
	echo "error: missing KernelSU module files" >&2
	exit 1
}
command -v zip >/dev/null 2>&1 || {
	echo "error: zip is required" >&2
	exit 1
}
command -v unzip >/dev/null 2>&1 || {
	echo "error: unzip is required" >&2
	exit 1
}

mkdir -p "$stage_dir/kmod" "$output_dir"
cp "$module_dir/module.prop" "$stage_dir/module.prop"
cp "$module_dir/profile.sh" "$stage_dir/profile.sh"
cp "$module_dir/service.sh" "$stage_dir/service.sh"
cp "$module_dir/uninstall.sh" "$stage_dir/uninstall.sh"
cp "$module_file" "$stage_dir/kmod/K100PM_CHG_RUNTIME.ko"
chmod 0755 "$stage_dir/service.sh" "$stage_dir/uninstall.sh"

(
	cd "$stage_dir"
	zip -q -r "$output_file" module.prop profile.sh service.sh uninstall.sh kmod
)
unzip -t "$output_file" >/dev/null
printf '%s\n' "output: $output_file"
