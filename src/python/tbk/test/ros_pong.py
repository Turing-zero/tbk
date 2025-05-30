import time,rospy
from std_msgs.msg import String

rospy.init_node('pong', anonymous=True)

count = 0
puber = rospy.Publisher('pong', String, queue_size=10)
def f(msg):
    global count
    count += 1
    puber.publish("pong")
suber = rospy.Subscriber('ping', String, f)
while True:
    # if count >= 10000:
    #     break
    time.sleep(0.1)
print("------------finished-------------")