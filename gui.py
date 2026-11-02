"""Small desktop interface for the existing LMS executable. Python 3.9+."""
import codecs
import os
from pathlib import Path
import queue
import subprocess
import threading
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

ROOT = Path(__file__).resolve().parent
# Only real input prompts may open a field; output may also contain ': '.
PROMPTS = frozenset(['Active [y/n]',
 'Active enrolment [y/n]',
 'Adjusted feedback (blank keeps existing)',
 'Adjusted raw mark (blank keeps existing)',
 'Administrator display name',
 'Administrator password (12-128 bytes)',
 'Assigned [y/n]',
 'Assignment ID',
 'Class deadline (+02:00)',
 'Class extension reason',
 'Course ID',
 'Course code',
 'Course title',
 'Current password',
 'Deadline YYYY-MM-DD HH:MM:SS (+02:00)',
 'Department ID',
 'Department name',
 'Display name',
 'Expected assessment revision',
 'Expected row version from work list',
 'Extended deadline (+02:00)',
 'HoD user ID',
 'Initial administrator username',
 'Instructions',
 'Lecturer user ID',
 'Maximum mark',
 'Moderation reason',
 'New backup filename',
 'New base deadline (+02:00)',
 'New destination filename (never auto-opened)',
 'New name',
 'New password',
 'New temporary password',
 'Password',
 'Raw mark',
 'Reason',
 'Reason for this saved mark',
 'Role [STUDENT/LECTURER/HOD/ADMIN]',
 'Student user ID',
 'Submission ID',
 'Temporary password (12-128 bytes)',
 'Title',
 'User ID',
 'Username',
 'Work file path',
 'Written feedback',
 'lms'])
LABELS = {
    'login': 'Sign in', 'logout': 'Sign out', 'passwd': 'Change password',
    'whoami': 'My account', 'work': 'View submissions', 'submit': 'Submit work',
    'results': 'My results', 'moderate': 'Review marks', 'release': 'Publish result',
    'return': 'Return for changes', 'reopen': 'Reopen result',
    'extend': 'Extend one deadline', 'extend-class': 'Extend class deadline',
    'set-hod': 'Assign department head', 'enrol': 'Enrol student',
}
MENU_GROUPS = {
    'Account': {'login', 'logout', 'passwd', 'whoami'},
    'Learning': {'courses', 'assignments', 'submit', 'results', 'export-work'},
    'Assessment': {'create-assignment', 'edit-assignment', 'extend', 'extend-class',
                   'work', 'mark', 'moderate', 'release', 'return', 'reopen', 'history'},
}


def label(command):
    return LABELS.get(command, command.replace('-', ' ').capitalize())


