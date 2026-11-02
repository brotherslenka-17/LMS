# MODERATED LEARNING MANAGEMENT SYSTEM
## Planning, requirements and software design
National University of Lesotho | Department of Mathematics and Computer Science

CS4433 Software Engineering - Individual Final Project

**Document:** GR-01, revision 0.1

**Status:** PROPOSED BASELINE - GATE 1 AND GATE 2 REVIEW REQUIRED

**Student name:** Lenka RS | **Student number:** 202321267

This package addresses the work that may precede implementation: project planning, requirements engineering and system design. It includes prospective verification, deployment and maintenance designs to demonstrate traceability. It is not the final 6,000-8,000-word submission, a working application, an approved design, or evidence that tests or reviews have occurred. The assignment explicitly prohibits implementation before Gate 2. The student confirmed that Gate 2 has not yet been approved. All school policy resolutions below are proposals requiring review, not additional interviews or agreements that actually took place.

## Abstract
The proposed LMS supports administrator, lecturer, Head of Department and student roles on an offline school workstation. Its central control is a separation between marking, moderation and release: a saved mark is not automatically a visible result. A layered C++17 design uses SQLite for persistent data, a repository abstraction for transactions, and a replaceable late-penalty strategy. Submission versions preserve earlier work; assessment events record actor, time, old and new values, and reasons. Requirements are measurable and connect to prospective source modules and tests. Particular attention is given to ambiguous deadline boundaries, the meaning of a late day, resubmission eligibility, extensions, stale approvals and the distinction between 800 registered learners and 800 concurrent sessions. This review package provides a seven-week dependency schedule, risk register, specifications, UML models and a normalised draft schema. The remaining phases require genuine implementation, measured verification results, supervisor reviews and repository evidence.

## Reading guide
1. Introduction and baseline decisions
2. Project management plan
3. Requirements engineering and business rules
4. Fully dressed use cases
5. Software design and UML
6. Database design and dictionary
7. Prospective verification and traceability
8. Configuration, deployment and maintenance plans
9. Professional issues and review decisions
10. References and appendices

# 1. Introduction and baseline decisions
The case study concerns a secondary school whose assessment records must remain private until departmental checking is complete. Four roles share one application, but their authority differs both by action and by resource. A lecturer assigned to one course must not mark another course merely because both users are lecturers. Similarly, a Head of Department may moderate only courses belonging to their department. These scope restrictions are part of the business model and must be checked by services, even when the console menu hides unavailable commands.

The system includes user and course administration, enrolment, assignment creation, individual extensions, versioned submissions, marking, written feedback, moderation, release and assessment history. Payments, video conferencing, automated grading, plagiarism detection and web or mobile interfaces remain outside the assignment scope. The proposed delivery is a console program. This limits installation work and provides a testable interface for older hardware; it does not establish usability automatically. A task-based user trial remains necessary.

The review baseline interprets the capacity requirement as 800 stored student accounts, not simultaneous interactive users. The wording in the brief is ambiguous, so this interpretation needs explicit approval at Gate 1. One active console session on a supervised school workstation is proposed. This fits the permitted single-computer deployment but may create an operational bottleneck at submission time. If the school requires many concurrent sessions, the architecture and schedule must be reopened before Gate 2. A shared SQLite database file on a network drive is not the proposed solution; SQLite's own deployment guidance distinguishes local embedded use from workloads needing a server database (SQLite, n.d.-b).

The schedule uses relative working days. Confirm submission and viva arrangements with the supervisor. Do not infer gate approval from a proposed schedule. The supplied stakeholder statements are the only elicitation evidence presently available; additional interviews, observations and usability trials are future activities.

# 2. Project management plan
## 2.1 Feasibility assessment
| Dimension | Assessment and evidence needed | Recommendation |
|---|---|---|
| Technical | C++17, CMake, SQLite, libsodium, Catch2 and cppcheck are a coherent candidate toolchain. A workstation build and hashing benchmark have not yet been performed. | Feasible subject to a dependency and hardware check after approval. |
| Operational | Four role menus can match familiar school responsibilities. Shared-machine queues, training and backup ownership remain unresolved. | Obtain principal and IT acceptance of the single-session workflow. |
| Economic | Reuse of an existing workstation avoids assuming new hardware. Development labour, training, removable storage and support still have costs. | Use a transparent estimate; obtain quotations before procurement. |
| Legal | Learner identities and marks are personal information. The Lesotho Data Protection Act is a relevant starting point for a Lesotho school (Kingdom of Lesotho). | School must confirm lawful handling, access and retention policy before real data use. |
| Schedule | Seven weeks provide 35 working days. Design approval is a hard predecessor of implementation, and the baseline has no task float. | Feasible with tightly controlled scope and prompt reviews; approval delays threaten completion. |

The provisional effort budget is 175 hours: planning 15, requirements 25, design 35, implementation 40, verification 25, deployment 10, maintenance 15 and final synthesis 10. At an illustrative planning rate of M100 per hour, labour is M17,500. This is a scenario estimate, not a quoted price or a student fee. Hardware cost is assumed to be zero only if the existing computer passes the agreed environment check. An additional M1,000 planning allowance covers storage and incidentals, with M1,850 contingency on the M18,500 subtotal, giving M20,350. Replace allowances with actual quotations where expenditure is required.

The project is recommended conditionally, rather than declared unconditionally feasible. A console system reduces implementation scope but increases the importance of a usable workflow. A local database removes internet dependence but transfers operational responsibility to backup discipline. These trade-offs are acceptable only if the school accepts the deployment workload and identifies a technician responsible for recovery.

