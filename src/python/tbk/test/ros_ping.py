import time,rospy
from std_msgs.msg import String

rospy.init_node('ping', anonymous=True)

count = 0
puber = rospy.Publisher('ping', String, queue_size=3)
def f(msg):
    global count
    count += 1
    puber.publish("ping")

suber = rospy.Subscriber('pong', String, f)

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