# Getting Started

> **Relevant source files**
> * [.github/workflows/build.yml](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/.github/workflows/build.yml)
> * [.gitignore](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/.gitignore)
> * [CMakeLists.txt](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt)
> * [tusb_config.h](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/tusb_config.h)
> * [usb_descriptors.c](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c)

This page provides an overview of the prerequisites, build process, and initial configuration needed to develop and deploy the Orinayo firmware. It covers the essential steps from cloning the repository to flashing a working firmware image onto the Raspberry Pi Pico 2 W hardware.

For detailed system architecture, see [Architecture](./3-architecture.md). For operational details about individual subsystems, see [Bluetooth Input System](./4-bluetooth-input-system.md) and [Musical Processing](./5-musical-processing.md).

## Scope and Prerequisites

The Orinayo firmware targets the **Raspberry Pi Pico 2 W** (RP2350-based board with CYW43 Bluetooth/WiFi chip) and requires the following components:

| Component | Requirement | Notes |
| --- | --- | --- |
| **Target Hardware** | Raspberry Pi Pico 2 W | RP2350 dual-core ARM, CYW43 wireless chip |
| **Build System** | CMake 3.12+ | Configured in [CMakeLists.txt L20](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L20-L20) |
| **SDK** | Pico SDK | Must be installed with `PICO_SDK_PATH` environment variable set |
| **Compiler** | ARM GCC toolchain | Included with Pico SDK setup |
| **Input Devices** | Bluetooth controllers | HID gamepads or specialized MIDI devices (see [Hardware Requirements](./2.1-hardware-requirements.md)) |
| **Output Devices** | USB host or UART MIDI | DAW, hardware synthesizer, or both simultaneously |

The build system is configured for the **rp2350-arm-s** platform and **pico2_w** board as specified in [CMakeLists.txt L23-L24](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L23-L24)

**Sources:** [CMakeLists.txt L19-L24](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L19-L24)

## Quick Start Workflow

```mermaid
flowchart TD

Clone["Clone Repository<br>github.com/Jus-Be/orinayo-pico"]
SDK["Set PICO_SDK_PATH<br>environment variable"]
Config["Optional: Modify configs<br>tusb_config.h<br>btstack_config.h"]
Build["Build with CMake<br>mkdir build && cd build<br>cmake .. && make"]
Artifacts["Generated Artifacts<br>orinayo.uf2<br>orinayo.bin<br>orinayo.elf"]
Flash["Flash to Pico 2 W<br>USB mass storage or<br>debug probe"]
Run["Connect Bluetooth controller<br>and MIDI output"]

Clone --> SDK
SDK --> Config
Config --> Build
Build --> Artifacts
Artifacts --> Flash
Flash --> Run
```

The workflow follows a standard embedded development pattern with three configuration points before building. For automated builds, see the GitHub Actions workflow in [.github/workflows/build.yml](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/.github/workflows/build.yml)

