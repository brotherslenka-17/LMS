# MODERATED LEARNING MANAGEMENT SYSTEM
CS4433 Software Engineering | National University of Lesotho

Lenka RS | Student number: 202321267

Final engineering report, v1.1

## Abstract
This project implements an offline moderated learning management system in C++17 and SQLite. It separates lecturer marking, departmental moderation and result publication, with scoped access for students, lecturers, Heads of Department and administrators. Immutable submission versions preserve received work, while each assessment change commits together with an attributable history event. The implementation uses Argon2id password hashes, prepared SQL, transactional migrations and an integer late-penalty strategy. Verification comprises 31 automated cases with 289 assertions, eight black-box console checks and 98.9% business-layer line coverage. A measured synthetic dataset contains 800 students and 16,000 submissions. After the actual local v1.0 release, CR1 adds whole-class extensions while preserving later individual accommodations. This report distinguishes executed developer evidence from school acceptance and hosted CI, which are not claimed as completed. The source, manuals, diagrams, local Git history, binaries and raw evidence accompany the report.

## Evidence and approval status
The user reported “it is approved” before implementation. This is the recorded authority for proceeding beyond the design gate; it does not identify a supervisor, signature or meeting. The seven-week schedule is a planning baseline, not a fabricated execution timeline. Implementation and verification occurred in the assisted workspace.

A private repository exists at https://github.com/brotherslenka-17/CS4433-Moderated-LMS. At report assembly, automatic approval review blocked uploading the full coursework payload pending explicit approval for that destination. The repository therefore does not yet contain the application, and no hosted CI pass is asserted. The included Git bundle preserves the actual local implementation history and releases. This outstanding configuration-management step is visible throughout the evidence index.

# 1. Introduction and baseline decisions
The case study concerns a secondary school whose assessment records must remain private until departmental checking is complete. Four roles share one application, but their authority differs both by action and by resource. A lecturer assigned to one course must not mark another course merely because both users are lecturers. Similarly, a Head of Department may moderate only courses belonging to their department. These scope restrictions are part of the business model and must be checked by services, even when the console menu hides unavailable commands.

The system includes user and course administration, enrolment, assignment creation, individual extensions, versioned submissions, marking, written feedback, moderation, release and assessment history. Payments, video conferencing, automated grading, plagiarism detection and web or mobile interfaces remain outside the assignment scope. The delivered interface is a console program. This limits installation work and provides a testable interface for older hardware; it does not establish usability automatically. A task-based user trial remains necessary.

The implemented baseline interprets the capacity requirement as 800 stored student accounts, not simultaneous interactive users. This interpretation was recorded in the design package before the user reported that the design was approved; no independent school sign-off is claimed. One active console session on a supervised school workstation is the implemented deployment model. This fits the permitted single-computer deployment but may create an operational bottleneck at submission time. If the school requires many concurrent sessions, the architecture and schedule must be reopened before Gate 2. A shared SQLite database file on a network drive is not the selected solution; SQLite's own deployment guidance distinguishes local embedded use from workloads needing a server database (SQLite, n.d.-b).

The schedule uses relative working days. Confirm submission and viva arrangements with the supervisor. Do not infer gate approval from a proposed schedule. The supplied stakeholder statements are the only elicitation evidence presently available; additional interviews, observations and usability trials are future activities.

# 2. Project management plan
## 2.1 Feasibility assessment
| Dimension | Assessment and evidence needed | Recommendation |
|---|---|---|
| Technical | The C++17 toolchain builds on Linux; local hashing and workload benchmarks have passed. The school PC remains untested. | Technically demonstrated locally; validate school hardware before operational use. |
| Operational | Four role menus can match familiar school responsibilities. Shared-machine queues, training and backup ownership remain unresolved. | Obtain principal and IT acceptance of the single-session workflow. |
| Economic | Reuse of an existing workstation avoids assuming new hardware. Development labour, training, removable storage and support still have costs. | Use a transparent estimate; obtain quotations before procurement. |
| Legal | Learner identities and marks are personal information. The Lesotho Data Protection Act is a relevant starting point for a Lesotho school (Kingdom of Lesotho). | School must confirm lawful handling, access and retention policy before real data use. |
| Schedule | Seven weeks provide 35 working days. Design approval is a hard predecessor of implementation, and the baseline has no task float. | Feasible with tightly controlled scope and prompt reviews; approval delays threaten completion. |

The provisional effort budget is 175 hours: planning 15, requirements 25, design 35, implementation 40, verification 25, deployment 10, maintenance 15 and final synthesis 10. At an illustrative planning rate of M100 per hour, labour is M17,500. This is a scenario estimate, not a quoted price or a student fee. Hardware cost is assumed to be zero only if the existing computer passes the agreed environment check. An additional M1,000 planning allowance covers storage and incidentals, with M1,850 contingency on the M18,500 subtotal, giving M20,350. Replace allowances with actual quotations where expenditure is required.

The local implementation establishes technical feasibility, while school deployment remains conditional on operational checks. A console system reduces implementation scope but increases the importance of a usable workflow. A local database removes internet dependence but transfers operational responsibility to backup discipline. These trade-offs are acceptable only if the school accepts the deployment workload and identifies a technician responsible for recovery.

## 2.2 Work breakdown, dependencies and critical path
| Work package | Scheduled weeks | Deliverables and exit |
|---|---|---|
| Planning and management | 1 | Feasibility, work breakdown, schedule and initial risk register |
| Requirements engineering | 1-2 | Stakeholder/conflict analysis, SRS and use cases; Gate 1 in Week 2 |
| System design | 2-3 | Preliminary architecture in Week 2; detailed UML, patterns and database design; Gate 2 in Week 3 |
| Implementation | 4-5 | C++17 and SQLite implementation, CI definition and recorded code review |
| Verification and validation | 5-6 | Unit, integration, system and acceptance work; Gate 3 in Week 6 |
| Deployment | 6 | Packaging, installation guide, user manual and v1.0 release |
| Maintenance | 7 | CR1, impact analysis, v1.1 and regression evidence |
| Completion | 7 | Professional issues, final report and viva |

[[figure:D09_gantt]]

The schedule follows the supplied phase ranges. Planning and stakeholder analysis overlap in Week 1. Requirements are consolidated in Week 2 while architecture alternatives are explored. Gate 1 controls the requirements baseline; preliminary architecture work does not authorize coding. Detailed design is completed in Week 3, and implementation starts in Week 4 only after Gate 2 approval. Implementation and developer verification overlap in Week 5; formal system and acceptance work continues in Week 6 before the deployment decision.

The controlling milestone chain is Gate 1 -> Gate 2 -> integrated implementation -> Gate 3 -> v1.0 deployment -> CR1 regression and v1.1 -> final handover. Gate 3 and deployment share Week 6; maintenance and completion share Week 7. A delay to a mandatory gate can delay every downstream milestone. The supplied week ranges do not define activity-level effort or float, so this plan does not infer a numerical critical-path duration from overlapping phase labels. Optional scope can be reduced, while moderation, audit integrity and required verification remain mandatory.

At each gate, record reviewer, issues, decision and conditions. Close blocking conditions before proceeding. The weekly packs organise existing work against the schedule; they do not claim that seven separate weekly sessions or signed reviews occurred. The user reported design approval before implementation, while stakeholder acceptance and hosted CI retain their documented status.

## 2.3 Risk register
Likelihood and impact use 1 (low) to 5 (high); exposure is their product. The owner is the student developer unless a named school role is indicated. Values below preserve the initial planning assessment. They are not measured probabilities. The later risk-update table records developer reassessment without inventing gate meetings.

