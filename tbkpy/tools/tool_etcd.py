#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import subprocess
import re

from tbkpy.tools import etcdadm_bin, etcdctl_bin

def __custom_format(formatStr, kwargs):
    for k,v in kwargs.items():
        formatStr = formatStr.replace(f"{{{k}}}",v)
    return formatStr

def __get_cmd_additional_args(writeOutTable):
    args = ["--cluster","--command-timeout=1s"]
    if writeOutTable:
        args.append("--write-out=table")
    return args

def __before_format(output):
    output = output.split("\n")
    header = output[1]
    content = output[3:-2]
    pattern = r'\s+(.*?)\s*\|'
    keys = [x.replace(' ','_') for x in re.findall(pattern, header)]
    infos = [[] for _ in range(len(keys))]
    for line in content:
        if line == "":
            continue
        for i, value in enumerate(re.findall(pattern, line)):
            infos[i].append(value)
    return keys, infos

def api_health(formatStr=None):
    cmd = subprocess.run([etcdctl_bin(), "endpoint", "health"]+__get_cmd_additional_args(True), stdout=subprocess.PIPE)
    output = cmd.stdout.decode("utf-8")
    value = []
    if formatStr is None:
        return cmd.returncode, output
    if cmd.returncode != 0:
        return cmd.returncode, ""
    keys, infos = __before_format(output)
    if formatStr == "help":
        return cmd.returncode, f"Available keys: {', '.join(keys)}"
    for info in zip(*infos):
        value.append(__custom_format(formatStr, dict(zip(keys,info))))
    return cmd.returncode, "\n".join(value)
def api_status(formatStr=None):
    pass
def api_info(formatStr=None):
    cmd = subprocess.run([etcdctl_bin(), "endpoint", "status"]+__get_cmd_additional_args(True), stdout=subprocess.PIPE)
    output = cmd.stdout.decode("utf-8")
    value = []
    if formatStr is None:
        return cmd.returncode, output
    keys, infos = __before_format(output)
    if formatStr == "help":
        return cmd.returncode, f"Available keys: {', '.join(keys)}"
    for info in zip(*infos):
        value.append(__custom_format(formatStr, dict(zip(keys,info))))
    return cmd.returncode, "\n".join(value)

def api_ipinfo():
    cmd = subprocess.run([etcdadm_bin(), "info"], stdout=subprocess.PIPE)
    output = cmd.stdout.decode("utf-8")
    return cmd.returncode, output

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Tool to manage etcd')
    commands = parser.add_subparsers(dest='command', help='action')
    help = commands.add_parser('help', help='show help')
    health = commands.add_parser('health', help='show cluster health')
    healthFormatStr = health.add_argument('--format', help='custom format output')
    status = commands.add_parser('status', help='show cluster status')
    statusFormatStr = status.add_argument('--format', help='custom format output')
    info = commands.add_parser('info', help='show cluster info')
    infoFormatStr = info.add_argument('--format', help='custom format output')
    ip = commands.add_parser('ip', help='show cluster ip')
    args = parser.parse_args()

    if args.command == "health":
        res, output = api_health(args.format)
        print(output)
        exit(res)
    elif args.command == 'status':
        res, output = api_status(args.format)
        print(output)
        exit(res)
    elif args.command == 'info':
        res, output = api_info(args.format)
        print(output)
        exit(res)
    elif args.command == 'ip':
        res, output = api_ipinfo()
        print(output)
        exit(res)
    elif args.command == 'help':
        parser.print_help()
        exit(0)
    else:
        print(f"Command \"{args.command}\" not found.")
        exit(1)