#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os, argparse
import subprocess
import re

def __custom_format(formatStr, **kwargs):
    for k,v in kwargs.items():
        formatStr = formatStr.replace(f"{{{k}}}",v)
    return formatStr

def __get_etcdctl_bin():
    HOMEDIR = os.path.expanduser("~")
    return os.path.join(HOMEDIR,'.tbk/etcdadm/bin/etcdctl.sh')

def api_health():
    cmd = subprocess.run([__get_etcdctl_bin(), "endpoint", "health", "--cluster","--write-out=table"], stdout=subprocess.PIPE)
    output = cmd.stdout.decode("utf-8")
    print(output, end="")
    return cmd.returncode
def api_status(formatStr=None):
    pass
def api_info(formatStr=None):
    cmd = subprocess.run([__get_etcdctl_bin(), "endpoint", "status", "--cluster","--write-out=table"], stdout=subprocess.PIPE)
    output = cmd.stdout.decode("utf-8")
    value = ""
    if formatStr is None:
        print(output, end="")
        return cmd.returncode
    output = output.split("\n")
    header = output[1]
    content = output[3:-2]
    pattern = r'\s+(.*?)\s*\|'
    keys = [x.replace(' ','_') for x in re.findall(pattern, header)]
    if formatStr == "help":
        print("Available keys: " + ", ".join(keys))
        return cmd.returncode
    for line in content:
        if line == "":
            continue
        info = dict(zip(keys, re.findall(pattern, line)))
        value += __custom_format(formatStr, **info) + "\n"
    print(value, end="")
    return cmd.returncode

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Tool to manage etcd')
    commands = parser.add_subparsers(dest='command', help='action')
    help = commands.add_parser('help', help='show help')
    health = commands.add_parser('health', help='show cluster health')
    status = commands.add_parser('status', help='show cluster status')
    statusFormatStr = status.add_argument('--format', help='custom format output ()')
    info = commands.add_parser('info', help='show cluster info')
    infoFormatStr = info.add_argument('--format', help='custom format output')
    args = parser.parse_args()

    if args.command == "health":
        exit(api_health())
    elif args.command == 'status':
        exit(api_status(args.format))
    elif args.command == 'info':
        exit(api_info(args.format))
    elif args.command == 'help':
        parser.print_help()
        exit(0)
    else:
        print(f"Command \"{args.command}\" not found.")
        exit(1)