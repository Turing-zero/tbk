"""TBK - Turing-zero Bot Kits

统一的机器人通信框架，提供简单易用的Python接口。
"""

from ._core import *  # C++绑定

class Node:
    """简化的节点接口"""
    
    def __init__(self, name: str, transport: str = "udp", discovery: str = "etcd"):
        """创建节点
        
        Args:
            name: 节点名称
            transport: 传输方式 ("udp", "tcp")
            discovery: 发现服务 ("etcd")
        """
        self._node = self._create_node(name, transport, discovery)
    
    def publish(self, topic: str, message):
        """发布消息"""
        self._node.publish(topic, message)
    
    def subscribe(self, topic: str, callback):
        """订阅消息"""
        self._node.subscribe(topic, callback)
    
    def call_service(self, service: str, request):
        """调用服务"""
        return self._node.call_service(service, request)
    
    def advertise_service(self, service: str, handler):
        """提供服务"""
        self._node.advertise_service(service, handler)

# 简化的使用示例
def example_usage():
    # 创建节点 - 一行代码完成所有初始化
    node = Node("my_robot")
    
    # 发布消息
    node.publish("/cmd_vel", {"linear": 1.0, "angular": 0.5})
    
    # 订阅消息
    def on_sensor_data(data):
        print(f"Received: {data}")
    
    node.subscribe("/sensor_data", on_sensor_data)