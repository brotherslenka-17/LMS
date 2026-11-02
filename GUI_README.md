# School LMS window

Double-click **Open_GUI.bat** on Windows. Python 3 with Tkinter is needed.

1. The included Windows program is selected automatically (`build-windows/lms.exe`).
2. Choose a database file.
3. For a new database, click **First-time setup** to create an administrator.
4. Click **Open database**, then **Sign in**.

Choose an action on the left. Fill in each field and click **Continue**.
The buttons match your account role. Dates use school time (UTC+02:00).

You can also use the top menus: Account, Learning, Assessment and Administration.
Only options available to your role appear. Role and Yes/No fields have dropdown
choices. The status indicator shows Ready, Working, Waiting for your answer, or
an error message. Sign in with your username, not your display name.

The window uses the existing C++ program for login, permissions, marks and saving.
It does not change the database directly. Keep the four runtime DLLs beside
`build-windows/lms.exe`. Use this updated program with the GUI; older builds
do not include its input-prompt support. See README.md if you need to rebuild.
