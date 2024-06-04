import time
print("---------running test.py---------")
print("-------try to import tbkpy-------")
import tbkpy
print("------------success--------------")
print("tbkpy.add(1,2)=", tbkpy.add(1,2))
tbkpy.init()
suber = tbkpy.Subscriber(5,"test_Suber","test_msg")
print("test : ",suber.test())

print("---------------------------------")

def f(msg):
    print("in python : callback f")
    print("type : ",type(msg))
    print("msg : ",msg)

suber = tbkpy.Subscriber("test_Suber","test_msg",f)
puber = tbkpy.Publisher("test_Puber","test_msg")
while True:
    puber.publish("hello world")
    time.sleep(1)
print("------------finished-------------")
 