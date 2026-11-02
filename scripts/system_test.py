"""Black-box four-role CLI scenario. Uses synthetic data in a disposable directory."""
import datetime as dt
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time

binary = str(Path(sys.argv[1]).resolve())
output = Path(sys.argv[2]).resolve()
output.mkdir(parents=True, exist_ok=True)
results = []
transcripts = []
password = 'Synthetic-pass-42!'
changed = 'Changed-pass-99!'
with tempfile.TemporaryDirectory(prefix='lms-system-') as folder:
    work = Path(folder)
    db = work / 'school.db'
    (work / 'work.txt').write_text('A reasoned solution, synthetic system-test work.')
    def run(label, lines, init=False):
        command = [binary, str(db)] + (['--init'] if init else [])
        done = subprocess.run(command, input='\n'.join(lines) + '\n', text=True,
                              capture_output=True, cwd=work, timeout=45)
        transcripts.append(f'=== {label} ===\n{done.stdout}\n{done.stderr}')
        if done.returncode:
            raise AssertionError(f'{label}: exit {done.returncode}: {done.stderr}')
        return done.stdout
    def check(label, condition):
        results.append({'test': label, 'passed': bool(condition)})
        if not condition:
            raise AssertionError(label)
    run('bootstrap', ['admin', 'School administrator', password], True)
    setup = ['login','admin',password]
    for name, role in [('teacher','LECTURER'),('head','HOD'),('student','STUDENT')]:
        setup += ['add-user',name,name,role,password]
    setup += ['add-department','Science','set-hod','1','3','add-course','SCI1','Science','1',
              'set-lecturer','1','2','y','enrol','1','4','y','logout']
    for name in ['teacher','head','student']:
        setup += ['login',name,password,'passwd',password,changed,'logout']
    setup += ['quit']
    text = run('administration and initial password changes', setup)
    check('SYS01 administration and forced password change', 'Created course 1' in text and 'Error:' not in text)
    # Ten seconds provides room for local input, hashing and scheduler jitter.
    deadline = int(time.time()) + 10
    local = dt.datetime.fromtimestamp(deadline, dt.timezone(dt.timedelta(hours=2))).strftime('%Y-%m-%d %H:%M:%S')
    text = run('assignment and on-time submission', ['login','teacher',changed,'create-assignment','1','Lab','Explain the result',local,'80','logout','login','student',changed,'submit','1',str(work/'work.txt'),'results','quit'])
    check('SYS02 receipt and hidden results', 'RECEIPT submission=1 version=1' in text and 'No released results.' in text and 'Error:' not in text)
    time.sleep(max(0, deadline - time.time() + 1.2))
    text = run('mark moderate release and student view', ['login','teacher',changed,'mark','1','1','65','Good reasoning','Initial marking','logout','login','student',changed,'results','logout','login','head',changed,'moderate','1','1','','','Checked','release','1','2','Publish','logout','login','student',changed,'results','submit','1',str(work/'work.txt'),'courses','quit'])
    check('SYS03 mark through release', 'Lab | 65.00 / 80.00 | Good reasoning' in text)
    check('SYS04 results withheld before moderation', 'No released results.' in text)
    check('SYS05 rejected resubmission recovers to usable menu', 'Resubmission is only allowed' in text and 'SCI1 | Science' in text)
    text = run('reopen and stale approval denial', ['login','head',changed,'reopen','1','3','Appeal','release','1','4','Stale approval','logout','login','student',changed,'results','quit'])
    check('SYS06 reopen hides and invalidates approval', 'Only a current moderated result' in text and 'No released results.' in text)
    text = run('invalid commands and numeric input', ['login','teacher',changed,'create-assignment','nonsense','unknown-command','whoami','quit'])
    check('SYS07 malformed input does not crash', 'Enter a positive whole number' in text and 'Unknown command' in text and 'LECTURER' in text)
    future = int(time.time()) + 86400
    def school(t):
        return dt.datetime.fromtimestamp(t, dt.timezone(dt.timedelta(hours=2))).strftime('%Y-%m-%d %H:%M:%S')
    text = run('CR1 class extension displayed to student', ['login','teacher',changed,'create-assignment','1','Class task','Explain CR1',school(future),'100','extend-class','2',school(future+86400),'School closure','logout','login','student',changed,'assignments','1','quit'])
    check('SYS08 class extension and reason visible', 'Class extension:' in text and 'School closure' in text and school(future+86400) in text and 'Error:' not in text)
(output/'system_results.json').write_text(json.dumps(results, indent=2)+'\n')
(output/'system_transcript.txt').write_text('\n'.join(transcripts))
print(f'Passed {len(results)} black-box system checks.')