| ID | Category and risk | L | I | L x I | Mitigation and trigger | Owner |
|---|---|---|---|---|---|---|
| R01 | Technical: dependencies fail on school PC | 3 | 4 | 12 | Pin versions; reproduce build on target; trigger: unsupported compiler/library. | Developer |
| R02 | Project: Gate 2 approval delayed | 3 | 5 | 15 | Book reviews early; send package in advance; rebaseline on missed gate. | Developer / supervisor |
| R03 | Technical: grade change bypasses moderation | 3 | 5 | 15 | Central transitions; revision-bound approval; adversarial service tests. | Developer |
| R04 | Technical: power failure loses records | 3 | 5 | 15 | Atomic transactions; recovery drill; encrypted daily backup. | IT officer |
| R05 | People: teachers find CLI confusing | 3 | 3 | 9 | Role-specific command help; examples; task-based usability trial and training. | Developer / lecturer |
| R06 | People: password sharing or unattended session | 3 | 4 | 12 | Individual accounts; inactivity logout; no shared administrator password. | IT officer |
| R07 | Policy: partial days / extensions disputed | 4 | 4 | 16 | Approve boundary examples at Gate 1; publish rule and appeal route. | Principal / HoD |
| R08 | Project: excessive features consume schedule | 4 | 4 | 16 | Must baseline; change log; defer optional exports and GUI. | Developer |
| R09 | Technical: workstation clock changed | 2 | 5 | 10 | Restrict clock changes; record receipts; flag backwards movement. | IT officer |
| R10 | Operational: one-terminal queues unacceptable | 3 | 5 | 15 | Confirm workload before Gate 1; redesign if concurrent use is essential. | Principal / IT officer |

The gate update form for every risk is: current exposure, evidence since previous review, mitigation completed, residual exposure, owner decision and next review date. The baseline ratings are preserved, with later evidence-based reassessment in Appendix C. R03 remains high until direct-service access tests and mark-history rollback tests pass; a diagram alone does not retire it. R04 remains open until restoration of a real test backup is demonstrated. This connects risk closure to evidence rather than optimism.

# 3. Requirements engineering
## 3.1 Stakeholders and conflicts
| Stakeholder | Interest and decision responsibility | Conflict / clarification | Proposed resolution |
|---|---|---|---|
| Principal | Simple operation; trustworthy school results | Immediate lecturer publication conflicts with checked results. | Marking saves a draft; only HoD release exposes mark and feedback. |
| Lecturer | Efficient marking; discretion over extensions | Individual extensions may conflict with common deadline. | Authorized lecturer may extend for an enrolled student before that student's first assessment. |
| HoD | Department-wide review; consistent late policy | Meaning of a day and 10% is unspecified. | Started 24-hour periods; deduct 10% of maximum per period for up to three periods. |
| Student representative | Resubmission, feedback, late acceptance | Resubmission after deadline and marked work unspecified. | Preserve versions; replace only while unassessed and within effective deadline. Accept a first late submission even after three days. |
| IT officer | Offline use, protected passwords, performance | 800 accounts versus 800 concurrent sessions unspecified. | Baseline: 800 stored students and one interactive session; school deployment validation remains necessary. |
| Technician / school custodian | Recovery and privacy | Backup retention, account ownership and physical access unspecified. | Assign a custodian and approve retention/access policies before real deployment. |
| Supervisor / assessor | Waterfall gates, consistent evidence and viva | Implementation must not precede design approval. | User reported design approval before implementation; retain real execution evidence and outstanding school acceptance. |

The principal's release control takes precedence in the implemented resolution because the brief explicitly assigns checked publication to the HoD. The lecturer's underlying need is addressed through a clear pending-moderation queue, not an unauthorized early preview. Assessment history is readable by the department HoD and assigned lecturer; ordinary administrators manage accounts without a result-view privilege. Students may see a receipt and the status of their submission before release, but not raw marks, adjusted marks or written assessment feedback. Error messages and administrator screens must not become alternate disclosure paths.

A clarification meeting should ask the principal and HoD to approve the boundary table below, ask IT to approve the deployment load, and ask teachers to demonstrate a typical marking task. Record answers and unresolved disagreements. This is planned elicitation, not a claim that these meetings occurred. Any resulting requirement change receives a new version and an updated traceability row before Gate 1 closes.

## 3.2 Business rules and exact calculations
| Rule | Implemented baseline policy |
|---|---|
| BR01 | One active account has one role. A HoD may oversee multiple departments; each department has at most one current HoD. Cross-role appointments are outside this baseline. |
| BR02 | Service-layer authorization checks current active account and resource scope on every command. Students access only their own enrolled work and results. |
| BR03 | Store time as UTC epoch seconds; display school time UTC+02:00 with offset. Use the workstation clock; do not accept a student-supplied receipt time. |
| BR04 | Effective deadline D is max(base deadline, individual extension) in v1.0. An extension must increase the applicable deadline and include a reason. |
| BR05 | Submission at t = D is on time. For t > D, late days d = ceil((t-D)/86400). A started 24-hour period counts as one day. |
| BR06 | If d=0, final=raw. If 1<=d<=3, final=max(0, raw - d*0.10*maximum). If d>3, final=0. Penalty is on available maximum, not earned raw mark. |
| BR07 | A first valid submission is accepted at any lateness. Resubmission is allowed only in RECEIVED and at t<=D. Keep all versions; assess the latest one only. |
| BR08 | Marking starts strictly after the student's effective deadline. Freeze base deadline and maximum mark once the first submission exists. |
| BR09 | Individual extensions are allowed only before the target student's first assessment exists. They may correct lateness of received work without altering its historical receipt. |
| BR10 | Every saved mark/feedback revision requires a reason and a same-transaction audit event. Initial marking records old values as NULL. |
| BR11 | HoD approval and release are separate commands bound to the current revision. Adjustment or return after approval invalidates approval; released work must first be reopened by the HoD. |
| BR12 | Students see assessment values only when phase is RELEASED. Reopening hides those values until renewed moderation and release. |
| BR13 | Use integer hundredths of a mark (MarkUnits); round the total percentage deduction half up once, then subtract and clamp at zero. Raw marks permit up to two decimals. |
| BR14 | Accept any non-empty work file up to 10 MiB as opaque bytes. Do not execute, parse or automatically open attachments. Store filename as display metadata, not a filesystem path. |

Example: for maximum 80.00, raw 65.00 and 24 hours plus one second late, d=2 and the deduction is 16.00, giving 49.00. At exactly 72 hours the three-day deduction applies; at 72 hours plus one second the result becomes zero. This discontinuity follows the documented interpretation of the HoD statement in the design baseline. The system retains the work even when its computed mark is zero, so teachers can still give feedback and consider a procedural appeal.

When the maximum contains fractional marks, calculate deduction units using (maximum_units * d + 5) / 10 with integer division, for d between one and three. All arithmetic must use a sufficiently wide integer type. For maximum 10.05 and one day late, the 1.005 deduction rounds to 1.01. This avoids binary floating-point discrepancies across machines. Exactly at the deadline, no rounding operation or penalty is applied.

A no-submission record is not automatically converted to a zero in this baseline. That is a separate school policy requiring a missing-work workflow. An extension granted after a receipt but before assessment changes the current effective deadline; the original deadline_at_receipt remains evidence of what the system told the student at receipt time. The assessment stores the effective deadline actually used for the final calculation. Once assessment exists, the baseline rejects further extensions for that student rather than silently changing a released outcome.

## 3.3 Functional requirements
M=Must, S=Should, C=Could, W=Won't in this release. FR01-FR18 are implemented; FR19 has a consistent-backup command and manual restore procedure. FR20 is deferred. The verification section maps requirement families to executed tests.

