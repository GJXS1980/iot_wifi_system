#ifndef __MQTT_ARV_H
#define __MQTT_ARV_H
#include <ros/ros.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Float32.h>
#include "std_msgs/Int32MultiArray.h"
#include "ros/init.h"

#ifdef __cplusplus
extern "C"{
#endif

extern void joint_to_CartesianPose(float *joint_value,float *new_jt); // (调整到extern "C"区域内)

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"


#ifdef __cplusplus
}
#endif




#endif

