# Artemis

Artemis is a native KDE/Linux AI coding client built with C++20, Qt Quick,
Kirigami, and the Codex app-server protocol.

## Features

- Project and persistent Codex thread navigation
- Local project threads
- Streamed agent activity
- Git status and diff review
- AI-generated commit messages
- Separate persisted model settings for coding, commit messages, and thread titles
- AI-generated thread names after the first message
- Commit-and-push workflows for the current or a new feature branch
- SQLite persistence and diagnostics

## Requirements

- A C++20 compiler
- CMake 3.24 or newer
- Ninja
- Qt 6.8 or newer, including Qt Quick, Qt Quick Controls, SQL, and Test
- KDE Frameworks 6 Kirigami and Syntax Highlighting
- Git
- Codex CLI 0.141.0 or newer, installed and authenticated

### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake ninja extra-cmake-modules git \
  qt6-base qt6-declarative kirigami syntax-highlighting plasma-integration
```

### Debian and Ubuntu

Artemis requires Qt 6.8 or newer. Debian 13 (Trixie) provides a compatible
toolchain. Before building on Ubuntu, check `qtpaths6 --qt-version`; Ubuntu
24.04 LTS ships an older Qt release and cannot build Artemis from the standard
repositories.

On a compatible Debian or Ubuntu release:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build extra-cmake-modules git \
  qt6-base-dev qt6-declarative-dev libkirigami-dev \
  libkf6syntaxhighlighting-dev libqt6sql6-sqlite \
  qml6-module-org-kde-desktop
```

### Fedora

```bash
sudo dnf install gcc-c++ cmake ninja-build extra-cmake-modules git \
  qt6-qtbase-devel qt6-qtdeclarative-devel \
  kf6-kirigami-devel kf6-syntax-highlighting-devel \
  kf6-qqc2-desktop-style
```

## Build

Clone the repository and configure a release build:

```bash
git clone https://github.com/tylxr59/Artemis.git
cd Artemis
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the test suite:

```bash
ctest --test-dir build --output-on-failure
```

Run Artemis directly from the build directory:

```bash
./build/artemis
```

For a development build, use `-DCMAKE_BUILD_TYPE=Debug` when configuring.

## Install

To install system-wide under `/usr/local`:

```bash
sudo cmake --install build
```

To install only for the current user:

```bash
cmake -S . -B build-user -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build-user
cmake --install build-user
```

The executable is installed as `artemis`. Desktop and icon files are installed
under the selected prefix.

### Arch package

The repository includes a `PKGBUILD` for installing a tracked
`artemis-git` package:

```bash
makepkg -si
```

Run that command from the repository root. `makepkg` builds, tests, and
installs Artemis through pacman.

## Codex setup

Install the Codex CLI separately, then authenticate it:

```bash
codex login
codex --version
```

Artemis launches Codex through its app-server protocol. The composer offers
supervised, auto-accept edits, and full-access execution modes.

## Development

See [docs/development.md](docs/development.md) for the short development
workflow, [docs/architecture.md](docs/architecture.md) for the application
structure, and [docs/codex-protocol.md](docs/codex-protocol.md) for protocol
notes.