| ID | Priority | Verifiable requirement | Planned acceptance |
|---|---|---|---|
| FR01 | M | Authenticate active users, enforce required password change, and invalidate the session on logout. | AT01 valid/invalid/disabled/logout |
| FR02 | M | Enforce all four roles and course, department and ownership scope on every operation. | AT02 role-scope denial matrix |
| FR03 | M | Administrator creates, edits and deactivates accounts and resets credentials without displaying old passwords. | AT03 lifecycle and reset |
| FR04 | M | Administrator manages departments and courses, designates HoDs, and assigns lecturers. | AT04 valid and wrong-role mappings |
| FR05 | M | Administrator enrols or deactivates student enrolments with no duplicate course-student pair. | AT05 enrolment constraints |
| FR06 | M | Assigned lecturer creates an assignment with instructions, deadline and positive maximum; freeze base deadline and maximum after first receipt. | AT06 creation and frozen fields |
| FR07 | M | Assigned lecturer grants a reasoned, later individual deadline under BR09. | AT07 extension and rejection paths |
| FR08 | M | Student lists active enrolled courses and assignments with applicable deadlines and receipt status. | AT08 own versus other student |
| FR09 | M | Accept first work submission, validate file size and scope, save receipt and version atomically. | AT09 on-time/late/file/error |
| FR10 | M | Permit eligible pre-deadline resubmission, preserve previous work, and reject ineligible replacement without mutation. | AT10 time and version bounds |
| FR11 | M | Compute the penalty in BR05-BR06 and BR13 using the applicable deadline. | AT11 numeric boundary table |
| FR12 | M | Assigned lecturer marks latest work after the deadline, validates 0<=raw<=maximum, and supplies non-empty feedback. | AT12 marking and validation |
| FR13 | M | Department HoD reviews every marked submission, may adjust raw mark with reason, or return it for correction. | AT13 moderation and stale revision |
| FR14 | M | Department HoD releases only an approved current assessment; no lecturer or administrator can release it. | AT14 release guards |
| FR15 | M | Students view only their own released final mark and written feedback; all other assessment values are withheld. | AT15 disclosure checks |
| FR16 | M | Persist actor, UTC time, old/new raw and final marks, old/new feedback, phase and reason for every assessment revision. | AT16 history and rollback |
| FR17 | M | Restore authorized data after restart, translate input/database exceptions to safe errors, and avoid partial updates. | AT17 persistence and recovery |
| FR18 | M | HoD reopens a released result with reason; any change requires fresh approval and release. | AT18 reopen and reapproval |
| FR19 | S | Administrator creates and restores a consistent backup through an explicit maintenance workflow. | AT19 restore rehearsal |
| FR20 | C | Authorized HoD exports released results to a school-controlled CSV file. | AT20 if implemented |
| FR21 | W | Web/mobile interfaces, payments, plagiarism checks and automated grading are excluded. | Scope review only |

## 3.4 Non-functional requirements and verification conditions
| ID | Priority | Measurable target and verification |
|---|---|---|
| NFR01 | M | On the recorded target PC, login p95 <=2 seconds over 100 sequential trials with 800 student accounts; report hardware, hashing settings, dataset and cold/warm split. |
| NFR02 | M | Assignment list and released-results list p95 <=1 second over 100 trials with 800 students, 40 courses, 800 assignments and 16,000 submissions; one active session. |
| NFR03 | M | All Must workflows work with the network disconnected after installation; no runtime online dependency. |
| NFR04 | M | Zero real plaintext passwords in database or logs; synthetic test credentials are clearly labelled; all real passwords use libsodium Argon2id encoded hashes with unique salts; demonstrate wrong-password rejection. |
| NFR05 | M | Forced failure before commit leaves either all or none of a mark update, status change and audit event; verify across each mutation step. |
| NFR06 | M | Catch2 tests achieve at least 70% measured line coverage of src/domain and src/application; report numerator, denominator, exclusions and actual result. |
| NFR07 | M | Clean CMake C++17 build and all automated tests pass on the declared target OS; no unresolved high-severity static-analysis findings before release. |
| NFR08 | S | Four representative role users each complete their role's three core tasks after <=15 minutes of orientation; >=90% task success and no unauthorized disclosure. |
| NFR09 | M | Keyboard-only workflows; every error includes readable text; no meaning depends on colour. Verify using a terminal accessibility checklist. |
| NFR10 | M | After five consecutive failed logins, reject login for five minutes; inactivity of 15 minutes requires fresh authentication. Persist failed-attempt lockout state. |
| NFR11 | S | On reference dataset, recover from a daily backup within 30 minutes with <=24 hours of data loss; record restore time and integrity checks. |
| NFR12 | M | Denied access and malformed input do not terminate the application across the planned negative-input suite; logs contain neither passwords nor attachment bytes. |

Performance targets apply to an agreed baseline workstation, proposed as a dual-core 2 GHz CPU, 4 GiB RAM and local storage. These are school-target acceptance conditions. Local-host measurements are reported separately and do not establish target-PC performance. A build without security hashing is not a valid performance benchmark. Large attachment volumes are a capacity risk: 16,000 ten-megabyte files would be substantial, so the test dataset must disclose actual attachment sizes and free-disk requirements. The 10 MiB value is a per-file limit, not a promise that unlimited historical versions fit on old hardware.

# 4. Use case model
[[figure:D01_use_cases]]

D01 covers the four roles. Include means a required sub-behaviour; Submit assignment includes Validate submission. Extend means conditional added behaviour; Resubmit extends Submit assignment when previous work exists and the resubmission guards succeed. Role specialisation means each concrete actor inherits the authenticated user's interaction with session management. Authentication is normally a precondition for privileged use cases, rather than a misleading include that would log users in repeatedly for every action. The actor generalisation is shown on the same diagram.



## 4.1 UC06 - Submit assignment (fully dressed)
**Scope and level:** LMS, user goal. **Primary actor:** Student. **Supporting participants:** authenticated session, trusted clock and repository. **Trigger:** Student selects Submit on an assignment. **Preconditions:** Account and enrolment are active, assignment exists in an enrolled course, and the user is authenticated. **Stakeholder interests:** student needs a durable receipt, lecturer needs the correct version, and school needs trustworthy timing.

**Main success flow:** (1) Student selects an assignment from their own list. (2) System displays title, effective deadline and existing receipt status. (3) Student enters the path of a non-empty file to submit. (4) Service opens a transaction and rechecks account, enrolment, assignment and latest version. (5) System obtains trusted current time after the file has been read successfully and guards have been checked. (6) For a first submission, system creates the submission aggregate and version 1; lateness does not block receipt. (7) System stores bytes, display filename, receipt time and deadline-at-receipt. (8) System commits and returns a receipt containing assignment, version, timestamp and on-time/late classification.

**Alternatives:** A1, at step 4 previous work exists: require RECEIVED and t<=effective deadline, then append version n+1. A2, first receipt is late: accept and flag the appropriate late-day count, including more than three days. A3, an individual extension applies: use the later deadline and display it. A4, two requests target the same assignment: serialize within the write transaction; re-evaluate replacement eligibility after acquiring the lock.

**Exceptions:** E1 wrong course or inactive enrolment: deny without revealing other students' work. E2 empty, missing or oversized file: explain validation failure and leave old versions untouched. E3 replacement after the deadline or after assessment: reject without inserting a version. E4 disk full, lock timeout or database error: roll back and do not display a success receipt. E5 an expired or revoked session at command authorization: abort and require login. If commit succeeded but the screen failed before receipt display, the student can inspect the durable version list before retrying.

**Success postconditions:** Exactly one new immutable version is stored and attached to the authenticated student's submission. **Minimal guarantee:** Previous committed work is intact and no uncommitted receipt is claimed. **Business rules:** BR02-BR05, BR07-BR09 and BR14. **Special requirements:** NFR03, NFR05 and NFR12. **Test family:** AT09-AT10; unit boundaries UT01; integration IT01 and IT03.

## 4.2 UC07 - Mark submission (fully dressed)
**Primary actor:** Lecturer assigned to the course. **Trigger:** Lecturer selects unmarked work or revises a draft. **Preconditions:** Active assigned lecturer, latest immutable version exists, current time is strictly after the student's effective deadline, and phase is RECEIVED or MARKED. Moderated work must be explicitly returned to MARKED first; released work requires HoD reopening. **Interests:** teacher retains professional judgement over raw marks; the student receives written reasons; the school applies penalties consistently.

**Main success flow:** (1) Lecturer opens the latest submission and sees its receipt information. (2) System presents maximum mark and calculated lateness. (3) Lecturer enters raw mark, written feedback and reason. (4) Service rechecks authority, time, phase and expected row/revision inside the transaction. (5) Penalty strategy calculates the final mark. (6) System saves the assessment for that exact submission version and changes phase to MARKED. (7) In the same transaction it appends an event, with NULL old marks on first marking. (8) Commit succeeds; the lecturer sees confirmation and pending-moderation status. The student sees no assessment value.

**Alternatives:** A1 existing MARKED assessment: increment revision and preserve previous raw/final marks and feedback in history. A2 lateness exceeds three days: final is zero while raw mark and feedback remain recorded. A3 penalty exceeds earned raw mark: clamp final to zero. **Exceptions:** E1 raw mark outside range or over two decimal places; E2 blank feedback or reason; E3 another change invalidates expected revision; E4 database/audit insertion fails; E5 user or course assignment is revoked. Each exception rejects the command and preserves committed values.

