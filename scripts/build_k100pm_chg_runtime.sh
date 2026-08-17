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
module_common=$output_dir/${output_name}_common.c
module_lds=$output_dir/module.lds
module_file=$output_dir/$output_name.ko
clang_dir=$(dirname "$clang")
llvm_nm=${K100PM_LLVM_NM:-$clang_dir/llvm-nm}
llvm_readelf=${K100PM_LLVM_READELF:-$clang_dir/llvm-readelf}

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
[ -x "$llvm_nm" ] || fail "llvm-nm not executable: $llvm_nm"
[ -x "$llvm_readelf" ] || fail "llvm-readelf not executable: $llvm_readelf"
grep -q '^#define CONFIG_CFI_ICALL_NORMALIZE_INTEGERS 1$' \
	"$headers_dir/include/generated/autoconf.h" ||
	fail "target KCFI integer normalization setting is unavailable"

if [ "${K100PM_ALLOW_UNVERIFIED_TOOLCHAIN:-0}" != 1 ]; then
	"$clang" --version | sed -n '1p' | grep -q 'clang version 19.0.1' ||
		fail "KCFI ABI requires Android clang 19.0.1; set K100PM_ALLOW_UNVERIFIED_TOOLCHAIN=1 only for diagnostics"
fi

mkdir -p "$output_dir"
vermagic=$(tr -d '\r\n' < "$vermagic_file")
symbols='module_layout register_kretprobe unregister_kretprobe _printk memset param_ops_bool param_ops_uint'

{
	printf '%s\n' '#ifndef K100PM_CHG_RUNTIME_VERSIONS_H' '#define K100PM_CHG_RUNTIME_VERSIONS_H'
	printf '%s\n' 'static const struct modversion_info ____versions[] __used __section("__versions") = {'
	for symbol in $symbols; do
		line=$(awk -v s="$symbol" '$2 == s { print; exit }' "$symvers")
		[ -n "$line" ] || fail "missing CRC for symbol: $symbol"
		crc=$(printf '%s\n' "$line" | awk '{print $1}')
		printf '    { %s, "%s" },\n' "$crc" "$symbol"
	done
	printf '%s\n' '};'
	printf '%s\n' 'static const u32 ____version_ext_crcs[] __used __section("__version_ext_crcs") = {'
	for symbol in $symbols; do
		line=$(awk -v s="$symbol" '$2 == s { print; exit }' "$symvers")
		crc=$(printf '%s\n' "$line" | awk '{print $1}')
		printf '    %s,\n' "$crc"
	done
	printf '%s\n' '};'
	printf '%s\n' 'static const char ____version_ext_names[] __used __section("__version_ext_names") ='
	for symbol in $symbols; do
		printf '    "%s\\0"\n' "$symbol"
	done
	printf '%s\n' ';' '#endif'
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

{
	printf '%s\n' '#include <linux/module.h>'
	printf '%s\n' '#define INCLUDE_VERMAGIC'
	printf '%s\n' '#include <linux/build-salt.h>'
	printf '%s\n' '#include <linux/elfnote-lto.h>'
	printf '%s\n' '#include <linux/vermagic.h>'
	printf '%s\n' 'BUILD_SALT;'
	printf '%s\n' 'BUILD_LTO_INFO;'
	printf 'MODULE_INFO(vermagic, "%s");\n' "$vermagic"
} > "$module_common"

base_flags="--target=aarch64-linux-gnu -std=gnu11 -O2 \
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
	-fasynchronous-unwind-tables \
	-fno-delete-null-pointer-checks -fno-strict-overflow \
	-fno-optimize-sibling-calls -fno-omit-frame-pointer -ffixed-x18 \
	-mbranch-protection=pac-ret -mgeneral-regs-only \
	-mstrict-align -mno-outline-atomics -mcmodel=large"
kcfi_flags="-fsanitize=kcfi -fsanitize-cfi-icall-experimental-normalize-integers"

# shellcheck disable=SC2086
"$clang" $base_flags $kcfi_flags -c "$source_file" -o "$output_dir/$output_name.o"
# shellcheck disable=SC2086
"$clang" $base_flags $kcfi_flags -c "$module_meta" -o "$output_dir/${output_name}_mod.o"
# shellcheck disable=SC2086
"$clang" $base_flags -c "$module_common" -o "$output_dir/${output_name}_common.o"
"$linker" -r -m aarch64elf -z noexecstack --build-id=sha1 -T "$module_lds" \
	-o "$module_file" "$output_dir/$output_name.o" "$output_dir/${output_name}_mod.o" \
	"$output_dir/${output_name}_common.o"

for section in __versions __version_ext_crcs __version_ext_names .init.eh_frame .note.Linux; do
	"$llvm_readelf" -S "$module_file" | grep -q "$section" ||
		fail "missing required module ABI section: $section"
done

for typeid in __kcfi_typeid_init_module __kcfi_typeid_cleanup_module; do
	expected=$(awk -F= -v name="$typeid" '$1 == name { print $2; exit }' \
		"$project_root/kfci/kernel-typeids.txt")
	expected_hex=${expected#0x}
	actual=$("$llvm_nm" -a "$module_file" | awk -v name="$typeid" '$3 == name { print $1; exit }')
	[ -n "$expected" ] || fail "missing target KCFI type ID: $typeid"
	case "$actual" in
	*"$expected_hex") ;;
	*) fail "KCFI type ID mismatch for $typeid: expected $expected, got ${actual:-missing}" ;;
	esac
done

for symbol in $("$llvm_nm" -u "$module_file" | awk '{ print $2 }'); do
	case " $symbols " in
	*" $symbol "*) ;;
	*) fail "undeclared imported symbol with no CRC entry: $symbol" ;;
	esac
done

node_bin=${K100PM_NODE:-node}
command -v "$node_bin" >/dev/null 2>&1 || fail "node is required for CRC verification"
crc_check=$output_dir/${output_name}.built.symvers
"$node_bin" "$project_root/scripts/extract_module_crcs.mjs" "$module_file" "$crc_check" $symbols
while read -r crc symbol _; do
	expected=$(awk -v s="$symbol" '$2 == s { print $1; exit }' "$symvers")
	[ "$crc" = "$expected" ] ||
		fail "CRC mismatch for $symbol: expected ${expected:-missing}, got $crc"
done < "$crc_check"

echo "vermagic: $vermagic"
echo "output:   $module_file"
