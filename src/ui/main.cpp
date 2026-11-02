#include "lms/Services.hpp"
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sodium.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#endif
namespace {
using namespace lms;
void prompt(const std::string &text) {
    std::cout << text << ": ";
    if (std::getenv("LMS_GUI_PROMPTS"))
        std::cout << '\x1e';
    std::cout << std::flush;
}
std::string input(const std::string &prompt) {
    ::prompt(prompt);
    std::string value;
    if (!std::getline(std::cin, value))
        throw Error("Input closed.");
    return value;
}
struct Secret {
    std::string value;
    explicit Secret(const std::string &prompt) {
        ::prompt(prompt);
#ifdef _WIN32
        HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
        DWORD oldMode = 0;
        bool tty = GetConsoleMode(handle, &oldMode) != 0;
        if (tty)
            SetConsoleMode(handle, oldMode & ~ENABLE_ECHO_INPUT);
#else
        termios oldMode{};
        bool tty = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &oldMode) == 0;
        if (tty) {
            auto mode = oldMode;
            mode.c_lflag &= static_cast<tcflag_t>(~ECHO);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &mode);
        }
#endif
        bool read = static_cast<bool>(std::getline(std::cin, value));
        if (tty) {
#ifdef _WIN32
            SetConsoleMode(handle, oldMode);
#else
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldMode);
#endif
            std::cout << '\n';
        }
        require(read, "Input closed.");
    }
    ~Secret() {
        if (!value.empty())
            sodium_memzero(value.data(), value.size());
    }
    Secret(const Secret &) = delete;
    Secret &operator=(const Secret &) = delete;
};
Id number(const std::string &prompt) {
    auto value = input(prompt);
    std::size_t consumed = 0;
    Id id = 0;
    try {
        id = std::stoll(value, &consumed);
    } catch (const std::exception &) {
        throw Error("Enter a positive whole number.");
    }
    require(consumed == value.size() && id > 0, "Enter a positive whole number.");
    return id;
}
int revision(const std::string &prompt) {
    auto n = number(prompt);
    require(n <= std::numeric_limits<int>::max(), "Revision too large.");
    return static_cast<int>(n);
}
bool yes(const std::string &prompt) {
    auto s = input(prompt + " [y/n]");
    require(s == "y" || s == "n", "Enter y or n.");
    return s == "y";
}
Bytes readFile(const std::string &path) {
    require(std::filesystem::is_regular_file(path), "Select a regular work file.");
    auto size = std::filesystem::file_size(path);
    require(size > 0 && size <= 10485760, "File must contain 1 byte to 10 MiB.");
    std::ifstream in(path, std::ios::binary);
    require(static_cast<bool>(in), "Cannot read file.");
    Bytes bytes(static_cast<std::size_t>(size));
    in.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(size));
    require(in.gcount() == static_cast<std::streamsize>(size) &&
                in.peek() == std::char_traits<char>::eof(),
            "File changed or could not be fully read. Retry.");
    return bytes;
}
void writeFile(const std::string &path, const Bytes &bytes) {
    require(!std::filesystem::exists(path), "Destination already exists; choose a new name.");
    std::ofstream out(path, std::ios::binary);
    require(static_cast<bool>(out), "Cannot open export destination.");
    out.write(reinterpret_cast<const char *>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
    out.close();
    require(static_cast<bool>(out), "File export failed.");
}
void help(Role role) {
    std::cout << "Common: help, whoami, courses, assignments, passwd, logout, quit\n";
    if (role == Role::Admin)
        std::cout << "Administrator: users, add-user, edit-user, reset-password, departments, "
                     "add-department, rename-department, set-hod, add-course, edit-course, "
                     "set-lecturer, enrol, roster, backup\n";
    if (role == Role::Lecturer)
        std::cout << "Lecturer: create-assignment, edit-assignment, extend, extend-class, work, "
                     "export-work, "
                     "mark, history\n";
    if (role == Role::Hod)
        std::cout << "HoD: work, export-work, moderate, release, return, reopen, history\n";
    if (role == Role::Student)
        std::cout << "Student: submit, export-work, results\n";
}
void adminCommand(const std::string &cmd, const std::string &token, AdminService &admin) {
    if (cmd == "users") {
        for (const auto &u : admin.users(token))
            std::cout << u.id << " | " << u.username << " | " << u.name << " | " << roleName(u.role)
                      << " | active=" << u.active << '\n';
    } else if (cmd == "add-user") {
        auto user = input("Username");
        auto name = input("Display name");
        auto role = parseRole(input("Role [STUDENT/LECTURER/HOD/ADMIN]"));
        Secret password("Temporary password (12-128 bytes)");
        std::cout << "Created user " << admin.createUser(token, user, name, role, password.value)
                  << '\n';
    } else if (cmd == "edit-user") {
        auto id = number("User ID");
        auto name = input("Display name");
        bool active = yes("Active");
        admin.editUser(token, id, name, active);
    } else if (cmd == "reset-password") {
        auto id = number("User ID");
        Secret password("New temporary password");
        admin.resetPassword(token, id, password.value);
    } else if (cmd == "departments") {
        for (const auto &d : admin.departments(token))
            std::cout << d.id << " | " << d.name << '\n';
    } else if (cmd == "add-department") {
        auto name = input("Department name");
        std::cout << "Created department " << admin.createDepartment(token, name) << '\n';
    } else if (cmd == "rename-department") {
        auto id = number("Department ID");
        auto name = input("New name");
        admin.renameDepartment(token, id, name);
    } else if (cmd == "set-hod") {
        auto department = number("Department ID");
        auto user = number("HoD user ID");
        admin.assignHead(token, department, user);
    } else if (cmd == "add-course") {
        auto code = input("Course code");
        auto title = input("Course title");
        auto department = number("Department ID");
        std::cout << "Created course " << admin.createCourse(token, code, title, department)
                  << '\n';
    } else if (cmd == "edit-course") {
        auto id = number("Course ID");
        auto title = input("Course title");
        bool active = yes("Active");
        admin.editCourse(token, id, title, active);
    } else if (cmd == "set-lecturer") {
        auto course = number("Course ID");
        auto user = number("Lecturer user ID");
        bool assigned = yes("Assigned");
        admin.assignLecturer(token, course, user, assigned);
    } else if (cmd == "enrol") {
        auto course = number("Course ID");
        auto student = number("Student user ID");
        bool active = yes("Active enrolment");
        admin.enrol(token, course, student, active);
    } else if (cmd == "roster") {
        auto course = number("Course ID");
        for (auto id : admin.lecturers(token, course))
            std::cout << "Lecturer " << id << '\n';
        for (auto [id, active] : admin.enrolments(token, course))
            std::cout << "Student " << id << " active=" << active << '\n';
    } else if (cmd == "backup") {
        auto path = input("New backup filename");
        admin.backup(token, path);
        std::cout << "Backup created. Keep it on access-controlled encrypted storage.\n";
    } else
        throw Error("Unknown command. Type help.");
}
void learningCommand(const std::string &cmd, const std::string &token, AssessmentService &service) {
    if (cmd == "courses") {
        for (const auto &c : service.courses(token))
            std::cout << c.id << " | " << c.code << " | " << c.title
                      << " | department=" << c.department << " | active=" << c.active << '\n';
    } else if (cmd == "assignments") {
        auto course = number("Course ID");
        for (const auto &v : service.assignments(token, course)) {
            const auto &a = v.assignment;
            std::cout << a.id << " | " << a.title << " | maximum=" << formatMark(a.maximum)
                      << "\nInstructions: " << a.instructions
                      << "\nBase deadline: " << formatLocalTime(a.deadline)
                      << "\nEffective deadline: " << formatLocalTime(v.effective) << '\n';
            if (v.wholeClass)
                std::cout << "Class extension: " << formatLocalTime(v.wholeClass->deadline)
                          << "\nClass reason: " << v.wholeClass->reason << '\n';
            if (v.individual)
                std::cout << "Individual extension: " << formatLocalTime(v.individual->deadline)
                          << "\nReason: " << v.individual->reason << '\n';
            if (v.submission)
                std::cout << "Submission " << v.submission->id << " | " << v.submission->phase
                          << '\n';
        }
    } else if (cmd == "create-assignment") {
        auto course = number("Course ID");
        auto title = input("Title");
        auto instructions = input("Instructions");
        auto due = parseLocalTime(input("Deadline YYYY-MM-DD HH:MM:SS (+02:00)"));
        auto maximum = parseMark(input("Maximum mark"));
        std::cout << "Created assignment "
                  << service.createAssignment(token, course, title, instructions, due, maximum)
                  << '\n';
    } else if (cmd == "edit-assignment") {
        auto id = number("Assignment ID");
        auto due = parseLocalTime(input("New base deadline (+02:00)"));
        auto maximum = parseMark(input("Maximum mark"));
        service.editAssignment(token, id, due, maximum);
    } else if (cmd == "extend") {
        auto assignment = number("Assignment ID");
        auto student = number("Student user ID");
        auto due = parseLocalTime(input("Extended deadline (+02:00)"));
        auto reason = input("Reason");
        service.extendIndividual(token, assignment, student, due, reason);
    } else if (cmd == "extend-class") {
        auto assignment = number("Assignment ID");
        auto due = parseLocalTime(input("Class deadline (+02:00)"));
        auto reason = input("Class extension reason");
        service.extendClass(token, assignment, due, reason);
    } else if (cmd == "submit") {
        auto assignment = number("Assignment ID");
        auto path = input("Work file path");
        auto content = readFile(path);
        auto receipt = service.submit(token, assignment, path, content);
        std::cout << "RECEIPT submission=" << receipt.submission << " version=" << receipt.number
                  << " at " << formatLocalTime(receipt.received) << " | "
                  << (receipt.received <= receipt.deadlineAtReceipt ? "ON TIME" : "LATE") << '\n';
    } else if (cmd == "work") {
        auto assignment = number("Assignment ID");
        for (const auto &w : service.work(token, assignment)) {
            std::cout << "Submission " << w.submission.id << " student=" << w.submission.student
                      << " phase=" << w.submission.phase << " row=" << w.submission.rowVersion
                      << " version=" << w.version.number
                      << " received=" << formatLocalTime(w.version.received) << '\n';
            if (w.assessment)
                std::cout << "Raw=" << formatMark(w.assessment->raw)
                          << " final=" << formatMark(w.assessment->final)
                          << " revision=" << w.assessment->revision
                          << " feedback=" << w.assessment->feedback << '\n';
        }
    } else if (cmd == "export-work") {
        auto id = number("Submission ID");
        auto v = service.readWork(token, id);
        auto path = input("New destination filename (never auto-opened)");
        writeFile(path, v.content);
    } else if (cmd == "mark") {
        auto id = number("Submission ID");
        auto row = revision("Expected row version from work list");
        auto raw = parseMark(input("Raw mark"));
        auto feedback = input("Written feedback");
        auto reason = input("Reason for this saved mark");
        service.mark(token, id, row, raw, feedback, reason);
    } else if (cmd == "moderate") {
        auto id = number("Submission ID");
        auto rev = revision("Expected assessment revision");
        auto rawText = input("Adjusted raw mark (blank keeps existing)");
        std::optional<MarkUnits> raw;
        if (!rawText.empty())
            raw = parseMark(rawText);
        auto fb = input("Adjusted feedback (blank keeps existing)");
        std::optional<std::string> feedback;
        if (!fb.empty())
            feedback = fb;
        auto reason = input("Moderation reason");
        service.moderate(token, id, rev, raw, feedback, reason);
    } else if (cmd == "release" || cmd == "return" || cmd == "reopen") {
        auto id = number("Submission ID");
        auto rev = revision("Expected assessment revision");
        auto reason = input("Reason");
        auto action = cmd == "release" ? "RELEASE" : cmd == "return" ? "RETURN" : "REOPEN";
        service.transition(token, id, rev, action, reason);
    } else if (cmd == "results") {
        auto results = service.results(token);
        if (results.empty())
            std::cout << "No released results.\n";
        for (const auto &r : results)
            std::cout << r.assignment << " | " << formatMark(r.final) << " / "
                      << formatMark(r.maximum) << " | " << r.feedback << '\n';
    } else if (cmd == "history") {
        auto id = number("Submission ID");
        for (auto &e : service.history(token, id))
            std::cout << "revision=" << e.revision << " actor=" << e.actor
                      << " at=" << formatLocalTime(e.at) << " action=" << e.action
                      << " raw=" << (e.oldRaw ? formatMark(*e.oldRaw) : "NULL") << " -> "
                      << formatMark(e.newRaw)
                      << " final=" << (e.oldFinal ? formatMark(*e.oldFinal) : "NULL") << " -> "
                      << formatMark(e.newFinal) << " phase=" << e.oldPhase << " -> " << e.newPhase
                      << " reason=" << e.reason
                      << "\nOld feedback: " << e.oldFeedback.value_or("NULL")
                      << "\nNew feedback: " << e.newFeedback << '\n';
    } else
        throw Error("Unknown command. Type help.");
}
} // namespace
int main(int argc, char **argv) {
    using namespace lms;
#ifndef _WIN32
    umask(0077);
#endif
    try {
        if (argc == 2 && std::string(argv[1]) == "--version") {
            std::cout << "Moderated LMS " << LMS_VERSION << '\n';
            return 0;
        }
        if (argc < 2 || argc > 3) {
            std::cout << "Usage: lms <database-file> [--init]\n       lms --version\n";
            return 0;
        }
        require(argc != 3 || std::string(argv[2]) == "--init", "Unknown option.");
        SqliteRepository repo(argv[1]);
        SystemClock clock;
        TenPercentPenalty penalty;
        AuthService auth(repo, clock);
        AdminService admin(repo, auth, clock);
        AssessmentService service(repo, auth, clock, penalty);
        if (argc == 3) {
            auto username = input("Initial administrator username");
            auto name = input("Administrator display name");
            Secret password("Administrator password (12-128 bytes)");
            auth.bootstrap(username, name, password.value);
            std::cout << "Administrator created. Restart without --init.\n";
            return 0;
        }
        require(repo.userCount() > 0, "No users. Run with --init to provision the administrator.");
        std::cout << "Moderated LMS " << LMS_VERSION
                  << " | school time UTC+02:00 | type login or quit\n";
        std::string token;
        while (true) {
            try {
                auto cmd = input("lms");
                if (cmd == "quit")
                    break;
                if (cmd == "login") {
                    require(token.empty(), "Log out first.");
                    auto name = input("Username");
                    Secret password("Password");
                    token = auth.login(name, password.value);
                    auto ctx = auth.authorize(token, true);
                    std::cout << "Welcome " << ctx.name << " (" << roleName(ctx.role) << ")\n";
                    if (auth.mustChange(token))
                        std::cout << "Temporary password: use passwd now.\n";
                    help(ctx.role);
                    continue;
                }
                if (cmd == "logout") {
                    auth.logout(token);
                    token.clear();
                    std::cout << "Logged out.\n";
                    continue;
                }
                if (cmd == "passwd") {
                    Secret oldPassword("Current password");
                    Secret newPassword("New password");
                    auth.changePassword(token, oldPassword.value, newPassword.value);
                    std::cout << "Password changed.\n";
                    continue;
                }
                auto ctx = auth.authorize(token);
                if (cmd == "help") {
                    help(ctx.role);
                    continue;
                }
                if (cmd == "whoami") {
                    std::cout << ctx.id << " | " << ctx.name << " | " << roleName(ctx.role) << '\n';
                    continue;
                }
                if (ctx.role == Role::Admin && cmd != "courses" && cmd != "assignments")
                    adminCommand(cmd, token, admin);
                else
                    learningCommand(cmd, token, service);
                std::cout << "OK\n";
            } catch (const std::exception &e) {
                if (!std::cin)
                    break;
                std::cout << "Error: " << e.what() << '\n';
                try {
                    if (!token.empty())
                        auth.authorize(token, true);
                } catch (const Error &) {
                    token.clear();
                }
            }
        }
        auth.logout(token);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Startup error: " << e.what() << '\n';
        return 1;
    }
}
