# Repository working instructions

1. Read `AI_HANDOFF.md` and `PROJECT_STATE.json` before changing implementation. Default language for user-facing documentation: Chinese.
2. This repository is now a two-case documentation playbook: Silksong existing iOS IPA adaptation and Kirby original-game-file nxnative bring-up. The implementation in src/ and tools/ remains Silksong-specific; Kirby full host/game files are not included. Read docs/porting-standard/STANDARD.md before cross-project work.
3. Guide version is 1.2; current Silksong source is 0.2.19. Preserve 0.2.18/0.2.19 game geometry, render-resolution synchronization, joystick/input policy and profile keys unless the user explicitly requests changing that behavior.
4. Profile names and preferences intentionally retain `018`. Changing the visible bridge version must not silently discard user layouts.
5. Never copy engine offsets to another binary without UUID, SHA-256 and expected-byte checks. Same UUID alone does not distinguish this case's different patched engines.
6. Withdrawn 0.2.9/0.2.13 paths used incorrect GCHandle ABI. Do not restore generic managed UI raycasts, native menu GCHandle gates, raw managed pointer caches or executable helpers in Mach-O header padding.
7. Prefer small native visual layers and read-only, typed field observation. Keep touch hit testing separate from visual effects. Do not introduce a second confirm/cancel path without auditing existing game mappings.
8. Use `tools/build.py`, `tools/test.py`, `tools/verify.py`, and the identity-checked packager. Keep generated binaries in ignored `build/`; never commit IPA, UnityFramework, resources, signing credentials or raw device logs.
9. Build success/static analysis/synthetic tests are not device acceptance. Record new user screenshots, diagnostics and matching crash UUIDs separately from historical build reports.
10. Do not infer background-kill causes or promise save-state recovery from unrelated crash logs. Collect matching diagnostics first.
11. Each change needs a concise behavior/rationale/validation note; update state and hashes only when appropriate. New known-good baseline requires actual user/device evidence.
12. The owner explicitly requests Public visibility. Changing visibility does not grant public write permission. Follow LICENSE.md: original docs CC BY-ND 4.0, original code limited attributed verbatim redistribution, third-party material excluded. Check actual GitHub visibility separately; do not claim Public until verified. No existing instruction authorizes messaging other people or publishing release binaries.
