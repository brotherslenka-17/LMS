# Engineering record

Gate 2: the user reported “it is approved” before implementation in this session. No supervisor identity, signed approval or invented meeting date is supplied. The original proposed design is archived in approved_design_baseline.md; this record supersedes its pending implementation status only on the strength of that user report.

Actual implementation adjustments:
- Source is consolidated into Domain.cpp, Services.cpp and SqliteRepository.cpp instead of one file per small class. Responsibilities and public classes remain separated; the final traceability maps actual files.
- Session identifiers are random opaque tokens held in memory. A credential fingerprint invalidates sessions after reset without a new users-table column.
- Typed repository methods implement SQL-only-in-infrastructure. Transaction ownership stays in services.
- Students' assignment lists include the latest receipt and current on-time/late classification, so an extension updates the visible interpretation while preserving the original receipt deadline.
- Third-party SQLite and Catch2 are vendored; libsodium source is supplied for offline Linux compilation. No student password or school database is bundled.

Self-review and real changes:
- Compiler warning about misleading indentation: expanded conditional statements with clang-format before final builds.
- cppcheck shadowFunction: renamed a local assessment variable that shadowed mark().
- cppcheck virtualCallInConstructor: schema initialization now calls internal transaction primitives, not virtual methods from the constructor.
- cppcheck const and algorithm findings: use const references and standard transformation/filtering algorithms where appropriate.
- cppcheck false-positive stream-state pattern: use an unconditional command loop with explicit EOF exit; terminal restore/newline use one guard.
- Identified missing current receipt classification in the list projection: added receipt metadata and assertions before v1.0 release.

No application test failure was observed in the first complete 23-case run. The issues above are review/analysis findings, not fabricated runtime defects. The evidence files contain real executions. No external reviewer, Gate 3 acceptance or hosted CI success is claimed.

Storage experiment: the first large benchmark database under the shared workspace subsequently failed an SQLite integrity inspection. The same benchmark on a new local `/tmp` database passed, with 803 accounts, 16,000 submissions and 48,000 history events. The exact cause of the first failure is not established; do not label it a proven application defect or silently omit it. Added startup quick_check so damaged data fails closed instead of appearing to have missing users. Production deployment remains on a verified local filesystem with tested backups, as the baseline requires.

v1.1 review: the 24 baseline cases remain, six CR1 cases cover class-wide deadlines and migration, and a further atomicity case injects failure after each of the three assessment writes and before commit. Final result: 31 cases, 289 assertions, all passing. Eight black-box checks passed against the optimized release. The local performance fixture passed integrity_check and foreign_key_check.

Hosted publication: the private repository https://github.com/brotherslenka-17/CS4433-Moderated-LMS was created. Automatic approval review rejected uploading the full coursework payload without explicit user authorization for that destination. No source upload or hosted CI success is claimed. The final local bundle and source archive remain complete and reviewable. The compact hosted distribution can restore pinned vendor bytes using scripts/fetch_dependencies.py; the complete offline submission already includes them.
