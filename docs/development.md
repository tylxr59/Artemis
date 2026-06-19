# Development

## Dependencies

On Arch Linux:

```bash
sudo pacman -S --needed cmake ninja extra-cmake-modules \
  qt6-base qt6-declarative kirigami syntax-highlighting plasma-integration
```

Install and authenticate Codex separately:

```bash
codex login
```

## Build and test

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
cmake --build build --target artemis_qmllint
```

Run:

```bash
./build/artemis
```

The repository includes `.clang-format`, `.clang-tidy`, and `.qmllint.ini`.
To run clang-tidy, configure a build with Clang and point the tool at its
`compile_commands.json`.
