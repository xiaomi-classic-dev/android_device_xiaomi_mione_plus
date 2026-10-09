#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import os, shutil, sys
root, boot, local = map(Path, sys.argv[1:])
(root / 'sbin').mkdir(exist_ok=True)
original = root / 'init'
if original.is_symlink():
    stock = root / 'system/bin/init'
    original.unlink()
elif b'mione-lvm-init:' in original.read_bytes():
    stock = root / ('init.android' if (root / 'init.android').exists() else 'sbin/init.android')
else:
    stock = original
if b'mione-lvm-init:' in stock.read_bytes():
    raise ValueError('Stock recovery init was not preserved')
if stock != root / 'sbin/init.android':
    shutil.copyfile(str(stock), str(root / 'sbin/init.android'))
os.chmod(str(root / 'sbin/init.android'), 0o755)
for obsolete in ('init.android', 'lvm/lvm'):
    if (root / obsolete).exists(): (root / obsolete).unlink()
shutil.copyfile(str(boot / 'sbin/mione-lvm-recovery-init'), str(original))
shutil.copyfile(str(boot / 'sbin/mione-lvm'), str(root / 'sbin/mione-lvm'))
(root / 'lvm/etc').mkdir(parents=True, exist_ok=True)
shutil.copyfile(str(local / 'prebuilt/lvm'), str(root / 'sbin/mione-lvm2'))
shutil.copyfile(str(local / 'lvm.conf'), str(root / 'lvm/etc/lvm.conf'))
for n in ('init', 'sbin/mione-lvm', 'sbin/mione-lvm2'):
    os.chmod(str(root / n), 0o755)
shutil.copyfile(str(local / '../rootdir/etc/fstab.qcom'), str(root / 'system/etc/recovery.fstab.physical'))
prop = root / 'prop.default'
if not prop.exists(): prop = root / 'system/etc/prop.default'
if 'ro.mione.lvm.recovery=1' not in prop.read_text():
    with prop.open('a') as out: out.write('\nro.mione.lvm.recovery=1\n')
