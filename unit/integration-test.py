#!/usr/bin/python3

# Copyright: (C) 2025 Bastien Nocera <hadess@hadess.net>
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
    sys.stderr.write('Skipping tests, python-dbusmock not available (http://pypi.python.org/pypi/python-dbusmock).\n')
    sys.exit(77)

def count_adapters():
    try:
        count = 0
        for entry in os.walk('/sys/class/bluetooth'):
            count += 1
        return count
    except OSError:
        return 0

def is_root():
    return os.geteuid() == 0

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
        self.log = None

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
            [ self.btvirt_path(), '-l1' ], stdout=self.log, stderr=sys.stderr
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
            [ self.bluetoothd_path(), '--nodetach' ], stdout=self.log, stderr=sys.stderr
        )
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
        print(output)
        self.assertEqual(len(output.strip().split('\n')), num_adapters)
        self.assertRegex(output, '.*\\[default\\]')

    def test_btmgmt_stdin_hang(self):
        self.start_virt_if_needed()

        num_adapters = count_adapters()

        cmd = subprocess.run([ self.btmgmt_path(), 'info' ], stdin=subprocess.DEVNULL, check=False, capture_output=True)
        output = cmd.stdout.decode('UTF-8')
        # FIXME
        # Index list with 1 item
        # self.assertEqual(len(output.strip().split()), num_adapters)
        # self.assertRegex(output, '.*\\[default\\]')


if __name__ == '__main__':
    unittest.main()
