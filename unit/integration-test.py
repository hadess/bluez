#!/usr/bin/python3

# Copyright: (C) 2025 Bastien Nocera <hadess@hadess.net>
# Copyright: (C) 2023 cagney, for the AsciiDecoder
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import os
import sys
import subprocess
import unittest
import time
import re
from output_checker import OutputChecker

try:
    import gi
    from gi.repository import GLib
except ImportError as e:
    sys.stderr.write('Skipping tests, PyGobject not available for Python 3, or missing GI typelibs: %s\n' % str(e))
    sys.exit(77)

builddir = os.getenv('top_builddir', '.')

try:
    import dbusmock
    import dbus
except ImportError:
    sys.stderr.write('Skipping tests, python-dbusmock not available (https://pypi.python.org/pypi/python-dbusmock).\n')
    sys.exit(77)

try:
    import pexpect
except ImportError:
    sys.stderr.write('Skipping tests, pexpect not available (https://pypi.org/project/pexpect/).\n')
    sys.exit(77)

def count_adapters():
    try:
        for entry in os.walk('/sys/class/bluetooth'):
            return len(entry[1])
        return 0
    except OSError:
        return 0

def is_root():
    return os.geteuid() == 0

# https://stackoverflow.com/a/76543345
# CC-BY-SA 4.0 relicensed under GPLv2 or later
# see https://web.archive.org/web/20200805075926/http://ascii-table.com/ansi-escape-sequences.php
class AsciiDecoder(object):
    def __init__(self):
        self.buf = b''
    def encode(self, b, final=False):
        return b
    def decode(self, b, final=False):
        # escape sequences can be split so
        # work on lines
        self.buf = self.buf + b
        i = self.buf.find(b'\n')
        if i >= 0:
            c = self.buf[0:i+1]
            self.buf = self.buf[i+1:]
            d = re.sub(rb'\x1b\[[0-9;=?]*[HfABCDsuJKmhlr]', b'*', c)
            e = re.sub(rb'\x1b', b'<ESC>', d)
            if e != e:
                print(">", e, "<")
            return e
        return b''