## 2.2 Work breakdown, dependencies and critical path
| WBS | Deliverable and work package | Days | Predecessor | Milestone / exit |
|---|---|---|---|---|
| A | Feasibility, estimates, WBS, risks | 1-3 | None | Management baseline |
| B | Stakeholders, SRS, business rules, use cases | 4-10 | A | Gate 1 at day 10 |
| C | Architecture, UML, schema, design review | 11-15 | B and Gate 1 | Gate 2 at day 15 |
| D | Implement approved Must scope and developer tests | 16-23 | C and Gate 2 | Feature-complete candidate |
| E | Formal integration, system, acceptance, coverage and fixes | 24-28 | D | Gate 3 at day 28 |
| F | Technician installation, manuals, v1.0 tag | 29-30 | E and Gate 3 | Release v1.0 |
| G | CR1 analysis, change, regression, v1.1 tag | 31-33 | F | Release v1.1 |
| H | Final report consistency, evidence index and viva practice | 34-35 | G | Final submission |

[[figure:D09_gantt]]

The dependency network is A -> B -> C -> D -> E -> F -> G -> H. Durations are 3 + 7 + 5 + 8 + 5 + 2 + 3 + 2 = 35 working days. Earliest and latest starts coincide, so every listed package has zero total float and lies on the critical path. This is deliberately transparent: the plan does not disguise contingency as free time. Formal V&V follows implementation, while unit-test design occurs during design and developer tests accompany implementation. Requirements/design traceability and the report are maintained throughout their respective packages; H consolidates the existing record.

At each gate, record reviewer, issues, decision and conditions. A conditional approval is not treated as unrestricted permission: close any condition that prevents the next phase. If a gate slips, rebaseline the schedule and negotiate optional scope. Never remove moderation, password protection, audit integrity or the mandated tests to recover time. The preferred reductions are cosmetic console features and optional export capability.

## 2.3 Risk register
Likelihood and impact use 1 (low) to 5 (high); exposure is their product. The owner is the student developer unless a named school role is indicated. Values below are initial assessments, not measured probabilities. Gate 1, Gate 2 and Gate 3 residual exposures must be recorded after real reviews.

| ID | Category and risk | L | I | L x I | Mitigation and trigger | Owner |
|---|---|---|---|---|---|---|
| R01 | Technical: dependencies fail on school PC | 3 | 4 | 12 | Pin versions; reproduce build on target; trigger: unsupported compiler/library. | Developer |
| R02 | Project: Gate 2 approval delayed | 3 | 5 | 15 | Book reviews early; send package in advance; rebaseline on missed gate. | Developer / supervisor |
| R03 | Technical: grade change bypasses moderation | 3 | 5 | 15 | Central transitions; revision-bound approval; adversarial service tests. | Developer |
| R04 | Technical: power failure loses records | 3 | 5 | 15 | Atomic transactions; recovery drill; encrypted daily backup. | IT officer |
| R05 | People: teachers find CLI confusing | 3 | 3 | 9 | Numbered menus; examples; task-based usability trial and training. | Developer / lecturer |
| R06 | People: password sharing or unattended session | 3 | 4 | 12 | Individual accounts; inactivity logout; no shared administrator password. | IT officer |
| R07 | Policy: partial days / extensions disputed | 4 | 4 | 16 | Approve boundary examples at Gate 1; publish rule and appeal route. | Principal / HoD |
| R08 | Project: excessive features consume schedule | 4 | 4 | 16 | Must baseline; change log; defer optional exports and GUI. | Developer |
| R09 | Technical: workstation clock changed | 2 | 5 | 10 | Restrict clock changes; record receipts; flag backwards movement. | IT officer |
| R10 | Operational: one-terminal queues unacceptable | 3 | 5 | 15 | Confirm workload before Gate 1; redesign if concurrent use is essential. | Principal / IT officer |

The gate update form for every risk is: current exposure, evidence since previous review, mitigation completed, residual exposure, owner decision and next review date. No residual ratings are filled in prematurely. R03 remains high until direct-service access tests and mark-history rollback tests pass; a diagram alone does not retire it. R04 remains open until restoration of a real test backup is demonstrated. This connects risk closure to evidence rather than optimism.

# 3. Requirements engineering
## 3.1 Stakeholders and conflicts
| Stakeholder | Interest and decision responsibility | Conflict / clarification | Proposed resolution |
|---|---|---|---|
| Principal | Simple operation; trustworthy school results | Immediate lecturer publication conflicts with checked results. | Marking saves a draft; only HoD release exposes mark and feedback. |
| Lecturer | Efficient marking; discretion over extensions | Individual extensions may conflict with common deadline. | Authorized lecturer may extend for an enrolled student before that student's first assessment. |
| HoD | Department-wide review; consistent late policy | Meaning of a day and 10% is unspecified. | Started 24-hour periods; deduct 10% of maximum per period for up to three periods. |
| Student representative | Resubmission, feedback, late acceptance | Resubmission after deadline and marked work unspecified. | Preserve versions; replace only while unassessed and within effective deadline. Accept a first late submission even after three days. |
| IT officer | Offline use, protected passwords, performance | 800 accounts versus 800 concurrent sessions unspecified. | Propose 800 stored students and one interactive session; request explicit approval. |
| Technician / school custodian | Recovery and privacy | Backup retention, account ownership and physical access unspecified. | Assign a custodian and approve retention/access policies before real deployment. |
| Supervisor / assessor | Waterfall gates, consistent evidence and viva | Implementation must not precede design approval. | Review this package first; retain authentic decisions and later execution evidence. |

