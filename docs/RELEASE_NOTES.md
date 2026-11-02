# Release notes

## v1.0
- Four role types with an abstract User base and virtual dispatch.
- Course/department/user administration, scoped assignments, individual extensions and immutable file versions.
- Exact-deadline resubmission, started-day late penalty and integer mark arithmetic.
- Audited marking, moderation, publication, return and reopening.
- Argon2id credentials, forced password changes, lockout and session expiry/revocation.
- Parameterised SQLite persistence, transactional audit integrity and consistent backup.
- Catch2 automated tests, reproducible system script, coverage and static-analysis evidence.
- Local Git release; hosted link and CI must be populated when published.

Known limitations: one-workstation workflow; Linux execution verified, Windows build instructions supplied but not executed here; real stakeholder usability/acceptance and school-PC performance are external validation activities. Optional results CSV export is deferred. File work is opaque and never automatically executed. Privileged OS users remain trusted.

## v1.1
- CR1 adds whole-class extensions, audited reasons and visible deadline provenance.
- The latest of base, class and individual deadlines applies, including later enrolments.
- Any first assessment freezes further class extensions.
- Additive schema migration creates class_extensions without modifying existing work or marks.
- All 24 v1.0 cases, six CR1 cases and an additional four-stage atomicity case pass: 31 cases, 289 assertions; business line coverage 647/654 (98.9%).
- Eight black-box CLI checks pass, including the class extension student display.