**Sources:** [CMakeLists.txt L1-L67](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L1-L67)

 [.github/workflows/build.yml L1-L25](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/.github/workflows/build.yml#L1-L25)

## Build System Architecture

The build system implements a **two-tier library architecture** to separate Bluetooth infrastructure from application logic:

```mermaid
flowchart TD

CMakeRoot["CMakeLists.txt<br>Project: Orinayo<br>Version: YYYY.MM.DD"]
BTLib["add_library(orinayobt STATIC)"]
BTSources["pico_bluetooth.c<br>async_timer.c<br>display.c<br>storage.c"]
BTIncludes["bluepad32/include<br>btstack/src"]
BTLinks["pico_cyw43_arch_threadsafe_background<br>pico_btstack_classic<br>pico_btstack_ble<br>bluepad32"]
MainExe["add_executable(orinayo)"]
MainSources["main.c<br>usb_descriptors.c<br>tap_tempo.c<br>looper.c<br>note_scheduler.c<br>ghost_note.c"]
MainLinks["Links to orinayobt<br>+ pico_stdlib<br>+ tinyusb_device"]
PicoSDK["Pico SDK<br>PICO_SDK_PATH"]
TinyUSB["TinyUSB<br>Device stack"]
BTStack["BTstack<br>Classic + BLE"]
Bluepad32["Bluepad32<br>Controller support"]
ELF["orinayo.elf"]
BIN["orinayo.bin"]
UF2["orinayo.uf2<br>Drag-and-drop flash"]
HEX["orinayo.hex"]
MAP["orinayo.map"]

CMakeRoot --> BTLib
CMakeRoot --> MainExe
MainLinks --> BTLib
PicoSDK --> BTLib
PicoSDK --> MainExe
TinyUSB --> MainExe
BTStack --> BTLib
Bluepad32 --> BTLib
MainExe --> ELF

subgraph subGraph4 ["Output Artifacts"]
    ELF
    BIN
    UF2
    HEX
    MAP
    ELF --> BIN
    ELF --> UF2
    ELF --> HEX
    ELF --> MAP
end

subgraph subGraph3 ["External Dependencies"]
    PicoSDK
    TinyUSB
    BTStack
    Bluepad32
end

subgraph subGraph2 ["Main Executable: orinayo"]
    MainExe
    MainSources
    MainLinks
    MainExe --> MainSources
    MainExe --> MainLinks
end

subgraph subGraph1 ["Static Library: orinayobt"]
    BTLib
    BTSources
    BTIncludes
    BTLinks
    BTLib --> BTSources
    BTLib --> BTIncludes
    BTLib --> BTLinks
end

subgraph subGraph0 ["CMake Build Configuration"]
    CMakeRoot
end
```

### Build Target Details

| Target | Type | Source Files | Purpose |
| --- | --- | --- | --- |
| **orinayobt** | Static library | [pico_bluetooth.c, async_timer.c, display.c, storage.c](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/pico_bluetooth.c, async_timer.c, display.c, storage.c) | Bluetooth infrastructure and hardware abstraction ([CMakeLists.txt L39](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L39-L39) <br> ) |
| **orinayo** | Executable | [main.c, usb_descriptors.c, tap_tempo.c, looper.c, note_scheduler.c, ghost_note.c](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/main.c, usb_descriptors.c, tap_tempo.c, looper.c, note_scheduler.c, ghost_note.c) | Application logic and musical processing ([CMakeLists.txt L44](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L44-L44) <br> ) |

The separation allows the Bluetooth subsystem to be compiled with specific flags (e.g., `PICO_CYW43_ARCH_THREADSAFE_BACKGROUND` defined at [CMakeLists.txt L42](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L42-L42)

) without affecting the main application.

**Sources:** [CMakeLists.txt L38-L56](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L38-L56)

## Configuration Files Overview

Three configuration headers control different aspects of the system:

```mermaid
flowchart TD

TUSB["tusb_config.h<br>TinyUSB Device Configuration"]
BTSTACK["btstack_config.h<br>Bluetooth Stack Configuration"]
SDK["sdkconfig.h<br>Pico SDK Options"]
TUSBMode["CFG_TUSB_RHPORT0_MODE<br>OPT_MODE_DEVICE"]
TUSBOS["CFG_TUSB_OS<br>OPT_OS_PICO"]
MIDIEnable["CFG_TUD_MIDI = 1<br>CDC/MSC/HID = 0"]
MIDIBuf["CFG_TUD_MIDI_RX_BUFSIZE<br>CFG_TUD_MIDI_TX_BUFSIZE<br>64 bytes (FS) / 512 bytes (HS)"]
VID["idVendor = 0xCafe"]
PID["idProduct = USB_PID<br>Dynamic based on enabled classes"]
Strings["Manufacturer: Inspired Futures Ltd<br>Product: Orinayo Device"]

TUSB --> TUSBMode
TUSB --> TUSBOS
TUSB --> MIDIEnable
TUSB --> MIDIBuf
MIDIEnable --> VID
MIDIEnable --> PID

subgraph subGraph2 ["USB Descriptors"]
    VID
    PID
    Strings
    VID --> Strings
end

subgraph subGraph1 ["TinyUSB Configuration"]
    TUSBMode
    TUSBOS
    MIDIEnable
    MIDIBuf
end

subgraph subGraph0 ["Configuration Layer"]
    TUSB
    BTSTACK
    SDK
end
```

### Key Configuration Points

| File | Key Settings | Line References |
| --- | --- | --- |
| **tusb_config.h** | MIDI class enabled, CDC/HID disabled | [tusb_config.h L80-L84](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/tusb_config.h#L80-L84) |
|  | MIDI buffer sizes (64/512 bytes) | [tusb_config.h L87-L88](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/tusb_config.h#L87-L88) |
| **usb_descriptors.c** | USB VID: `0xCafe` | [usb_descriptors.c L51](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c#L51-L51) |
|  | USB PID: Dynamic bitmap | [usb_descriptors.c L35-L36](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c#L35-L36) |
|  | Device strings and metadata | [usb_descriptors.c L131-L139](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c#L131-L139) |

The USB Product ID is dynamically calculated based on enabled TinyUSB device classes using a bitmap at [usb_descriptors.c L34-L36](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c#L34-L36)

 allowing the same vendor ID to support different interface configurations.

**Sources:** [tusb_config.h L26-L94](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/tusb_config.h#L26-L94)

 [usb_descriptors.c L28-L67](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c#L28-L67)

 [usb_descriptors.c L131-L139](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c#L131-L139)

## Platform Configuration

The build targets the **RP2350** platform with specific settings:

```mermaid
flowchart TD

Platform["PICO_PLATFORM<br>rp2350-arm-s"]
Board["PICO_BOARD<br>pico2_w"]
CYW43["CYW43 Configuration<br>THREADSAFE_BACKGROUND mode"]
BTThread["Bluetooth stack runs<br>in background"]
USBSafe["USB operations must be<br>mutex-aware"]
AsyncCtx["async_context used for<br>timer scheduling"]

Platform --> Board
Board --> CYW43
CYW43 --> BTThread
CYW43 --> USBSafe
CYW43 --> AsyncCtx

subgraph subGraph0 ["CYW43 Implications"]
    BTThread
    USBSafe
    AsyncCtx
end
```

The `PICO_CYW43_ARCH_THREADSAFE_BACKGROUND` define at [CMakeLists.txt L42](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L42-L42)

 enables background processing of Bluetooth events, which is critical for maintaining low-latency MIDI output while handling wireless communication. This configuration requires careful synchronization in the application code (see [Synchronization Primitives](./8.3-synchronization-primitives.md)).

**Sources:** [CMakeLists.txt L23-L24](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L23-L24)

 [CMakeLists.txt L42](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L42-L42)

## CI/CD Pipeline

The repository includes a GitHub Actions workflow for automated building:

```mermaid
flowchart TD

Push["git push trigger"]
Checkout["actions/checkout@v4"]
Build["Pico-Build-Action@v1<br>source_dir: ."]
Artifacts["Upload artifacts<br>orinayo.uf2 and related files"]

Push --> Checkout
Checkout --> Build
Build --> Artifacts
```

The workflow at [.github/workflows/build.yml](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/.github/workflows/build.yml)

 automatically builds the firmware on every push, using the `samyarsadat/Pico-Build-Action@v1` action. Build artifacts (UF2, BIN, HEX, ELF, MAP files) are uploaded and available for download from the Actions tab.

**Sources:** [.github/workflows/build.yml L1-L25](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/.github/workflows/build.yml#L1-L25)

## Next Steps

For detailed information on specific aspects of getting started:

* **[Hardware Requirements](./2.1-hardware-requirements.md)**: Detailed specifications for Pico 2 W, supported Bluetooth controllers, and MIDI output connections
* **[Building and Flashing](./2.2-building-and-flashing.md)**: Step-by-step build instructions, flashing methods, and verification procedures
* **[Configuration](./2.3-configuration.md)**: In-depth guide to configuration options in sdkconfig.h, tusb_config.h, and btstack_config.h

Once the firmware is built and flashed, refer to [Bluetooth Input System](./4-bluetooth-input-system.md) for pairing controllers and [MIDI Output System](./6-midi-output-system.md) for connecting synthesizers.

**Sources:** [CMakeLists.txt L1-L67](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/CMakeLists.txt#L1-L67)

 [.github/workflows/build.yml L1-L25](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/.github/workflows/build.yml#L1-L25)

 [tusb_config.h L26-L94](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/tusb_config.h#L26-L94)

 [usb_descriptors.c L1-L180](https://github.com/Jus-Be/orinayo-pico/blob/122fa496/usb_descriptors.c#L1-L180)