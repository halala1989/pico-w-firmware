---
AIGC:
    Label: "1"
    ContentProducer: 001191440300708461136T1XGW3
    ProduceID: 0cd76fea74816a5f00d7e2597060aee7_4da37b2b9b8311f18cca525400e6dd8f
    ReservedCode1: LBdKCDT9g8OpRwwSmOvko9CTEtPHMbaX+/sFA14Y21UQhGPWE2L3xfhlANOaJfcSAclWR9fDMAhWLQAqFO5Ey0AxSo3PPap9qfCv+/iJA3EvbvElAHtHK+QiDlgy4+fGdCR4XR+Qj9EyRjiyFTB/jlqI2ukVUqDrwMU/elU1ISxuppvuxSHIxsLrRpw=
    ContentPropagator: 001191440300708461136T1XGW3
    PropagateID: 0cd76fea74816a5f00d7e2597060aee7_4da37b2b9b8311f18cca525400e6dd8f
    ReservedCode2: LBdKCDT9g8OpRwwSmOvko9CTEtPHMbaX+/sFA14Y21UQhGPWE2L3xfhlANOaJfcSAclWR9fDMAhWLQAqFO5Ey0AxSo3PPap9qfCv+/iJA3EvbvElAHtHK+QiDlgy4+fGdCR4XR+Qj9EyRjiyFTB/jlqI2ukVUqDrwMU/elU1ISxuppvuxSHIxsLrRpw=
---

# Pico W Keyboard Injector（Pico W 原生 BLE）

[![Build Pico W Firmware (UF2)](https://github.com/halala1989/pico-w-firmware/actions/workflows/pico-build.yml/badge.svg)](https://github.com/halala1989/pico-w-firmware/actions/workflows/pico-build.yml)

把 Raspberry Pi Pico W 变成一台「蓝牙键盘注入器」：Android 端 App 通过 BLE 按应用层帧协议
分包写入 Pico W，Pico W 重组出完整 UTF-8 文本后，以 USB HID 键盘的身份把字符"打"进 Windows 主机。

## 特性

- BLE：BTstack（经 pico-sdk 的 `pico_btstack_ble` / `pico_btstack_cyw43`）
- USB HID：TinyUSB（`tinyusb_device`），设备枚举为标准键盘（Boot Protocol）
- 应用层帧协议：`START → DATA×N → END / CANCEL`，乱序/重复/长度不符自动丢弃整条消息
- GitHub Actions 云端交叉编译，自动产出 `.uf2` 固件

## 架构

```
Android App (BLE GATT Client)
   │  通过 TX characteristic 写入帧
   ▼
Pico W (BTstack BLE Peripheral / GATT Server)
   │  packet_parser 重组 UTF-8 文本
   ▼
USB HID 键盘  →  Windows 主机自动打字
```

## 文档导航

| 文档 | 内容 |
|---|---|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | 系统架构、模块划分与数据流 |
| [docs/PROTOCOL.md](docs/PROTOCOL.md) | 应用层帧协议规范（与 Android 端对齐） |
| [docs/BUILD.md](docs/BUILD.md) | 本地构建与 GitHub Actions CI |
| [docs/FLASHING.md](docs/FLASHING.md) | 烧录与验证步骤 |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) | 固件开发指南（GATT/字符映射/调试） |
| [docs/ANDROID_INTEGRATION.md](docs/ANDROID_INTEGRATION.md) | Android 端对接说明 |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | 常见问题排障 |

## 目录结构

```
pico-w-firmware/
├── CMakeLists.txt          # 构建配置（stdio 走 UART，USB 专用于 HID）
├── btstack_config.h        # BTstack 配置（仅 BLE，裁剪 Classic）
├── pico_sdk_import.cmake   # 定位 Pico SDK（默认 2.1.0）
├── pico_w_keyboard.gatt    # 自定义 GATT 服务定义（编译期生成 profile）
├── README.md
├── .github/workflows/
│   └── pico-build.yml      # GitHub Actions 交叉编译（产 UF2）
├── docs/                   # 完整开发文档
└── src/
    ├── main.c              # BLE 初始化 / 广播 / GATT 写回调 / 主循环
    ├── packet_parser.c/.h  # 应用层帧协议解析与重组
    ├── usb_hid.c/.h        # TinyUSB HID 键盘注入 + ASCII→HID 映射
    └── tusb_config.h       # TinyUSB 配置
```

## 快速开始

```bash
export PICO_SDK_PATH=/path/to/pico-sdk   # 可选，未设置时自动拉取 2.1.0
cmake -B build -DPICO_BOARD=pico_w .
cmake --build build -j4
# 产物：build/pico_w_keyboard_firmware.uf2
```

不想装本地工具链？直接使用仓库的 GitHub Actions（见 [docs/BUILD.md](docs/BUILD.md)），
构建产物可在 Actions 的 Artifacts 中下载。

## 烧录

按住 Pico W 的 BOOTSEL 键插入 USB，出现 `RPI-RP2` 盘符后把 `.uf2` 拖入即可。
详细步骤见 [docs/FLASHING.md](docs/FLASHING.md)。

## 帧协议速查（详见 docs/PROTOCOL.md）

大端序，每帧 = `TYPE(1) + MESSAGE_ID(2) + SEQ(2) + PAYLOAD(N)`：

| TYPE | 值 | PAYLOAD |
|------|----|---------|
| START  | 0x01 | 总长度 4B + 总包数 2B |
| DATA   | 0x02 | UTF-8 分片，SEQ 为 0 起始的块号 |
| END    | 0x03 | 空 |
| CANCEL | 0x04 | 空 |

- 文本在收到 END 且累计字节数 = START 声明总长度时才算完整，随后注入。
- 乱序 / 重复 / 长度不符 / 缺 START 直接丢弃整条消息（解析器自动复位）。

GATT 服务（与 Android 端 BleManager.kt 对齐）：
- Service：`19B10000-E8F2-537E-4F6C-D104768A1214`
- TX Characteristic（可写）：`19B10001-E8F2-537E-4F6C-D104768A1214`
- 广播名：`PICO-W-KEYBOARD`（Android 端按此过滤）

## 已知限制

- 当前仅注入 ASCII（0x20~0x7E）可打印字符与少量控制键；多字节 UTF-8
  （中文等）在 usb_hid.c 中被跳过，后续可扩展为 Unicode 键盘映射。
- 固件侧当前只做字符映射，不含系统级快捷指令（如复制/粘贴/运行命令）能力。
- 尚未在真实硬件上编译烧录验证，接口已按 pico-sdk 2.1.0 静态核对。

## 相关仓库

- [pico-w-keyboard](https://github.com/halala1989/pico-w-keyboard)：Android BLE 客户端（分包/发送/扫描）

*（内容由AI生成，仅供参考）*
