cmake_minimum_required(VERSION 3.0.2)
project(iot_wifi_system)

set(IOT_SDK_PATH "./include/")

FILE(GLOB IOTLIDAR_SDK_SRC 
  "${IOT_SDK_PATH}/core/*.c"
  "${IOT_SDK_PATH}/core/sysdep/*.c"
  "${IOT_SDK_PATH}/core/utils/*.c"
  "${IOT_SDK_PATH}/external/*.c"
  "${IOT_SDK_PATH}/external/mbedtls/library/*.c"
  "${IOT_SDK_PATH}/portfiles/aiot_port/*.c"
)

add_compile_options(-std=c++11)
add_definitions(-std=c99)
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
)


catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES epidemic_prevention_system
#  CATKIN_DEPENDS roscpp rospy std_msgs
#  DEPENDS system_lib
)

include_directories(include ${catkin_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS})
include_directories(
    include	
    /opt/ros/melodic/include
    include/core
    include/core/sysdep
    include/core/utils
    include/portfiles
    include/portfiles/aiot_port
    include/external
    include/external/mbedtls
    include/external/mbedtls/include
    include/external/mbedtls/include/mbedtls
    include/external/mbedtls/library

)

include_directories(include ${catkin_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS})
include_directories(
    include
    /opt/ros/melodic/include
)

add_executable(IOT_WIFI_Node src/mqtt_arv.cpp src/cJSON.cpp ${IOTLIDAR_SDK_SRC})
target_link_libraries(IOT_WIFI_Node ${catkin_LIBRARIES})
add_executable(IOT_ARV_Node src/iot_arv.cpp src/cJSON.cpp ${IOTLIDAR_SDK_SRC})
target_link_libraries(IOT_ARV_Node ${catkin_LIBRARIES})

install(DIRECTORY include/${PROJECT_NAME}/
    DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)

install(TARGETS IOT_WIFI_Node IOT_ARV_Node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

# install(FILES
# launch/castlex_iot.launch
# launch/castlex_iot_device.launch
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

install(
  DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)



