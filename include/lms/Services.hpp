#pragma once
#include "lms/Repository.hpp"
#include <unordered_map>
namespace lms {
class AuthService {
  public:
    AuthService(IRepository &repo, IClock &clock);
    void bootstrap(const std::string &username, const std::string &name,
                   const std::string &password);
    std::string login(const std::string &username, const std::string &password);
    void logout(const std::string &token);
    Context authorize(const std::string &token, bool allowPasswordChange = false);
    bool mustChange(const std::string &token);
    void changePassword(const std::string &token, const std::string &oldPassword,
                        const std::string &newPassword);
    static std::string hashPassword(const std::string &password);

  private:
    struct Session {
        Id user{};
        TimePoint last{};
        std::string credential;
    };
    IRepository &repo_;
    IClock &clock_;
    std::unordered_map<std::string, Session> sessions_;
};
class AdminService {
  public:
    AdminService(IRepository &repo, AuthService &auth, IClock &clock)
        : repo_(repo), auth_(auth), clock_(clock) {}
    std::vector<UserRecord> users(const std::string &token);
    std::vector<Department> departments(const std::string &token);
    Id createUser(const std::string &token, const std::string &username, const std::string &name,
                  Role role, const std::string &password);
    void editUser(const std::string &token, Id user, const std::string &name, bool active);
    void resetPassword(const std::string &token, Id user, const std::string &password);
    Id createDepartment(const std::string &token, const std::string &name);
    void renameDepartment(const std::string &token, Id department, const std::string &name);
    void assignHead(const std::string &token, Id department, Id user);
    Id createCourse(const std::string &token, const std::string &code, const std::string &title,
                    Id department);
    void editCourse(const std::string &token, Id course, const std::string &title, bool active);
    void assignLecturer(const std::string &token, Id course, Id user, bool assigned);
    void enrol(const std::string &token, Id course, Id student, bool active);
    std::vector<std::pair<Id, bool>> enrolments(const std::string &token, Id course);
    std::vector<Id> lecturers(const std::string &token, Id course);
    void backup(const std::string &token, const std::string &path);

  private:
    Context admin(const std::string &token);
    UserRecord roleUser(Id id, Role role);
    void audit(Id actor, const std::string &entity, Id key, const std::string &action,
               const std::string &oldValue, const std::string &newValue);
    IRepository &repo_;
    AuthService &auth_;
    IClock &clock_;
};
class AssessmentService {
  public:
    AssessmentService(IRepository &repo, AuthService &auth, IClock &clock, ILatePenalty &penalty)
        : repo_(repo), auth_(auth), clock_(clock), penalty_(penalty) {}
    std::vector<Course> courses(const std::string &token);
    std::vector<AssignmentView> assignments(const std::string &token, Id course);
    Id createAssignment(const std::string &token, Id course, const std::string &title,
                        const std::string &instructions, TimePoint deadline, MarkUnits maximum);
    void editAssignment(const std::string &token, Id assignment, TimePoint deadline,
                        MarkUnits maximum);
    void extendIndividual(const std::string &token, Id assignment, Id student, TimePoint deadline,
                          const std::string &reason);
    void extendClass(const std::string &token, Id assignment, TimePoint deadline,
                     const std::string &reason);
    SubmissionVersion submit(const std::string &token, Id assignment, const std::string &filename,
                             const Bytes &content);
    std::vector<WorkView> work(const std::string &token, Id assignment);
    SubmissionVersion readWork(const std::string &token, Id submission);
    void mark(const std::string &token, Id submission, int expectedRow, MarkUnits raw,
              const std::string &feedback, const std::string &reason);
    void moderate(const std::string &token, Id submission, int expectedRevision,
                  std::optional<MarkUnits> raw, std::optional<std::string> feedback,
                  const std::string &reason);
    void transition(const std::string &token, Id submission, int expectedRevision,
                    const std::string &action, const std::string &reason);
    std::vector<ReleasedResult> results(const std::string &token);
    std::vector<AssessmentEvent> history(const std::string &token, Id submission);

  private:
    bool accessible(const Context &ctx, const Course &course);
    void lecturer(const Context &ctx, Id course);
    void hod(const Context &ctx, Id course);
    void workAccess(const Context &ctx, const Submission &submission, bool studentsAllowed);
    TimePoint deadline(const Assignment &assignment, Id student);
    void saveChange(const Context &ctx, Submission &submission, Assessment &assessment,
                    const std::optional<Assessment> &old, const std::string &oldPhase,
                    const std::string &action, const std::string &reason);
    IRepository &repo_;
    AuthService &auth_;
    IClock &clock_;
    ILatePenalty &penalty_;
};
} // namespace lms
