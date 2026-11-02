"""Run with: python -m unittest discover -s tests -p test_gui.py -v"""
import sys
import tempfile
import time
import tkinter as tk
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from gui import LMSWindow


class GuiTests(unittest.TestCase):
    def setUp(self):
        self.root = tk.Tk()
        self.root.withdraw()
        self.app = LMSWindow(self.root)
        self.folder = tempfile.TemporaryDirectory()
        self.app.database.set(str(Path(self.folder.name) / 'school.db'))

    def tearDown(self):
        self.app.close()
        self.folder.cleanup()

    def wait_for(self, condition):
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            self.root.update()
            if condition():
                return
            time.sleep(0.01)
        self.fail('Timed out. Output: ' + self.app.output.get('1.0', 'end'))

    def answer(self, prompt, value):
        self.wait_for(lambda: self.app.prompt.get() == prompt and str(self.app.entry.cget('state')) == 'normal')
        self.app.entry.insert(0, value)
        self.app.answer()

    def test_role_menu_and_expired_session(self):
        self.app.line('Common: help, whoami, courses, assignments, passwd, logout, quit')
        self.app.line('Student: submit, export-work, results')
        self.assertIn('submit', self.app.commands)
        self.assertNotIn('mark', self.app.commands)
        self.app.line('Error: Session expired. Please log in.')
        self.assertEqual(self.app.commands, ['login'])

    def test_input_controls_fit_in_small_window(self):
        self.root.deiconify()
        self.root.geometry('800x600')
        self.root.update()
        for widget in (self.app.entry, self.app.next):
            bottom = widget.winfo_rooty() + widget.winfo_height()
            self.assertTrue(widget.winfo_viewable())
            self.assertLessEqual(bottom, self.root.winfo_rooty() + self.root.winfo_height())

    def test_choice_fields_and_all_role_menus(self):
        self.app.configure_input('Role [STUDENT/LECTURER/HOD/ADMIN]')
        self.assertEqual(tuple(self.app.entry.cget('values')), ('STUDENT', 'LECTURER', 'HOD', 'ADMIN'))
        self.assertEqual(str(self.app.entry.cget('state')), 'readonly')
        self.app.configure_input('Active [y/n]')
        self.assertEqual(tuple(self.app.entry.cget('values')), ('y', 'n'))
        self.app.configure_input('Password')
        self.assertEqual(self.app.entry.cget('show'), '*')
        self.assertEqual(tuple(self.app.entry.cget('values')), ())
        groups = [
            'Administrator: users, add-user, edit-user, reset-password, departments, add-department, rename-department, set-hod, add-course, edit-course, set-lecturer, enrol, roster, backup',
            'Lecturer: create-assignment, edit-assignment, extend, extend-class, work, export-work, mark, history',
            'HoD: work, export-work, moderate, release, return, reopen, history',
            'Student: submit, export-work, results',
        ]
        for group in groups:
            self.app.line('Common: help, whoami, courses, assignments, passwd, logout, quit')
            self.app.line(group)
            top = [self.app.menubar.entrycget(i, 'label') for i in range(self.app.menubar.index('end') + 1)]
            self.assertEqual(top[0], 'File')
            self.assertEqual(len(top), len(set(top)))
            items = [menu.entrycget(i, 'label') for menu in self.app.action_menus for i in range(menu.index('end') + 1)]
            self.assertEqual(len(items), len([c for c in self.app.commands if c not in ('help', 'quit')]))
            self.assertEqual(len(items), len(set(items)))

    def test_output_is_not_an_input_prompt(self):
        self.app.process = object()
        self.app.events.put('Base deadline: ')
        self.app.poll()
        self.app.process = None
        self.assertEqual(self.app.prompt.get(), '')

    def test_real_program_setup_login_and_recovery(self):
        if not self.app.executable.get():
            self.skipTest('Build the LMS executable first')
        self.app.start(True)
        self.answer('Initial administrator username', 'guiadmin')
        self.answer('Administrator display name', 'GUI test administrator')
        self.wait_for(lambda: 'password' in self.app.prompt.get().lower())
        self.assertEqual(self.app.entry.cget('show'), '*')
        self.answer('Administrator password (12-128 bytes)', 'Synthetic-pass-42!')
        self.wait_for(lambda: self.app.process is None)
        self.assertIn('Administrator created', self.app.output.get('1.0', 'end'))
        self.app.start()
        self.wait_for(lambda: self.app.command_ready)
        self.app.action_menus[0].invoke(0)
        self.answer('Username', 'guiadmin')
        self.answer('Password', 'Wrong-password-42!')
        self.wait_for(lambda: self.app.command_ready)
        self.assertNotIn('add-user', self.app.commands)
        self.app.send_command('login')
        self.answer('Username', 'guiadmin')
        self.answer('Password', 'Synthetic-pass-42!')
        self.wait_for(lambda: self.app.command_ready)
        self.assertIn('add-user', self.app.commands)
        self.assertEqual(self.app.indicator.cget('text'), 'Ready')
        self.app.send_command('add-user')
        self.answer('Username', 'guistudent')
        self.answer('Display name', 'GUI Student')
        self.wait_for(lambda: str(self.app.entry.cget('state')) == 'readonly')
        self.app.entry.set('STUDENT')
        self.app.answer()
        self.answer('Temporary password (12-128 bytes)', 'Synthetic-pass-42!')
        self.wait_for(lambda: self.app.command_ready)
        self.assertIn('Created user 2', self.app.output.get('1.0', 'end'))
        self.app.send_command('add-department')
        self.answer('Department name', 'Science')
        self.wait_for(lambda: self.app.command_ready)
        self.assertIn('Created department 1', self.app.output.get('1.0', 'end'))
        self.app.send_command('set-hod')
        self.answer('Department ID', 'not a number')
        self.wait_for(lambda: self.app.command_ready)
        self.assertIn('Enter a positive whole number', self.app.output.get('1.0', 'end'))
        self.app.send_command('logout')
        self.wait_for(lambda: self.app.command_ready)
        self.assertEqual(self.app.commands, ['login'])


if __name__ == '__main__':
    unittest.main()
