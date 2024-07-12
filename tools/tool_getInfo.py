import os
import etcd3
from graphviz import Digraph
import tbk_pb2 as tbkpb

def __client():
    pkipath = os.path.join(os.path.expanduser("~"),'.tbk/etcdadm/pki')
    return etcd3.client(
        host='127.0.0.1',
        port=2379,
        ca_cert=os.path.join(pkipath,'ca.crt'),
        cert_key=os.path.join(pkipath,'etcdctl-etcd-client.key'),
        cert_cert=os.path.join(pkipath,'etcdctl-etcd-client.crt')
    )

if __name__ == "__main__":
    processes = {}
    publishers = {}
    subscribers = {}
    etcd = __client()

    prefix = '/tbk/ps'
    res = etcd.get_prefix(prefix)

    for r in res:
        key,value = r[1].key.decode(),r[0]
        # print(key,value)
        keys = key[len(prefix):].split('/')[1:]
        info = None
        if len(keys) == 1:
            info = tbkpb.State()
            info.ParseFromString(value)
            processes[info.uuid] = info
        elif len(keys) == 3:
            if keys[1] == "pubs":
                info = tbkpb.Publisher()
                info.ParseFromString(value)
                publishers[info.uuid] = info
            elif keys[1] == "subs":
                info = tbkpb.Subscriber()
                info.ParseFromString(value)
                subscribers[info.uuid] = info
        else:
            print("Error: key error:",key)

    g = Digraph('G', filename='__temp__tbknodes.gv',format='svg',engine='fdp')
    g.attr(compound='true')
    g.attr('node', shape='box')
    for uuid,info in processes.items():
        with g.subgraph(name='cluster_'+uuid) as c:
            c.attr(label="{}({}:{})".format(info.node_name,info.pid,uuid),color='grey',style='filled')
            c.node_attr['style'] = 'filled'
            for pub,info in publishers.items():
                if info.puuid == uuid:
                    with c.subgraph(name="cluster_ns_"+info.ns) as cc:
                        cc.attr(label=info.ns,color='white',style='filled')
                        cc.node_attr['style'] = 'filled'
                        cc.node(pub, label="PUB({}:{})\n\n{}".format(info.name,info.msg_name,str(info)))
            for sub,info in subscribers.items():
                if info.puuid == uuid:
                    with c.subgraph(name="cluster_ns_"+info.ns) as cc:
                        cc.attr(label=info.ns,color='white',style='filled')
                        cc.node_attr['style'] = 'filled'
                        cc.node(sub, label="SUB({}:{})\n\n{}".format(info.name,info.msg_name,str(info)))
    for pub,info in publishers.items():
        for sub in info.subs:
            g.edge(pub,sub.uuid)
    print(g.source)
    g.view()
    print("Done.")