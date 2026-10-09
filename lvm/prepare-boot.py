#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
import os
import shutil
import sys
from pathlib import Path
root, local = map(Path, sys.argv[1:])
# Keep the stock init in the generated ramdisk. Never edit system/core/init.
wrapper = root / 'sbin/mione-lvm-init'
stock = root / 'init'
original = stock
if b'mione-lvm-init:' in original.read_bytes():
    original = root / ('init.android' if (root / 'init.android').exists() else 'sbin/init.android')
if b'mione-lvm-init:' in original.read_bytes():
    raise ValueError('Stock init was not preserved')
if original != root / 'sbin/init.android':
    shutil.copyfile(str(original), str(root / 'sbin/init.android'))
os.chmod(str(root / 'sbin/init.android'), 0o755)
for obsolete in ('init.android', 'lvm/lvm'):
    if (root / obsolete).exists(): (root / obsolete).unlink()
shutil.copyfile(str(wrapper), str(stock))
os.chmod(str(stock), 0o755)
shutil.copyfile(str(local / '../rootdir/etc/fstab.qcom'), str(root / 'fstab.qcom.physical'))
(root / 'lvm/etc').mkdir(parents=True, exist_ok=True)
shutil.copyfile(str(local / 'prebuilt/lvm'), str(root / 'sbin/mione-lvm2'))
os.chmod(str(root / 'sbin/mione-lvm2'), 0o755)
shutil.copyfile(str(local / 'lvm.conf'), str(root / 'lvm/etc/lvm.conf'))
