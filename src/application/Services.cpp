#include "lms/Services.hpp"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <sodium.h>
namespace lms {
namespace {
void validUsername(const std::string &name) {
    require(name.size() >= 3 && name.size() <= 64,
            "Username must contain 3-64 ASCII letters, digits, dot, hyphen or underscore.");
    for (unsigned char c : name)
        require((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                    c == '_' || c == '-' || c == '.',
                "Invalid username character.");
}
void validDeadline(TimePoint deadline) {
    require(deadline >= 946684800 && deadline <= 4133980799LL, "Deadline must be in 2000-2100.");
}
void validMaximum(MarkUnits maximum) {
    require(maximum >= 100 && maximum <= 100000, "Maximum mark must be 1.00-1000.00.");
}
void clearApproval(Assessment &a) {
    a.moderator = 0;
    a.moderatedAt = 0;
    a.releasedBy = 0;
    a.releasedAt = 0;
}
} // namespace
AuthService::AuthService(IRepository &repo, IClock &clock) : repo_(repo), clock_(clock) {
    require(sodium_init() >= 0, "Cryptography initialization failed.");
}
std::string AuthService::hashPassword(const std::string &password) {
    require(password.size() >= 12 && password.size() <= 128, "Password must contain 12-128 bytes.");
    require(password.find('\0') == std::string::npos, "Invalid password.");
    char hash[crypto_pwhash_STRBYTES];
    require(crypto_pwhash_str_alg(
                hash, password.data(), password.size(), crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_ARGON2ID13) == 0,
            "Password hashing failed (memory unavailable).");
    return hash;
}
void AuthService::bootstrap(const std::string &username, const std::string &name,
                            const std::string &password) {
    validUsername(username);
    textRequired(name, 120, "Name");
    auto hash = hashPassword(password);
    UnitOfWork tx(repo_);
    require(repo_.userCount() == 0,
            "Initialization is only available for an empty school database.");
    UserRecord user;
    user.username = username;
    user.name = name;
    user.hash = hash;
    user.role = Role::Admin;
    user.mustChange = false;
    auto id = repo_.addUser(user);
    repo_.adminEvent(id, clock_.now(), "USER", id, "BOOTSTRAP", "", "ADMIN",
                     "Initial administrator provisioned interactively");
    tx.commit();
}
std::string AuthService::login(const std::string &username, const std::string &password) {
    require(password.size() <= 128, "Login failed.");
    UnitOfWork tx(repo_);
    auto user = repo_.userByName(username);
    auto now = clock_.now();
    require(user && user->active && now >= user->lockedUntil,
            "Login failed or account temporarily locked.");
    if (user->lockedUntil != 0) {
        user->failed = 0;
        user->lockedUntil = 0;
    }
    if (crypto_pwhash_str_verify(user->hash.c_str(), password.data(), password.size()) != 0) {
        ++user->failed;
        if (user->failed >= 5)
            user->lockedUntil = now + 300;
        repo_.saveUser(*user);
        tx.commit();
        throw Error("Login failed or account temporarily locked.");
    }
    user->failed = 0;
    user->lockedUntil = 0;
    repo_.saveUser(*user);
    tx.commit();
    unsigned char bytes[32];
    char hex[65];
    randombytes_buf(bytes, sizeof bytes);
    sodium_bin2hex(hex, sizeof hex, bytes, sizeof bytes);
    sessions_[hex] = {user->id, now, user->hash};
    return hex;
}
void AuthService::logout(const std::string &token) {
    sessions_.erase(token);
}
Context AuthService::authorize(const std::string &token, bool allowPasswordChange) {
    auto found = sessions_.find(token);
    require(found != sessions_.end(), "Please log in.");
    auto now = clock_.now();
    if (now < found->second.last || now - found->second.last >= 900) {
        sessions_.erase(found);
        throw Error("Session expired. Please log in.");
    }
    auto u = repo_.user(found->second.user);
    if (!u || !u->active || u->hash != found->second.credential) {
        sessions_.erase(found);
        throw Error("Session revoked. Please log in.");
    }
    require(allowPasswordChange || !u->mustChange,
            "Change the temporary password before continuing.");
    found->second.last = now;
    auto polymorphic = makeUser(u->id, u->role);
    return {polymorphic->id(), polymorphic->role(), u->name};
}
bool AuthService::mustChange(const std::string &token) {
    auto ctx = authorize(token, true);
    return repo_.user(ctx.id)->mustChange;
}
void AuthService::changePassword(const std::string &token, const std::string &oldPassword,
                                 const std::string &newPassword) {
    UnitOfWork tx(repo_);
    auto ctx = authorize(token, true);
    auto u = repo_.user(ctx.id);
    require(oldPassword.size() <= 128 &&
                crypto_pwhash_str_verify(u->hash.c_str(), oldPassword.data(), oldPassword.size()) ==
                    0,
            "Current password is incorrect.");
    require(oldPassword != newPassword, "Choose a different password.");
    u->hash = hashPassword(newPassword);
    u->mustChange = false;
    u->failed = 0;
    u->lockedUntil = 0;
    repo_.saveUser(*u);
    repo_.adminEvent(ctx.id, clock_.now(), "USER", ctx.id, "PASSWORD_CHANGE", "redacted",
                     "redacted", "User changed own password");
    tx.commit();
    sessions_.at(token).credential = u->hash;
}
Context AdminService::admin(const std::string &token) {
    auto ctx = auth_.authorize(token);
    require(ctx.role == Role::Admin, "Administrator access required.");
    return ctx;
}
UserRecord AdminService::roleUser(Id id, Role role) {
    auto u = repo_.user(id);
    require(u && u->active && u->role == role, "Active user of the required role not found.");
    return *u;
}
void AdminService::audit(Id actor, const std::string &entity, Id key, const std::string &action,
                         const std::string &oldValue, const std::string &newValue) {
    repo_.adminEvent(actor, clock_.now(), entity, key, action, oldValue, newValue,
                     "Authorized administrator command");
}
std::vector<UserRecord> AdminService::users(const std::string &token) {
    admin(token);
    auto out = repo_.users();
    for (auto &u : out)
        u.hash.clear();
    return out;
}
std::vector<Department> AdminService::departments(const std::string &token) {
    admin(token);
    return repo_.departments();
}
Id AdminService::createUser(const std::string &token, const std::string &username,
                            const std::string &name, Role role, const std::string &password) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    validUsername(username);
    textRequired(name, 120, "Name");
    UserRecord u;
    u.username = username;
    u.name = name;
    u.role = role;
    u.hash = AuthService::hashPassword(password);
    auto id = repo_.addUser(u);
    audit(ctx.id, "USER", id, "CREATE", "", username + ":" + roleName(role));
    tx.commit();
    return id;
}
void AdminService::editUser(const std::string &token, Id user, const std::string &name,
                            bool active) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    textRequired(name, 120, "Name");
    auto u = repo_.user(user);
    require(u.has_value(), "User not found.");
    require(active || u->role != Role::Admin || !u->active || repo_.activeAdmins() > 1,
            "Cannot deactivate the last active administrator.");
    auto old = u->name + ":" + std::to_string(u->active);
    u->name = name;
    u->active = active;
    repo_.saveUser(*u);
    audit(ctx.id, "USER", user, "EDIT", old, name + ":" + std::to_string(active));
    tx.commit();
}
void AdminService::resetPassword(const std::string &token, Id user, const std::string &password) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    auto u = repo_.user(user);
    require(u.has_value(), "User not found.");
    u->hash = AuthService::hashPassword(password);
    u->mustChange = true;
    u->failed = 0;
    u->lockedUntil = 0;
    repo_.saveUser(*u);
    audit(ctx.id, "USER", user, "PASSWORD_RESET", "redacted", "redacted");
    tx.commit();
}
Id AdminService::createDepartment(const std::string &token, const std::string &name) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    textRequired(name, 120, "Department");
    auto id = repo_.addDepartment(name);
    audit(ctx.id, "DEPARTMENT", id, "CREATE", "", name);
    tx.commit();
    return id;
}
void AdminService::renameDepartment(const std::string &token, Id department,
                                    const std::string &name) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    textRequired(name, 120, "Department");
    std::string old;
    for (const auto &d : repo_.departments())
        if (d.id == department)
            old = d.name;
    require(!old.empty(), "Department not found.");
    repo_.renameDepartment(department, name);
    audit(ctx.id, "DEPARTMENT", department, "RENAME", old, name);
    tx.commit();
}
void AdminService::assignHead(const std::string &token, Id department, Id user) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    roleUser(user, Role::Hod);
    auto old = repo_.head(department);
    repo_.setHead(department, user);
    audit(ctx.id, "DEPARTMENT", department, "HEAD", std::to_string(old), std::to_string(user));
    tx.commit();
}
Id AdminService::createCourse(const std::string &token, const std::string &code,
                              const std::string &title, Id department) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    textRequired(code, 32, "Code");
    textRequired(title, 160, "Title");
    auto id = repo_.addCourse({0, code, title, department, true});
    audit(ctx.id, "COURSE", id, "CREATE", "", code + ":" + title);
    tx.commit();
    return id;
}
void AdminService::editCourse(const std::string &token, Id course, const std::string &title,
                              bool active) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    textRequired(title, 160, "Title");
    auto c = repo_.course(course);
    auto old = c.title + ":" + std::to_string(c.active);
    c.title = title;
    c.active = active;
    repo_.saveCourse(c);
    audit(ctx.id, "COURSE", course, "EDIT", old, title + ":" + std::to_string(active));
    tx.commit();
}
void AdminService::assignLecturer(const std::string &token, Id course, Id user, bool assigned) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    roleUser(user, Role::Lecturer);
    repo_.course(course);
    bool old = repo_.teaches(course, user);
    repo_.setLecturer(course, user, assigned);
    audit(ctx.id, "COURSE", course, "LECTURER", std::to_string(user) + ":" + std::to_string(old),
          std::to_string(user) + ":" + std::to_string(assigned));
    tx.commit();
}
void AdminService::enrol(const std::string &token, Id course, Id student, bool active) {
    UnitOfWork tx(repo_);
    auto ctx = admin(token);
    roleUser(student, Role::Student);
    repo_.course(course);
    bool old = repo_.enrolled(course, student);
    repo_.setEnrolment(course, student, active);
    audit(ctx.id, "COURSE", course, "ENROL", std::to_string(student) + ":" + std::to_string(old),
          std::to_string(student) + ":" + std::to_string(active));
    tx.commit();
}
std::vector<std::pair<Id, bool>> AdminService::enrolments(const std::string &token, Id course) {
    admin(token);
    repo_.course(course);
    return repo_.enrolments(course);
}
std::vector<Id> AdminService::lecturers(const std::string &token, Id course) {
    admin(token);
    repo_.course(course);
    return repo_.lecturers(course);
}
void AdminService::backup(const std::string &token, const std::string &path) {
    admin(token);
    textRequired(path, 1000, "Backup path");
    repo_.backup(path);
}