**Success postconditions:** One current assessment exists, one history event records the revision, and publication is blocked. **Minimal guarantee:** No unaudited mark change survives. **Rules:** BR02, BR06, BR08, BR10-BR13. **Tests:** AT11-AT12, AT16; UT02; IT02-IT04.

## 4.3 UC08/UC09 - Moderate and release marks (fully dressed)
**Primary actor:** HoD assigned to the course's department. **Trigger:** HoD opens the departmental moderation queue. **Preconditions:** Active HoD; course belongs to an assigned department; assessment is MARKED for moderation or MODERATED for release. **Interests:** principal requires reviewed results, HoD needs complete departmental visibility, and students require a stable published outcome.

**Main success flow:** (1) HoD selects a department course and assignment, then lists its submissions and current revisions. (2) HoD opens the work, raw mark, deduction, final mark and feedback. (3) HoD enters an approval reason, optionally adjusting raw mark or feedback. (4) Service checks department scope and expected revision inside a transaction. (5) If raw mark changed, recompute final mark with the same approved penalty policy; save all old/new values. (6) Append the moderation event, set MODERATED and bind approval to the new revision. (7) Commit approval. (8) Through a separate Release command, HoD selects the approved current revision. (9) Service rechecks scope, phase and revision, stores release metadata and event, and commits. (10) Student can now read the final mark and written feedback.

**Alternatives:** A1 return for correction: record reason and transition to MARKED, removing approval metadata. A2 correction after release: HoD reopens to MARKED, logs the reason and hides the result until the full approval/release path repeats. A3 multiple submissions: v1.0 processes each as an explicit command; batch release is not assumed. **Exceptions:** E1 wrong department; E2 unmarked or unapproved assessment; E3 stale expected revision; E4 missing reason or out-of-range adjusted raw mark; E5 failure to persist audit. Roll back each failed command in full.

**Success postconditions:** Released result is the current reviewed revision; both approval and release events remain attributable. **Minimal guarantee:** No unrevised approval silently authorizes a changed mark, and failed release leaves visibility unchanged. **Rules:** BR02, BR10-BR13. **Tests:** AT13-AT16, AT18; IT02-IT04. Separating approve and release makes a review hold possible without giving students a preview.

# 5. Software design document
## 5.1 Architecture and alternatives
[[figure:D02_components]]

The layered design separates console parsing, application use cases, domain rules and persistence. The composition root creates the database connection, repository, penalty policy, clock and services. The UI passes typed commands and authenticated context; it does not construct SQL. Services coordinate scope checks, domain calculation and transaction boundaries. Domain objects contain policy without dependencies on console streams or database handles. The SQLite adapter uses bound parameters and translates library error codes to application exceptions.

A monolithic main function was rejected because permission checks and SQL would be scattered among menu branches, making bypass tests and policy changes difficult. A web client/server application would support broader access, but the brief explicitly excludes web interfaces and its operational burden is unnecessary for the implemented single-session workload. A GUI remains a possible console replacement, but building both now would consume time without proving the required learning outcomes. Layering is chosen to make each required behaviour independently reviewable, rather than because more layers are inherently better.

The transaction boundary belongs to the application command. BEGIN IMMEDIATE is used for assessment commands that write, followed by fresh authorization and version checks, then mutations and commit. A bounded busy timeout avoids an indefinite frozen interface. A failed statement or exception invokes rollback through an RAII UnitOfWork. SQLite describes transactions and limitations on concurrent writers in its transaction documentation (SQLite, n.d.-c). File reading occurs before opening the write transaction to avoid holding a lock during user interaction; receipt time is then taken at validation, not from a user-supplied file timestamp.

## 5.2 Class design, patterns and SOLID
[[figure:D03a_classes]]
[[figure:D03b_classes]]
[[figure:D03c_classes]]
[[figure:D03d_records]]
[[figure:D03e_persistence]]

User is abstract with a virtual destructor and pure virtual role() method. Student, Lecturer, HeadOfDepartment and Administrator override role(). This is genuine polymorphism used when constructing session behaviour and available actions. It is not sufficient for authorization by itself: an in-memory role object does not prove present ownership of a resource. Services reload relevant role and mapping data from persistent records before protected mutations.

Strategy isolates TenPercentPenalty behind ILatePenalty. A later school policy can replace the calculation without editing submission persistence or console menus. The strategy is deterministic and receives raw units, maximum units and elapsed seconds; it performs no clock reads or database queries. This makes exact-boundary and rounding tests straightforward. The effective deadline belongs to the assignment/extension policy, so the penalty strategy does not need to know whether a deadline arose from an individual or future class extension.

Repository separates domain-oriented data operations from SQLite details. IRepository exposes operations such as loading submission aggregates and saving an assessment with its event; SqliteRepository implements them using prepared statements. A single repository interface is practical at this scale, but should be split by responsibility if it grows excessively. UnitOfWork controls an atomic operation, and RAII releases statements, connections and transaction state on error. These are supporting design techniques, not claims that every class is a named pattern.

The single responsibility principle is reflected in separate input parsing, policy calculation and persistence. Open/closed design is achieved for penalty policy through substitution, while avoiding a speculative plugin framework. Role subtypes obey the User contract, supporting substitution without downcasts. Interface segregation is pursued through small ILatePenalty and IClock contracts; IRepository size is a review concern. Dependency inversion makes AssessmentService depend on these abstractions, allowing FakeClock and controlled repository failure doubles in tests. Each principle is justified by a concrete testing or maintenance need rather than listed as a slogan.

## 5.3 Behavioural models
[[figure:D04_state]]

The stored workflow phase is RECEIVED, MARKED, MODERATED or RELEASED. Submitted versus Late is a derived classification inside RECEIVED, determined from receipt time and the current applicable deadline. This avoids wrongly treating lateness as mutually exclusive with later marking or release. The receipt preserves the deadline displayed when work arrived. Assessment freezes the effective deadline and late-day count used in its calculation so historical outcomes can be explained even if a future version supports broader policy changes.

[[figure:D05_activity]]
[[figure:D06a_submit_sequence]]
[[figure:D06b_moderate_sequence]]

The activity swimlanes distinguish human responsibility from system calculation. The sequences make two otherwise easy-to-miss guarantees explicit: resubmission checks occur inside the write transaction, and release rechecks the exact approved revision. Expected row_version and assessment revision values detect stale edits. A denied or conflicted command returns a refresh instruction; it never overwrites a more recent teacher decision without notice. The console may be single-session, but explicit revision handling protects future adapters and accidental repeated commands.

## 5.4 Error handling and security design
The console parses full input lines into bounded values rather than allowing stream failures to poison the next prompt. Numeric conversions reject trailing text, non-finite values, excessive decimals and range violations. Domain validation errors produce actionable messages. Database exceptions are logged with an operation identifier and sanitized detail; students do not receive SQL, secret hashes or file contents. A final UI boundary catches expected operational exceptions and returns to a safe menu. Fatal startup failures, such as an inaccessible database, must terminate with a clear explanation rather than pretending the school data is empty.

Passwords use libsodium's encoded password-hash API and verification function. Its documentation describes Argon2id as the default algorithm and stores the required salt and parameters in the encoded result (libsodium, n.d.). The design therefore stores one password_hash string and avoids home-grown cryptography or a fixed shared salt. The implementation uses the interactive Argon2id setting with 64 MiB memory, benchmarked locally; repeat on the target PC. Password reset issues a temporary credential requiring change; it never recovers the previous password. Wrong-password, forced-change, salt uniqueness, lockout and reset-revocation tests have executed successfully.

Attachment bytes are stored in SQLite rather than referenced through student-controlled paths. This supports atomic receipt and avoids a database pointing to a missing file after a filesystem failure. The trade-off is larger databases and backups, so enforce file-size limits and monitor capacity. Display filenames are stripped of path components and terminal control characters; authorized work export asks for a new local destination and refuses an existing file. Opening attachments is a teacher-controlled external action, not automatic execution by the LMS.

# 6. Database design
[[figure:D07a_erd]]
[[figure:D07b_erd]]