The principal's release control takes precedence in the proposed resolution because the brief explicitly assigns checked publication to the HoD. The lecturer's underlying need is addressed through a clear pending-moderation queue, not an unauthorized early preview. Assessment history is readable by the department HoD and assigned lecturer; ordinary administrators manage accounts without a result-view privilege. Students may see a receipt and the status of their submission before release, but not raw marks, adjusted marks or written assessment feedback. Error messages and administrator screens must not become alternate disclosure paths.

A clarification meeting should ask the principal and HoD to approve the boundary table below, ask IT to approve the deployment load, and ask teachers to demonstrate a typical marking task. Record answers and unresolved disagreements. This is planned elicitation, not a claim that these meetings occurred. Any resulting requirement change receives a new version and an updated traceability row before Gate 1 closes.

## 3.2 Business rules and exact calculations
| Rule | Proposed policy |
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

Example: for maximum 80.00, raw 65.00 and 24 hours plus one second late, d=2 and the deduction is 16.00, giving 49.00. At exactly 72 hours the three-day deduction applies; at 72 hours plus one second the result becomes zero. This discontinuity follows the proposed interpretation of the HoD statement and must be approved explicitly. The system retains the work even when its computed mark is zero, so teachers can still give feedback and consider a procedural appeal.

When the maximum contains fractional marks, calculate deduction units using (maximum_units * d + 5) / 10 with integer division, for d between one and three. All arithmetic must use a sufficiently wide integer type. For maximum 10.05 and one day late, the 1.005 deduction rounds to 1.01. This avoids binary floating-point discrepancies across machines. Exactly at the deadline, no rounding operation or penalty is applied.

A no-submission record is not automatically converted to a zero in this baseline. That is a separate school policy requiring a missing-work workflow. An extension granted after a receipt but before assessment changes the current effective deadline; the original deadline_at_receipt remains evidence of what the system told the student at receipt time. The assessment stores the effective deadline actually used for the final calculation. Once assessment exists, the baseline rejects further extensions for that student rather than silently changing a released outcome.

## 3.3 Functional requirements
All rows are normative proposals. M=Must, S=Should, C=Could, W=Won't in this release. Acceptance test identifiers refer to planned tests, not completed runs.

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
| NFR04 | M | Zero plaintext passwords in database, logs and fixtures; all real passwords use libsodium Argon2id encoded hashes with unique salts; demonstrate wrong-password rejection. |
| NFR05 | M | Forced failure before commit leaves either all or none of a mark update, status change and audit event; verify across each mutation step. |
| NFR06 | M | Catch2 tests achieve at least 70% measured line coverage of src/domain and src/application; report numerator, denominator, exclusions and actual result. |
| NFR07 | M | Clean CMake C++17 build and all automated tests pass on the declared target OS; no unresolved high-severity static-analysis findings before release. |
| NFR08 | S | Four representative role users each complete their role's three core tasks after <=15 minutes of orientation; >=90% task success and no unauthorized disclosure. |
| NFR09 | M | Keyboard-only workflows; every error includes readable text; no meaning depends on colour. Verify using a terminal accessibility checklist. |
| NFR10 | M | After five consecutive failed logins, reject login for five minutes; inactivity of 15 minutes requires fresh authentication. Persist failed-attempt lockout state. |
| NFR11 | S | On reference dataset, recover from a daily backup within 30 minutes with <=24 hours of data loss; record restore time and integrity checks. |
| NFR12 | M | Denied access and malformed input do not terminate the application across the planned negative-input suite; logs contain neither passwords nor attachment bytes. |

Performance targets apply to an agreed baseline workstation, proposed as a dual-core 2 GHz CPU, 4 GiB RAM and local storage. These are acceptance conditions, not observed results. A build without security hashing is not a valid performance benchmark. Large attachment volumes are a capacity risk: 16,000 ten-megabyte files would be substantial, so the test dataset must disclose actual attachment sizes and free-disk requirements. The 10 MiB value is a per-file limit, not a promise that unlimited historical versions fit on old hardware.

# 4. Use case model
[[figure:D01_use_cases]]

D01 covers the four roles. Include means a required sub-behaviour; Submit assignment includes Validate submission. Extend means conditional added behaviour; Resubmit extends Submit assignment when previous work exists and the resubmission guards succeed. Role specialisation means each concrete actor inherits the authenticated user's interaction with session management. Authentication is normally a precondition for privileged use cases, rather than a misleading include that would log users in repeatedly for every action. Additional role-generalisation detail appears in D01b.

[[figure:D01b_actor_generalisation]]

## 4.1 UC06 - Submit assignment (fully dressed)
**Scope and level:** LMS, user goal. **Primary actor:** Student. **Supporting participants:** authenticated session, trusted clock and repository. **Trigger:** Student selects Submit on an assignment. **Preconditions:** Account and enrolment are active, assignment exists in an enrolled course, and the user is authenticated. **Stakeholder interests:** student needs a durable receipt, lecturer needs the correct version, and school needs trustworthy timing.

**Main success flow:** (1) Student selects an assignment from their own list. (2) System displays title, effective deadline and existing receipt status. (3) Student selects a non-empty file and confirms submission. (4) Service opens a transaction and rechecks account, enrolment, assignment and latest version. (5) System obtains trusted current time after the file has been read successfully and guards have been checked. (6) For a first submission, system creates the submission aggregate and version 1; lateness does not block receipt. (7) System stores bytes, display filename, receipt time and deadline-at-receipt. (8) System commits and returns a receipt containing assignment, version, timestamp and on-time/late classification.

