# Checks completed — 25 October 2026

Built and tested on this Windows computer with MSYS2 UCRT64.

- C++ tests: 31 cases, 289 assertions, no failures.
- System checks: all 8 passed, covering account setup, submissions, marking,
  moderation, publication, reopening, invalid input and class extensions.
- GUI tests: all 3 passed, covering real setup and login, wrong passwords,
  department creation, invalid numbers, logout, expired-session recovery
  and distinguishing output from input prompts.
- Python syntax check passed. No resource warnings in the final GUI run.

Fixed executable detection, expired-session menus, file-picker controls and
process cleanup. Added explicit prompt markers so normal output cannot be
mistaken for a question. Terminal use still works normally.

Start with Open_GUI.bat. The Windows executable and its four dependency DLLs
are in build-windows. Test databases were temporary; no school data was used.

These checks cover the tested workflows. They do not prove every possible
input or replace a person trying the full interface.