## 6.1 Normalisation and mapping
The v1.0 schema has thirteen tables; v1.1 adds class_extensions, giving fourteen. Atomic column values and separate version rows establish first normal form. Composite-key relationships such as enrolments and lecturer_courses hold facts about the whole course-user association, avoiding partial dependencies. Department names are stored only in departments, course titles only in courses, and account details only in users. Assessment records refer to the submission and its immutable version rather than duplicating the student's name or course title. These choices avoid non-key transitive dependencies in the operational relations and support third normal form.

Assessment events intentionally retain old and new values. They describe distinct historical facts at a revision, rather than acting as a second editable source of the current mark. The unique submission/revision pair prevents duplicate historical identities. Receipt-time deadlines and assessment-time effective deadlines are separate historical snapshots with different meanings. They must not be confused with the current base deadline; their duplication is intentional temporal evidence, not an accidental denormalisation used to simplify a join.

Single-table inheritance maps the four User subclasses to one users table with a role discriminator. Four nearly identical user tables were rejected because they would duplicate identity and authentication fields. Many-to-many course links become association tables, even though an object model may expose them as collections. The C++ aliases MarkUnits and TimePoint map to integers; the phase is a checked string. These aliases are not strong wrapper types. Services and strategies have no tables because they implement behaviour rather than persistent entities.

| Table | Domain mapping and key | Principal data / relationships |
|---|---|---|
| users | User and four concrete role types; id | username, display_name, hash, role, active, forced change, failed login and lockout |
| departments | Department; id | Unique department name |
| department_heads | HeadOfDepartment assignment; department_id | One current HoD per department; a HoD may serve multiple departments |
| courses | Course; id | Unique code, title, department FK, active flag |
| lecturer_courses | Lecturer-Course association; course_id + lecturer_id | One row per teaching assignment |
| enrolments | Student-Course association; course_id + student_id | Active enrolment flag; identity retained when disabled |
| assignments | Assignment; id | Course/creator FKs, title, instructions, UTC deadline, maximum units |
| individual_extensions | IndividualExtension; assignment_id + student_id | Later deadline, grantor, time and reason |
| submissions | Submission aggregate; id | Unique assignment/student pair, phase, concurrency version |
| submission_versions | SubmissionVersion; submission_id + version_no | Immutable receipt, deadline snapshot, filename and bytes |
| assessments | Assessment; submission_id | Exact version FK, raw/final units, late days, feedback, revision, reviewer/release metadata |
| assessment_events | AssessmentEvent; id | Unique submission/revision, actor/time, old/new marks/feedback/phase, reason |
| admin_events | AdministrativeEvent; id | Actor/time, entity locator, action, old/new value and reason |

## 6.2 Data dictionary and constraints
The SQL migrations are the authoritative column-level definition; the generated Markdown dictionary lists every column, SQL type, nullability, default and key participation. Identifiers are SQLite INTEGER keys represented by the Id alias of std::int64_t in C++. Times use signed 64-bit epoch seconds. Mark units use signed 64-bit integers in the application with a defined assignment maximum range of 1.00 to 1,000.00. Name, feedback and file limits are explicit in the DDL. Foreign keys use restrictive deletion by default, so deactivation preserves assessment attribution.

Database constraints enforce identities, basic ranges, unique enrolments, valid phase labels and version references. Foreign-key enforcement must be enabled and checked on every connection, as the SQLite documentation explains (SQLite, n.d.-a). Append-only triggers block ordinary updates/deletes of events and submission versions. These controls do not make the database tamper-proof against an operating-system owner who can replace the file or change its schema. Restricted filesystem permissions and backups remain necessary.

Cross-table policy checks are service responsibilities: raw<=assignment maximum; lecturer and HoD scope; student role and active enrolment; extension later than applicable deadline; phase transitions; and exact parity between current assessment revision and its event. Current assessment, event and phase must be saved in one transaction. The schema alone is not represented as enforcing all application invariants. Executed integration tests deliberately fail writes at different transaction stages and establish that mark, phase and event changes roll back together.

# 7. Implementation and engineering decisions
The delivered source comprises three core translation units, a console composition root, typed headers and SQL migrations. Domain.cpp owns parsing, time formatting, role construction and the deterministic late policy. Services.cpp contains AuthService, AdminService and AssessmentService, keeping use-case orchestration separate from input prompts. SqliteRepository.cpp owns the prepared statements, relational mapping, schema creation, migrations and backup API. main.cpp converts console input into service calls and presents results. The shared public headers specify contracts without exposing SQL to the console. Consolidating small classes into these files is a documented implementation adjustment from the proposed one-file-per-service layout; it does not change the dependency direction.

The abstract User base has a virtual destructor and a pure virtual role method. Each concrete role overrides it, and makeUser returns unique ownership through the base type. Authorization invokes the virtual method, so the inheritance is exercised rather than included only for a marking criterion. It is deliberately a small model: role inheritance does not encode every resource permission. The application reloads the active account and relevant enrolment, teaching or department relationship. This supports revocation immediately on the next protected command and avoids trusting a menu or a stale session role as proof of authority.

An authenticated session is an opaque random token kept in AuthService memory. Its record contains user identity, last activity and a credential fingerprint. Password reset changes that fingerprint and invalidates the old session; account deactivation is checked afresh. Tokens disappear when the process ends. Failed-login counters and lockout deadlines persist in SQLite, preventing a process restart from clearing a five-minute lockout. A backwards clock movement invalidates a session rather than extending its lifetime. These measures do not defend against a privileged operating-system user altering the machine clock or database; that boundary belongs to workstation administration.

Passwords are hashed through libsodium's password-hash API, with algorithm metadata, salt and cost encoded in one stored string. Temporary credentials require change before normal commands. The terminal suppresses interactive password echo, and the console does not print stored hashes. Synthetic passwords in the tests are explicitly labelled test data. The system never attempts reversible password recovery: administrators reset credentials, and the audit identifies the action without storing the new plaintext. The benchmark uses a shared synthetic hash to construct its large fixture efficiently; separate security tests establish that independently created real account hashes have distinct salts.

Assessment mutations acquire an immediate SQLite transaction and re-evaluate policy before writing. A UnitOfWork object rolls back if an exception exits without commit. First marking writes a current assessment, updates submission phase and appends an event with NULL old marks. Subsequent edits preserve previous raw/final values and feedback. Moderation and release each produce their own revision. Expected row versions protect lecturer marking against changed submissions; expected assessment revisions protect departmental decisions against stale approvals. SQLite serializes writers, while these revision checks identify a logically outdated command even when it can acquire the database lock.

The persistence adapter binds user-supplied values into prepared statements. Table and query structures are fixed by implementation; no user string becomes SQL syntax. Foreign keys are enabled and verified for each connection, synchronous durability is configured, and a two-second busy timeout prevents an indefinite wait on another writer. Startup checks schema compatibility and runs quick_check, reporting damaged data rather than silently displaying an apparently empty school. An unsupported future schema is rejected. The program catches application and standard exceptions at the console boundary, emits a readable error and normally returns to the command loop. Fatal startup errors exit instead of continuing with an unusable database.

Work files are read before the write transaction, bounded at 10 MiB and stored as opaque BLOBs. The submitted path is reduced to a safe display filename. Storing bytes with the receipt avoids committing a record whose external attachment was never saved. Versions are immutable through normal SQL operations, and the latest version is derived rather than maintained as an independently editable pointer. This favors consistency over unlimited storage: repeated large submissions can grow the database significantly, so the custodian must monitor disk space and define retention outside the application. Work export writes a new destination without automatically opening untrusted content.

The implementation intentionally leaves the optional released-results CSV export out of scope. It includes authorized work export and consistent database backup, which serve different purposes. User roles and usernames are immutable after account creation; display names and activity status can change. A changed role requires a separately accountable account. There is no automatic missing-submission zero, because that would introduce a policy not defined in the agreed submission workflow. These limits keep the delivered Must requirements auditable and give future changes a clear starting point rather than an undocumented collection of partial features.

# 8. Verification, validation and measured results
## 8.1 V-Model evidence and reproducibility
Verification follows the V-Model mapping: policy and detailed design connect to unit checks, repository and service interactions connect to integration checks, and SRS workflows connect to black-box console scenarios. The tests use Catch2 2.13.10 and CTest; coverage is generated with gcovr over GCC-instrumented business code. The package retains commands, JUnit XML, coverage JSON and HTML, system transcripts, performance output and static-analysis output. A test count is not inferred from the XML tests attribute alone: Catch2 reports assertion totals there, while testcase elements identify the 31 named cases.

