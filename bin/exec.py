#!/usr/bin/python3

import argparse
import subprocess


MODULES = [
    "record",
    "log",
    "server",
    "partition",
    "broker"
]


def run(command):
    for mod in MODULES:
        if command == "build":
            command = "all"

        subprocess.run(["make", "-j", "4", command, "-C", mod], shell=False, check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Run build command on all modules"
    )
    # TODO(AD) Add static analysis etc
    parser.add_argument(
        "command",
        choices=["build", "unittest", "integrationtest", "clean"],
        help="build command"
    )
    args = parser.parse_args()
    run(args.command)
