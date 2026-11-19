# Security policy

## Supported versions

| Version | Supported |
|---|---|
| 1.x | Yes |
| 0.x | No: upgrade to 1.x |

Security fixes are made on `main` and included in the next 1.x patch
release. When a `release/vX.x` maintenance branch exists for an older
major line (see [CONTRIBUTING.md](CONTRIBUTING.md) "Versioning and
releases"), it is listed here with the fixes it receives.

## Reporting a vulnerability

Please **do not open a public issue** for security problems.

Report privately through GitHub's
[private vulnerability reporting](https://github.com/tmitchell-dev/trelvis/security/advisories/new)
("Report a vulnerability" on the repository's *Security* tab). Include the
TreLVis and LVGL versions, the platform, and steps or code to reproduce.

You should receive an acknowledgement within a week. Fixes are coordinated in
a private advisory and credited in the release notes unless you prefer
otherwise.

Vulnerabilities in LVGL itself should be reported to the
[LVGL project](https://github.com/lvgl/lvgl/security).