| Evidence | v1.0 observed | v1.1 observed |
|---|---|---|
| Catch2 cases / assertions | 24 / 230, no failures | 31 / 289, no failures |
| Business lines | 628 / 635, 98.9% | 647 / 654, 98.9% |
| Business functions | 63 / 64 | 64 / 65 |
| Business branches | 917 / 1504, 61.0% | 958 / 1560, 61.4% |
| Black-box CLI checks | 7 passed | 8 passed |
| cppcheck configured findings | 0 | 0 |
| Hosted CI | Not executed | Pending authorized source upload |

Coverage includes src/domain and src/application only, matching the required business-logic measure. UI, infrastructure, tests and third-party sources are excluded from that denominator; they are not claimed to have 98.9% coverage. Integration tests nevertheless exercise actual SQLite statements and migrations. The distinction matters because a narrow coverage number can look impressive while leaving important behavior untested. The separately reported 61.4% branch coverage is lower than line coverage, including compiler-generated exceptional paths. The project does not claim exhaustive path coverage or absence of defects on the strength of the 70% line threshold.

FakeClock makes time-dependent tests deterministic. Boundary partitions include one second before the deadline, the deadline itself, one second late, each full-day boundary and one second beyond three days. Numeric partitions include zero, maximum, out-of-range raw marks, a penalty larger than the earned mark and fractional rounding. Submission tests cover empty files, one byte, the 10 MiB limit and one byte over it. Negative identity tests use other students, unassigned lecturers and another department's HoD. They assert denial and unchanged state, which is stronger than checking only whether a visible menu hides a command.

Fault-injection tests deliberately interrupt assessment persistence at multiple write stages. The assertions verify that current marks, phase, revision and history remain mutually consistent after rollback. Other tests reject ordinary edits to immutable versions and audit events, duplicate relationship keys, invalid files, corrupt databases and future schema versions. Backup tests reopen a consistent copy and compare work and history. These checks establish the exception guarantee under injected SQL failures; they are not a physical power-loss experiment. A real shutdown/recovery drill remains part of target deployment, where filesystem and hardware behavior also matter.

## 8.2 Black-box and acceptance distinction
The system script starts independent executable processes using a temporary synthetic school. It provisions four identities, changes their temporary credentials, creates an assignment, receives work, waits past a real deadline, marks, moderates and releases. It checks that results remain hidden before release and become hidden again after reopening. Invalid commands and malformed numbers are followed by another valid command to show that the interface remains usable. The v1.1 scenario additionally creates a class extension and verifies the new deadline and reason in the student's assignment list. All eight checks passed against the optimized release executable.

These are executed developer system tests, not evidence that four school representatives accepted the product. Formal acceptance should give each role three core tasks after at most fifteen minutes of orientation, record independent success and errors, and check the ninety-percent target. The teacher should mark and correct, the HoD should moderate and release, the student should submit and retrieve feedback, and the administrator should configure and restore. The observer must record assistance rather than silently completing failed tasks. NFR08 and final school acceptance remain open because no genuine participant session occurred in this workspace.

The reference environment is recorded in evidence/environment.json: Linux, GCC 13.3, SQLite 3.50.4, libsodium 1.0.20 and the supplied Catch2 version. The machine exposes an AMD EPYC host with nine virtual CPUs. This is not the proposed older school PC, and measurements must not be relabelled as school performance. Runtime operation needs no network calls; vendored dependencies support offline builds once the compiler tools exist. The compact hosted source distribution restores identical pinned dependency bytes with checksums, whereas the submission archive already contains them. First-time build provisioning and offline application operation are separate claims.

## 8.3 Performance and capacity
The optimized benchmark constructs 800 students, three staff accounts, forty courses, eight hundred assignments, sixteen thousand submissions and forty-eight thousand audit events. Each attachment is only 128 bytes, and each measured operation runs one hundred sequential trials against a warm database. The ninety-fifth percentile is the ninety-fifth sorted sample. It includes actual Argon2id verification for login; hashing is not disabled to improve the result. The synthetic hash reuse accelerates fixture creation, not the timed verification operation. Cold startup, maximum-size attachment storage and simultaneous sessions are not measured by this experiment.

| v1.1 operation | Local p95 | Target | Interpretation |
|---|---|---|---|
| Login | 62.0424 ms | <=2000 ms | Pass on measured host |
| Assignment list | 0.887098 ms | <=1000 ms | Pass on measured host |
| Released result list | 0.607813 ms | <=1000 ms | Pass on measured host |
| Database inspection | integrity_check=ok; no FK violations | Consistent fixture | Passed after benchmark |

The first v1.0 large fixture created under the shared workspace later failed integrity inspection. Its root cause was not established. Repeating the experiment on a fresh local temporary filesystem produced the expected records and passed inspection; v1.1 also passed. The response was to add startup integrity checking, retain the incident in the review record and keep deployment on verified local storage. It would be misleading either to call this a proven application defect or to omit it because a later run passed. The residual risk requires a target-machine backup and recovery exercise before real records are entrusted to the system.

## 8.4 Defects, review findings and residual risks
| ID | Actual finding and severity | Action / retest | Status |
|---|---|---|---|
| D01 | Compiler indentation warning; low | Formatted conditionals, rebuilt without warning | Closed locally |
| D02 | cppcheck shadowing, const and constructor dispatch findings; medium | Renamed local, used const/algorithms and nonvirtual initialization; analysis rerun | Closed locally |
| D03 | Assignment projection omitted current receipt classification; medium | Added receipt metadata and extension assertions before v1.0 | Closed locally |
| D04 | Shared-workspace benchmark integrity failure; high operational concern | Fresh local experiment passed; quick_check added; no proven cause | Mitigated; school filesystem drill open |
| D05 | Hosted upload approval block; delivery constraint | Retained full offline archive and Git bundle; exact destination documented | Awaiting explicit upload approval |

No application test failure was observed in the first complete test run. The log therefore records actual review findings instead of inventing a red-to-green history. Each closed issue links to the source and evidence present in the v1.0 implementation commit; CR1 has its own later commit and regression evidence. Developer risk reassessment reduces unauthorized-release exposure after adversarial tests but cannot eliminate it. Hardware compatibility, usability and operational recovery retain significant residual exposure because local automation does not test their actual deployment conditions.

# 9. Configuration management and quality assurance
The local repository uses main as its reviewed release branch and short-lived feature branches for core implementation and CR1. The design baseline was committed first, the implemented system and measured evidence followed, and a merge established v1.0 before the class-extension change. This is genuine local history from the current development sessions, not an artificially backdated seven-week timeline. The final Git bundle preserves branches and annotated local tags, allowing an assessor to inspect exactly what changed between releases without depending on an available hosted service. The source archive supplies a convenient checkout, while the bundle supplies provenance.

The workflow in .github/workflows/ci.yml checks out source, configures Python, installs build and analysis tools, verifies pinned dependencies, configures CMake, builds, runs CTest, enforces seventy-percent business line coverage, executes cppcheck and runs the console system test. It uploads evidence even when a step fails. The Python environment is explicit to avoid relying on an operating-system-managed pip environment. At report assembly the workflow exists but has not run on GitHub, because automatic approval review rejected the full source upload pending specific user authorization. The repository link is real; a successful Actions run link is intentionally absent.

The hosted distribution is compact: it keeps first-party code, schema, test scripts, checksums and concise evidence, with a checksum-verifying dependency restoration script. The full submission archive includes unchanged upstream SQLite, Catch2 and libsodium source bytes for offline reproduction. This packaging distinction does not alter application behavior. Every dependency is pinned and its license is retained. Future security updates require a controlled version change and regression run rather than silently downloading a moving latest release. A compromised or unexpected download fails the SHA-256 check before it is accepted as a build input.

| Metric, v1.1 first-party source | Result | Review implication |
|---|---|---|
| Non-comment source lines, lizard | 1751 | Excludes vendored dependencies and tests |
| Functions | 144 | Mean CCN 2.7 |
| Application Services.cpp | 623 NLOC; 50 functions; mean CCN 2.5 | Business responsibilities remain testable |
| Console learningCommand | CCN 34 | Main maintenance hotspot |
| Console main | CCN 23 | Initialization/dispatch complexity warrants future extraction |

