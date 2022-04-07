#!/usr/bin/env python3
#coding=utf-8

import configparser
#import numpy as np
import sys
import time
import base64
import json
from aliyunsdkcore.client import AcsClient
from aliyunsdkcore.acs_exception.exceptions import ClientException
from aliyunsdkcore.acs_exception.exceptions import ServerException
from aliyunsdkcore.auth.credentials import AccessKeyCredential
from aliyunsdkcore.auth.credentials import StsTokenCredential
from aliyunsdkiot.request.v20180120.PubRequest import PubRequest
from aliyunsdkiot.request.v20180120.SetDevicePropertyRequest import SetDevicePropertyRequest

import rospy
from std_msgs.msg import Int32, Int32MultiArray, Float32, Float32MultiArray, Bool
from collections import OrderedDict
import os
import ast
import time

class IOT_CONTROL:
    def __init__(self):
        rospy.init_node("iot_control_node")
        self.access_key_id = 'LTAI5tSMqhMTVP3gLbbs11MP'
        self.access_key_secret = '8iqEsbm29B0qbIrxLqbPg41I3wJ81s'
        self.region_id = 'cn-shanghai'
        # 构建一个阿里云客户端，用于发起请求。
        self.credentials = AccessKeyCredential(self.access_key_id, self.access_key_secret)
        self.client = AcsClient(region_id=self.region_id, credential=self.credentials)
        self.device_list = [["0", "0", "0"]]
        self.device_name_list = []
        self.iot_config_path = rospy.get_param("~config_path", '/home/grantli/castlex_ws/iot_wifi_system/scripts/device_config.ini')
        self.read_ini()
        self.local_time = None
        self.request = SetDevicePropertyRequest()  # PubRequest()
        # 设置返回数据格式
        self.request.set_accept_format('json')
        # self.request.set_Qos(0)
        self.iot_control_pub = rospy.Publisher('/iot_control_state', Bool, queue_size=5)
        rospy.Subscriber("/iot_control", Int32MultiArray, self.set_iot_state)
        rospy.Subscriber("/Trashcan_STOP_Topic", Int32, self.Trashcan_STOP_Callback)
        rospy.Subscriber("/Trashcan_RESET_Topic", Int32, self.Trashcan_RESET_Callback)
        rospy.Subscriber("/Trashcan_CMD_Topic", Int32, self.Control_Trashcan_Callback)
        rospy.Subscriber("/Door_CMD_Topic", Int32, self.Control_Door_Callback)
        rospy.Subscriber("/Lighting_CMD_Topic", Int32, self.Control_Lighting_Callback)
        rospy.Subscriber("/Gateway_CMD_Topic", Int32, self.Control_Gateway_Callback)
        while not rospy.is_shutdown():
            time.sleep(0.1)

    #  读取ini配置文件, ini文件中的设备顺序需要和控制话题id相对应
    def read_ini(self):
        # new add
        # curpath = os.path.dirname(os.path.realpath(__file__))
        # cfgpath=os.path.join(curpath, self.iot_config_path)
        iot_device_config = configparser.ConfigParser()
        iot_device_config.read(self.iot_config_path)
        selections = iot_device_config.sections()
        for i in range(len(selections)):
            device_Name = iot_device_config.get(selections[i], 'device_Name')
            device_ProductKey = iot_device_config.get(selections[i], 'device_ProductKey')
            device_Control = iot_device_config.get(selections[i], 'device_control')
            self.device_list.append([device_Name, device_ProductKey, device_Control])
            self.device_name_list.append(device_Name)

    # json转base64
    def encode_base64(self, msg, data):
        iot_msg = OrderedDict()
        # {"method": "thing.service.property.set", "id": "1926119961", "params": {"Gateway_SW": 1}, "version": "1.0.0"}
        iot_msg["method"] = "thing.service.property.set"
        iot_msg["id"] = str(int(self.local_time))
        iot_msg["params"] = {msg:data}
        iot_msg["version"] = "1.0.0"
        # iot_msg[msg] = data
        iot_req_json = json.dumps(iot_msg, separators=(',', ':'))
        iot_send_msg = str(base64.b64encode(iot_req_json))
        return iot_send_msg

    # 接受控制话题, [0,1]: 第一位为设备id, 第二位为状态
    def set_iot_state(self, data):
        current_time = time.localtime(time.time())
        self.local_time = time.mktime(current_time)
        iot_control_data = data.data
        # print(iot_control_data)
        iot_name = self.device_list[iot_control_data[0]][0]
        iot_productKey = self.device_list[iot_control_data[0]][1]
        iot_msg = self.device_list[iot_control_data[0]][2]
        # print(iot_name, iot_productKey, iot_msg)
        # send_msg = self.encode_base64(iot_msg, iot_control_data[1])
        send_msg = OrderedDict()
        send_msg[iot_msg] = iot_control_data[1]
        iot_req_json = json.dumps(send_msg, separators=(',', ':'))
        # 设置参数（"set_"+${请求参数的名称}）
        self.request.set_ProductKey(iot_productKey) # 要设置属性值的设备所隶属的产品ProductKey
        self.request.set_DeviceName(iot_name)   # 要设置属性值的设备名称列表。最多支持传入100个设备名称
        # 要设置的属性信息，数据格式为JSON，每个属性信息由标识符与属性值（key:value）构成，多个属性用英文逗号隔开
        self.request.set_Items(iot_req_json)    
        # self.request.set_TopicFullName("/sys/"+iot_productKey+"/"+iot_name+"/thing/service/property/set")
        #发起请求，并得到响应
        self.response = self.client.do_action_with_exception(self.request)
        # 对结果进行处理，提取反馈状态
        result = str(self.response, encoding='utf-8')
        result = json.loads(result)
        # print(type(result.get('Success')))
        self.iot_control_pub.publish(result.get('Success'))
        
    # 兼容旧版本
    def old_contrl_iot(self, type, data):
        iot_control_data = data
        iot_name = self.device_list[type][0]
        iot_productKey = self.device_list[type][1]
        iot_msg = self.device_list[type][2]
        # send_msg = self.encode_base64(iot_msg, iot_control_data[1])
        send_msg = OrderedDict()
        send_msg[iot_msg] = iot_control_data
        iot_req_json = json.dumps(send_msg, separators=(',', ':'))
        # print(iot_name, iot_productKey, iot_msg)
        self.request.set_ProductKey(iot_productKey)
        self.request.set_DeviceName(iot_name)
        self.request.set_Items(iot_req_json)
        # self.request.set_TopicFullName("/sys/"+iot_productKey+"/"+iot_name+"/thing/service/property/set")
        # self.request.set_MessageContent(send_msg)
        self.response = self.client.do_action_with_exception(self.request)

    def Control_Trashcan_Callback(self, data):
        self.old_contrl_iot(0, data.data)

    def Control_Door_Callback(self, data):
        if data.data == 1:
            self.old_contrl_iot(2, 1)
        if data.data == 2:
            self.old_contrl_iot(3, 1)

    def Control_Lighting_Callback(self, data):
        self.old_contrl_iot(4, (data.data >> 0) & 1)
        self.old_contrl_iot(5, (data.data >> 1) & 1)
        self.old_contrl_iot(6, (data.data >> 2) & 1)

    def Control_Gateway_Callback(self, data):
        self.old_contrl_iot(1, data.data)

    def Trashcan_STOP_Callback(self, data):
        if data.data == 1:
            send_msg = OrderedDict()
            send_msg["stop"] = data.data
            iot_req_json = json.dumps(send_msg, separators=(',', ':'))
            iot_name = self.device_list[0][0]
            iot_productKey = self.device_list[0][1]
            self.request.set_ProductKey(iot_productKey)
            self.request.set_DeviceName(iot_name)
            self.request.set_Items(iot_req_json)
            self.response = self.client.do_action_with_exception(self.request)
            print(str(self.response))

    def Trashcan_RESET_Callback(self, data):
        if data.data == 1:
            send_msg = OrderedDict()
            send_msg["reset"] = data.data
            iot_req_json = json.dumps(send_msg, separators=(',', ':'))
            iot_name = self.device_list[0][0]
            iot_productKey = self.device_list[0][1]
            self.request.set_ProductKey(iot_productKey)
            self.request.set_DeviceName(iot_name)
            self.request.set_Items(iot_req_json)
            self.response = self.client.do_action_with_exception(self.request)
            print(str(self.response))

# if __name__ == "__main__":
#     iot_control()
#     rospy.spin()