**Alternatives:** A1, at step 4 previous work exists: require RECEIVED and t<=effective deadline, then append version n+1. A2, first receipt is late: accept and flag the appropriate late-day count, including more than three days. A3, an individual extension applies: use the later deadline and display it. A4, two requests target the same assignment: serialize within the write transaction; re-evaluate replacement eligibility after acquiring the lock.

**Exceptions:** E1 wrong course or inactive enrolment: deny without revealing other students' work. E2 empty, missing or oversized file: explain validation failure and leave old versions untouched. E3 replacement after the deadline or after assessment: reject without inserting a version. E4 disk full, lock timeout or database error: roll back and do not display a success receipt. E5 logout or inactivity expiry before commit: abort authorization and require login. If commit succeeded but the screen failed before receipt display, the student can inspect the durable version list before retrying.

**Success postconditions:** Exactly one new immutable version is stored and attached to the authenticated student's submission. **Minimal guarantee:** Previous committed work is intact and no uncommitted receipt is claimed. **Business rules:** BR02-BR05, BR07-BR09 and BR14. **Special requirements:** NFR03, NFR05 and NFR12. **Test family:** AT09-AT10; unit boundaries UT01; integration IT01 and IT03.

## 4.2 UC07 - Mark submission (fully dressed)
**Primary actor:** Lecturer assigned to the course. **Trigger:** Lecturer selects unmarked work or revises a draft. **Preconditions:** Active assigned lecturer, latest immutable version exists, current time is strictly after the student's effective deadline, and phase is RECEIVED or MARKED. Moderated work must be explicitly returned to MARKED first; released work requires HoD reopening. **Interests:** teacher retains professional judgement over raw marks; the student receives written reasons; the school applies penalties consistently.

**Main success flow:** (1) Lecturer opens the latest submission and sees its receipt information. (2) System presents maximum mark and calculated lateness. (3) Lecturer enters raw mark, written feedback and reason. (4) Service rechecks authority, time, phase and expected row/revision inside the transaction. (5) Penalty strategy calculates the final mark. (6) System saves the assessment for that exact submission version and changes phase to MARKED. (7) In the same transaction it appends an event, with NULL old marks on first marking. (8) Commit succeeds; the lecturer sees confirmation and pending-moderation status. The student sees no assessment value.

**Alternatives:** A1 existing MARKED assessment: increment revision and preserve previous raw/final marks and feedback in history. A2 lateness exceeds three days: final is zero while raw mark and feedback remain recorded. A3 penalty exceeds earned raw mark: clamp final to zero. **Exceptions:** E1 raw mark outside range or over two decimal places; E2 blank feedback or reason; E3 another change invalidates expected revision; E4 database/audit insertion fails; E5 user or course assignment is revoked. Each exception rejects the command and preserves committed values.

**Success postconditions:** One current assessment exists, one history event records the revision, and publication is blocked. **Minimal guarantee:** No unaudited mark change survives. **Rules:** BR02, BR06, BR08, BR10-BR13. **Tests:** AT11-AT12, AT16; UT02; IT02-IT04.

## 4.3 UC08/UC09 - Moderate and release marks (fully dressed)
**Primary actor:** HoD assigned to the course's department. **Trigger:** HoD opens the departmental moderation queue. **Preconditions:** Active HoD; course belongs to an assigned department; assessment is MARKED for moderation or MODERATED for release. **Interests:** principal requires reviewed results, HoD needs complete departmental visibility, and students require a stable published outcome.

**Main success flow:** (1) System lists all marked submissions in the department, including course, assignment and current revision. (2) HoD opens the work, raw mark, deduction, final mark and feedback. (3) HoD enters an approval reason, optionally adjusting raw mark or feedback. (4) Service checks department scope and expected revision inside a transaction. (5) If raw mark changed, recompute final mark with the same approved penalty policy; save all old/new values. (6) Append the moderation event, set MODERATED and bind approval to the new revision. (7) Commit approval. (8) Through a separate Release command, HoD selects the approved current revision. (9) Service rechecks scope, phase and revision, stores release metadata and event, and commits. (10) Student can now read the final mark and written feedback.

**Alternatives:** A1 return for correction: record reason and transition to MARKED, removing approval metadata. A2 correction after release: HoD reopens to MARKED, logs the reason and hides the result until the full approval/release path repeats. A3 multiple submissions: v1.0 processes each as an explicit command; batch release is not assumed. **Exceptions:** E1 wrong department; E2 unmarked or unapproved assessment; E3 stale expected revision; E4 missing reason or out-of-range adjusted raw mark; E5 failure to persist audit. Roll back each failed command in full.

**Success postconditions:** Released result is the current reviewed revision; both approval and release events remain attributable. **Minimal guarantee:** No unrevised approval silently authorizes a changed mark, and failed release leaves visibility unchanged. **Rules:** BR02, BR10-BR13. **Tests:** AT13-AT16, AT18; IT02-IT04. Separating approve and release makes a review hold possible without giving students a preview.

# 5. Software design document
## 5.1 Architecture and alternatives
[[figure:D02_components]]

The layered design separates console parsing, application use cases, domain rules and persistence. The composition root creates the database connection, repository, penalty policy, clock and services. The UI passes typed commands and authenticated context; it does not construct SQL. Services coordinate scope checks, domain calculation and transaction boundaries. Domain objects contain policy without dependencies on console streams or database handles. The SQLite adapter uses bound parameters and translates library error codes to application exceptions.

