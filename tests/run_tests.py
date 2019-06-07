"""Runs the tests in a particular directory."""
from __future__ import print_function

import os
import subprocess
import sys

class colour:
    GREEN = '\033[92m'
    RED = '\033[91m'
    END = '\033[0m'

    def __init__(self):
        self.g = ""
        self.r = ""
        self.e = ""
        if os.isatty(1):
            self.g = self.GREEN
            self.r = self.RED
            self.e = self.END

    def in_green(self, text):
        print(self.g + text + self.e)

    def in_red(self, text):
        print(self.r + text + self.e)


def spawn(col, label, argv):
    print("%-25s... " % (label,), end="")
    p = subprocess.Popen(argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, errs = p.communicate()
    result = p.returncode
    if result == 0:
        col.in_green("PASS")
    else:
        col.in_red("FAIL")
        print(out, file=sys.stdout)
        print(errs, file=sys.stderr)
    return result, errs

def run_one_test(col, dirname, name):
    label = "%s/%s" % (dirname, name)
    if name.endswith(".sh"):
        result, _ = spawn(col, label, ["sh", name])
        return result

def run_tests(col, dirname):
    os.chdir(dirname)
    result = 0
    names = os.listdir(".")
    for name in names:
        if os.path.isfile(name):
            rv = run_one_test(col, dirname, name)
            if rv > result:
                result = rv
    return result

        
def main(args):
    subdir = args[1]
    if len(args) != 2:
        usage(sys.stderr)
    col = colour()
    return run_tests(col, args[1])


if __name__ == '__main__':
    sys.exit(main(sys.argv))
