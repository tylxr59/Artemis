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

A process launched before the rebuild continues using the old executable and
embedded QML, even if `/proc/<pid>/exe` points to `build/artemis (deleted)`.
Do not kill, restart, or otherwise interrupt a running Artemis process unless
the user explicitly asks you to. Instead, report that existing instances must
be restarted before they can be used to verify the QML change.

For QML changes, use this verification sequence:

```bash
cmake --build build --target artemis
cmake --build build --target artemis_qmllint
ctest --test-dir build --output-on-failure
```