A monolithic main function was rejected because permission checks and SQL would be scattered among menu branches, making bypass tests and policy changes difficult. A web client/server application would support broader access, but the brief explicitly excludes web interfaces and its operational burden is unnecessary for the proposed single-session workload. A GUI remains a possible console replacement, but building both now would consume time without proving the required learning outcomes. Layering is chosen to make each required behaviour independently reviewable, rather than because more layers are inherently better.

The transaction boundary belongs to the application command. BEGIN IMMEDIATE is proposed for commands that write, followed by fresh authorization and version checks, then mutations and commit. A bounded busy timeout avoids an indefinite frozen interface. A failed statement or exception invokes rollback through an RAII UnitOfWork. SQLite describes transactions and limitations on concurrent writers in its transaction documentation (SQLite, n.d.-c). File reading occurs before opening the write transaction to avoid holding a lock during user interaction; receipt time is then taken at validation, not from a user-supplied file timestamp.

## 5.2 Class design, patterns and SOLID
[[figure:D03a_classes]]
[[figure:D03b_classes]]
[[figure:D03c_classes]]

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

Passwords will use libsodium's encoded password-hash API and verification function. Its documentation describes Argon2id as the default algorithm and stores the required salt and parameters in the encoded result (libsodium, n.d.). The design therefore stores one password_hash string and avoids home-grown cryptography or a fixed shared salt. Select and benchmark a documented parameter setting on the target machine. Password reset issues a temporary credential requiring change; it never recovers the previous password. No actual password implementation or benchmark has occurred in this package.

Attachment bytes are stored in SQLite rather than referenced through student-controlled paths. This supports atomic receipt and avoids a database pointing to a missing file after a filesystem failure. The trade-off is larger databases and backups, so enforce file-size limits and monitor capacity. Display filenames must be stripped of path components and terminal control characters; export, if later implemented, chooses an application-controlled destination. Opening attachments is a teacher-controlled external action, not automatic execution by the LMS.

# 6. Database design
[[figure:D07a_erd]]
[[figure:D07b_erd]]

## 6.1 Normalisation and mapping
The draft schema has thirteen tables. Atomic column values and separate version rows establish first normal form. Composite-key relationships such as enrolments and lecturer_courses hold facts about the whole course-user association, avoiding partial dependencies. Department names are stored only in departments, course titles only in courses, and account details only in users. Assessment records refer to the submission and its immutable version rather than duplicating the student's name or course title. These choices avoid non-key transitive dependencies in the operational relations and support third normal form.

Assessment events intentionally retain old and new values. They describe distinct historical facts at a revision, rather than acting as a second editable source of the current mark. The unique submission/revision pair prevents duplicate historical identities. Receipt-time deadlines and assessment-time effective deadlines are separate historical snapshots with different meanings. They must not be confused with the current base deadline; their duplication is intentional temporal evidence, not an accidental denormalisation used to simplify a join.

Single-table inheritance maps the four User subclasses to one users table with a role discriminator. Four nearly identical user tables were rejected because they would duplicate identity and authentication fields. Many-to-many course links become association tables, even though an object model may expose them as collections. Value objects such as MarkUnits and TimePoint map to integers, while SubmissionPhase maps to checked text. Services and strategies have no tables because they implement behaviour rather than persistent entities.

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
The companion schema is the authoritative column-level definition; the generated Markdown dictionary lists every column, SQL type, nullability, default and key participation. Identifiers are SQLite INTEGER keys represented by a dedicated 64-bit ID type in C++. Times use signed 64-bit epoch seconds. Mark units use signed 64-bit integers in the application with a defined assignment maximum range of 1.00 to 1,000.00. Name, feedback and file limits are explicit in the DDL. Foreign keys use restrictive deletion by default, so deactivation preserves assessment attribution.

Database constraints enforce identities, basic ranges, unique enrolments, valid phase labels and version references. Foreign-key enforcement must be enabled and checked on every connection, as the SQLite documentation explains (SQLite, n.d.-a). Append-only triggers block ordinary updates/deletes of events and submission versions. These controls do not make the database tamper-proof against an operating-system owner who can replace the file or change its schema. Restricted filesystem permissions and backups remain necessary.

Cross-table policy checks are service responsibilities: raw<=assignment maximum; lecturer and HoD scope; student role and active enrolment; extension later than applicable deadline; phase transitions; and exact parity between current assessment revision and its event. Current assessment, event and phase must be saved in one transaction. The schema alone is not represented as enforcing all application invariants. Planned integration tests must deliberately fail the event insert to prove that no mark update remains committed.

# 7. Prospective verification and traceability
## 7.1 V-Model strategy
| Level | Artefact verified / validated | Entry criterion | Exit criterion and evidence |
|---|---|---|---|
| Unit | Detailed domain design and policy formulas | Approved design; isolated domain code builds | Catch2 policy, state and rounding tests pass; recorded business-layer line coverage >=70%. |
| Integration | Architecture, repository contracts and schema | Unit suite green; clean temporary databases available | Persistence, authorization queries, transaction failure and audit tests pass. |
| System | SRS and fully dressed use cases | Integrated candidate builds; repeatable dataset and scripts | All Must requirements and exception flows pass; no unresolved critical/high defects. |
| Acceptance | School needs and agreed conflict resolutions | System suite green; school/supervisor-approved scenarios | Real reviewer records outcomes, usability observations and approval or outstanding conditions. |

