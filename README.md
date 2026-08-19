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

# Pico W Keyboard Injector（方案A：Pico W 原生 BLE）

把 Raspberry Pi Pico W 变成一台「蓝牙键盘注入器」：Android 端 App 通过 BLE 把文本
（按应用层帧协议分包）写入 Pico W，Pico W 重组出完整 UTF-8 文本后，以 USB HID
键盘的身份把字符"打"进 Windows 主机。

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

- BLE：BTstack（经 pico-sdk 的 `pico_btstack_ble` / `pico_btstack_cyw43`）
- USB HID：TinyUSB（`tinyusb_device`），设备枚举为标准键盘（Boot Protocol）

## 目录结构

```
pico-w-firmware/
├── CMakeLists.txt          # 构建配置（stdio 走 UART，USB 专用于 HID）
├── btstack_config.h        # BTstack 配置（仅 BLE，裁剪 Classic）
├── pico_sdk_import.cmake   # 定位 Pico SDK（默认 2.1.0）
├── pico_w_keyboard.gatt    # 自定义 GATT 服务定义
├── README.md
└── src/
    ├── main.c              # BLE 初始化 / 广播 / GATT 写回调 / 主循环
    ├── packet_parser.c/.h  # 应用层帧协议解析与重组
    ├── usb_hid.c/.h        # TinyUSB HID 键盘注入
    └── tusb_config.h       # TinyUSB 配置
```

## 构建

依赖：
- Raspberry Pi Pico SDK（2.1.0，可自动下载）
- Pico W 需要 SDK 内置的 BTstack 支持（默认包含）
- ARM 交叉编译工具链 + CMake

```bash
export PICO_SDK_PATH=/path/to/pico-sdk   # 可选，未设置时自动拉取 2.1.0
cmake -B build -DPICO_BOARD=pico_w .
cmake --build build -j4
```

产物：`build/pico_w_keyboard_firmware.uf2`

## 烧录

按住 Pico W 的 BOOTSEL 键插入 USB，出现 `RPI-RP2` 盘符后把 `.uf2` 拖入即可。

## 帧协议（与 Android 端 BlePacketizer.kt 对齐）

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
*（内容由AI生成，仅供参考）*
