#include "lms/Services.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>
using namespace lms;
namespace {
template <typename Action> double p95(Action action) {
    std::vector<double> samples;
    for (int i = 0; i < 100; ++i) {
        auto start = std::chrono::steady_clock::now();
        action();
        samples.push_back(
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                .count());
    }
    std::sort(samples.begin(), samples.end());
    return samples[94];
}
} // namespace
int main(int argc, char **argv) {
    try {
        require(argc == 2, "Supply a new benchmark database path.");
        SqliteRepository repo(argv[1]);
        require(repo.userCount() == 0, "Use an empty benchmark database.");
        SystemClock clock;
        TenPercentPenalty penalty;
        AuthService auth(repo, clock);
        const std::string password = "Synthetic-benchmark-42!";
        auth.bootstrap("admin", "Benchmark admin", password);
        auto hash = AuthService::hashPassword(password);
        UnitOfWork tx(repo);
        auto add = [&](const std::string &name, Role role) {
            UserRecord u;
            u.username = name;
            u.name = name;
            u.role = role;
            u.hash = hash;
            u.mustChange = false;
            return repo.addUser(u);
        };
        auto teacher = add("teacher", Role::Lecturer), head = add("head", Role::Hod);
        std::vector<Id> students;
        for (int i = 0; i < 800; ++i)
            students.push_back(add("student" + std::to_string(i), Role::Student));
        auto dept = repo.addDepartment("Benchmark department");
        repo.setHead(dept, head);
        Id firstCourse = 0;
        auto now = clock.now();
        for (int c = 0; c < 40; ++c) {
            auto course = repo.addCourse(
                {0, "C" + std::to_string(c), "Course " + std::to_string(c), dept, true});
            if (c == 0)
                firstCourse = course;
            repo.setLecturer(course, teacher, true);
            for (int s = 0; s < 20; ++s)
                repo.setEnrolment(course, students[c * 20 + s], true);
            for (int j = 0; j < 20; ++j) {
                auto aid = repo.addAssignment({0, course, teacher, "Task " + std::to_string(j),
                                               "Synthetic benchmark instructions", now - 10, 8000});
                for (int s = 0; s < 20; ++s) {
                    auto id = repo.addSubmission(aid, students[c * 20 + s]);
                    repo.addVersion({id, 1, now - 20, now - 10, "work.txt", Bytes(128, 'a')});
                    Assessment a;
                    a.submission = id;
                    a.version = 1;
                    a.raw = 6500;
                    a.final = 6500;
                    a.deadline = now - 10;
                    a.feedback = "Synthetic feedback";
                    a.marker = teacher;
                    a.revision = 3;
                    a.moderator = head;
                    a.moderatedAt = now - 5;
                    a.releasedBy = head;
                    a.releasedAt = now;
                    repo.saveAssessment(a);
                    repo.saveSubmission({id, aid, students[c * 20 + s], "RELEASED", 4});
                    for (int r = 1; r <= 3; ++r) {
                        AssessmentEvent e;
                        e.submission = id;
                        e.actor = r == 1 ? teacher : head;
                        e.revision = r;
                        e.at = now - 3 + r;
                        e.action = r == 1 ? "MARK" : r == 2 ? "MODERATE" : "RELEASE";
                        if (r > 1) {
                            e.oldRaw = 6500;
                            e.oldFinal = 6500;
                            e.oldFeedback = "Synthetic feedback";
                        }
                        e.newRaw = 6500;
                        e.newFinal = 6500;
                        e.newFeedback = "Synthetic feedback";
                        e.oldPhase = r == 1 ? "RECEIVED" : r == 2 ? "MARKED" : "MODERATED";
                        e.newPhase = r == 1 ? "MARKED" : r == 2 ? "MODERATED" : "RELEASED";
                        e.reason = "Synthetic benchmark fixture";
                        repo.addEvent(e);
                    }
                }
            }
        }
        tx.commit();
        AssessmentService service(repo, auth, clock, penalty);
        auto token = auth.login("student0", password);
        double login = p95([&] {
            auto t = auth.login("student0", password);
            auth.logout(t);
        });
        double list = p95([&] {
            require(service.assignments(token, firstCourse).size() == 20, "Invalid list fixture.");
        });
        double results =
            p95([&] { require(service.results(token).size() == 20, "Invalid result fixture."); });
        std::cout << "{\"students\":800,\"staff\":3,\"courses\":40,\"assignments\":800,"
                     "\"submissions\":16000,\"audit_events\":48000,\"attachment_bytes_each\":128,"
                     "\"trials_each\":100,\"login_p95_ms\":"
                  << login << ",\"assignment_list_p95_ms\":" << list
                  << ",\"results_p95_ms\":" << results
                  << ",\"hash_memory_mib\":64,\"sample\":\"sequential warm database; synthetic "
                     "shared fixture hash\"}\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