The acceptance reviewer must not be invented. Until a real review occurs, a developer walkthrough is system/self testing, not stakeholder acceptance. CI success is also not a substitute for acceptance: it proves only the executed automated checks on the declared environment. Coverage is a structural measure; even 100% does not prove correct authorization unless tests assert forbidden outcomes.

## 7.2 Boundary and equivalence design
| Test ID | Input / partition | Expected result |
|---|---|---|
| UT01a | Receipt at D-1 second and at D | On time; no deduction |
| UT01b | D+1 second and D+86,400 seconds | One late day; 10% maximum deduction |
| UT01c | D+86,401 and D+172,800 | Two late days; 20% maximum deduction |
| UT01d | D+172,801 and D+259,200 | Three late days; 30% maximum deduction |
| UT01e | D+259,201 | Final zero, first work still accepted |
| UT02a | Max 80.00, raw 65.00, two days | Final 49.00 |
| UT02b | Max 80.00, raw 5.00, one day | Final 0.00, never negative |
| UT02c | Max 10.05, raw 10.05, one day | Deduction 1.01; final 9.04 |
| UT02d | Raw -0.01, 0, max, max+0.01 | Reject, accept, accept, reject |
| UT03 | Extension D2>D; receipt between D and D2 | On time under effective deadline |
| UT04 | Resubmit at D; at D+1; after marking | Accept; reject; reject; preserve prior bytes |
| IT01 | Restart after committed receipt | Same version and identical bytes recovered |
| IT02 | Force failure inserting assessment event | No mark, phase or revision change committed |
| IT03 | Second student / wrong lecturer / other HoD | Denied with no mutation or assessment disclosure |
| IT04 | Approve revision r, edit, release old r | Stale/unapproved release rejected |
| IT05 | File sizes 0, 1, 10 MiB, 10 MiB+1 | Reject, accept, accept, reject |
| IT06 | Invalid FK / duplicate enrolment / history edit | Database constraint rejects invalid change |

Equivalence partitions also include active versus disabled users, enrolled versus unenrolled students, own versus other department, approved versus unapproved results, valid versus blank feedback, and connected versus disconnected networking. Tests use FakeClock instead of waiting real days or rewriting system time. Error-injection tests need controlled failure points rather than random crashes. A restore drill must verify data contents and access controls, not merely whether a database file can be opened.

## 7.3 Requirements traceability matrix
The following paths are planned implementation targets. They do not exist as completed application source in this package. The Markdown companion provides editable rows. UC01=session; UC02=school administration; UC03=assignment; UC04=extension; UC05=list; UC06=submit; UC07=mark; UC08=moderate; UC09=release/reopen; UC10=results/history.

| FR | Use case | Design / tables | Planned source | Planned tests |
|---|---|---|---|---|
| FR01 | UC01 | AuthService, User; users | application/AuthService.cpp | AT01, IT03 |
| FR02 | UC01-10 | Session, all services; role mappings | application/Authorization.cpp | AT02, IT03 |
| FR03 | UC02 | AdminService; users, admin_events | application/AdminService.cpp | AT03 |
| FR04 | UC02 | Course/Department; course and role mappings | application/AdminService.cpp | AT04, IT06 |
| FR05 | UC02 | Enrolment; enrolments | application/AdminService.cpp | AT05, IT06 |
| FR06 | UC03 | Assignment; assignments | application/AssessmentService.cpp | AT06 |
| FR07 | UC04 | IndividualExtension; individual_extensions | application/AssessmentService.cpp | AT07, UT03 |
| FR08 | UC05 | Assignment; courses, enrolments, extensions | application/AssessmentService.cpp | AT08 |
| FR09 | UC06 | Submission, D05/D06a; submissions, versions | application/AssessmentService.cpp | AT09, IT01, IT05 |
| FR10 | UC06 | SubmissionVersion, D04/D06a; versions | application/AssessmentService.cpp | AT10, UT04 |
| FR11 | UC07 | ILatePenalty / TenPercentPenalty | domain/TenPercentPenalty.cpp | AT11, UT01-02 |
| FR12 | UC07 | Assessment, D04; assessments | application/AssessmentService.cpp | AT12, UT02, IT02 |
| FR13 | UC08 | Assessment, D06b; assessments/events | application/AssessmentService.cpp | AT13, IT03-04 |
| FR14 | UC09 | Assessment, D04/D06b; assessments/events | application/AssessmentService.cpp | AT14, IT04 |
| FR15 | UC10 | Release visibility; assessments/submissions | application/AssessmentService.cpp | AT15, IT03 |
| FR16 | UC07-09 | AssessmentEvent, UnitOfWork; assessment_events | infrastructure/SqliteRepository.cpp | AT16, IT02, IT06 |
| FR17 | All | Repository, UnitOfWork; all persistent tables | infrastructure/SqliteRepository.cpp | AT17, IT01-02 |
| FR18 | UC09 | Reopen transition D04; assessments/events | application/AssessmentService.cpp | AT18, IT04 |
| FR19 | UC02 | Maintenance workflow, D08 | infrastructure/BackupService.cpp | AT19 (Should) |
| FR20 | UC10 | Authorized export adapter | application/ExportService.cpp | AT20 (Could) |
| FR21 | Scope | Explicit exclusions | None | Gate scope review |

