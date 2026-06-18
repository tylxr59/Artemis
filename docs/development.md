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
```

Run:

```bash
./build/artemis
```
