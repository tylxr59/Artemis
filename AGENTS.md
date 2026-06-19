# Repository Notes

## QML changes

Artemis embeds its QML files in the application executable through
`qt_add_qml_module`.

After changing a QML file, rebuild the actual application target:

```bash
cmake --build build --target artemis
```

Running only `artemis_qmllint` is not sufficient. That target validates and
copies QML sources for linting, but it does not rebuild the QML cache embedded
in `build/artemis`.

Restart every running Artemis process after rebuilding. A process launched
before the rebuild continues using the old executable and embedded QML, even
if `/proc/<pid>/exe` points to `build/artemis (deleted)`.

For QML changes, use this verification sequence:

```bash
cmake --build build --target artemis
cmake --build build --target artemis_qmllint
ctest --test-dir build --output-on-failure
```
