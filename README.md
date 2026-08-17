# K100PM Charging Runtime

[![Android](https://img.shields.io/badge/Android-16-3DDC84?logo=android&logoColor=white)](https://source.android.com/)
[![KernelSU](https://img.shields.io/badge/KernelSU-module-607D8B)](https://kernelsu.org/)
[![Architecture](https://img.shields.io/badge/arch-arm64-455A64)](https://developer.android.com/ndk/guides/abis)

面向 Songyuan K100 Pro Max（`M098FE`）的充电运行时模块。模块通过 KCFI/ABI 对齐的 kretprobe，在厂商充电策略真正下发参数的边界修改电流、CV、PPS、热控和 FG 关机阈值。

它不是 DTBO 补丁，也不伪造 PD/APDO 协商。模块只对已经进入内核充电策略的值做有范围的改写。

> 当前版本：`v0.2.0`  
> 目标内核：`6.12.69-android16-6-g0d80ee00f747-ab15461283-4k`  
> 目标设备：`songyuan / M098FE`

## 运行行为

默认 `profile.conf` 打开以下开关：

| 开关 | 默认值 | 作用 |
| --- | ---: | --- |
| `armed` | `1` | 注册 K100PM 充电 kretprobe |
| `boost` | `1` | 修改 Buck JEITA、CV、CP 和 FFC 阶梯 |
| `pps` | `1` | 修改第三方 PPS 表的电流目标，不改 PD 合约 |
| `thermal` | `1` | 修改 K100PM 充电热控 voter 的电流目标 |
| `temperature_spoof` | `1` | 仅钳制充电 FG 的策略温度输出 |
| `cutoff` | `1` | 修改 FG 关机电压字段 |
| `observe` | `0` | 关闭逐次改写日志，避免刷屏 |

### 温度电流表

温度分段使用未修改的 FG 原始温度，单位为 `0.1°C`。它与 `temperature_ceiling` 相互独立：后者只是充电策略看到的温度钳位，不是真实温度，也不是电流表的温度边界。

| FG 原始温度 | 电流目标 | CV 目标 | 行为 |
| --- | ---: | ---: | --- |
| `18-23°C` | `14.27A` | 按原有低温映射 | 保留低温保护 |
| `23-48°C` | `19.40A` | `4.54/4.53/4.52V` | 主高功率区 |
| `48-55°C` | `19.40A` | `4.52V` | 延长高功率区 |
| `55-58°C` | `4.14A` | `4.12V` | 高温恢复电流档 |
| `>=58°C` | 保留原值 | 保留原值 | 保留原厂停充路径 |

关机阈值默认改写为：

```text
firmware: 2600 mV
delay:    2600 mV
software: 2550 mV
```

### 运行时挂点

| 组件 | 入口 | 修改内容 |
| --- | --- | --- |
| Buck | `platform_class_buckchg_ops_set_ichg` | Buck 充电电流 |
| Buck | `platform_class_buckchg_ops_set_term_volt` | CV/目标电压 |
| 快充策略 | `mca_quick_charge_*_voter_cb` | 分流、CP、FFC、PPS 和热控电流 |
| 充电 FG | `strategy_fg_ops_get_temp`、`strategy_fg_ops_get_thermal_temp` | 读取原始策略温度并按 `profile.conf` 钳制输出 |
| 关机策略 | `fg_update_status`、`mca_battery_shutdown_update_vcutoff_para` | FG vcutoff 字段 |

## 安装

1. 构建或下载 `K100PM_CHG_RUNTIME-KernelSU.zip`。
2. 在 KernelSU Manager 中选择“从本地安装模块”。
3. 重启设备。
4. 用 root shell 验证模块状态：

```sh
su -c 'grep -i k100pm /proc/modules'
su -c 'cat /sys/module/k100pm_chg_runtime/parameters/{armed,boost,pps,thermal,temperature_spoof,cutoff}'
su -c 'cat /sys/module/k100pm_chg_runtime/parameters/{ichg_hits,cv_hits,curve_hits,pps_hits,thermal_hits,temperature_hits,cutoff_hits}'
```

模块配置文件为安装目录下的 `/data/adb/modules/k100pm_chg_runtime/profile.conf`。它使用简单的 `KEY=value` 语法，由 `service.sh` 在加载 KO 前读取：

```sh
K100PM_ARMED=1
K100PM_BOOST=1
K100PM_PPS=1
K100PM_THERMAL=1
K100PM_TEMPERATURE_SPOOF=1
K100PM_CUTOFF=1
K100PM_OBSERVE=0
K100PM_TEMPERATURE_CEILING=360
K100PM_CUTOFF_FW_MV=2600
K100PM_CUTOFF_DELAY_MV=2600
K100PM_CUTOFF_SW_MV=2550
```

`K100PM_TEMPERATURE_CEILING` 使用 `0.1°C`，因此 `360` 表示 `36.0°C`。修改配置后重启模块或重启设备使其重新加载。

## 从源码构建

仓库已包含目标设备的构建依赖：

```text
kheaders/   内核头文件
abi/        内核版本、配置和 vermagic
crc/        Module.symvers
kfci/       KCFI 类型 ID 和编译参数
```

构建环境需要 Android Clang 19.0.1、`ld.lld`、Node.js 和 `zip`：

```sh
export K100PM_CLANG=/path/to/clang-r536225/bin/clang
export K100PM_LD_LLD=/path/to/clang-r536225/bin/ld.lld

scripts/build_k100pm_chg_runtime.sh
scripts/package_kernelsu_module.sh
```

产物：

```text
out/K100PM_CHG_RUNTIME.ko
dist/K100PM_CHG_RUNTIME-KernelSU.zip
```

构建脚本会校验：

- vermagic 和目标架构
- `__versions`、`__version_ext_*` 和 CRC
- `init_module` / `cleanup_module` KCFI type ID
- 未声明的外部符号

## 仓库结构

```text
src/                    KO 源码
kernelsu/               KernelSU 模块脚本和 profile.conf
scripts/                构建、CRC 校验和打包脚本
kheaders/               目标内核头文件
abi/                    目标内核 ABI 基线
crc/                    Module.symvers
kfci/                   KCFI 类型 ID 和编译基线
```

## 边界与风险

- 这是 K100PM 专用模块，只面向 README 顶部列出的设备与内核基线。
- 需要匹配的内核版本、`vermagic`、CRC 和 KCFI 配置。换内核后必须重新获取对应基线。
- 模块不修改真实电池温度，不修改 CPU、主板、连接器或 USB 温度，也不绕过硬件 OCP/OVP/过温保护。
- `temperature_spoof` 只影响 K100PM 充电策略的 FG 温度输入；系统电池温度读数保持真实值。
- 没有 PPS/APDO 协商时，PPS 映射不会凭空建立快充协议；电脑 USB 的 SDP 小电流也不会验证高功率曲线。
- 高温档只修改软件策略目标，实际功率仍受适配器、线缆、CP、板温和硬件保护共同限制。

## 许可

本项目按 GNU General Public License v2.0 only 发布，详见 [LICENSE](LICENSE)。
