#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import argparse

parser = argparse.ArgumentParser(
                    prog=f'{os.path.basename(__file__)}',
                    description='cmdline tool to test tbk toolbox - Params',
                    epilog='Enjoy the program! :)')
# example of use:
# - list all parameters
#   tool_param.py list
# - get params in a particular group
#   tool_param.py list group_name
# - get a particular param
#   tool_param.py get group_name param_name
# - set a particular param
#   tool_param.py set group_name param_name value
# - save all params to a file
#   tool_param.py save file_name
# - load all params from a file
#   tool_param.py load file_name

# how to use
# python tool_param.py list
# python tool_param.py list [group_name]
# python tool_param.py get [group_name] [param_name]
# python tool_param.py set [group_name] [param_name] [value]
# python tool_param.py save [file_name]
# python tool_param.py load [file_name]


commands = parser.add_subparsers(dest='command', help='action')
help = commands.add_parser('help', help='show help')
list = commands.add_parser('list', help='list all parameters')
list.add_argument('-p', '--prefix', nargs='?', default="", help='prefix')
save = commands.add_parser('save', help='save all parameters to a file')
save.add_argument('file', help='file name')
load = commands.add_parser('load', help='load all parameters from a file')
load.add_argument('file', help='file name')
get = commands.add_parser('get', help='get a parameter')
get.add_argument('param', help='parameter name')
set = commands.add_parser('set', help='set a parameter that already exists')
set.add_argument('param', help='parameter name')
set.add_argument('value', help='parameter value')
put = commands.add_parser('put', help='put a parameter that may or may not exist')
put.add_argument('param', help='parameter name')
put.add_argument('value', help='parameter value')
del_ = commands.add_parser('del', help='delete a parameter')
del_.add_argument('param', help='parameter name')

args = parser.parse_args()


import etcd3

def __client():
    pkipath = os.path.join(os.path.expanduser("~"),'.tbk/etcdadm/pki')
    return etcd3.client(
        host='127.0.0.1',
        port=2379,
        ca_cert=os.path.join(pkipath,'ca.crt'),
        cert_key=os.path.join(pkipath,'etcdctl-etcd-client.key'),
        cert_cert=os.path.join(pkipath,'etcdctl-etcd-client.crt')
    )

def api_list(_prefix=None):
    etcd = __client()
    prefix = '/tbk/params/'+(_prefix if _prefix else '')
    res = dict([(r[1].key.decode(),r[0].decode()) for r in etcd.get_prefix(prefix)])
    return res
def api_get(param):
    etcd = __client()
    r = etcd.get(f"/tbk/params/{param}")
    if r[0] is None:
        return False,f"Key \"{args.param}\" Not found."
    return (True,r[0].decode())
def api_put(param, value):
    etcd = __client()
    r = etcd.put(f"/tbk/params/{param}",value)
    return True, "OK"
def api_set(param, value):
    res,v = api_get(param)
    if res:
        return api_put(param,value)
    else:
        return False,v
def api_del(param):
    etcd = __client()
    r = etcd.delete(f"/tbk/params/{param}")
    return r, f"Key \"{param}\" Not found." if r == False else "OK"
def api_save(file):
    pass
def api_load(file):
    pass

if __name__ == "__main__":
    if args.command == 'list':
        res = api_list(args.prefix)
        for k,v in res.items():
            print(k,v)
        exit(0)
    elif args.command == 'get':
        res,v = api_get(args.param)
        print(v)
        exit(res)
    elif args.command == 'put':
        res,v = api_put(args.param,args.value)
        print(v)
        exit(res)
    elif args.command == 'set':
        res,v = api_set(args.param,args.value)
        print(v)
        exit(res)
    elif args.command == 'del':
        res,v = api_del(args.param)
        print(v)
        exit(res)
    elif args.command == 'save':
        pass
    elif args.command == 'load':
        pass
    elif args.command == 'help':
        parser.print_help()
    else:
        print("Not implemented yet.")
        exit(1)