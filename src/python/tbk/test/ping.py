import time
import tbkpy._core as tbkpy

tbkpy.init("test_ping")

count = 0
puber = tbkpy.Publisher("Ping_Puber","ping")
def f(msg):
    global count
    count += 1
    puber.publish("ping")
suber = tbkpy.Subscriber("Pong_Suber","pong",f)

input("Press Enter to continue...")
# time
t = time.time()
puber.publish("ping")

while True:
    if count >= 10000:
        break
    time.sleep(0.01)
t = time.time() - t
print("------------finished: {:3f}/{} -------------".format(t,count))
