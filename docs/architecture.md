# Artemis architecture

Artemis is a native Qt Quick/Kirigami application. QML owns presentation only;
C++ services own provider communication, Git operations, persistence, and state.

The provider boundary is `AgentProvider`. `CodexClient` implements it using one
`codex app-server` subprocess and newline-delimited JSON-RPC. Raw Codex events
are normalized into provider-neutral domain events before reaching QML.

SQLite stores Artemis project grouping, thread bindings, preferences, and
managed worktree metadata. Codex remains the source of truth for conversation
history and authentication.

Git operations use argument-array `QProcess` invocations. This preserves the
user's hooks, signing configuration, credential helpers, attributes, and Git
version behavior.