Actual execution logs will include test version, command, date, environment, expected and actual results, status and defect reference. The defect log columns are defect ID, triggering test, severity, reproduction steps, root cause, fix commit, retest command and outcome. There are presently no application test results or application defects because implementation is not authorized yet. Draft-schema structural checks are separately described in the package QA record and must not be reported as application integration tests.

# 8. Configuration, deployment and maintenance plans
## 8.1 Repository and quality assurance
After approval, create a hosted repository with main as the releasable branch and short-lived feature branches named for requirements. Commit planning and design history honestly; do not fabricate a seven-week commit chronology. Keep each functional change with its requirements and tests. Pull requests can document self review in this individual project, but must not imply a second reviewer when none participated. Require build and test checks before merging and retain issue links for defects or requirement changes.

The proposed CMake layout is include/lms, src/domain, src/application, src/infrastructure, src/ui, tests/unit, tests/integration, docs and migrations. CI should configure C++17, build, run ctest, run cppcheck and generate business-layer coverage on every push and pull request. Pin dependency versions at implementation time and preserve the commands that reproduce the environment. A workflow file alone is not CI evidence; save the successful hosted run URL and logs after it really executes.

Coding conventions require descriptive types, const correctness, unique ownership, RAII and no owning raw pointers outside documented library adapters. Avoid catch-all suppression and SQL string interpolation. Self review should examine permission checks, lifetime safety, exception guarantees, boundary arithmetic, secret handling and audit completeness. Record issues found and changes made. Metrics should include actual source lines and cyclomatic complexity of submit, mark, moderate and release. Targets guide review; do not invent metric values or equate a low line count with good design.

## 8.2 Deployment preparation
[[figure:D08_deployment]]

The technician guide must later specify the tested operating system, compiler/runtime packages, exact build commands, dependency versions, writable data location and startup command. The first administrator account is provisioned interactively; no shared default password is shipped. Install files and school data must be separate so upgrading the executable does not overwrite the database. Stop the application before any manual database copy, or use a verified consistent backup operation. Store backups on encrypted, access-controlled media and rehearse recovery on an isolated copy.

Role manuals should walk through real screenshots or terminal transcripts once the interface exists. The administrator manual covers account lifecycle, course setup, enrolments and custodian procedures. The lecturer manual covers assignment creation, individual extension, marking, feedback and correction. The HoD manual covers the departmental queue, adjustment, return, approval, release and reopening. The student manual covers course lists, receipt interpretation, late submission, resubmission and released results. A written manual is not accepted as proof that its steps match a working interface until executed against the release candidate.

## 8.3 CR1 maintenance design, reserved for after v1.0
CR1 is perfective maintenance: it adds a useful workflow for whole-class deadline extensions and improves what students see. It changes intended behaviour rather than repairing a defect against the original individual-extension requirement. The change must occur after an authentic v1.0 release, preserving both version histories and regression evidence. No v1.1 implementation or tag is created at the design-review stage.

Proposed new FR22 (Must in v1.1) allows an assigned lecturer to extend an assignment deadline for the whole enrolled class. Proposed FR23 requires the student assignment list to show base, class and individual deadlines, the effective deadline and the reason an extension applies. Set effective deadline to max(base, class, individual), so a class change never removes a later individual accommodation. A whole-class extension means all enrolled students for that assignment, including later enrolments. It is allowed only before any assessment exists for that assignment. This restriction avoids silently recalculating published or already marked work; broader retrospective corrections require a separately approved policy.

| Affected artefact | Planned CR1 revision |
|---|---|
| Requirements / rules | Add FR22-FR23; revise FR07-FR08 and BR04/BR09; approve eligibility restriction. |
| Use cases / diagrams | UC04 adds class branch; UC05 displays provenance; update D01, D03c, D06a and D07. |
| Classes | Add ClassExtension value/entity; update effectiveDeadline policy and AssessmentService extension commands. |
| Database | Add class_extensions keyed by assignment_id, with deadline, grantor, timestamp and reason; audit via admin_events. |
| UI / manual | Class extension command, student deadline labels and examples. |
| Tests | CR01 class affects all; CR02 individual later wins; CR03 shorter request rejected; CR04 other-course lecturer denied; CR05 after assessment rejected; CR06 list provenance; rerun all existing tests. |
| Migration / release | Backup before migration; verify FK/integrity and v1.0 data survival; tag v1.1 after regression evidence. |

A candidate migration is a design proposal only. It should add the class_extensions relation rather than overwrite assignments.deadline_utc, because preserving base and revised deadlines supports clear student display and historical explanation. A rollback plan restores the v1.0 backup and matching executable, acknowledging loss of any data entered since the backup. Never promise a destructive downgrade can preserve all newer data without a tested migration path.

# 9. Professional issues and gate decisions
The Lesotho legal source used here is the Data Protection Act, available from the Central Bank of Lesotho. For a school operating in Lesotho, it is a relevant primary source for personal-information handling; applicability and any updated requirements must be confirmed by the school before deployment. This design proposes data minimisation, restricted learner records and an accountable custodian. It does not claim that technical controls alone demonstrate legal compliance. Retention periods and the process for learner access or correction require an explicit school policy (Kingdom of Lesotho).

Ethically, a consistently applied late penalty can still disadvantage students facing illness, disability or unreliable transport. An individual extension with recorded reasons offers a controlled accommodation path. Students need to see the deadline and understand the penalty before submitting. They also need a human appeal route; the software should not present a computed zero as an unquestionable educational judgement. Sensitive reasons should be concise and access restricted, avoiding unnecessary medical details in ordinary assignment lists.

