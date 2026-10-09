#!/usr/bin/env python
# SPDX-License-Identifier: Apache-2.0
# BOARD_RECOVERY_IMAGE_PREPARE runs after TWRP's stock resources are copied.
# Generate the device extension in the ramdisk; never edit the TWRP checkout.
from __future__ import print_function
import os
import shutil
import hashlib
import re
import struct
import sys
import xml.etree.ElementTree as ET

source, root, local = sys.argv[1:]
def save(tree, path):
    tree.write(path, encoding='utf-8', xml_declaration=True)

extension = ET.parse(os.path.join(local, 'pages.xml')).getroot()
minimum = re.search(r'MIONE_SYSTEM_MIN_MIB\s+(\d+)', open(os.path.join(local, 'capacity.h')).read()).group(1)
portrait = ET.parse(source)
pages = portrait.getroot().find('pages')
advanced = next(p for p in pages if p.get('name') == 'advanced')
listing = next(n for n in advanced if n.tag == 'listbox' and n.get('style') == 'advanced_listbox')
for index, item in enumerate(extension.findall('listitem'), 1):
    listing.insert(index, item)
for page in extension.find('pages'):
    for node in page.iter():
        for key, value in list(node.attrib.items()):
            node.set(key, value.replace('@MIN_MIB@', minimum))
        if node.text:
            node.text = node.text.replace('@MIN_MIB@', minimum)
    pages.append(page)
save(portrait, os.path.join(root, 'twres', 'portrait.xml'))
ui_path = os.path.join(root, 'twres', 'ui.xml')
ui = ET.parse(ui_path)
variables = ui.getroot().find('variables')
for name, value in [('mione_system_mib', '1024'), ('mione_confirm', ''), ('mione_ready', '0'),
                    ('mione_phase', 'idle'), ('mione_restore', '0'), ('mione_mode', 'apply'),
                    ('mione_rollback_ready', '0'), ('mione_return', 'mione_resize'),
                    ('mione_raw_system', '0'), ('mione_raw_data', '0')]:
    old = next((v for v in variables if v.get('name') == name), None)
    if old is not None:
        variables.remove(old)
    ET.SubElement(variables, 'variable', name=name, value=value)
save(ui, ui_path)
for language in ('en', 'zh_CN'):
    lang_path = os.path.join(root, 'twres', 'languages', language + '.xml')
    lang = ET.parse(lang_path)
    resources = lang.getroot().find('resources')
    for string in ET.parse(os.path.join(local, language + '.xml')).getroot():
        if string.text:
            string.text = string.text.replace('@MIN_MIB@', minimum)
        old = next((s for s in resources if s.get('name') == string.get('name')), None)
        if old is not None:
            resources.remove(old)
        resources.append(string)
    save(lang, lang_path)
obsolete = os.path.join(root, 'sbin', 'mione-repartition')
if os.path.lexists(obsolete):
    os.unlink(obsolete)
# Wrap only the generated recovery ramdisk, retaining Android init byte-for-byte.
original = os.path.join(root, 'init')
if b'mione-lvm-init:' in open(original, 'rb').read():
    original = os.path.join(root, 'init.android') if os.path.exists(os.path.join(root, 'init.android')) else os.path.join(root, 'sbin', 'init.android')
if b'mione-lvm-init:' in open(original, 'rb').read():
    raise ValueError('Stock Android init was not preserved')
if original != os.path.join(root, 'sbin', 'init.android'):
    shutil.copyfile(original, os.path.join(root, 'sbin', 'init.android'))
os.chmod(os.path.join(root, 'sbin', 'init.android'), 0o755)
for obsolete in ('init.android', 'lvm/lvm'):
    path = os.path.join(root, obsolete)
    if os.path.lexists(path): os.unlink(path)
shutil.copyfile(os.path.join(root, 'sbin', 'mione-lvm-init'), os.path.join(root, 'init'))
os.chmod(os.path.join(root, 'init'), 0o755)
if 'ro.mione.lvm.recovery=1' not in open(os.path.join(root, 'default.prop')).read():
    with open(os.path.join(root, 'default.prop'), 'a') as stream:
        stream.write('\nro.mione.lvm.recovery=1\n')
shutil.copyfile(os.path.join(root, 'etc', 'recovery.fstab'), os.path.join(root, 'etc', 'recovery.fstab.physical'))
os.makedirs(os.path.join(root, 'lvm', 'etc')) if not os.path.isdir(os.path.join(root, 'lvm', 'etc')) else None
shutil.copyfile(os.path.join(local, 'prebuilt', 'lvm'), os.path.join(root, 'sbin', 'mione-lvm2'))
os.chmod(os.path.join(root, 'sbin', 'mione-lvm2'), 0o755)
shutil.copyfile(os.path.join(local, 'lvm.conf'), os.path.join(root, 'lvm', 'etc', 'lvm.conf'))
settings_hook = os.path.join(root, 'sbin', 'postrecoveryboot.sh')
shutil.copyfile(os.path.join(local, 'postrecoveryboot.sh'), settings_hook)
os.chmod(settings_hook, 0o755)
# Match the current TWRP InfoManager format without changing recovery code.
data_source = os.path.join(os.path.dirname(source), '..', '..', '..', 'data.cpp')
settings_version = int(re.search(r'#define\s+FILE_VERSION\s+(0x[0-9a-fA-F]+)',
                                 open(data_source).read()).group(1), 16)
with open(os.path.join(root, 'etc', 'mione-twrp-defaults'), 'wb') as stream:
    stream.write(struct.pack('<I', settings_version))
    for value in (b'tw_storage_path\0', b'/data/media\0'):
        stream.write(struct.pack('<H', len(value)))
        stream.write(value)
with open(os.path.join(root, 'etc', 'mione-lvm-ready'), 'w') as stream:
    stream.write('mione-lvm-v1\n')
# The build system records these hashes before invoking this device hook.
# Refresh its existing list after changing the generated theme files.
hash_path = os.path.join(root, 'ramdisk-files.sha256sum')
if os.path.exists(hash_path):
    lines = []
    for line in open(hash_path):
        if '  ' not in line:
            continue
        unused, name = line.rstrip('\n').split('  ', 1)
        path = os.path.join(root, name)
        if name.lstrip('./') in ('sbin/mione-repartition', 'init.android', 'lvm/lvm'):
            continue
        if not os.path.isfile(path):
            raise ValueError('Missing ramdisk file: ' + name)
        with open(path, 'rb') as stream:
            digest = hashlib.sha256(stream.read()).hexdigest()
        lines.append(digest + '  ' + name + '\n')
    with open(hash_path, 'w') as stream:
        stream.writelines(lines)
        old_names = set(line.rstrip('\n').split('  ', 1)[1] for line in lines)
        for name in ('sbin/init.android', 'etc/recovery.fstab.physical', 'sbin/mione-lvm2', 'lvm/etc/lvm.conf', 'etc/mione-lvm-ready',
                     'sbin/postrecoveryboot.sh', 'etc/mione-twrp-defaults'):
            if name not in old_names:
                with open(os.path.join(root, name), 'rb') as source_stream:
                    stream.write(hashlib.sha256(source_stream.read()).hexdigest() + '  ' + name + '\n')
print('MiOne LVM UI added: %s..2560 MiB' % minimum)
