1.生成python的动态链接库
在scripts下面运行：

```
python3 setup.py build_ext --inplace
```

2.生成deb
在castlex_bringup下面运行：

```
bloom-generate rosdebian --os-name ubuntu --ros-distro melodic

fakeroot debian/rules binary
```

# 使用说明
```c++
//控制IOT模块的相关话题
   ros::Subscriber Cmd_Trashcan_Sub = n.subscribe("/Trashcan_CMD_Topic", 10, &Control_Trashcan_Callback);  			1:kai 0:guan
   ros::Subscriber Cmd_Trashcan_Stop = n.subscribe("/Trashcan_STOP_Topic", 10, &Trashcan_STOP_Callback);			1:stop
   ros::Subscriber Cmd_Trashcan_Reset = n.subscribe("/Trashcan_RESET_Topic", 10, &Trashcan_RESET_Callback);                     1:reset
   ros::Subscriber Cmd_Trashcan_bar_Sub = n.subscribe("/Trashcan_bar_CMD_Topic", 10, &Control_Trashcan_bar_Callback);           1:kai 0:guan
   ros::Subscriber Cmd_Door_Sub = n.subscribe("/Door_CMD_Topic", 10, &Control_Door_Callback);                                   xx: 0 - 3
   ros::Subscriber Cmd_Lighting_Sub = n.subscribe("/Lighting_CMD_Topic", 10, &Control_Lighting_Callback);                       xxx: 0-7
   ros::Subscriber Cmd_Gateway_Sub = n.subscribe("/Gateway_CMD_Topic", 10, &Control_Gateway_Callback);                          1:kai 0:guan

//发布IOT模块的相关话题
        Curtain_pub 	= n.advertise<std_msgs::Int32MultiArray>("/Curtain_State", 10);       1:kai 0:guan

        Lighting_pub 	= n.advertise<std_msgs::Int32MultiArray>("/Lighting_State", 10);      xxx: 0-7 

        Door_pub 	= n.advertise<std_msgs::Int32>("/Door_State", 10);                    xx : 0-3    

        Gateway_pub 	= n.advertise<std_msgs::Int32>("/Gateway_State", 10);                 1:kai 0:guan


    # 订阅貨艙是否要开启消息
    rospy.Subscriber("Warehouse_control", Int32, self.Warehouse_control)                  1:kai 0:guan
    # 货仓呼吸灯话题
    rospy.Subscriber("Warehouse_light_control", Int32, self.Warehouse_light_control)      0~8400

```

#   物联网模块
```bash
物联网灯：
000(0) 关闭所有灯
001(1) 只打开灯1
010(2) 只打开灯2
011(3) 只关闭灯3
100(4) 只打开灯3
101(5) 只关闭灯2
110(6) 只关闭灯1
111(7) 打开所有灯

物联网门铃：
00(0) 不按门铃
01(1) 按1号门铃
10(2) 按2号门铃
11(3) 按所有门铃

```




