# Moderated Learning Management System

C++17 offline console application for CS4433. Roles: student, lecturer, department head and administrator. All result changes are audited and students see only released marks.

Repository: https://github.com/brotherslenka-17/CS4433-Moderated-LMS (private; the owner grants assessor access).

The compact hosted checkout restores pinned dependency bytes with `python3 scripts/fetch_dependencies.py` before building. The complete submission ZIP includes these bytes for offline builds. The script verifies SHA-256 checksums and makes no request when all files already match.

## Build on Linux

Install a C++17 compiler, CMake >=3.16, make and libsodium development files. On Ubuntu/Debian:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake libsodium-dev
python3 scripts/fetch_dependencies.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/lms school.db --init
./build/lms school.db
```

SQLite 3.50.4 and Catch2 2.13.10 are vendored. A libsodium 1.0.20 source archive is included; where compilers and CMake are already installed, `sh scripts/build_offline_linux.sh` builds without network access. Source compilation needs substantially more resources than the installed application.

## Windows (MSYS2 UCRT64)

Install MSYS2, then use its **UCRT64** terminal:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-libsodium
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-windows
ctest --test-dir build-windows --output-on-failure
./build-windows/lms.exe school.db --init
./build-windows/lms.exe school.db
```

Use a project path such as `C:/Users/Administrator/Documents/CS4433_LMS`. The Windows source path is supported by conditional console/time code; this package was executed on Linux, so Windows remains a target-machine validation step. Run from UCRT64 so dependency DLLs are available. Do not distribute an EXE alone without its required runtime DLLs and a clean-machine check.

## First use

`--init` works only on an empty database and prompts for a private administrator password. No default credentials exist. Passwords require 12-128 bytes. Administrator-created accounts must change their temporary password using `passwd` before any other task. In a real terminal, password input is hidden. Synthetic test passwords in tests and scripts are not school credentials.

Read `docs/USER_MANUAL.md` for role workflows, exact command prompts, deadline rules, recovery and the viva demonstration. Type `help` after login. Dates are `YYYY-MM-DD HH:MM:SS` in school time UTC+02:00; storage is UTC.

## Verification

```sh
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DLMS_COVERAGE=ON
cmake --build build-coverage --parallel 2
ctest --test-dir build-coverage --output-on-failure
python3 -m pip install gcovr==8.6
python3 -m gcovr --root . --filter 'src/(domain|application)/' --fail-under-line 70 --print-summary
cppcheck --enable=warning,performance,portability --error-exitcode=1 --std=c++17 --suppress=missingIncludeSystem -I include -I build-coverage/generated src
python3 scripts/system_test.py build-coverage/lms build-coverage/system-evidence
```

Coverage excludes UI, infrastructure and third-party code from the **business-layer** denominator. Integration tests still execute the repository. See `evidence/` for actual results; hosted CI is not claimed until a real workflow run exists.

## Configuration management

Use `main` for reviewed code and short-lived feature branches for changes. This individual project uses recorded self-review; no second-person approval or historical pull request is implied. Tags `v1.0` and `v1.1` identify actual local releases. The supplied Git bundle preserves commits and tags. To create a checkout from it:

```sh
git clone CS4433_LMS.bundle CS4433_LMS
cd CS4433_LMS
git log --oneline --all --decorate
```

The private repository https://github.com/brotherslenka-17/CS4433-Moderated-LMS exists. Upload of the full source was blocked by automatic approval review pending explicit authorization for that destination. It currently contains only its initial README. The local Git bundle is complete; no hosted CI pass is claimed. After authorized publication, record the actual Actions run URL and grant the assessor access.

## Data and limits

One interactive workstation session is the approved design baseline; 800 means stored students. No shared network-drive SQLite file. Work is stored as immutable BLOB versions, 1 byte to 10 MiB each. Total capacity depends on disk space and versions retained. The database is not encrypted by SQLite: protect the OS account, use disk encryption, restrict file access and encrypt removable backups. A privileged OS user can bypass application controls; this is not a remote tamper-proof server.

The assignment's optional released-results CSV export is deferred. Work export and consistent database backup are implemented. Restore by closing every LMS process, preserving the current database, copying a known-good backup to a new path and opening that path with the matching release. Never restore over a running database.