bool AssessmentService::accessible(const Context &ctx, const Course &c) {
    if (ctx.role == Role::Admin)
        return true;
    if (!c.active)
        return false;
    if (ctx.role == Role::Student)
        return repo_.enrolled(c.id, ctx.id);
    if (ctx.role == Role::Lecturer)
        return repo_.teaches(c.id, ctx.id);
    return repo_.head(c.department) == ctx.id;
}
void AssessmentService::lecturer(const Context &ctx, Id course) {
    require(ctx.role == Role::Lecturer && accessible(ctx, repo_.course(course)),
            "Assigned lecturer access required.");
}
void AssessmentService::hod(const Context &ctx, Id course) {
    require(ctx.role == Role::Hod && accessible(ctx, repo_.course(course)),
            "Department HoD access required.");
}
void AssessmentService::workAccess(const Context &ctx, const Submission &sub,
                                   bool studentsAllowed) {
    auto a = repo_.assignment(sub.assignment);
    require(ctx.role != Role::Admin && accessible(ctx, repo_.course(a.course)),
            "Work access denied.");
    if (ctx.role == Role::Student)
        require(studentsAllowed && sub.student == ctx.id, "Work access denied.");
}
TimePoint AssessmentService::deadline(const Assignment &a, Id student) {
    auto e = repo_.extension(a.id, student);
    auto c = repo_.classExtension(a.id);
    auto effective = c ? std::max(a.deadline, c->deadline) : a.deadline;
    return e ? std::max(effective, e->deadline) : effective;
}
std::vector<Course> AssessmentService::courses(const std::string &token) {
    auto ctx = auth_.authorize(token);
    std::vector<Course> out;
    const auto all = repo_.courses();
    std::copy_if(all.begin(), all.end(), std::back_inserter(out),
                 [&](const Course &course) { return accessible(ctx, course); });
    return out;
}
std::vector<AssignmentView> AssessmentService::assignments(const std::string &token, Id course) {
    auto ctx = auth_.authorize(token);
    require(accessible(ctx, repo_.course(course)), "Course access denied.");
    std::vector<AssignmentView> out;
    for (const auto &a : repo_.assignments(course)) {
        AssignmentView v;
        v.assignment = a;
        v.wholeClass = repo_.classExtension(a.id);
        v.effective = v.wholeClass ? std::max(a.deadline, v.wholeClass->deadline) : a.deadline;
        if (ctx.role == Role::Student) {
            v.effective = deadline(a, ctx.id);
            v.individual = repo_.extension(a.id, ctx.id);
            v.submission = repo_.submissionFor(a.id, ctx.id);
            if (v.submission)
                v.receipt = repo_.latestVersion(v.submission->id, false);
        }
        out.push_back(v);
    }
    return out;
}
Id AssessmentService::createAssignment(const std::string &token, Id course,
                                       const std::string &title, const std::string &instructions,
                                       TimePoint due, MarkUnits maximum) {
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    lecturer(ctx, course);
    textRequired(title, 160, "Title");
    textRequired(instructions, 10000, "Instructions");
    validDeadline(due);
    require(due > clock_.now(), "A new assignment deadline must be in the future.");
    validMaximum(maximum);
    auto id = repo_.addAssignment({0, course, ctx.id, title, instructions, due, maximum});
    repo_.adminEvent(ctx.id, clock_.now(), "ASSIGNMENT", id, "CREATE", "",
                     title + ":" + std::to_string(due), "Lecturer created assignment");
    tx.commit();
    return id;
}
void AssessmentService::editAssignment(const std::string &token, Id assignment, TimePoint due,
                                       MarkUnits maximum) {
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    auto a = repo_.assignment(assignment);
    lecturer(ctx, a.course);
    require(!repo_.hasSubmissions(a.id),
            "Base deadline and maximum are frozen after the first receipt.");
    validDeadline(due);
    require(due > clock_.now(), "Deadline must be in the future.");
    validMaximum(maximum);
    auto old = std::to_string(a.deadline) + ":" + std::to_string(a.maximum);
    a.deadline = due;
    a.maximum = maximum;
    repo_.saveAssignment(a);
    repo_.adminEvent(ctx.id, clock_.now(), "ASSIGNMENT", a.id, "EDIT", old,
                     std::to_string(due) + ":" + std::to_string(maximum),
                     "Unsubmitted assignment adjusted");
    tx.commit();
}
void AssessmentService::extendIndividual(const std::string &token, Id assignment, Id student,
                                         TimePoint due, const std::string &reason) {
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    auto a = repo_.assignment(assignment);
    lecturer(ctx, a.course);
    auto u = repo_.user(student);
    require(u && u->active && u->role == Role::Student && repo_.enrolled(a.course, student),
            "Active enrolled student required.");
    auto sub = repo_.submissionFor(assignment, student);
    require(!sub || !repo_.assessment(sub->id), "Extension is unavailable after first assessment.");
    auto old = deadline(a, student);
    validDeadline(due);
    require(due > old, "Extension must increase the effective deadline.");
    textRequired(reason, 1000, "Reason");
    repo_.saveExtension({assignment, student, ctx.id, due, clock_.now(), reason});
    repo_.adminEvent(ctx.id, clock_.now(), "INDIVIDUAL_EXTENSION", assignment, "EXTEND",
                     std::to_string(student) + ":" + std::to_string(old),
                     std::to_string(student) + ":" + std::to_string(due), reason);
    tx.commit();
}
void AssessmentService::extendClass(const std::string &token, Id assignment, TimePoint due,
                                    const std::string &reason) {
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    auto a = repo_.assignment(assignment);
    lecturer(ctx, a.course);
    require(!repo_.hasAssessments(assignment),
            "Class extension unavailable after any first assessment.");
    auto previous = repo_.classExtension(assignment);
    auto old = previous ? std::max(a.deadline, previous->deadline) : a.deadline;
    validDeadline(due);
    require(due > old, "Class extension must increase the class deadline.");
    textRequired(reason, 1000, "Reason");
    repo_.saveClassExtension({assignment, ctx.id, due, clock_.now(), reason});
    repo_.adminEvent(ctx.id, clock_.now(), "CLASS_EXTENSION", assignment, "EXTEND",
                     std::to_string(old), std::to_string(due), reason);
    tx.commit();
}