class LMSWindow:
    def __init__(self, window):
        self.window = window
        self.process = None
        self.events = queue.Queue()
        self.pending = ''
        self.command_ready = False
        self.last_error = False
        self.window.title('School LMS')
        self.window.geometry('1020x720')
        self.window.minsize(800, 600)
        style = ttk.Style()
        style.theme_use('clam')
        style.configure('TFrame', background='#f4f6fa')
        style.configure('TLabel', background='#f4f6fa', font=('Segoe UI', 10))
        style.configure('TButton', font=('Segoe UI', 10), padding=7)
        style.configure('Title.TLabel', font=('Segoe UI', 23, 'bold'))
        frame = ttk.Frame(window, padding=22)
        frame.pack(fill='both', expand=True)
        ttk.Label(frame, text='School LMS', style='Title.TLabel').pack(anchor='w')
        ttk.Label(frame, text='Courses, assignments and results in one place.').pack(anchor='w', pady=(0, 16))
        dashboard = ttk.Frame(frame)
        dashboard.pack(fill='x', pady=(0, 10))
        self.indicator = tk.Label(dashboard, text='Disconnected', bg='#e2e8f0', fg='#334155',
                                  font=('Segoe UI', 10, 'bold'), padx=10, pady=4)
        self.indicator.pack(side='left')
        self.activity = tk.StringVar(value='Welcome — open your database to begin')
        ttk.Label(dashboard, textvariable=self.activity).pack(side='left', padx=12)
        self.progress = ttk.Progressbar(dashboard, mode='indeterminate', length=90)
        self.progress.pack(side='right')
        self.menubar = tk.Menu(window, tearoff=False)
        window.configure(menu=self.menubar)
        file_menu = tk.Menu(self.menubar, tearoff=False)
        file_menu.add_command(label='Open database', command=self.start)
        file_menu.add_command(label='First-time setup', command=lambda: self.start(True))
        file_menu.add_separator()
        file_menu.add_command(label='Exit', command=self.close)
        self.menubar.add_cascade(label='File', menu=file_menu)
        self.action_menus = []
        settings = ttk.LabelFrame(frame, text='Open your school database', padding=10)
        settings.pack(fill='x')
        self.executable = tk.StringVar(value=self.find_executable())
        self.database = tk.StringVar(value=str(ROOT / 'school.db'))
        for row, (title, variable, picker) in enumerate([
            ('LMS program', self.executable, self.pick_program),
            ('Database', self.database, self.pick_database),
        ]):
            ttk.Label(settings, text=title).grid(row=row, column=0, sticky='w', padx=(0, 10))
            ttk.Entry(settings, textvariable=variable).grid(row=row, column=1, sticky='ew', pady=3)
            ttk.Button(settings, text='Browse', command=picker).grid(row=row, column=2, padx=5)
        settings.columnconfigure(1, weight=1)
        self.connect = ttk.Button(settings, text='Open database', command=self.start)
        self.connect.grid(row=2, column=1, sticky='w', pady=5)
        self.setup = ttk.Button(settings, text='First-time setup', command=lambda: self.start(True))
        self.setup.grid(row=2, column=2)
        body = ttk.Frame(frame)
        body.pack(fill='both', expand=True, pady=14)
        navigation = ttk.Frame(body, width=250)
        navigation.pack(side='left', fill='y', padx=(0, 16))
        navigation.pack_propagate(False)
        canvas = tk.Canvas(navigation, width=225, background='#f4f6fa', highlightthickness=0)
        scrollbar = ttk.Scrollbar(navigation, orient='vertical', command=canvas.yview)
        scrollbar.pack(side='right', fill='y')
        canvas.pack(side='left', fill='both', expand=True)
        canvas.configure(yscrollcommand=scrollbar.set)
        self.menu = ttk.Frame(canvas)
        menu_id = canvas.create_window((0, 0), window=self.menu, anchor='nw')
        self.menu.bind('<Configure>', lambda event: canvas.configure(scrollregion=canvas.bbox('all')))
        canvas.bind('<Configure>', lambda event: canvas.itemconfigure(menu_id, width=event.width))
        right = ttk.Frame(body)
        right.pack(side='left', fill='both', expand=True)
        right.columnconfigure(0, weight=1)
        right.rowconfigure(1, weight=1)
        self.status = tk.StringVar(value='Choose the LMS program, then open a database.')
        ttk.Label(right, textvariable=self.status, wraplength=610).grid(row=0, column=0, sticky='w', pady=(0, 8))
        self.output = tk.Text(right, wrap='word', font=('Segoe UI', 11), background='white',
                              foreground='#243047', relief='flat', padx=12, pady=12, state='disabled', height=8, width=40)
        self.output.grid(row=1, column=0, sticky='nsew')
        self.prompt = tk.StringVar(value='')
        ttk.Label(right, textvariable=self.prompt, wraplength=610).grid(row=2, column=0, sticky='w', pady=(12, 4))
        entry_row = ttk.Frame(right)
        entry_row.grid(row=3, column=0, sticky='ew')
        self.entry = ttk.Combobox(entry_row, font=('Segoe UI', 11), state='disabled')
        self.entry.pack(side='left', fill='x', expand=True)
        self.entry.bind('<Return>', lambda event: self.answer())
        self.browse = ttk.Button(entry_row, text='Choose file', command=self.pick_work, state='disabled')
        self.browse.pack(side='left', padx=5)
        self.next = ttk.Button(entry_row, text='Continue', command=self.answer, state='disabled')
        self.next.pack(side='left')
        self.set_menu(['login'])
        window.protocol('WM_DELETE_WINDOW', self.close)
        window.after(50, self.poll)

    @staticmethod
    def find_executable():
        for relative in ['build-windows/lms.exe', 'build/Release/lms.exe', 'build/lms.exe', 'build/lms']:
            candidate = ROOT / relative
            if candidate.is_file():
                return str(candidate)
        return ''

    def pick_program(self):
        path = filedialog.askopenfilename(title='Choose the built LMS program')
        if path:
            self.executable.set(path)

    def pick_database(self):
        path = filedialog.asksaveasfilename(title='Choose or name a database', defaultextension='.db',
                                          filetypes=[('School database', '*.db'), ('All files', '*.*')],
                                          confirmoverwrite=False)
        if path:
            self.database.set(path)

    def start(self, initialize=False):
        if self.process is not None:
            return
        program = Path(self.executable.get())
        if not program.is_file():
            messagebox.showinfo('Choose a program', 'Build the C++ LMS first, then choose its program file.')
            return
        if not self.database.get().strip():
            messagebox.showinfo('Choose a database', 'Choose a database file first.')
            return
        args = [str(program.resolve()), str(Path(self.database.get()).resolve())]
        if initialize:
            args.append('--init')
        try:
            self.process = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                            stderr=subprocess.STDOUT, cwd=ROOT, bufsize=0,
                                            env=dict(os.environ, LMS_GUI_PROMPTS='1'),
                                            creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        except OSError as error:
            messagebox.showerror('Could not open LMS', str(error))
            return
        self.pending = ''
        self.command_ready = False
        self.last_error = False
        self.set_indicator('Working', '#dbeafe', '#1e40af', busy=True)
        self.set_menu(['login'])
        self.connect.configure(state='disabled')
        self.setup.configure(state='disabled')
        self.status.set('Connected. Follow the steps below.')
        self.activity.set('Create your administrator account' if initialize else 'Database open — sign in to begin')
        self.reader = threading.Thread(target=self.read_output, args=(self.process,), daemon=True)
        self.reader.start()

    def read_output(self, process):
        decoder = codecs.getincrementaldecoder('utf-8')(errors='replace')
        while True:
            data = process.stdout.read(1)
            if not data:
                break
            text = decoder.decode(data)
            if text:
                self.events.put(text)
        self.events.put(None)

    def set_menu(self, commands):
        for child in self.menu.winfo_children():
            child.destroy()
        self.commands = commands
        for _ in self.action_menus:
            self.menubar.delete(1)
        self.action_menus = []
        groups = {name: [c for c in commands if c in members] for name, members in MENU_GROUPS.items()}
        groups['Administration'] = [c for c in commands if c not in set().union(*MENU_GROUPS.values()) and c not in ('help', 'quit')]
        for name, actions in groups.items():
            if not actions:
                continue
            menu = tk.Menu(self.menubar, tearoff=False)
            for command in actions:
                menu.add_command(label=label(command), command=lambda c=command: self.send_command(c), state='disabled')
            self.menubar.add_cascade(label=name, menu=menu)
            self.action_menus.append(menu)
        ttk.Label(self.menu, text='What would you like to do?').pack(anchor='w', pady=(0, 8))
        self.buttons = []
        for command in commands:
            if command in ('help', 'quit'):
                continue
            button = ttk.Button(self.menu, text=label(command), command=lambda c=command: self.send_command(c))
            button.pack(fill='x', pady=1)
            self.buttons.append(button)

    def set_indicator(self, text, background, foreground, busy=False):
        self.indicator.configure(text=text, bg=background, fg=foreground)
        if busy:
            self.progress.start(12)
        else:
            self.progress.stop()

    def configure_input(self, prompt):
        choices = ()
        if prompt.endswith('[y/n]'):
            choices = ('y', 'n')
        elif prompt == 'Role [STUDENT/LECTURER/HOD/ADMIN]':
            choices = ('STUDENT', 'LECTURER', 'HOD', 'ADMIN')
        self.entry.configure(values=choices, show='*' if 'password' in prompt.lower() else '',
                             state='disabled' if prompt == 'lms' else 'readonly' if choices else 'normal')
        if choices:
            self.entry.set('')

    def append(self, text):
        self.output.configure(state='normal')
        self.output.insert('end', text + '\n')
        self.output.see('end')
        self.output.configure(state='disabled')

    def line(self, text):
        if text.startswith(('Error:', 'Startup error:')):
            self.last_error = True
            self.set_indicator('Needs attention', '#fee2e2', '#991b1b')
        if text.startswith('Welcome '):
            self.activity.set(text)
        if ': ' in text and text.split(': ', 1)[0] in ('Common', 'Administrator', 'Lecturer', 'HoD', 'Student'):
            group, commands = text.split(': ', 1)
            new = [c.strip() for c in commands.split(',')]
            self.set_menu(new if group == 'Common' else list(dict.fromkeys(self.commands + new)))
        else:
            self.append('Done.' if text == 'OK' else text)
        if text == 'Logged out.' or (text.startswith('Error:') and 'Please log in.' in text):
            self.set_menu(['login'])
            self.activity.set('Sign in to use your account')

    def poll(self):
        while True:
            try:
                text = self.events.get_nowait()
            except queue.Empty:
                break
            if text is None:
                if self.pending.strip():
                    self.line(self.pending.rstrip('\r\n'))
                self.pending = ''
                if self.process:
                    self.process.wait()
                    self.process.stdin.close()
                    self.process.stdout.close()
                self.status.set('LMS closed. You can open the database again.')
                self.process = None
                self.command_ready = False
                self.set_indicator('Disconnected', '#e2e8f0', '#334155')
                self.activity.set('Open your database to begin')
                self.prompt.set('')
                self.connect.configure(state='normal')
                self.setup.configure(state='normal')
                self.entry.configure(state='disabled')
                self.next.configure(state='disabled')
                self.browse.configure(state='disabled')
                self.set_menu(['login'])
                continue
            self.pending += text
            while '\n' in self.pending:
                line, self.pending = self.pending.split('\n', 1)
                self.line(line.rstrip('\r'))
        if self.process and self.pending.endswith(': \x1e') and self.pending[:-3] in PROMPTS:
            prompt = self.pending[:-3]
            self.pending = ''
            self.command_ready = prompt == 'lms'
            self.prompt.set('' if self.command_ready else prompt)
            self.configure_input(prompt)
            self.next.configure(state='disabled' if self.command_ready else 'normal')
            self.browse.configure(state='normal' if any(s in prompt.lower() for s in ('file path', 'filename')) else 'disabled')
            self.status.set('Choose an action.' if self.command_ready else 'Fill in this field, then select Continue.')
            if self.last_error and self.command_ready:
                self.set_indicator('Check the message below', '#fee2e2', '#991b1b')
            elif self.command_ready:
                self.set_indicator('Ready', '#dcfce7', '#166534')
            else:
                self.set_indicator('Waiting for your answer', '#fef3c7', '#92400e')
            if prompt.endswith('[y/n]'):
                self.status.set('Choose y for Yes or n for No, then select Continue.')
            elif 'username' in prompt.lower():
                self.status.set('Use your username, not your display name. No spaces.')
            elif 'deadline' in prompt.lower():
                self.status.set('Enter YYYY-MM-DD HH:MM:SS. School time is UTC+02:00.')
            if not self.command_ready:
                self.entry.focus_set()
        for button in self.buttons:
            button.configure(state='normal' if self.process and self.command_ready else 'disabled')
        for menu in self.action_menus:
            for index in range(menu.index('end') + 1):
                menu.entryconfigure(index, state='normal' if self.process and self.command_ready else 'disabled')
        self.window.after(50, self.poll)

    def write(self, value):
        if not self.process or self.process.poll() is not None:
            return
        try:
            self.process.stdin.write((value + '\n').encode('utf-8'))
            self.process.stdin.flush()
        except (OSError, BrokenPipeError):
            self.status.set('Connection closed. Open the database again.')

    def send_command(self, command):
        if self.command_ready:
            self.command_ready = False
            self.last_error = False
            self.activity.set(label(command))
            self.set_indicator('Working', '#dbeafe', '#1e40af', busy=True)
            self.append('\n' + label(command))
            self.write(command)

    def answer(self):
        if str(self.entry.cget('state')) not in ('normal', 'readonly'):
            return
        value = self.entry.get()
        if self.entry.cget('values') and value not in self.entry.cget('values'):
            self.status.set('Choose an option from the list first.')
            return
        if '\n' in value or '\r' in value:
            messagebox.showinfo('One line only', 'Please enter one line at a time.')
            return
        self.entry.set('')
        self.set_indicator('Working', '#dbeafe', '#1e40af', busy=True)
        self.entry.configure(state='disabled')
        self.next.configure(state='disabled')
        self.browse.configure(state='disabled')
        self.write(value)

    def pick_work(self):
        if str(self.entry.cget('state')) != 'normal':
            return
        save = 'filename' in self.prompt.get().lower()
        path = filedialog.asksaveasfilename() if save else filedialog.askopenfilename()
        if path:
            self.entry.delete(0, 'end')
            self.entry.insert(0, path)

    def close(self):
        if self.process and self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        if self.process:
            self.reader.join(timeout=2)
            self.process.stdin.close()
            self.process.stdout.close()
        self.window.destroy()


if __name__ == '__main__':
    LMSWindow(tk.Tk()).window.mainloop()
