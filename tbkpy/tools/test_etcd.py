import etcd3

etcd = etcd3.client()
res = etcd.get_all()
for r in res:
    print(r[0],r[1].key)

# def watch_cb(*args, **kwargs):
#     print("watch_cb called with args: %s, kwargs: %s" % (args, kwargs))

events_iter, cancel = etcd.watch_prefix('/tbk')
for event in events_iter:
    print(event)