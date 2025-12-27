# YTX Client

- [YTX Client](#ytx-client)
  - [Introduction](#introduction)
  - [Developer Tutorial](#developer-tutorial)
    - [Build](#build)
  - [User Tutorial](#user-tutorial)
  - [Support Me](#support-me)

## Introduction

**YTX** is a lightweight ERP software designed for small businesses, with full support for **multi-user online operation**. It provides a complete solution for managing daily operations across key modules such as **Finance**, **Inventory**, **Task**, **Sale**, **Purchase**, and **Partner**.

Originally built as a desktop application, YTX has evolved into a **fully-featured online ERP system**. With the support of the companion **ytx-server backend service**, it can be deployed on a **local server** using PostgreSQL to enable **real-time collaboration** within teams, without relying on third-party cloud services.

With its **user-friendly interface** and **modular design**, YTX is an efficient and flexible tool for small enterprises seeking modern business management.

## Developer Tutorial

- [Struct Map: Usage](tutorial/developer/struct_map.md) — Reference table mapping Qt structures to PostgreSQL fields

### Build

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
- External Dependencies:

  - Clone the [yxlsx](https://github.com/ytxerp/yxlsx) repository into the `external/` folder of the project:

        ```bash
        git clone https://github.com/ytxerp/yxlsx.git external/yxlsx
        ```

  - Add it in CMakeLists:

        ```cmake
        add_subdirectory(external/yxlsx/)
        ```

## User Tutorial

- Config Location
  - QStandardPaths::AppConfigLocation
  - Mac: `~/Library/Preferences/YTX`
  - Win: `C:/Users/<USER>/AppData/Local/YTX`
- Data Location

  - QStandardPaths::AppDataLocation
  - Mac: `~/Library/Application Support/YTX`
  - Win: `C:/Users/<USER>/AppData/Roaming/YTX`

- [Item: Usage Guide](tutorial/user/item.md)

## Support Me

If YTX has been helpful to you, I’d be truly grateful for your support. Your encouragement helps me keep improving and creating!

Also may the force be with you!

[<img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" width="160" height="40">](https://buymeacoffee.com/ytx.cash)
