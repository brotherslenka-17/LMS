"""Restore pinned vendor bytes for the compact GitHub checkout. Offline ZIP already includes them."""
import hashlib
import io
from pathlib import Path
import urllib.request
import zipfile

root = Path(__file__).resolve().parents[1] / 'vendor'
checks = dict((line.split()[1], line.split()[0]) for line in (root/'SHA256SUMS').read_text().splitlines())
def valid(name):
    p = root/name
    return p.exists() and hashlib.sha256(p.read_bytes()).hexdigest() == checks[name]
def save(name, data):
    if hashlib.sha256(data).hexdigest() != checks[name]:
        raise RuntimeError('Checksum mismatch: '+name)
    (root/name).write_bytes(data)
def get(url):
    with urllib.request.urlopen(url, timeout=60) as r:
        return r.read()
if any(not valid(n) for n in ['sqlite3.c','sqlite3.h','sqlite3ext.h']):
    with zipfile.ZipFile(io.BytesIO(get('https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip'))) as z:
        for n in ['sqlite3.c','sqlite3.h','sqlite3ext.h']:
            save(n,z.read('sqlite-amalgamation-3500400/'+n))
for name, url in {
 'catch.hpp':'https://raw.githubusercontent.com/catchorg/Catch2/v2.13.10/single_include/catch2/catch.hpp',
 'Catch2-LICENSE.txt':'https://raw.githubusercontent.com/catchorg/Catch2/v2.13.10/LICENSE.txt',
 'libsodium-1.0.20.tar.gz':'https://download.libsodium.org/libsodium/releases/libsodium-1.0.20.tar.gz'
}.items():
    if not valid(name):
        save(name,get(url))
print('All pinned vendor checksums verified.')
