#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import subprocess
from tbkpy.tools import etcdadm_bin, etcdctl_bin
def api_run_action(action):
    cmd = subprocess.run([etcdadm_bin(), action], stdout=subprocess.PIPE)
    output = cmd.stdout.decode("utf-8")
    return cmd.returncode, output

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Interface for etcdadm')
    commands = parser.add_subparsers(dest='command', help='action')
    init = commands.add_parser('init', help='initialize etcd')
    reset = commands.add_parser('reset', help='reset etcd')
    help = commands.add_parser('help', help='show help')
    args = parser.parse_args()
    if args.command == "init" or args.command == "reset":
        res, output = api_run_action(args.command)
        print(output)
        exit(res)
    elif args.command == "help":
        parser.print_help()
        exit(0)
    else:
        print("Unknown command")
        exit(1)