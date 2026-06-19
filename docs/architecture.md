# Artemis architecture

Artemis is a native Qt Quick/Kirigami application. QML owns presentation only;
C++ services own provider communication, Git operations, persistence, and state.

The provider boundary is `AgentProvider`. `CodexClient` implements it using one
`codex app-server` subprocess and newline-delimited JSON-RPC. Raw Codex events
are normalized into provider-neutral domain events before reaching QML.

SQLite stores Artemis project grouping, local thread bindings, and preferences.
Codex remains the source of truth for conversation history and authentication.
Schema changes are applied as ordered, transactional migrations.

Git operations use argument-array `QProcess` invocations. This preserves the
user’s hooks, signing configuration, credential helpers, attributes, and Git
version behavior. User-triggered Git workflows are asynchronous and carry
explicit startup and execution timeouts.

Platform-specific desktop discovery and launch behavior lives under
`src/platform`. QML dialogs and focused views are separate components; the
application controller coordinates them without implementing platform launch
details itself.
