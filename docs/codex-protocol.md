# Codex protocol notes

Artemis requires Codex CLI 0.141.0 or newer.

Startup:

1. Run `codex --version`.
2. Start `codex app-server --listen stdio://`.
3. Send `initialize` with `capabilities.experimentalApi` enabled. Artemis uses
   experimental turn fields such as `collaborationMode`.
4. Send the `initialized` notification.
5. Load models and project-matching threads.

The client tolerates unknown JSON fields and unknown notification methods.
Requests have a 60-second transport timeout. A crashed process is restarted
with exponential backoff capped at 16 seconds.

Coding threads are persistent. Commit-message and thread-title generation
threads are ephemeral. Generated titles are applied with `thread/name/set`.