Accessibility requires more than choosing a terminal. Keyboard operation and text-labelled errors help, but a trial with relevant assistive technology is still necessary. The programme should avoid colour-only indicators, excessive screen clearing and unreadable tables. A lecturer's marking feedback should be visible as ordinary text once released. Supporting users with disabilities may require additional changes after consultation, so do not claim universal accessibility from a checklist alone.

Professional integrity applies to the project record itself. The student must understand and be able to explain every submitted design and later code component during the viva. Disclose assistance in accordance with module rules. Do not describe generated diagrams as supervisor-approved or planned tests as executed. The purpose of this package is to make the next review concrete and defensible, while keeping claims proportionate to the work actually completed.

## 9.1 Gate 1 review checklist
| Decision | Required review outcome | Present status |
|---|---|---|
| Deployment workload | Approve one active session and 800 stored students, or revise design. | Pending |
| Deadline policy | Approve started days, exact boundary, three-day discontinuity and rounding. | Pending |
| Extensions and resubmission | Approve pre-assessment extension and pre-deadline replacement rules. | Pending |
| Functional baseline | Accept Must scope and the missing-work exclusion. | Pending |
| Measurable quality | Confirm reference workstation and measurable targets. | Pending |
| Privacy and operation | Assign custodian and decide retention/appeal policies. | Pending |

## 9.2 Gate 2 review checklist
| Review item | Evidence supplied | Approval field |
|---|---|---|
| Architecture and alternatives | D02, layered responsibilities and rejected alternatives | Reviewer to complete |
| Detailed design and patterns | D03a-c, User inheritance, Strategy and Repository | Reviewer to complete |
| Behavioural consistency | D04-D06, exact transitions and atomic operations | Reviewer to complete |
| Persistent model | D07, SQL schema, dictionary and service-owned invariants | Reviewer to complete |
| Verification readiness | V-Model plan, boundary oracles and traceability | Reviewer to complete |
| Risk and schedule | Initial register, dependencies and gate update procedure | Reviewer to complete |

Reviewer name: ____________________

Decision: approve / approve subject to stated conditions / revise and resubmit.

Conditions and actions: ___________________________________________________

Implementation may start only when Gate 2 requirements are met. The next deliverables are the actual C++17 application, Catch2 suite, CMake build, hosted Git history, CI runs, measured coverage, static-analysis results, genuine system and acceptance records, deployment evidence, v1.0 release and the implemented CR1 v1.1 release. The final report then incorporates those results and must meet the 6,000-8,000-word requirement excluding diagrams, tables and appendices.

## 9.3 Viva preparation questions
1. Why is RECEIVED separate from the derived Submitted/Late classification?
2. Why is a maximum-mark deduction different from a percentage of the earned mark?
3. What happens at exactly the deadline and at 72 hours plus one second?
4. How does the design stop a lecturer from seeing another course's work?
5. Why does a release command check a revision as well as a role?
6. How can an event-insert failure leave the old mark unchanged?
7. What does a salted password hash protect, and what does it not protect?
8. Which invariants are SQL constraints and which belong to services?
9. Why are some values deliberately stored as historical snapshots?
10. How would CR1 change the deadline calculation without modifying the penalty strategy?

# 10. References
Sources are identified by author and title. No quotations from the sources are reproduced.

Kingdom of Lesotho *Data Protection Act*. Primary legislative copy hosted by the Central Bank of Lesotho. https://centralbank.org.ls/wp-content/uploads/Data_Protection_Act_2011.pdf

libsodium (n.d.) *The pwhash API*. https://doc.libsodium.org/password_hashing/default_phf

National University of Lesotho *CS4433 Software Engineering - Individual Assignment*. Supplied six-page assignment brief. Submission arrangements require confirmation.

Object Management Group *Unified Modeling Language, version 2.5.1*. https://www.omg.org/spec/UML/2.5.1/About-UML

SQLite (n.d.-a) *SQLite Foreign Key Support*. https://www.sqlite.org/foreignkeys.html

SQLite (n.d.-b) *Appropriate Uses For SQLite*. https://www.sqlite.org/whentouse.html

SQLite (n.d.-c) *Transaction*. https://www.sqlite.org/lang_transaction.html

# Appendix A. Marking-scheme readiness map
| Assessment component | Marks | Evidence prepared now | Still required |
|---|---|---|---|
| Planning | 10 | Feasibility, WBS, Gantt, critical path and ten risks | Real gate updates and schedule rebaselines |
| Requirements | 14 | Stakeholders, 21 FRs, 12 NFRs, use cases | Approved resolutions and Gate 1 record |
| Design | 24 | Architecture, classes, patterns, behaviour, ERD, DDL | Supervisor review and approved Gate 2 baseline |
| Implementation | 21 | Prospective design and source mapping | C++17 system implementing all Musts |
| V&V | 12 | Strategy, boundary oracles, planned tests | Executed Catch2/system/acceptance tests and actual coverage |
| CM and QA | 6 | Branching, CI and review plans | Hosted repository, real CI, metrics and analysis records |
| Deployment | 4 | Deployment model and manual structure | Tested install, actual manuals and v1.0 |
| Maintenance | 6 | CR1 impact analysis and proposed policy | Actual v1.1 change and regression evidence |
| Professional issues | 3 | Privacy, fairness, accessibility and integrity discussion | Confirm school policy and actual controls |

The readiness map identifies coverage of the rubric, not an estimated mark. An outstanding result depends on correctness, implementation, consistent evidence and the student's viva performance. No grade is guaranteed by this document.
