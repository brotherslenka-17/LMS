# User and technician manual

## Setup and operation

Build using README.md. Keep the executable and database separate. Run `lms school.db --init` once, enter your chosen administrator identity and password, then run `lms school.db`. At the `lms:` prompt, enter `login`, username and password. Type each command and response on a separate line. `help` lists the commands for your role. `whoami` displays your user ID. `logout` closes the session; `quit` exits. Do not share accounts.

New and reset accounts must enter `passwd`, current temporary password, then a different password of 12-128 bytes. A session expires after 15 minutes without a command. Five wrong passwords lock that account for five minutes. Password resets and account deactivation revoke existing sessions. Save typed work outside the LMS before login; the application accepts files rather than acting as an editor.

## Administrator

1. `add-department`: enter department name. Record the displayed ID.
2. `add-user`: enter username, display name, one of STUDENT/LECTURER/HOD/ADMIN, then a temporary password. Give each person their own temporary credential securely. `users` lists IDs and roles without password hashes.
3. `set-hod`: department ID, HoD user ID. Only an active HOD account is accepted.
4. `add-course`: course code, title, department ID. `courses` lists course IDs.
5. `set-lecturer`: course ID, lecturer ID, y to assign or n to revoke.
6. `enrol`: course ID, student ID, y to activate or n to deactivate enrolment. Repeating activation updates one relationship rather than duplicating it.
7. `roster`: course ID; lists teaching and enrolment IDs.
8. `edit-user`: user ID, new display name, active y/n. The last active administrator cannot be deactivated. User roles and usernames are fixed after creation; create a separately accountable identity if a different role is required.
9. `reset-password`: user ID and new temporary password. The old password is never recovered or shown.
10. `rename-department`: department ID and new name. `edit-course`: course ID, title and active y/n. Historical relationships are retained; departments with assessment history are not deleted.
11. `backup`: a new destination filename. The application uses SQLite's consistent backup API and rejects an existing destination to avoid accidental overwrite. Place completed backups on encrypted storage under the school custodian's control.

Administrators manage the school configuration; they do not gain an application command for marking, moderation or reading learner work. OS-level possession of the database remains a separate trust boundary.

## Lecturer

Use `courses`, then `assignments` with a course ID. You can work only on assigned active courses.

`create-assignment` asks for course ID, title, instructions, deadline and maximum mark. Enter the deadline in `YYYY-MM-DD HH:MM:SS` format, using school time UTC+02:00. A new deadline must be in the future; maximum is 1.00-1000.00. `edit-assignment` changes deadline and maximum only until the first submission arrives.

`extend` asks for assignment ID, student ID, later deadline and reason. The student must be actively enrolled. The new deadline must increase the current effective deadline. Extensions stop once that student's first assessment exists. Concise reasons should avoid unnecessary medical details because the affected student sees their extension reason.

`work` asks for assignment ID and lists submissions, student IDs, receipt times, phase, latest work version, row version and assessment revision. `export-work` asks for submission ID and a new destination filename; the program writes the authorized attachment and does not automatically open or execute it. Treat attachments as untrusted documents.

`mark` asks for submission ID, the row version shown by `work`, raw mark, written feedback and reason. Marking is permitted strictly after that student's effective deadline. The LMS computes the penalty. Refresh `work` after each mutation: the row version changes. A stale version is rejected to prevent overwriting newer work. A marked result remains hidden from students. Approved results must be returned by the HoD before lecturer revision; released results must be reopened by the HoD.

`history` asks for submission ID and displays every assessment event, actor, time, old/new marks and feedback, phase and reason within your course scope.

## Head of Department

Use `courses`, `assignments`, then `work` to inspect every submission in your departments. `export-work` saves a selected attachment for manual review.

`moderate` asks for submission ID and current assessment revision. Enter an adjusted raw mark or leave blank to retain it; enter adjusted feedback or leave blank to retain it; enter a reason. Any adjusted raw mark receives the same late-penalty calculation. Approval creates a new revision but does not publish it.

