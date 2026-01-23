# YTX Client

- [YTX Client](#ytx-client)
  - [Introduction](#introduction)
  - [License](#license)
  - [Developer Tutorial](#developer-tutorial)
    - [Requirement](#requirement)
    - [Dependency](#dependency)
    - [Step](#step)
  - [User Tutorial](#user-tutorial)
    - [Location](#location)
    - [Guide](#guide)
  - [Support Me](#support-me)

## Introduction

**YTX** is a lightweight ERP software designed for small businesses, with full support for **multi-user online operation**. It provides a complete solution for managing daily operations across key modules such as **Finance**, **Inventory**, **Task**, **Sale**, **Purchase**, and **Partner**.

Originally built as a desktop application, YTX has evolved into a **fully-featured online ERP system**. With the support of the companion **ytx-server backend service**, it can be deployed on a **local server** using PostgreSQL to enable **real-time collaboration** within teams, without relying on third-party cloud services.

With its **user-friendly interface** and **modular design**, YTX is an efficient and flexible tool for small enterprises seeking modern business management.

## License

**YTX Client** is open source software licensed under the **GNU Lesser General Public License v3.0 (LGPLv3)**, in compliance with the Qt framework's open source license requirements.

**Important Notes:**

- **Client**: The YTX client application is **open source** and available in this repository.
- **Server**: The ytx-server backend service is **proprietary software** and is **not open source**. For server licensing and deployment inquiries, please contact the maintainer.

This project uses the Qt framework under LGPLv3. You can find more information about Qt's licensing at [https://www.qt.io/licensing/](https://www.qt.io/licensing/).

By using, modifying, or distributing this software, you agree to comply with the terms of the LGPLv3 license. See the [LICENSE](LICENSE) file for full details.

## Developer Tutorial

### Requirement

- Qt
  - Qt: 6.9.3
    1. Desktop
    2. Additional Libraries
       - Qt Image Formats
       - Qt Websockets
  - Build Tools
- Qt Creator 17.0.2
- CMake: 3.30.5
- Compiler: GCC 12+ or LLVM/Clang 14+

### Dependency

- Clone the [yxlsx](https://github.com/ytxerp/yxlsx) repository into the `external/` folder of the project:

```bash
  git clone https://github.com/ytxerp/yxlsx.git external/yxlsx
```

- Add it in CMakeLists.txt:

```cmake
  add_subdirectory(external/yxlsx/)
```

### Step

1. Open the project in Qt Creator
2. Configure the build kit with Qt 6.9.3
3. Build the project using CMake

## User Tutorial

### Location

- Config Location: `QStandardPaths::AppConfigLocation`
  - macOS: `~/Library/Preferences/YTX`
  - Windows: `C:/Users/<USER>/AppData/Local/YTX`
- Data Location: `QStandardPaths::AppDataLocation`
  - macOS: `~/Library/Application Support/YTX`
  - Windows: `C:/Users/<USER>/AppData/Roaming/YTX`

### Guide

- [Inventory](tutorial/user/inventory.md)

## Support Me

If YTX has been helpful to you, I'd be truly grateful for your support. Your encouragement helps me keep improving and creating!

May the force be with you!

[<img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" width="160" height="40">](https://buymeacoffee.com/ytx.cash)

---

**Note**: For commercial use, enterprise support, or server deployment services, please contact the project maintainer.
