import time
import tbkpy._core as tbkpy

tbkpy.init("test_pong")

count = 0
puber = tbkpy.Publisher("Pong_Puber","pong")
def f(msg):
    global count
    count += 1
    puber.publish("pong")
suber = tbkpy.Subscriber("Ping_Suber","ping",f)
while True:
    if count >= 10000:
        break
    time.sleep(0.01)
print("------------finished-------------")