Refresh `work`. `release` asks for submission ID, current approved revision and reason. The result becomes visible only after this command commits. `return` changes a marked or moderated result back to MARKED and records why reconsideration is needed. `reopen` is available only for RELEASED results; it hides the result, removes approval and starts a fresh correction/moderation/release cycle. Old published values remain in the audit history. `history` explains the entire record.

Example revision sequence for a new submission: lecturer marks revision 1; HoD approves revision 2; HoD releases revision 3. Reopening produces revision 4. Always use the current number from the list rather than assuming this example applies after extra corrections.

## Student

`courses` lists active enrolled courses. `assignments` asks for course ID and shows instructions, maximum and applicable deadlines, extension details and your receipt status.

`submit` asks for assignment ID and a work-file path. The file must contain 1 byte to 10 MiB. On success, record the receipt's submission ID, immutable version number and school-time timestamp. An error is not a receipt. A first late submission is accepted, including more than three days late. A replacement is allowed only before or exactly at the effective deadline and before marking. Earlier bytes remain stored.

`results` shows only your released final marks and written feedback. “No released results” can mean marking/moderation is still pending. A reopened result is hidden until the correction has been approved and released. `export-work` lets you retrieve your own latest bytes into a new local file; it never exposes another student's work.

## Late policy examples

Receipt exactly at the deadline is on time. Any positive fraction of a 24-hour late period counts as a day. Up to three days, deduct 10% of the assignment maximum per late day, clamped at zero. Beyond exactly 72 hours, the final result is zero; work and feedback are retained. Maximum 80, raw 65, two late days gives 49, not 52. Marks use integer hundredths and the total deduction rounds half up once. Individual accommodations can extend the applicable deadline before assessment.

## Recovery and common errors

- “Please log in” or “Session expired”: log in again. A backwards clock movement also invalidates the session.
- “Change the temporary password”: use `passwd` before attempting another workflow.
- “Stale”: refresh `work`, inspect the newer values, and re-enter the command with the current row/revision.
- “Wait until after ... deadline”: marking has not opened for that student, possibly because of an extension.
- “Database ... constraint, storage or lock”: no success receipt is issued. Check duplicate keys, disk space and other running processes; correct the issue and retry. Do not repeatedly submit without first checking your assignment list for a committed receipt.
- “Cannot open database”: check the path and permissions. The application does not silently fall back to a fresh in-memory database.

## Backup and restore drill

As administrator, create a backup using `backup`. Close all LMS instances before restoration. Keep the current database under a separate name; copy the chosen backup to a new database path. Open that path with the corresponding release and verify login, a known receipt, its exported bytes, a result and its audit history. For v1.1, opening a v1.0 database runs a transactionally applied migration; preserve the v1.0 backup first. Do not open a v1.1 database with v1.0. Backups contain personal records and password hashes, so restrict and encrypt them. The software does not determine the school's retention period.

## Viva demonstration (15-20 minutes)

Show setup and four identities; show a student receipt and blocked early result; demonstrate an invalid scope request; explain the fake-clock boundary tests; show marking then moderation then release; reopen a result and inspect its audit; demonstrate CR1 in v1.1; show CTest, coverage, static analysis, actual local tags and the hosted CI run when available. Explain why each state change is atomic and why role menus alone are insufficient authorization.

## v1.1: whole-class extensions

`extend-class` is available to the lecturer assigned to the assignment's course. Enter assignment ID, a later class deadline and a brief reason. It updates every enrolled student's effective deadline and also applies to students enrolled later. It is rejected after any assessment has been created for that assignment, even if a result was subsequently reopened.

Students use `assignments` to see the original deadline, class extension and reason, any individual extension and the effective deadline. The effective value is the latest of the three. A later personal accommodation is never shortened. Existing receipt timestamps and original receipt deadlines remain immutable; the current receipt classification can change to on-time when an extension applies. Resubmission becomes available again only while the work remains RECEIVED and the new effective deadline has not passed.