Metrics guided self-review rather than acting as a substitute for it. The two console functions exceed the chosen complexity warning threshold, principally because they dispatch many explicit commands. They remain an acknowledged maintenance concern, while the policy and transaction logic lives in smaller service functions. A future command-table refactor should retain the black-box transcript tests so that prompt order and recovery behavior remain stable. Refactoring immediately before release without a functional need would add risk without improving the demonstrated business requirements. No unresolved configured cppcheck warning remains, but this does not mean the console complexity has disappeared.

The coding conventions emphasize const-correctness, RAII, explicit lifetimes, bounded inputs and one error boundary around each console operation. Repository methods bind parameters, the UI owns no SQL and services own no terminal prompts. The review record lists the changes actually made and identifies the absence of a second reviewer. A self-reviewed individual project can still supply inspectable evidence, but it must not describe its own review as an independent audit. The assessor can reproduce the commands and inspect the tests' expected outcomes rather than relying solely on a statement that the suite passed.

# 10. Deployment, operation and release
[[figure:D08_deployment]]
The release targets one supervised workstation using local SQLite storage. The Linux build has been executed; Windows MSYS2 UCRT64 instructions are supplied because conditional terminal and time code exists, but no Windows run is claimed. The install procedure begins with a C++17 compiler, CMake and libsodium, or the supplied offline dependency build script. CMake builds the executable and tests, CTest validates the checkout, and the technician starts the program with a chosen database path and --init exactly once. This prompts for the first administrator's identity and a private password. There are no shipped default credentials or real school records.

Program files and data should be separate so replacing an executable does not replace the school database. The Linux executable sets a restrictive creation mask; the custodian must still verify directory ownership, disk encryption and backup access. SQLite does not encrypt the database in this implementation. One active console session and a local file avoid assuming unsupported network-filesystem locking or multi-user service availability. A shared-machine queue may be an operational limitation even when query performance is excellent. If simultaneous classroom access is required, it is a new architectural requirement rather than an option to place the same file on a network share.

The role manual matches the implemented prompts. An administrator creates departments and identities, assigns the HoD and lecturer, enrols students and manages backups. A lecturer creates assignments, grants extensions, exports work and enters marks with feedback and a reason. The HoD reviews the selected course and assignment, moderates the current revision, refreshes the work list and releases that revision. A student lists assignments, submits a local file, records the receipt and views only released results. Commands and responses are entered on separate lines. The manual explains temporary-password changes, stale revisions, expired sessions, invalid input and missing success receipts.

Backup uses SQLite's consistent backup API and rejects an existing destination. Restoration is deliberately an explicit offline technician procedure: stop all instances, preserve the current database separately, copy a known-good backup to a new path, and open that path with the appropriate release. Validate a known login, exported attachment, released result and audit trail, not merely the absence of a startup error. The application has a tested backup/reopen integration scenario, but no full reference-dataset thirty-minute recovery drill or daily operational backup history is claimed. These are acceptance activities for the designated school custodian.

For upgrade, preserve the v1.0 database and executable before starting v1.1. The repository constructor applies schema migration 002 transactionally and sets the new schema version only with a successful migration. A failed migration leaves the prior schema intact. Downgrade is not an in-place deletion of the new table: restore the pre-upgrade backup with the matching v1.0 executable, explicitly accepting the loss of work entered after that backup. The v1.0 executable rejects a future schema version, protecting users from accidentally treating newer data as an older format. Release notes list this compatibility boundary and the remaining operational limitations.

# 11. Maintenance change CR1
CR1 is perfective maintenance: it adds a class-wide deadline extension and clearer student deadline information beyond the original individual-extension workflow. It was implemented after the actual local v1.0 tag, so the delivered history can demonstrate a baseline, impact analysis, changed source and regression result. The new ClassExtension record stores assignment, deadline, grantor, timestamp and reason. Migration 002 adds one relation keyed by assignment_id; it does not overwrite the original assignment deadline or receipt snapshots. Administrative history records the change for accountability. This approach preserves the distinction between an original requirement and its later adjustment.

FR22 requires an assigned lecturer to grant a later deadline for the entire enrolled class. FR23 requires the assignment list to show the base deadline, any class and individual extension, the effective deadline and the applicable reasons. The effective deadline is max(base, class, individual), so class-level relief never removes a later personal accommodation. The extension applies to later enrolments as well as the roster at grant time, because it is attached to the assignment rather than copied into existing student rows. This avoids partial updates and makes future list behavior predictable without a mass rewrite of individual extensions.

The class command rejects shorter or equal requests, missing reasons and unauthorized lecturers. It is also rejected once any assessment exists for the assignment, including a result later reopened for correction. That freeze prevents a broad deadline change from silently recalculating already assessed marks. Existing received work can become on-time under the new deadline while retaining its original receipt snapshot. Replacement becomes available again only when the submission remains RECEIVED and the effective deadline has not passed. The penalty strategy itself is unchanged: it receives elapsed time against the newly resolved deadline and continues to apply the same numerical policy.

| Impact area | Implemented change | Verification |
|---|---|---|
| SRS / policy | FR22-FR23; max of three deadlines; pre-assessment freeze | CR01-CR07 |
| Domain / services | ClassExtension, AssignmentView, extendClass and deadline resolution | Six added Catch2 cases |
| Persistence | class_extensions; additive schema 1-to-2 migration | CR08 data/history preservation |
| Interface | extend-class; visible class date and reason | SYS08 black-box check |
| Design / manual | Updated use cases, records, ERD, sequence and role guidance | Report/source consistency review |
| Release | v1.1 after v1.0; backup and downgrade instructions | Bundle, tags and release notes |

The regression suite retains all twenty-four baseline cases, adds six CR1 cases with thirty-eight assertions and adds one atomicity case with twenty-one assertions. Together they test class propagation, later enrolment, precedence of individual accommodation, invalid scope, rejected shorter deadlines, freezing after assessment, receipt reclassification, reopened resubmission eligibility and additive migration. The migration case reconstructs the version-one schema by removing only the added class relation and setting the old schema marker, then verifies surviving data on reopen. It is a controlled schema compatibility test, not a claim that a real school's production database was upgraded. All thirty-one cases and the new student-display system check passed.

The change demonstrates why deadline resolution and the penalty calculation were separated. CR1 modifies data retrieval and effective-date selection without changing the numerical strategy, moderation state machine or student-result query. The main risks were inconsistent displays, shortening individual accommodations and retrospective mark changes. Each risk has a direct assertion or guard rather than a general statement that regression testing was performed. Future extensions such as retrospective departmental changes would need a new policy for existing assessments and explicit audit/reapproval behavior. They should not bypass the present freeze by editing SQL manually.

# 12. Professional issues and evaluation
Learner identities, work, marks and feedback are personal information. The Data Protection Act is the primary Lesotho source consulted in the design baseline; the school must confirm applicable current obligations and its own retention and access policy before real deployment. The software contributes data minimisation, scoped access, account attribution and correction history, but those controls do not establish legal compliance by themselves. Password hashes and backup files remain sensitive. Only the custodian should hold operational copies, and training must explain that possession of the database bypasses the application's role boundary.

A consistent late penalty can still disadvantage students affected by illness, disability or unreliable transport. Individual and class extensions provide controlled accommodation, while visible deadline provenance lets a student understand which rule applies. Reasons should be concise and avoid unnecessary medical details, especially when a class-wide reason becomes visible to every enrollee. A human appeal process remains necessary. Reopening hides the old result while preserving its history, enabling correction and renewed approval without pretending the previous publication never happened. The system records accountable decisions; it does not replace professional educational judgement.

The console supports keyboard operation and ordinary text errors without color-only meaning. It avoids automatic execution of submitted documents and provides written feedback as readable text. These design decisions support accessibility, but they do not demonstrate universal assistive-technology compatibility. The target terminal, screen reader, font size and user's experience all affect usability. The proposed role-based acceptance exercise should include relevant accessibility needs and record failures as change requests. A fast program with confusing prompts does not satisfy the school's operational need merely because its code and tests are correct.