SubmissionVersion AssessmentService::submit(const std::string &token, Id assignment,
                                            const std::string &filename, const Bytes &content) {
    auto leaf = safeFilename(filename);
    require(!content.empty() && content.size() <= 10485760, "Work must contain 1 byte to 10 MiB.");
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    require(ctx.role == Role::Student, "Student access required.");
    auto a = repo_.assignment(assignment);
    require(accessible(ctx, repo_.course(a.course)), "Active enrolment required.");
    auto due = deadline(a, ctx.id);
    auto now = clock_.now();
    auto sub = repo_.submissionFor(assignment, ctx.id);
    int version = 1;
    if (sub) {
        require(sub->phase == "RECEIVED" && now <= due,
                "Resubmission is only allowed before the effective deadline and before marking.");
        version = repo_.latestVersion(sub->id, false).number + 1;
        ++sub->rowVersion;
        repo_.saveSubmission(*sub);
    } else {
        sub =
            Submission{repo_.addSubmission(assignment, ctx.id), assignment, ctx.id, "RECEIVED", 1};
    }
    SubmissionVersion v{sub->id, version, now, due, leaf, content};
    repo_.addVersion(v);
    tx.commit();
    v.content.clear();
    return v;
}
std::vector<WorkView> AssessmentService::work(const std::string &token, Id assignment) {
    auto ctx = auth_.authorize(token);
    auto a = repo_.assignment(assignment);
    require((ctx.role == Role::Lecturer || ctx.role == Role::Hod) &&
                accessible(ctx, repo_.course(a.course)),
            "Teaching staff access required.");
    std::vector<WorkView> out;
    const auto all = repo_.submissions(assignment);
    std::transform(all.begin(), all.end(), std::back_inserter(out), [&](const Submission &sub) {
        return WorkView{sub, repo_.latestVersion(sub.id, false), repo_.assessment(sub.id)};
    });
    return out;
}
SubmissionVersion AssessmentService::readWork(const std::string &token, Id submission) {
    auto ctx = auth_.authorize(token);
    auto sub = repo_.submission(submission);
    workAccess(ctx, sub, true);
    return repo_.latestVersion(sub.id, true);
}
void AssessmentService::saveChange(const Context &ctx, Submission &sub, Assessment &a,
                                   const std::optional<Assessment> &old,
                                   const std::string &oldPhase, const std::string &action,
                                   const std::string &reason) {
    textRequired(reason, 1000, "Reason");
    a.revision = old ? old->revision + 1 : 1;
    ++sub.rowVersion;
    AssessmentEvent e;
    e.submission = sub.id;
    e.actor = ctx.id;
    e.revision = a.revision;
    e.at = clock_.now();
    e.action = action;
    if (old) {
        e.oldRaw = old->raw;
        e.oldFinal = old->final;
        e.oldFeedback = old->feedback;
    }
    e.newRaw = a.raw;
    e.newFinal = a.final;
    e.newFeedback = a.feedback;
    e.oldPhase = oldPhase;
    e.newPhase = sub.phase;
    e.reason = reason;
    repo_.saveAssessment(a);
    repo_.saveSubmission(sub);
    repo_.addEvent(e);
}
void AssessmentService::mark(const std::string &token, Id submission, int expectedRow,
                             MarkUnits raw, const std::string &feedback,
                             const std::string &reason) {
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    auto sub = repo_.submission(submission);
    auto assignment = repo_.assignment(sub.assignment);
    lecturer(ctx, assignment.course);
    require(sub.rowVersion == expectedRow, "Stale work version. Refresh before marking.");
    require(sub.phase == "RECEIVED" || sub.phase == "MARKED",
            "Return or reopen this work before revising it.");
    auto due = deadline(assignment, sub.student);
    require(clock_.now() > due, "Wait until after the student's effective deadline.");
    textRequired(feedback, 10000, "Feedback");
    auto v = repo_.latestVersion(sub.id, false);
    auto old = repo_.assessment(sub.id);
    Assessment a;
    a.submission = sub.id;
    a.version = v.number;
    a.raw = raw;
    a.final = penalty_.calculate(raw, assignment.maximum, v.received - due);
    a.days = lateDays(v.received - due);
    a.deadline = due;
    a.feedback = feedback;
    a.marker = ctx.id;
    auto oldPhase = sub.phase;
    sub.phase = "MARKED";
    saveChange(ctx, sub, a, old, oldPhase, old ? "REVISE" : "MARK", reason);
    tx.commit();
}
void AssessmentService::moderate(const std::string &token, Id submission, int expectedRevision,
                                 std::optional<MarkUnits> raw, std::optional<std::string> feedback,
                                 const std::string &reason) {
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    auto sub = repo_.submission(submission);
    auto assignment = repo_.assignment(sub.assignment);
    hod(ctx, assignment.course);
    auto old = repo_.assessment(sub.id);
    require(old && sub.phase == "MARKED", "A marked result is required.");
    require(old->revision == expectedRevision, "Stale assessment. Refresh before moderation.");
    auto a = *old;
    if (raw)
        a.raw = *raw;
    if (feedback) {
        textRequired(*feedback, 10000, "Feedback");
        a.feedback = *feedback;
    }
    auto v = repo_.latestVersion(sub.id, false);
    a.final = penalty_.calculate(a.raw, assignment.maximum, v.received - a.deadline);
    a.moderator = ctx.id;
    a.moderatedAt = clock_.now();
    sub.phase = "MODERATED";
    saveChange(ctx, sub, a, old, "MARKED", "MODERATE", reason);
    tx.commit();
}
void AssessmentService::transition(const std::string &token, Id submission, int expectedRevision,
                                   const std::string &action, const std::string &reason) {
    UnitOfWork tx(repo_);
    auto ctx = auth_.authorize(token);
    auto sub = repo_.submission(submission);
    hod(ctx, repo_.assignment(sub.assignment).course);
    auto old = repo_.assessment(sub.id);
    require(old && old->revision == expectedRevision,
            "Missing or stale assessment. Refresh first.");
    auto a = *old;
    auto oldPhase = sub.phase;
    if (action == "RELEASE") {
        require(sub.phase == "MODERATED" && a.moderator != 0,
                "Only a current moderated result may be released.");
        sub.phase = "RELEASED";
        a.releasedBy = ctx.id;
        a.releasedAt = clock_.now();
    } else if (action == "RETURN") {
        require(sub.phase == "MARKED" || sub.phase == "MODERATED",
                "Only marked or moderated results can be returned.");
        sub.phase = "MARKED";
        clearApproval(a);
    } else if (action == "REOPEN") {
        require(sub.phase == "RELEASED", "Only a released result can be reopened.");
        sub.phase = "MARKED";
        clearApproval(a);
    } else
        throw Error("Unknown assessment transition.");
    saveChange(ctx, sub, a, old, oldPhase, action, reason);
    tx.commit();
}
std::vector<ReleasedResult> AssessmentService::results(const std::string &token) {
    auto ctx = auth_.authorize(token);
    require(ctx.role == Role::Student, "Student result view required.");
    std::vector<ReleasedResult> out;
    for (const auto &c : repo_.courses())
        if (accessible(ctx, c))
            for (const auto &a : repo_.assignments(c.id)) {
                auto s = repo_.submissionFor(a.id, ctx.id);
                if (s && s->phase == "RELEASED") {
                    auto result = repo_.assessment(s->id);
                    require(result && result->releasedBy != 0, "Result integrity error.");
                    out.push_back({s->id, a.title, a.maximum, result->final, result->feedback});
                }
            }
    return out;
}
std::vector<AssessmentEvent> AssessmentService::history(const std::string &token, Id submission) {
    auto ctx = auth_.authorize(token);
    auto sub = repo_.submission(submission);
    workAccess(ctx, sub, false);
    return repo_.events(sub.id);
}
} // namespace lms
