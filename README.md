<div align="center">
  <h1>YTX Client</h1>
  <p><em>Client for YTX Double-Entry ERP Program.</em></p>
</div>

<div align="center">
  <a href="https://github.com/ytxerp/ytx-client/stargazers""><img src="https://img.shields.io/github/stars/ytxerp/ytx-client?style=flat-square" alt="Stars" /></a>
  <a href="https://github.com/ytxerp/ytx-client/releases""><img src="https://img.shields.io/github/v/tag/ytxerp/ytx-client?label=version&style=flat-square" alt="Version" /></a>
  <a href="LICENSE""><img src="https://img.shields.io/badge/license-GPL--3.0-blue.svg?style=flat-square" alt="License" /></a>
  <a href="https://github.com/ytxerp/ytx-client/commits""><img src="https://img.shields.io/github/commit-activity/m/ytxerp/ytx-client?style=flat-square" alt="Commits" /></a>
  <a href="https://x.com/ytxerp""><img src="https://img.shields.io/badge/twitter-ytxerp-black?style=flat-square&logo=x" alt="X" /></a>
</div>

## Introduction

**YTX** is a lightweight ERP software designed for businesses, with full support for **multi-user online operation**. It provides a complete solution for managing daily operations across key modules such as **Finance**, **Inventory**, **Task**, **Sale**, **Purchase**, and **Partner**.

Originally built as a desktop application, YTX has evolved into a **fully-featured online ERP system**. With the support of the companion **ytx-server backend service**, it can be deployed on a **local server** using PostgreSQL to enable **real-time collaboration** within teams, without relying on third-party cloud services.

With its **user-friendly interface** and **modular design**, YTX is an efficient and flexible tool for enterprises seeking modern business management.

## License

YTX Client is open source software licensed under the GNU Lesser General Public License v3 (LGPLv3).

This product uses the Qt framework, which is licensed under the GNU Lesser General Public License v3.

Qt is © The Qt Company Ltd.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

**Important Notes:**

- **Client**: The YTX client application is **open source** and available in this repository.

For more information about Qt licensing, see: <https://www.qt.io/licensing/>

By using, modifying, or distributing this software, you agree to comply with the terms of the LGPLv3 license. See the [LICENSE](LICENSE) file for full details.

## Server

The ytx-server backend service is **proprietary software** and is **not open source**.
For server licensing and deployment inquiries, please **contact the maintainer**.

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

#### yxlsx

- Clone the [yxlsx](https://github.com/ytxerp/yxlsx) repository into the `external/` folder of the project:

```bash
  git clone https://github.com/ytxerp/yxlsx.git external/yxlsx
```

- Add it in CMakeLists.txt:

```cmake
  add_subdirectory(external/yxlsx/)
```

#### openssl

```bash
# Debian / Ubuntu
sudo apt-get install libssl-dev

# macOS (Homebrew)
brew install openssl

# Windows (Qt / LLVM-MinGW)

# Download and install MSYS2 from: https://www.msys2.org/
#
# Open the MSYS2 shell and update the system, Close the shell when prompted, then reopen and run again.
pacman -Syu
#
# Launch "MSYS2 MinGW64" (important: NOT MSYS, NOT UCRT)
#
# Install OpenSSL for MinGW-w64:
pacman -S mingw-w64-x86_64-openssl
#
# After installation, OpenSSL will be located at: D:/msys64/mingw64
#
# Use this path as OPENSSL_ROOT_DIR when building with Qt + LLVM-MinGW
```

#### zstd

```bash
# Debian / Ubuntu
sudo apt-get install libzstd-dev

# macOS (Homebrew)
brew install zstd

# Windows (Qt / LLVM-MinGW)
# Download and install MSYS2 from: https://www.msys2.org/
#
# Open the MSYS2 shell and update the system, Close the shell when prompted, then reopen and run again.
pacman -Syu
#
# Launch "MSYS2 MinGW64" (important: NOT MSYS, NOT UCRT)
#
# Install zstd for MinGW-w64:
pacman -S mingw-w64-x86_64-zstd
#
# After installation, zstd will be located at: D:/msys64/mingw64
#
# Use this path as zstd_ROOT when building with Qt + LLVM-MinGW
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
