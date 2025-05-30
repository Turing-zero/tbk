import time
import tbkpy._core as tbkpy

tbkpy.init("test_node")
data = tbkpy.Data()
print("data: ", data)
ep = tbkpy.EPInfo()
ep.ns = "test_ns"
ep.name = "test_name"
ep.msg_name = "test_msg_name"
ep.msg_type = "test_msg_type"
ep.msg_type_url = "test_msg_type_url"
print("ep: ", ep)
print("ep.ns: ", ep.ns)
print("ep.name: ", ep.name)
print("ep.msg_name: ", ep.msg_name)
print("ep.msg_type: ", ep.msg_type)
print("ep.msg_type_url: ", ep.msg_type_url)

puber = tbkpy.Publisher(ep)

import time
while True:
    time.sleep(1)