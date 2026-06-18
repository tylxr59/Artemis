# Artemis

Artemis is a native KDE/Linux AI coding client built with C++20, Qt Quick,
Kirigami, and the Codex app-server protocol.

Current vertical slice:

- Project and persistent Codex thread navigation
- Local and managed-worktree threads
- Streamed agent activity
- Git status and diff review
- AI-generated commit messages
- Separate persisted model settings for coding, commit messages, and thread titles
- AI-generated thread names after the first message
- Commit-and-push workflows for the current or a new feature branch
- SQLite persistence and diagnostics

See [development documentation](docs/development.md) for setup and build steps.

Codex CLI 0.141.0 or newer must be installed and authenticated. The composer
offers supervised, auto-accept edits, and full-access execution modes.