class Tests(dbusmock.DBusTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.dbus_con = cls.get_dbus(True)

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()

    def setUp(self):
        super().setUp()
        self.daemon_log = OutputChecker()

    def stop_virt(self):
        if self.virt:
            try:
                self.virt.terminate()
            except OSError:
                pass
            self.assertEqual(self.virt.wait(timeout=3000), 0)

        self.virt = None

    def start_virt(self):
        if not is_root():
            self.skipTest('btvirt cannot start as we are not root')

        num_adapters = count_adapters()

        self.virt = subprocess.Popen(
            [ self.btvirt_path(), '-l1' ],
        )
        self.addCleanup(self.stop_virt)

        self.assert_eventually(lambda: count_adapters() > num_adapters)

    def start_virt_if_needed(self):
        if count_adapters() == 0:
            self.start_virt()

    def stop_daemon(self):
        if self.daemon:
            try:
                self.daemon.terminate()
            except OSError:
                pass
            self.assertEqual(self.daemon.wait(timeout=3000), 0)

        self.daemon = None

    def daemon_check(self):
        if not self.dbus_con.name_has_owner('org.bluez'):
            return False

        try:
            p = dbus.Interface(self.dbus_con.get_object('org.bluez', '/'),
                               dbus_interface=dbus.INTROSPECTABLE_IFACE)
            p.Introspect()
            return True
        except dbus.exceptions.DBusException as e:
            last_exc = e
            if '.UnknownInterface' in str(e):
                return False

    def start_daemon(self):
        if not is_root():
            self.skipTest('bluetoothd cannot start as we are not root')

        self.daemon = subprocess.Popen(
            [ self.bluetoothd_path(), '--nodetach', '-f', self.bluetooth_config_path() ],
            stdout=subprocess.DEVNULL, stderr=self.daemon_log.fd
        )
        self.daemon_log.writer_attached()
        self.addCleanup(self.stop_daemon)

        self.assert_eventually(lambda: self.daemon_check())

    def start_daemon_if_needed(self):
        # Only start daemon if one is not already available
        if not self.daemon_check():
            self.start_daemon()

    def binary_path(self, binary):
        builddir = os.getenv('top_builddir', '.')
        return os.path.join(builddir, binary)

    def bluetoothctl_path(self):
        return self.binary_path('client/bluetoothctl')

    def btmgmt_path(self):
        return self.binary_path('tools/btmgmt')

    def btvirt_path(self):
        return self.binary_path('emulator/btvirt')

    def bluetoothd_path(self):
        return self.binary_path('src/bluetoothd')

    def bluetooth_config_path(self):
        srcdir = os.getenv('top_srcdir', '.')
        return os.path.join(srcdir, 'src/main.conf')

    def assert_eventually(self, condition, message=None, timeout=5000, keep_checking=0):
        """Assert that condition function eventually returns True.

        Timeout is in milliseconds, defaulting to 5000 (5 seconds). message is
        printed on failure.
        """
        if not keep_checking:
            if condition():
                return

        done = False

        def on_timeout_reached():
            nonlocal done
            done = True

        source = GLib.timeout_add(timeout, on_timeout_reached)
        while not done:
            if condition():
                GLib.source_remove(source)
                if keep_checking > 0:
                    self.assert_condition_persists(
                        condition, message, timeout=keep_checking
                    )
                return
            GLib.MainContext.default().iteration(False)

        self.fail(message() if message else f"timed out waiting for {condition}")

    def test_bluetoothctl_no_output(self):
        self.start_virt_if_needed()
        self.start_daemon_if_needed()

        num_adapters = count_adapters()

        cmd = subprocess.run([ self.bluetoothctl_path(), 'list' ], check=False, capture_output=True)
        output = cmd.stdout.decode('UTF-8')
        self.assertEqual(len(output.strip().split('\n')), num_adapters)
        self.assertRegex(output, '.*\\[default\\]')

    def test_btmgmt_stdin_hang(self):
        self.start_virt_if_needed()

        num_adapters = count_adapters()

        cmd = subprocess.run([ self.btmgmt_path(), 'info' ], stdin=subprocess.DEVNULL, check=False, capture_output=True)
        output = cmd.stdout.decode('UTF-8')
        self.assertRegex(output, '.*Index list with ' + str(num_adapters) + ' item')

    def test_btmgmt_help_hang(self):
        devnull = open(os.devnull, 'w')
        cmd_log = OutputChecker(out=devnull)
        cmd = subprocess.Popen([ self.btmgmt_path(), '--help' ], stdout=cmd_log.fd)
        cmd_log.writer_attached()

        # Check that btmgmt --help doesn't hang
        self.assertEqual(cmd.wait(timeout=1), 0)
        devnull.close()

        # Check that it contains the help output
        cmd_log.check_line('Send HCI Command and wait for Event', timeout=1)

    def test_bluetoothd_failed_to_set_default_config(self):
        if self.daemon_check():
            # We need to start bluetoothd ourselves, so skip the test
            self.skipTest('Test requires that we start bluetoothd ourselves')

        self.start_virt_if_needed()
        self.start_daemon()

        self.daemon_log.check_line('Battery Provider Manager created', timeout=1)
        self.daemon_log.check_no_line('Failed to set default system config for',
                                      failmsg='Error output when using stock configuration')

    def test_bluetoothctl_completion_crash(self):
        child = pexpect.spawn(self.bluetoothctl_path())
        child._decoder = AsciiDecoder()
        child.expect(r'\[bluetoothctl\]> ')
        child.sendline(' \t')
        child.expect(r'\[bluetoothctl\]> ')

        time.sleep(0.5)

        self.assertTrue(child.isalive())


if __name__ == '__main__':
    unittest.main()
