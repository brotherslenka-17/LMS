#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "lms/Services.hpp"
#include <filesystem>
#include <fstream>
#include <limits>
#include <sodium.h>
using namespace lms;
namespace {
constexpr const char *password = "Synthetic-pass-42!";
struct FakeClock final : IClock {
    TimePoint value{1800000000};
    TimePoint now() const override { return value; }
};
struct FaultRepository final : SqliteRepository {
    using SqliteRepository::SqliteRepository;
    bool failEvent{};
    int failStage{};
    void saveAssessment(const Assessment &a) override {
        SqliteRepository::saveAssessment(a);
        if (failStage == 1)
            throw Error("Injected after assessment write");
    }
    void saveSubmission(const Submission &s) override {
        SqliteRepository::saveSubmission(s);
        if (failStage == 2)
            throw Error("Injected after phase write");
    }
    void commit() override {
        if (failStage == 4)
            throw Error("Injected before commit");
        SqliteRepository::commit();
    }
    void addEvent(const AssessmentEvent &e) override {
        if (failEvent)
            throw Error("Injected audit write failure");
        SqliteRepository::addEvent(e);
        if (failStage == 3)
            throw Error("Injected after event write");
    }
};
struct Fixture {
    FaultRepository repo{":memory:"};
    FakeClock clock;
    TenPercentPenalty penalty;
    AuthService auth{repo, clock};
    AdminService admin{repo, auth, clock};
    AssessmentService service{repo, auth, clock, penalty};
    std::string root, student, student2, teacher, teacher2, hod, hod2;
    Id sid{}, sid2{}, tid{}, tid2{}, hid{}, hid2{}, department{}, otherDepartment{}, course{},
        otherCourse{}, assignment{};
    TimePoint due{};
    Fixture() {
        auth.bootstrap("admin", "Administrator", password);
        root = auth.login("admin", password);
        auto syntheticHash = AuthService::hashPassword(password);
        auto add = [&](const char *name, Role role) {
            UserRecord u;
            u.username = name;
            u.name = name;
            u.hash = syntheticHash;
            u.role = role;
            u.mustChange = false;
            return repo.addUser(u);
        };
        sid = add("student", Role::Student);
        sid2 = add("student2", Role::Student);
        tid = add("teacher", Role::Lecturer);
        tid2 = add("teacher2", Role::Lecturer);
        hid = add("head", Role::Hod);
        hid2 = add("head2", Role::Hod);
        department = admin.createDepartment(root, "Science");
        otherDepartment = admin.createDepartment(root, "Arts");
        admin.assignHead(root, department, hid);
        admin.assignHead(root, otherDepartment, hid2);
        course = admin.createCourse(root, "SCI1", "Science", department);
        otherCourse = admin.createCourse(root, "ART1", "Arts", otherDepartment);
        admin.assignLecturer(root, course, tid, true);
        admin.assignLecturer(root, otherCourse, tid2, true);
        admin.enrol(root, course, sid, true);
        admin.enrol(root, course, sid2, true);
        refresh();
        due = clock.value + 100;
        assignment =
            service.createAssignment(teacher, course, "Lab", "Explain your work", due, 8000);
    }
    void refresh() {
        root = auth.login("admin", password);
        student = auth.login("student", password);
        student2 = auth.login("student2", password);
        teacher = auth.login("teacher", password);
        teacher2 = auth.login("teacher2", password);
        hod = auth.login("head", password);
        hod2 = auth.login("head2", password);
    }
    Id submit() {
        return service.submit(student, assignment, "work.txt", Bytes{1, 2, 3}).submission;
    }
    Id marked() {
        auto id = submit();
        clock.value = due + 1;
        service.mark(teacher, id, repo.submission(id).rowVersion, 6500, "Clear reasoning",
                     "Initial marking");
        return id;
    }
    Id released() {
        auto id = marked();
        service.moderate(hod, id, 1, {}, {}, "Checked");
        service.transition(hod, id, 2, "RELEASE", "Approved publication");
        return id;
    }
};
std::string tempPath(const std::string &suffix) {
    static int count = 0;
    return (std::filesystem::temp_directory_path() /
            ("cs4433-test-" + std::to_string(++count) + suffix))
        .string();
}
} // namespace
TEST_CASE("UT01 all late-day boundaries", "[unit][FR11]") {
    TenPercentPenalty p;
    for (auto elapsed : {-1LL, 0LL})
        REQUIRE(p.calculate(6500, 8000, elapsed) == 6500);
    for (auto elapsed : {1LL, 86400LL})
        REQUIRE(p.calculate(6500, 8000, elapsed) == 5700);
    for (auto elapsed : {86401LL, 172800LL})
        REQUIRE(p.calculate(6500, 8000, elapsed) == 4900);
    for (auto elapsed : {172801LL, 259200LL})
        REQUIRE(p.calculate(6500, 8000, elapsed) == 4100);
    REQUIRE(p.calculate(6500, 8000, 259201) == 0);
    REQUIRE(lateDays(std::numeric_limits<TimePoint>::max()) > 3);
}
TEST_CASE("UT02 mark partitions and rounding", "[unit][FR11]") {
    TenPercentPenalty p;
    REQUIRE(p.calculate(500, 8000, 1) == 0);
    REQUIRE(p.calculate(1005, 1005, 1) == 904);
    REQUIRE(p.calculate(0, 8000, 0) == 0);
    REQUIRE(p.calculate(8000, 8000, 0) == 8000);
    REQUIRE_THROWS_AS(p.calculate(-1, 8000, 0), Error);
    REQUIRE_THROWS_AS(p.calculate(8001, 8000, 0), Error);
    REQUIRE_THROWS_AS(p.calculate(0, 99, 0), Error);
    REQUIRE_THROWS_AS(p.calculate(0, 100001, 0), Error);
}
TEST_CASE("UT05 input partitions reject malformed values", "[unit]") {
    REQUIRE(parseMark("65.5") == 6550);
    REQUIRE(parseMark("0") == 0);
    REQUIRE(parseMark("1000.00") == 100000);
    REQUIRE(formatMark(904) == "9.04");
    for (auto s : {"", "-1", "1.234", "1.", ".2", "NaN", "12x", "1001", "1.2.3"})
        REQUIRE_THROWS_AS(parseMark(s), Error);
    REQUIRE(safeFilename("../folder/work.txt") == "work.txt");
    REQUIRE(safeFilename("C:\\work\\lab.txt") == "lab.txt");
    for (auto s : {"", "..", ".", "bad\033.txt"})
        REQUIRE_THROWS_AS(safeFilename(s), Error);
    REQUIRE_THROWS_AS(textRequired("   ", 5, "Name"), Error);
    REQUIRE_THROWS_AS(textRequired("abcdef", 5, "Name"), Error);
}
TEST_CASE("UT06 school time round trip and calendar validation", "[unit]") {
    auto time = parseLocalTime("2026-11-07 14:30:00");
    REQUIRE(formatLocalTime(time) == "2026-11-07 14:30:00 +02:00");
    for (auto s : {"2026-02-30 10:00:00", "2026-11-07 25:00:00", "not a time",
                   "1999-12-31 12:00:00", "2026-01-01 00:00:00x"})
        REQUIRE_THROWS_AS(parseLocalTime(s), Error);
    SystemClock c;
    REQUIRE(c.now() > 946684800);
}
TEST_CASE("UT07 concrete roles obey abstract User contract", "[unit][FR02]") {
    for (auto role : {Role::Student, Role::Lecturer, Role::Hod, Role::Admin}) {
        auto u = makeUser(7, role);
        REQUIRE(u->id() == 7);
        REQUIRE(u->role() == role);
        REQUIRE(parseRole(roleName(role)) == role);
    }
    REQUIRE_THROWS_AS(parseRole("UNKNOWN"), Error);
}
TEST_CASE_METHOD(Fixture, "AT01 session authentication logout inactivity and clock reversal",
                 "[integration][FR01]") {
    REQUIRE(auth.authorize(student).id == sid);
    REQUIRE_THROWS_AS(auth.login("missing", password), Error);
    REQUIRE_THROWS_AS(auth.authorize("forged"), Error);
    auth.logout(student);
    REQUIRE_THROWS_AS(auth.authorize(student), Error);
    clock.value += 900;
    REQUIRE_THROWS_AS(auth.authorize(teacher), Error);
    auto t = auth.login("teacher", password);
    --clock.value;
    REQUIRE_THROWS_AS(auth.authorize(t), Error);
}
TEST_CASE_METHOD(Fixture, "AT01 lockout persists and expires", "[integration][FR01]") {
    for (int i = 0; i < 5; ++i)
        REQUIRE_THROWS_AS(auth.login("student", "incorrect"), Error);
    REQUIRE(repo.user(sid)->failed == 5);
    AuthService restarted(repo, clock);
    REQUIRE_THROWS_AS(restarted.login("student", password), Error);
    clock.value += 300;
    REQUIRE_NOTHROW(restarted.login("student", password));
    REQUIRE(repo.user(sid)->failed == 0);
}
TEST_CASE_METHOD(Fixture, "AT03 credentials salted reset invalidates sessions and requires change",
                 "[integration][FR03]") {
    auto id = admin.createUser(root, "newstudent", "New Student", Role::Student, password);
    auto id2 = admin.createUser(root, "otherstudent", "Other", Role::Student, password);
    REQUIRE(repo.user(id)->hash != repo.user(id2)->hash);
    REQUIRE(repo.user(id)->hash.find("$argon2id$") == 0);
    auto token = auth.login("newstudent", password);
    REQUIRE(auth.mustChange(token));
    REQUIRE_THROWS_AS(service.courses(token), Error);
    REQUIRE_THROWS_AS(auth.changePassword(token, "bad", "Different-pass-99!"), Error);
    REQUIRE_THROWS_AS(auth.changePassword(token, password, password), Error);
    auth.changePassword(token, password, "Different-pass-99!");
    REQUIRE_FALSE(auth.mustChange(token));
    REQUIRE_NOTHROW(service.courses(token));
    admin.resetPassword(root, sid, "Temporary-pass-99!");
    REQUIRE_THROWS_AS(auth.authorize(student), Error);
    auto reset = auth.login("student", "Temporary-pass-99!");
    REQUIRE(auth.mustChange(reset));
    REQUIRE_THROWS_AS(AuthService::hashPassword("short"), Error);
    REQUIRE_THROWS_AS(admin.createUser(root, "bad user", "Name", Role::Student, password), Error);
    for (auto &u : admin.users(root))
        REQUIRE(u.hash.empty());
}
TEST_CASE_METHOD(Fixture, "AT03 account deactivation and last administrator protection",
                 "[integration][FR03]") {
    REQUIRE_THROWS_AS(admin.editUser(root, 1, "Administrator", false), Error);
    admin.editUser(root, sid, "Renamed", false);
    REQUIRE(repo.user(sid)->name == "Renamed");
    REQUIRE_THROWS_AS(auth.login("student", password), Error);
    REQUIRE_THROWS_AS(auth.authorize(student), Error);
    REQUIRE_THROWS_AS(admin.createDepartment(teacher, "Other"), Error);
    REQUIRE_THROWS_AS(admin.editUser(root, 999, "Missing", true), Error);
    REQUIRE_THROWS_AS(auth.bootstrap("second", "Second", password), Error);
}
TEST_CASE_METHOD(Fixture, "AT04 AT05 school administration constraints and mapping changes",
                 "[integration][FR04][FR05]") {
    admin.renameDepartment(root, department, "Natural Science");
    REQUIRE(admin.departments(root).at(0).name == "Natural Science");
    REQUIRE_THROWS_AS(admin.createDepartment(root, "Natural Science"), Error);
    REQUIRE_THROWS_AS(admin.assignHead(root, department, tid), Error);
    REQUIRE_THROWS_AS(admin.createCourse(root, "BAD", "Bad course", 999), Error);
    REQUIRE_THROWS_AS(admin.assignLecturer(root, course, sid, true), Error);
    REQUIRE_THROWS_AS(admin.enrol(root, course, tid, true), Error);
    admin.enrol(root, course, sid, true);
    REQUIRE(admin.enrolments(root, course).size() == 2);
    REQUIRE(admin.lecturers(root, course) == std::vector<Id>{tid});
    admin.assignLecturer(root, course, tid, false);
    REQUIRE(service.courses(teacher).empty());
    admin.assignLecturer(root, course, tid, true);
    admin.enrol(root, course, sid, false);
    REQUIRE(service.courses(student).empty());
    admin.editCourse(root, course, "Renamed science", false);
    REQUIRE(service.courses(teacher).empty());
    REQUIRE(service.courses(root).size() == 2);
    REQUIRE_THROWS_AS(admin.renameDepartment(root, 999, "Missing"), Error);
}
TEST_CASE_METHOD(Fixture, "AT06 assignment validation edits and freezing", "[integration][FR06]") {
    REQUIRE_THROWS_AS(service.createAssignment(student, course, "Task", "Text", due, 8000), Error);
    REQUIRE_THROWS_AS(service.createAssignment(teacher2, course, "Task", "Text", due, 8000), Error);
    REQUIRE_THROWS_AS(service.createAssignment(teacher, course, "Past", "Text", clock.value, 8000),
                      Error);
    REQUIRE_THROWS_AS(service.createAssignment(teacher, course, "Zero", "Text", due, 0), Error);
    service.editAssignment(teacher, assignment, due + 10, 10000);
    REQUIRE(repo.assignment(assignment).maximum == 10000);
    submit();
    REQUIRE_THROWS_AS(service.editAssignment(teacher, assignment, due + 20, 9000), Error);
    REQUIRE_THROWS_AS(service.assignments(student, otherCourse), Error);
    REQUIRE(service.assignments(student, course).size() == 1);
}
TEST_CASE_METHOD(Fixture, "AT07 extension changes effective deadline preserves receipt snapshot",
                 "[integration][FR07][FR08]") {
    auto sub = submit();
    service.extendIndividual(teacher, assignment, sid, due + 200, "Approved accommodation");
    auto list = service.assignments(student, course);
    REQUIRE(list[0].effective == due + 200);
    REQUIRE(list[0].receipt->number == 1);
    REQUIRE(list[0].receipt->received <= list[0].effective);
    REQUIRE(list[0].individual->reason == "Approved accommodation");
    REQUIRE(repo.latestVersion(sub, false).deadlineAtReceipt == due);
    REQUIRE(service.assignments(student2, course)[0].effective == due);
    REQUIRE_THROWS_AS(service.extendIndividual(teacher, assignment, sid, due + 100, "Shorter"),
                      Error);
    REQUIRE_THROWS_AS(
        service.extendIndividual(teacher2, assignment, sid, due + 400, "Wrong lecturer"), Error);
    REQUIRE_THROWS_AS(service.extendIndividual(teacher, assignment, tid, due + 400, "Wrong role"),
                      Error);
    clock.value = due + 1;
    REQUIRE_THROWS_AS(service.mark(teacher, sub, 1, 6000, "Feedback", "Too early"), Error);
    clock.value = due + 201;
    service.mark(teacher, sub, 1, 6000, "Feedback", "Initial");
    REQUIRE(repo.assessment(sub)->final == 6000);
    REQUIRE_THROWS_AS(
        service.extendIndividual(teacher, assignment, sid, due + 500, "Already assessed"), Error);
}
TEST_CASE_METHOD(Fixture, "AT09 AT10 exact deadline resubmission preserves prior bytes",
                 "[integration][FR09][FR10]") {
    auto sub = submit();
    clock.value = due;
    auto r = service.submit(student, assignment, "fixed.txt", Bytes{4, 5});
    REQUIRE(r.number == 2);
    REQUIRE(r.submission == sub);
    REQUIRE(repo.latestVersion(sub, true).content == Bytes{4, 5});
    REQUIRE(repo.submission(sub).rowVersion == 2);
    ++clock.value;
    REQUIRE_THROWS_AS(service.submit(student, assignment, "late.txt", Bytes{9}), Error);
    REQUIRE(repo.latestVersion(sub, true).content == Bytes{4, 5});
    service.mark(teacher, sub, 2, 5000, "Correct", "Mark latest");
    REQUIRE(repo.assessment(sub)->version == 2);
    REQUIRE_THROWS_AS(service.submit(student, assignment, "another.txt", Bytes{9}), Error);
}
TEST_CASE_METHOD(Fixture, "AT09 first late work accepted after three days and marks zero",
                 "[integration][FR09][FR11]") {
    clock.value = due + 259201;
    refresh();
    auto sub = submit();
    REQUIRE(repo.submission(sub).phase == "RECEIVED");
    service.mark(teacher, sub, 1, 6500, "Late but useful", "First assessment");
    REQUIRE(repo.assessment(sub)->days == 4);
    REQUIRE(repo.assessment(sub)->final == 0);
}
TEST_CASE_METHOD(Fixture, "IT05 attachment size and malformed file partitions",
                 "[integration][FR09]") {
    REQUIRE_THROWS_AS(service.submit(student, assignment, "empty.txt", {}), Error);
    REQUIRE_THROWS_AS(service.submit(student, assignment, "large.bin", Bytes(10485761, 0)), Error);
    auto receipt = service.submit(student, assignment, "max.bin", Bytes(10485760, 7));
    REQUIRE(repo.latestVersion(receipt.submission, true).content.size() == 10485760);
    REQUIRE_THROWS_AS(service.submit(teacher, assignment, "file", Bytes{1}), Error);
    REQUIRE_THROWS_AS(service.submit(student, assignment, "bad\nfile", Bytes{1}), Error);
}
TEST_CASE_METHOD(Fixture,
                 "IT03 direct service authorization blocks resource enumeration and work theft",
                 "[integration][FR02]") {
    auto id = submit();
    REQUIRE_THROWS_AS(service.readWork(student2, id), Error);
    REQUIRE_THROWS_AS(service.readWork(teacher2, id), Error);
    REQUIRE_THROWS_AS(service.readWork(root, id), Error);
    REQUIRE_THROWS_AS(service.history(student, id), Error);
    REQUIRE_THROWS_AS(service.work(student, assignment), Error);
    REQUIRE_THROWS_AS(service.work(hod2, assignment), Error);
    REQUIRE(service.readWork(student, id).content == Bytes{1, 2, 3});
    REQUIRE(service.readWork(teacher, id).content == Bytes{1, 2, 3});
    admin.enrol(root, course, sid, false);
    REQUIRE_THROWS_AS(service.readWork(student, id), Error);
}
TEST_CASE_METHOD(Fixture, "AT12 marking range feedback authority timing and stale version",
                 "[integration][FR12]") {
    auto id = submit();
    REQUIRE_THROWS_AS(service.mark(teacher, id, 1, 6500, "Good", "Early"), Error);
    clock.value = due + 1;
    REQUIRE_THROWS_AS(service.mark(teacher2, id, 1, 6500, "Good", "Wrong course"), Error);
    REQUIRE_THROWS_AS(service.mark(teacher, id, 2, 6500, "Good", "Stale"), Error);
    REQUIRE_THROWS_AS(service.mark(teacher, id, 1, 8001, "Good", "Invalid range"), Error);
    REQUIRE_THROWS_AS(service.mark(teacher, id, 1, 6500, "", "Empty feedback"), Error);
    REQUIRE_THROWS_AS(service.mark(teacher, id, 1, 6500, "Good", ""), Error);
    REQUIRE_FALSE(repo.assessment(id));
    service.mark(teacher, id, 1, 6500, "Good", "Initial");
    service.mark(teacher, id, 2, 6600, "Revised feedback", "Correction");
    auto events = service.history(teacher, id);
    REQUIRE(events.size() == 2);
    REQUIRE_FALSE(events[0].oldRaw);
    REQUIRE(events[1].oldRaw == 6500);
    REQUIRE(events[1].newRaw == 6600);
    REQUIRE(events[1].oldFeedback == "Good");
    REQUIRE(events[1].actor == tid);
}
TEST_CASE_METHOD(Fixture, "IT02 transaction rollback prevents unaudited marks",
                 "[integration][FR16][FR17]") {
    auto id = submit();
    clock.value = due + 1;
    repo.failEvent = true;
    REQUIRE_THROWS_AS(service.mark(teacher, id, 1, 6000, "Feedback", "Injected failure"), Error);
    REQUIRE_FALSE(repo.assessment(id));
    REQUIRE(repo.submission(id).phase == "RECEIVED");
    REQUIRE(repo.submission(id).rowVersion == 1);
    REQUIRE(repo.events(id).empty());
    repo.failEvent = false;
    service.mark(teacher, id, 1, 6000, "Feedback", "Recovered");
    repo.failEvent = true;
    REQUIRE_THROWS_AS(service.moderate(hod, id, 1, 7000, {}, "Injected moderation failure"), Error);
    REQUIRE(repo.assessment(id)->raw == 6000);
    REQUIRE(repo.assessment(id)->revision == 1);
    REQUIRE(repo.submission(id).phase == "MARKED");
    REQUIRE(repo.events(id).size() == 1);
}
TEST_CASE_METHOD(Fixture, "AT13 AT14 AT15 moderation release visibility and department separation",
                 "[integration][FR13][FR14][FR15]") {
    auto id = marked();
    REQUIRE(service.results(student).empty());
    REQUIRE_THROWS_AS(service.transition(hod, id, 1, "RELEASE", "Not approved"), Error);
    REQUIRE_THROWS_AS(service.moderate(hod2, id, 1, {}, {}, "Wrong department"), Error);
    REQUIRE_THROWS_AS(service.moderate(hod, id, 99, {}, {}, "Stale"), Error);
    REQUIRE_THROWS_AS(service.moderate(hod, id, 1, 8001, {}, "Invalid adjustment"), Error);
    service.moderate(hod, id, 1, 7000, std::string("Moderated feedback"), "Review adjustment");
    REQUIRE(service.results(student).empty());
    REQUIRE_THROWS_AS(service.transition(teacher, id, 2, "RELEASE", "Wrong role"), Error);
    REQUIRE_THROWS_AS(service.transition(hod, id, 1, "RELEASE", "Stale revision"), Error);
    service.transition(hod, id, 2, "RELEASE", "Checked publication");
    auto results = service.results(student);
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].final == 7000);
    REQUIRE(results[0].feedback == "Moderated feedback");
    REQUIRE(service.results(student2).empty());
    REQUIRE_THROWS_AS(service.results(root), Error);
    REQUIRE_THROWS_AS(
        service.mark(teacher, id, repo.submission(id).rowVersion, 5000, "Changed", "Bypass"),
        Error);
    REQUIRE(repo.events(id).size() == 3);
    REQUIRE(repo.events(id)[1].oldRaw == 6500);
    REQUIRE(repo.events(id)[1].newRaw == 7000);
}
TEST_CASE_METHOD(Fixture, "AT18 reopening hides results and requires fresh moderation",
                 "[integration][FR18]") {
    auto id = released();
    service.transition(hod, id, 3, "REOPEN", "Appeal");
    REQUIRE(service.results(student).empty());
    REQUIRE(repo.assessment(id)->moderator == 0);
    REQUIRE_THROWS_AS(service.transition(hod, id, 4, "RELEASE", "Old approval"), Error);
    service.mark(teacher, id, repo.submission(id).rowVersion, 7500, "Rechecked",
                 "Appeal correction");
    service.moderate(hod, id, 5, {}, {}, "Reapproved");
    service.transition(hod, id, 6, "RELEASE", "Republish");
    REQUIRE(service.results(student)[0].final == 7500);
    REQUIRE(repo.events(id).size() == 7);
}
TEST_CASE_METHOD(Fixture, "AT13 returning an approved mark invalidates approval",
                 "[integration][FR13]") {
    auto id = marked();
    service.moderate(hod, id, 1, {}, {}, "Reviewed");
    service.transition(hod, id, 2, "RETURN", "Please reconsider");
    REQUIRE(repo.submission(id).phase == "MARKED");
    REQUIRE(repo.assessment(id)->moderator == 0);
    REQUIRE_THROWS_AS(service.transition(hod, id, 3, "RELEASE", "Old approval"), Error);
    REQUIRE_THROWS_AS(service.transition(hod, id, 3, "REOPEN", "Not released"), Error);
    REQUIRE_THROWS_AS(service.transition(hod, id, 3, "UNKNOWN", "Bad command"), Error);
}
TEST_CASE_METHOD(Fixture, "IT01 AT19 persistent backup reopens with work and audit intact",
                 "[integration][FR17][FR19]") {
    auto id = released();
    auto path = tempPath(".db");
    std::filesystem::remove(path);
    admin.backup(root, path);
    REQUIRE_THROWS_AS(admin.backup(root, path), Error);
    REQUIRE_THROWS_AS(admin.backup(teacher, tempPath("-denied.db")), Error);
    {
        SqliteRepository restored(path);
        REQUIRE(restored.latestVersion(id, true).content == Bytes{1, 2, 3});
        REQUIRE(restored.events(id).size() == 3);
        REQUIRE(restored.submission(id).phase == "RELEASED");
        AuthService secondAuth(restored, clock);
        auto t = secondAuth.login("student", password);
        AssessmentService second(restored, secondAuth, clock, penalty);
        REQUIRE(second.results(t)[0].final == 6500);
    }
    std::filesystem::remove(path);
}
TEST_CASE_METHOD(Fixture, "IT06 database rejects duplicate keys and immutable-history mutations",
                 "[integration][FR16]") {
    auto id = marked();
    auto path = tempPath("-constraints.db");
    std::filesystem::remove(path);
    admin.backup(root, path);
    sqlite3 *db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    auto execute = [&](const char *sql) {
        return sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    };
    REQUIRE(execute("PRAGMA foreign_keys=ON") == SQLITE_OK);
    REQUIRE(execute("UPDATE submission_versions SET filename='changed'") != SQLITE_OK);
    REQUIRE(execute("DELETE FROM submission_versions") != SQLITE_OK);
    REQUIRE(execute("UPDATE assessment_events SET reason='changed'") != SQLITE_OK);
    REQUIRE(execute("DELETE FROM assessment_events") != SQLITE_OK);
    REQUIRE(execute("UPDATE admin_events SET reason='changed'") != SQLITE_OK);
    REQUIRE(execute("DELETE FROM admin_events") != SQLITE_OK);
    REQUIRE(execute("INSERT INTO enrolments VALUES(99999,99999,1)") != SQLITE_OK);
    sqlite3_close(db);
    std::filesystem::remove(path);
    REQUIRE(repo.assessment(id)->raw == 6500);
}
TEST_CASE("IT07 invalid database and unsupported future schema fail safely",
          "[integration][FR17]") {
    auto invalid = tempPath("-invalid.db");
    {
        std::ofstream out(invalid);
        out << "This is not a SQLite database";
    }
    REQUIRE_THROWS_AS(SqliteRepository(invalid), Error);
    std::filesystem::remove(invalid);
    auto future = tempPath("-future.db");
    {
        SqliteRepository repo(future);
    }
    sqlite3 *db = nullptr;
    REQUIRE(sqlite3_open(future.c_str(), &db) == SQLITE_OK);
    REQUIRE(sqlite3_exec(db, "PRAGMA user_version=99", nullptr, nullptr, nullptr) == SQLITE_OK);
    sqlite3_close(db);
    REQUIRE_THROWS_AS(SqliteRepository(future), Error);
    std::filesystem::remove(future);
}
TEST_CASE_METHOD(Fixture, "CR01 CR06 class extension reaches all lists and later enrolments",
                 "[integration][CR1]") {
    service.extendClass(teacher, assignment, due + 300, "School event");
    for (const auto &token : {student, student2}) {
        auto view = service.assignments(token, course).at(0);
        REQUIRE(view.effective == due + 300);
        REQUIRE(view.wholeClass->reason == "School event");
        REQUIRE(view.assignment.deadline == due);
    }
    auto newId = admin.createUser(root, "laterstudent", "Later student", Role::Student, password);
    auto login = auth.login("laterstudent", password);
    auth.changePassword(login, password, "Later-student-99!");
    admin.enrol(root, course, newId, true);
    REQUIRE(service.assignments(login, course)[0].effective == due + 300);
    REQUIRE(service.assignments(teacher, course)[0].effective == due + 300);
}
TEST_CASE_METHOD(Fixture, "CR02 later individual accommodation always wins", "[integration][CR1]") {
    service.extendIndividual(teacher, assignment, sid, due + 500, "Accommodation");
    service.extendClass(teacher, assignment, due + 300, "School event");
    REQUIRE(service.assignments(student, course)[0].effective == due + 500);
    REQUIRE(service.assignments(student2, course)[0].effective == due + 300);
    service.extendClass(teacher, assignment, due + 600, "Longer school closure");
    REQUIRE(service.assignments(student, course)[0].effective == due + 600);
    REQUIRE_THROWS_AS(service.extendIndividual(teacher, assignment, sid, due + 550, "Shorter"),
                      Error);
}
TEST_CASE_METHOD(Fixture, "CR03 CR04 invalid or unauthorized class requests leave data unchanged",
                 "[integration][CR1]") {
    REQUIRE_THROWS_AS(service.extendClass(teacher2, assignment, due + 300, "Wrong teacher"), Error);
    REQUIRE_THROWS_AS(service.extendClass(student, assignment, due + 300, "Wrong role"), Error);
    REQUIRE_THROWS_AS(service.extendClass(teacher, assignment, due, "Not later"), Error);
    REQUIRE_THROWS_AS(service.extendClass(teacher, assignment, due + 300, ""), Error);
    REQUIRE_FALSE(repo.classExtension(assignment));
    service.extendClass(teacher, assignment, due + 300, "Valid");
    REQUIRE_THROWS_AS(service.extendClass(teacher, assignment, due + 200, "Shorter"), Error);
    REQUIRE(repo.classExtension(assignment)->deadline == due + 300);
}
TEST_CASE_METHOD(Fixture, "CR05 any first assessment freezes class extensions",
                 "[integration][CR1]") {
    auto id = marked();
    REQUIRE_THROWS_AS(service.extendClass(teacher, assignment, due + 300, "Too late"), Error);
    REQUIRE_FALSE(repo.classExtension(assignment));
    REQUIRE(repo.assessment(id)->raw == 6500);
}
TEST_CASE_METHOD(Fixture,
                 "CR07 extension reclassifies received work and reopens resubmission window",
                 "[integration][CR1]") {
    clock.value = due + 1;
    auto first = service.submit(student, assignment, "late.txt", Bytes{1});
    REQUIRE(first.received > first.deadlineAtReceipt);
    service.extendClass(teacher, assignment, due + 100, "Closure");
    auto view = service.assignments(student, course)[0];
    REQUIRE(view.receipt->received <= view.effective);
    REQUIRE(view.receipt->deadlineAtReceipt == due);
    auto replacement = service.submit(student, assignment, "revised.txt", Bytes{2});
    REQUIRE(replacement.number == 2);
    clock.value = due + 101;
    service.mark(teacher, first.submission, 2, 6500, "Good", "Initial");
    REQUIRE(repo.assessment(first.submission)->final == 6500);
    REQUIRE(repo.assessment(first.submission)->deadline == due + 100);
}
TEST_CASE_METHOD(Fixture, "CR08 additive v1 migration preserves existing results and audits",
                 "[integration][CR1]") {
    auto id = released();
    auto path = tempPath("-migration.db");
    std::filesystem::remove(path);
    admin.backup(root, path);
    // Remove only the additive v1.1 relation to reconstruct the verified v1.0 schema.
    sqlite3 *db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    REQUIRE(sqlite3_exec(db, "DROP TABLE class_extensions; PRAGMA user_version=1;", nullptr,
                         nullptr, nullptr) == SQLITE_OK);
    sqlite3_close(db);
    {
        SqliteRepository migrated(path);
        REQUIRE(migrated.events(id).size() == 3);
        REQUIRE(migrated.assessment(id)->final == 6500);
        REQUIRE(migrated.latestVersion(id, true).content == Bytes{1, 2, 3});
        REQUIRE_FALSE(migrated.classExtension(assignment));
    }
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt *stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "PRAGMA user_version", -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    REQUIRE(sqlite3_column_int(stmt, 0) == 2);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::filesystem::remove(path);
}

TEST_CASE_METHOD(Fixture, "IT08 failure after every assessment write rolls back the entire command",
                 "[integration][FR16][FR17][NFR05]") {
    auto id = submit();
    clock.value = due + 1;
    for (int stage = 1; stage <= 4; ++stage) {
        repo.failStage = stage;
        REQUIRE_THROWS_AS(service.mark(teacher, id, 1, 6000, "Feedback", "Fault probe"), Error);
        REQUIRE_FALSE(repo.assessment(id));
        REQUIRE(repo.submission(id).phase == "RECEIVED");
        REQUIRE(repo.submission(id).rowVersion == 1);
        REQUIRE(repo.events(id).empty());
    }
    repo.failStage = 0;
    service.mark(teacher, id, 1, 6000, "Feedback", "Recovery");
    REQUIRE(repo.assessment(id)->raw == 6000);
}
