#!/usr/bin/env python
#coding=utf-8

from iot_control import IOT_CONTROL

if __name__ == "__main__":
  try:
    IOT_CONTROL()
  except  rospy.ROSInterruptException:
    rospy.logerr("IOT failed")