Professional integrity also applies to this submission. AI assistance was used to develop and review code, tests, diagrams and documentation in the workspace. The student must disclose assistance according to module rules, inspect the work and be able to explain it during the viva. No supervisor signature, external review, invented meeting, hosted CI pass or target-school benchmark is supplied. Remaining actions are explicitly identified: authorize the source upload, verify hosted CI and arrange genuine school acceptance. This preserves a defensible engineering record while delivering the complete local implementation and reproducible evidence.

The main architectural strength is the separation of receipt, assessment, approval and publication, reinforced by transactional history. The chief deployment limitation is the single-workstation trust model, followed by untested school usability and storage recovery. CR1 provides evidence that a scoped requirement change can reuse the policy and persistence structure without destabilizing earlier behavior. The next iteration should prioritize the actual school's feedback and recovery exercise, then address console complexity if it hinders maintenance. The project is assessed through traceable behavior and measured results, not through a promised grade.

# 13. References
Sources are identified by author and title. The supplied brief and dependency provenance are preserved in the companion materials.

Kingdom of Lesotho *Data Protection Act*. Primary legislative copy hosted by the Central Bank of Lesotho. https://centralbank.org.ls/wp-content/uploads/Data_Protection_Act_2011.pdf

libsodium (n.d.) *The pwhash API*. https://doc.libsodium.org/password_hashing/default_phf

National University of Lesotho *CS4433 Software Engineering - Individual Assignment*. Supplied six-page assignment brief. Submission arrangements require confirmation.

Object Management Group *Unified Modeling Language, version 2.5.1*. https://www.omg.org/spec/UML/2.5.1/About-UML

SQLite (n.d.-a) *SQLite Foreign Key Support*. https://www.sqlite.org/foreignkeys.html

SQLite (n.d.-b) *Appropriate Uses For SQLite*. https://www.sqlite.org/whentouse.html

SQLite (n.d.-c) *Transaction*. https://www.sqlite.org/lang_transaction.html

SQLite (n.d.-d) *Online Backup API*. https://www.sqlite.org/backup.html

# Appendix A. Requirements traceability
Use-case keys: UC01 session; UC02 administration; UC03 assignment; UC04 extension; UC05 list; UC06 submit; UC07 mark; UC08 moderate; UC09 release/reopen; UC10 results/history. Test names below are actual families in tests/test_lms.cpp. Services means src/application/Services.cpp; DAL means src/infrastructure/SqliteRepository.cpp; Domain means src/domain/Domain.cpp.

| Requirement | Use case / design | Actual source / tables | Executed evidence |
|---|---|---|---|
| FR01 | UC01; AuthService | Services; users | AT01; AT03 reset; SYS01 |
| FR02 | All; User and scope guards | Services; role mappings | AT02/IT03; UT07; AT13 |
| FR03 | UC02; AdminService | Services; users/admin_events | AT03 account, reset, deactivation |
| FR04-FR05 | UC02; relationship records | Services/DAL; courses/enrolments | AT04/AT05; IT06; SYS01 |
| FR06 | UC03; Assignment | Services; assignments | AT06; SYS02 |
| FR07-FR08 | UC04-05; AssignmentView | Services; individual_extensions | AT07; ownership IT03 |
| FR09-FR10 | UC06; D04-D06a | Services/DAL; submissions/versions | AT09/AT10; IT05; SYS02/SYS05 |
| FR11 | UC07; ILatePenalty | Domain | UT01/UT02; AT09 late-zero |
| FR12 | UC07; Assessment | Services; assessments/events | AT12; IT02; SYS03 |
| FR13-FR14 | UC08-09; D06b | Services; scope/phase/revision | AT13/AT14; SYS03/SYS06 |
| FR15 | UC10; RELEASED query | Services/DAL; results join | AT15; SYS02-SYS04/SYS06 |
| FR16 | UC07-09; UnitOfWork | Services/DAL; assessment_events | IT02; IT06; history assertions |
| FR17 | All; DAL and safe loop | DAL/UI; all tables | IT01/IT07; SYS05/SYS07 |
| FR18 | UC09; reopen transition | Services; assessment/event | AT18; SYS06 |
| FR19 | UC02; backup / offline restore | DAL; SQLite backup API | IT01/AT19; full school drill pending |
| FR20 | Optional result CSV | Deferred | Not claimed |
| FR21 | Excluded interfaces | Scope only | Scope review |
| FR22-FR23 | UC04-05; ClassExtension | Services/DAL/UI; class_extensions | CR01-CR08; SYS08 |

# Appendix B. Quality target disposition
| Target | Evidence / disposition |
|---|---|
| NFR01-NFR02 speed | Local-host warm workload passes; school-target benchmark pending |
| NFR03 offline | No runtime network dependency; offline dependencies included |
| NFR04 credentials | Argon2id, salt and reset tests pass; synthetic fixture passwords are not real credentials |
| NFR05 atomicity | Multi-stage injected failures roll back mark, phase and event |
| NFR06 coverage | 647/654 business lines, 98.9%; exceeds 70% gate |
| NFR07 build/analysis | Linux CMake build and configured cppcheck pass; Windows unexecuted |
| NFR08 usability | Real four-role user study pending |
| NFR09 accessibility | Keyboard/text design implemented; target assistive-technology trial pending |
| NFR10 sessions | Persisted lockout and inactivity tests pass |
| NFR11 recovery | Consistent backup/reopen passes; timed full-dataset school drill pending |
| NFR12 recovery from input | Negative suite and black-box continuation checks pass |

# Appendix C. Risk updates and acceptance handover
These are developer reassessments following implementation and CR1, not claimed supervisor gate scores. Initial risk IDs refer to the planning register. Scores remain qualitative likelihood times impact.

| Risk | Initial | After implementation | After CR1 / remaining action |
|---|---|---|---|
| R01 dependency/target compatibility | 12 | 8 | 8; local Linux proven, school/Windows check pending |
| R02 approval delay | 15 | 5 | 5; user reported design approval; hosted upload approval outstanding |
| R03 release bypass | 15 | 5 | 5; adversarial scope/revision tests retained |
| R04 data loss | 15 | 10 | 10; storage incident mitigated; operational drill pending |
| R05 CLI usability | 9 | 9 | 9; command manual and real user trial needed |
| R06 password sharing / unattended session | 12 | 8 | 8; timeout tested; user behavior still requires training |
| R07 disputed time / extension policy | 16 | 8 | 8; exact boundaries tested; appeal policy needs school ownership |
| R08 excessive feature scope | 16 | 8 | 8; CR1 isolated; optional export deferred |
| R09 changed workstation clock | 10 | 10 | 10; backwards session guard; OS clock remains trusted |
| R10 single-terminal queues | 15 | 15 | 15; no school workload trial; validate before deployment |

Before school use, the responsible reviewer records their identity, runs the role tasks and recovery drill, resolves remaining conditions and signs acceptance outside this developer report. Before academic hand-in, authorize the private repository source upload and give the assessor repository access. Then record the actual passing Actions run URL. The bundle and evidence permit local review even while those external steps remain open.

# Appendix D. Viva route and evidence index
| Minutes | Demonstration | Explain / evidence |
|---|---|---|
| 0-2 | Scope, four roles and architecture | One workstation; resource scope beyond menus |
| 2-5 | Student submit / receipt / denied other work | Immutable versions and exact deadline |
| 5-9 | Lecturer mark, HoD moderate and release | Integer deduction; separate revision-bound approval |
| 9-11 | Reopen and history | Hidden result; old/new values and atomicity |
| 11-14 | v1.1 class extension | max(base,class,individual); freeze after first assessment |
| 14-17 | CTest, coverage and local tags | Real logs; 31 cases; scope of coverage |
| 17-20 | Trade-offs and questions | Backup, OS trust, CLI complexity and genuine outstanding acceptance |

Project README contains exact build commands; docs/USER_MANUAL.md contains role prompts and recovery; docs/REVIEW_RECORD.md records actual engineering findings; docs/RELEASE_NOTES.md describes both releases. evidence/v1.0-* and evidence/v1.1-* contain the raw outputs. migrations contains executable SQL. The diagram portfolio includes editable PlantUML sources and rendered PNG/SVG sheets. The final package supplies a source checkout, local Git bundle, Linux release binaries, this report and its editable Markdown.
