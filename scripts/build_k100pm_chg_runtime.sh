#!/bin/sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
headers_dir=${K100PM_HEADERS_DIR:-$project_root/kheaders}
clang=${K100PM_CLANG:-}
linker=${K100PM_LD_LLD:-}
symvers=${K100PM_MODULE_SYMVERS:-$project_root/crc/Module.symvers}
output_dir=${K100PM_OUTPUT_DIR:-$project_root/out}
module_name=${K100PM_MODULE_NAME:-k100pm_chg_runtime}
output_name=${K100PM_OUTPUT_NAME:-K100PM_CHG_RUNTIME}
source_file=$project_root/src/K100PM_CHG_RUNTIME.c
vermagic_file=$project_root/abi/vermagic.txt
versions_header=$output_dir/${output_name}_versions.h
module_meta=$output_dir/${output_name}_mod.c
module_lds=$output_dir/module.lds

fail() {
	echo "error: $*" >&2
	exit 1
}

[ -n "$clang" ] || fail "K100PM_CLANG must point to Android clang 19.0.1"
[ -n "$linker" ] || fail "K100PM_LD_LLD must point to Android ld.lld 19.0.1"
[ -x "$clang" ] || fail "clang not executable: $clang"
[ -x "$linker" ] || fail "ld.lld not executable: $linker"
[ -f "$source_file" ] || fail "missing source: $source_file"
[ -f "$headers_dir/include/linux/compiler-version.h" ] || fail "incomplete K100PM kheaders"
[ -f "$headers_dir/include/linux/kconfig.h" ] || fail "incomplete K100PM kheaders"
[ -f "$headers_dir/arch/arm64/include/asm/module.lds.h" ] || fail "missing arm64 module linker fragment"
[ -f "$symvers" ] || fail "missing K100PM Module.symvers"
[ -f "$vermagic_file" ] || fail "missing K100PM vermagic"

if [ "${K100PM_ALLOW_UNVERIFIED_TOOLCHAIN:-0}" != 1 ]; then
	"$clang" --version | sed -n '1p' | grep -q 'clang version 19.0.1' ||
		fail "KCFI ABI requires Android clang 19.0.1; set K100PM_ALLOW_UNVERIFIED_TOOLCHAIN=1 only for diagnostics"
fi

mkdir -p "$output_dir"
vermagic=$(tr -d '\r\n' < "$vermagic_file")
symbols='module_layout register_kretprobe unregister_kretprobe _printk memset'

{
	printf '%s\n' '#ifndef K100PM_CHG_RUNTIME_VERSIONS_H' '#define K100PM_CHG_RUNTIME_VERSIONS_H'
	printf '#define K100PM_CHG_RUNTIME_VERMAGIC "%s"\n' "$vermagic"
	printf '%s\n' 'static const struct modversion_info ____versions[] __used __section("__versions") = {'
	for symbol in $symbols; do
		line=$(awk -v s="$symbol" '$2 == s { print; exit }' "$symvers")
		[ -n "$line" ] || fail "missing CRC for symbol: $symbol"
		crc=$(printf '%s\n' "$line" | awk '{print $1}')
		printf '    { %s, "%s" },\n' "$crc" "$symbol"
	done
	printf '%s\n' '};' '#endif'
} > "$versions_header"

"$clang" --target=aarch64-linux-gnu -E -P -x assembler-with-cpp \
	-D__KERNEL__ -D__ASSEMBLY__ \
	-include "$headers_dir/include/linux/compiler-version.h" \
	-include "$headers_dir/include/linux/kconfig.h" \
	-I"$headers_dir/arch/arm64/include" -I"$headers_dir/arch/arm64/include/generated" \
	-I"$headers_dir/include" -I"$headers_dir/arch/arm64/include/uapi" \
	-I"$headers_dir/arch/arm64/include/generated/uapi" \
	-I"$headers_dir/include/uapi" -I"$headers_dir/include/generated/uapi" \
	"$headers_dir/arch/arm64/include/asm/module.lds.h" -o "$module_lds"

{
	printf '%s\n' '#include <linux/module.h>'
	printf '%s\n' 'struct module __this_module' '__section(".gnu.linkonce.this_module") = {'
	printf '    .name = "%s",\n' "$module_name"
	printf '%s\n' '    .init = init_module,'
	printf '%s\n' '#ifdef CONFIG_MODULE_UNLOAD' '    .exit = cleanup_module,' '#endif'
	printf '%s\n' '    .arch = MODULE_ARCH_INIT,' '};'
	printf 'MODULE_INFO(name, "%s");\n' "$module_name"
} > "$module_meta"

common_flags="--target=aarch64-linux-gnu -std=gnu11 -O2 \
	-D__KERNEL__ -DMODULE -DK100PM_CHG_RUNTIME_GENERATED_VERSIONS=1 \
	-DKBUILD_MODNAME=\"$module_name\" -DKBUILD_BASENAME=\"$module_name\" \
	-include $headers_dir/include/linux/compiler-version.h \
	-include $headers_dir/include/linux/kconfig.h \
	-I$output_dir -I$project_root/src \
	-I$headers_dir/arch/arm64/include -I$headers_dir/arch/arm64/include/generated \
	-I$headers_dir/include -I$headers_dir/arch/arm64/include/uapi \
	-I$headers_dir/arch/arm64/include/generated/uapi \
	-I$headers_dir/include/uapi -I$headers_dir/include/generated/uapi \
	-fno-pic -fno-PIE -fno-common -fno-builtin -fno-stack-protector \
	-fno-asynchronous-unwind-tables -fno-unwind-tables \
	-fno-delete-null-pointer-checks -fno-strict-overflow \
	-fno-optimize-sibling-calls -fno-omit-frame-pointer -ffixed-x18 \
	-fsanitize=kcfi -mbranch-protection=pac-ret -mgeneral-regs-only \
	-mstrict-align -mno-outline-atomics -mcmodel=large"

# shellcheck disable=SC2086
"$clang" $common_flags -c "$source_file" -o "$output_dir/$output_name.o"
# shellcheck disable=SC2086
"$clang" $common_flags -c "$module_meta" -o "$output_dir/${output_name}_mod.o"
"$linker" -r -m aarch64elf -z noexecstack --build-id=sha1 -T "$module_lds" \
	-o "$output_dir/$output_name.ko" "$output_dir/$output_name.o" "$output_dir/${output_name}_mod.o"

echo "vermagic: $vermagic"
echo "output:   $output_dir/$output_name.ko"
