#include "mqtt_arv.h"
#include "cJSON.h"
#include <signal.h>

// mod
//      定义订阅和发布的话题
//#define         sub_topic       "/broadcast/a19EYxYfg19/IOTmsg"
//#define         pub_topic  	"/broadcast/a19EYxYfg19/IOTmsg"

void       *mqtt_handle = NULL;

typedef enum IOT_Device_MSG
{
        	trashcan = 0,
		door1,
		door2,
		lighting1,
		lighting2,
		lighting3,
		trashcan_bar,
		lighting_bar,
                gateway,
		elevator_floor,
		EXIT,        
}IOT_Device_MSG_t;

 struct IOT_MSG
{       
                int  Curtain_state;             //窗帘状态
                int Curtain_bar;                //窗帘进度条
		int Door1_status;               //门铃1状态
                int Door2_status;               //门铃2状态
                int Light_status;               //灯光状态
                int Light1_status;              //灯光1状态
                int Light2_status;              //灯光2状态
                int Light_bar;                  //灯光进度条
                int Gateway;                    //闸道状态
		int Elevator_floor;		//电梯楼层状态
                int Warehouse_state;    	//仓库锁头状态

} IOT_STATE;

ros::Publisher  Curtain_pub ;
ros::Publisher  Lighting_pub ;
ros::Publisher  Door_pub ;
ros::Publisher  Warehouse_pub ;
ros::Publisher  Gateway_pub ;
ros::Publisher  Elevator_floor_pub ;

/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;

static pthread_t g_mqtt_process_thread;
static pthread_t g_mqtt_recv_thread;
static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;
//static uint8_t publish_process_thread_running = 0;
//static pthread_t publish_iot_state;                     //发布话题每个物联网模块的状态信息

/* 日志回调函数,code值的定义见core/aiot_state_api.h*/
int32_t demo_state_logcb(int32_t code, char *message)
{
    //printf("%s", message);
    return 0;
}

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) 
        {
        /* 调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接已成功 */
        case AIOT_MQTTEVT_CONNECT: {
            printf("AIOT_MQTTEVT_CONNECT\n");
            /* 处理SDK建连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络状况被动断连后, 自动发起重连已成功 */
        case AIOT_MQTTEVT_RECONNECT: 
        {
            printf("AIOT_MQTTEVT_RECONNECT\n");
            /* 处理SDK重连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络的状况而被动断开了连接, network是底层读写失败, heartbeat是没有按预期得到服务端心跳应答 */
        case AIOT_MQTTEVT_DISCONNECT: 
        {
            const char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            printf("AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
            /* 被动断连, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        default: {

        }
    }
}


void  IOT_MSG_Subscribe(unsigned char *JSON)
{
        cJSON *root = NULL;
        cJSON *Item = NULL;	 

        root=cJSON_Parse((char *)JSON);
        if(! root) 
        {
                        return;
        }
                for(int i= 0;i<EXIT;i++)
                {
			switch(i)
			{
				case trashcan:
							 Item = cJSON_GetObjectItem(root,"trashcan_p");
							 if(Item != NULL)
							 { 
									IOT_STATE.Curtain_state= Item->valueint;
									i = EXIT;
							 }
					break;	
							 
				case trashcan_bar:
							Item = cJSON_GetObjectItem(root,"pull_p");
							if(Item != NULL)
							{
								        IOT_STATE.Light_bar= Item->valueint;
									i = EXIT;
							}	
							break; 
				case door1:
						 Item = cJSON_GetObjectItem(root,"door1");
						 if(Item != NULL)
						 {
								IOT_STATE.Door1_status = Item->valueint;
								i = EXIT;
						 }
					break;
				case door2:
						 Item = cJSON_GetObjectItem(root,"door2");
						 if(Item != NULL)
						 {
								IOT_STATE.Door2_status = Item->valueint;
								i = EXIT;
						 }
					break;
				
				case lighting3:
						 Item = cJSON_GetObjectItem(root,"lighting3");
						 if(Item != NULL)
						 {
								IOT_STATE.Light_status = Item->valueint;
								i = EXIT;
						 }
					break;	
				case lighting1:
						 Item = cJSON_GetObjectItem(root,"lighting1");
						 if(Item != NULL)
						 {
								IOT_STATE.Light1_status = Item->valueint;
								i = EXIT;
						 }
					break;	
				case lighting2:
						 Item = cJSON_GetObjectItem(root,"lighting2");
						 if(Item != NULL)
						 {
								IOT_STATE.Light2_status = Item->valueint;
								i = EXIT;
						 }
					break;			
				case lighting_bar:
							Item = cJSON_GetObjectItem(root,"lighting_bar_p");
							if(Item != NULL)
							{
									IOT_STATE.Curtain_state = Item->valueint;	
									i = EXIT;
							}	
					break;
				case gateway:
						 Item = cJSON_GetObjectItem(root,"gateway_p");
						 if(Item != NULL)
						 {
								IOT_STATE.Gateway = Item->valueint;	
						 }
						 Item = cJSON_GetObjectItem(root,"target_floor_p");
						 if(Item != NULL)
						 {
							IOT_STATE.Elevator_floor = Item->valueint;
							i = EXIT;
						 }
					break;	

	

				default :
					break;	
			}
	
	}
	
	cJSON_Delete(root);

}

//发布话题每个物联网模块的状态信息
void * Update_Iot_State(void *parameter)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();

                cJSON_AddNumberToObject(root,"broadcast",1);
                send_data =cJSON_PrintUnformatted(root);    
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }              


        	std_msgs::Int32MultiArray        Curtain_State;        
        	std_msgs::Int32MultiArray        Lighting_State; 
        	std_msgs::Int32                  Door_State; 
        	std_msgs::Int32                  Warehouse_State; 
        	std_msgs::Int32                  Gateway_State; 
		std_msgs::Int32                  floor; 

                Curtain_State.data.push_back(IOT_STATE.Curtain_state);
                Curtain_State.data.push_back(IOT_STATE.Curtain_bar);
         
                Lighting_State.data.push_back(IOT_STATE.Light1_status);
 	 	Lighting_State.data.push_back(IOT_STATE.Light2_status);
		Lighting_State.data.push_back(IOT_STATE.Light_status);

		int door_state= IOT_STATE.Door1_status | IOT_STATE.Door2_status<<1;
                Door_State.data = door_state;
	
                Warehouse_State.data = IOT_STATE.Warehouse_state;
                Gateway_State.data = IOT_STATE.Gateway;
                floor.data =  IOT_STATE.Elevator_floor;

                Curtain_pub.publish(Curtain_State);
                Lighting_pub.publish(Lighting_State);
                Door_pub.publish(Door_State);
                Warehouse_pub.publish(Warehouse_State);
                Gateway_pub.publish(Gateway_State);   
		Elevator_floor_pub.publish(floor);

	        cJSON_Delete(root);	
		cJSON_free(send_data);

}

//接收话题，控制每一个模块的状态
void  Control_Trashcan_Callback( const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();

                cJSON_AddNumberToObject(root,"trashcan",cmd->data);
                  send_data =cJSON_PrintUnformatted(root);     
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }
                      
	        cJSON_Delete(root);	
		cJSON_free(send_data);
		usleep(1000*500);
 		Update_Iot_State(NULL);
} 

void Trashcan_STOP_Callback(const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();

                cJSON_AddNumberToObject(root,"stop",cmd->data);
                  send_data =cJSON_PrintUnformatted(root);     
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }
                      
	        cJSON_Delete(root);	
		cJSON_free(send_data);

}


void Trashcan_RESET_Callback(const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();

                cJSON_AddNumberToObject(root,"reset",cmd->data);
                  send_data =cJSON_PrintUnformatted(root);     
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }
                      
	        cJSON_Delete(root);	
		cJSON_free(send_data);

}

void  Control_Trashcan_bar_Callback( const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();

                cJSON_AddNumberToObject(root,"trashcan",cmd->data);
                  send_data =cJSON_PrintUnformatted(root);     
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }

	        cJSON_Delete(root);	
		cJSON_free(send_data);

 		Update_Iot_State(NULL);
}

//3bit 控制3个灯 1 2 4 
void  Control_Lighting_Callback( const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();
                cJSON_AddNumberToObject(root,"lighting_1",(cmd->data>>0)&0x01);
 		cJSON_AddNumberToObject(root,"lighting_2",(cmd->data>>1)&0x01);
                cJSON_AddNumberToObject(root,"lighting_3",(cmd->data>>2)&0x01);

		send_data =cJSON_PrintUnformatted(root);    		
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }              
	        cJSON_Delete(root);	
		cJSON_free(send_data);
	 	Update_Iot_State(NULL);
}

void  Control_Lighting_bar_Callback( const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();

                cJSON_AddNumberToObject(root,"lighting_bar",cmd->data);
                  send_data =  cJSON_PrintUnformatted(root);     
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }             
	        cJSON_Delete(root);	
		cJSON_free(send_data);
}


void Control_Gateway_Callback( const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();
                cJSON_AddNumberToObject(root,"gateway",cmd->data);
                  send_data =cJSON_PrintUnformatted(root);    
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }              
	        cJSON_Delete(root);	
		cJSON_free(send_data);

}

void Control_Elevator_Callback( const std_msgs::Int32::ConstPtr& cmd)
{

                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

		int floor = cmd->data;

		if(floor>5) floor=5;
		else if(floor<1) floor=1;
			
                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();
                cJSON_AddNumberToObject(root,"target_floor",floor);
                  send_data =cJSON_PrintUnformatted(root);    
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }              
	        cJSON_Delete(root);	
		cJSON_free(send_data);


}

void Update_IOT_Callback ( const std_msgs::Int32::ConstPtr& cmd)
{
	if(cmd->data == 1)
	{
      		 Update_Iot_State(NULL); 
	}

}

//2bit 控制
void  Control_Door_Callback( const std_msgs::Int32::ConstPtr& cmd)
{
                ros::NodeHandle node("~");
                std::string pub_topic_param;
                node.param<std::string>("pub_topic_param", pub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

                char  *pub_topic =  const_cast<char *>(pub_topic_param.c_str());

                char *send_data = NULL;
                cJSON *root = cJSON_CreateObject();
                
                cJSON_AddNumberToObject(root,"door_1",(cmd->data>>0)&0X01);
 		cJSON_AddNumberToObject(root,"door_2",(cmd->data>>1)&0X01);
                send_data =  cJSON_PrintUnformatted(root);     

                  printf("send_data:%s\n",send_data);
                if(send_data != NULL)
                {
                         int  res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)send_data, (uint32_t)strlen(send_data), 0);
                        if (res < 0) 
                        {
                                printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
                        }
                }              
	        cJSON_Delete(root);	
		cJSON_free(send_data);
}

/* MQTT默认消息处理回调, 从服务器收到MQTT消息时, 且无对应用户回调处理时被调用 */
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{

    switch (packet->type) 
    {
        case AIOT_MQTTRECV_HEARTBEAT_RESPONSE:
         {
            //printf(" \n");
            /* 处理服务器对心跳的回应 */
        }
        break;

        case AIOT_MQTTRECV_SUB_ACK:
         {
            printf("suback, res: -0x%04X, packet id: %d, max qos: %d\n",
                   -packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
            /* 处理服务器对订阅请求的回应, 一般不处理 */
        }
        break;

        case AIOT_MQTTRECV_PUB: 
        {
            //printf("话题信息：qos: %d, topic: %.*s\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
            printf("\t来自IOT模块广播消息\n");
	    printf("长度: %d   \t内容: %s\n", packet->data.pub.payload_len, packet->data.pub.payload);
            IOT_MSG_Subscribe(packet->data.pub.payload);                

	    //更新模块数据信息	    			
            /* {
                char *payload = "pong";
                char resp_topic[256];
                if(packet->data.pub.topic_len > 256) {
                    break;
                }

                memset(resp_topic, 0, sizeof(resp_topic));
                memcpy(resp_topic, packet->data.pub.topic, packet->data.pub.topic_len);

                aiot_mqtt_pub(handle, resp_topic, (uint8_t *)payload, (uint32_t)strlen(payload), 0);
            } */
        }
        break;

        case AIOT_MQTTRECV_PUB_ACK: 
        {
            printf("puback, packet id: %d\n", packet->data.pub_ack.packet_id);
            /* 处理服务器对QoS1上报消息的回应, 一般不处理 */
        }
        break;

        default: {

        }
    }
}

/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS1消息重发 */
void *demo_mqtt_process_thread(void *args)
{
    int32_t res = STATE_SUCCESS;
   // 将自己设置为分离状态
   pthread_detach(pthread_self());  

    while (g_mqtt_process_thread_running) 
    {
        res = aiot_mqtt_process(args);
        if (res == STATE_USER_INPUT_EXEC_DISABLED) 
        {
            break;
        }
        sleep(1);
    }
    return NULL;
}

/* 执行aiot_mqtt_recv的线程, 包含网络自动重连和从服务器收取MQTT消息 */
void *demo_mqtt_recv_thread(void *args)
{
    int32_t res = STATE_SUCCESS;
    // 将自己设置为分离状态
    pthread_detach(pthread_self());  

    while (g_mqtt_recv_thread_running) 
    {
        res = aiot_mqtt_recv(args);
        if (res < STATE_SUCCESS) {
            if (res == STATE_USER_INPUT_EXEC_DISABLED)
             {
                break;
            }
            sleep(1);
        }
    }
    return NULL;
}


void signal_handler(int signum)
{
        int res;
	if(signum == SIGINT)
	{		
                /* 断开MQTT连接*/
                res = aiot_mqtt_disconnect(mqtt_handle);
                if (res < STATE_SUCCESS) 
                {
                        aiot_mqtt_deinit(&mqtt_handle);
                        printf("aiot_mqtt_disconnect failed: -0x%04X\n", -res);
                }

                /* 销毁MQTT实例*/
                res = aiot_mqtt_deinit(&mqtt_handle);
                if (res < STATE_SUCCESS)
                {
                        printf("aiot_mqtt_deinit failed: -0x%04X\n", -res);
                }

                g_mqtt_recv_thread_running = 0;
                g_mqtt_recv_thread_running = 0;
                //pthread_detach(g_mqtt_process_thread);
                //pthread_detach(g_mqtt_recv_thread);
                printf("\n已强制结束程序！！！\n");
		exit(0);
	}
}

int main(int argc, char *argv[])
{

   ros::init(argc, argv, "IOT_WIFI_Node");	
   ros::NodeHandle n;           //创建句柄
    ros::NodeHandle node("~");
    std::string product_key_param, device_name_param, device_secret_param, host_param, sub_topic_param;
    // 定义窗帘复位值类型
    std_msgs::Int32 Trashcan_Reset_data;

//控制IOT模块的相关话题
   ros::Subscriber Cmd_Trashcan_Sub = n.subscribe("/Trashcan_CMD_Topic", 10, &Control_Trashcan_Callback);
   ros::Subscriber Cmd_Trashcan_Stop = n.subscribe("/Trashcan_STOP_Topic", 10, &Trashcan_STOP_Callback);
   ros::Subscriber Cmd_Trashcan_Reset = n.subscribe("/Trashcan_RESET_Topic", 10, &Trashcan_RESET_Callback);
   ros::Subscriber Cmd_Trashcan_bar_Sub = n.subscribe("/Trashcan_bar_CMD_Topic", 10, &Control_Trashcan_bar_Callback);
   ros::Subscriber Cmd_Door_Sub = n.subscribe("/Door_CMD_Topic", 10, &Control_Door_Callback);
   ros::Subscriber Cmd_Lighting_Sub = n.subscribe("/Lighting_CMD_Topic", 10, &Control_Lighting_Callback);
   ros::Subscriber Cmd_Lighting_bar_Sub = n.subscribe("/Lighting_bar_CMD_Topic", 10, &Control_Lighting_bar_Callback);
   ros::Subscriber Cmd_Gateway_Sub = n.subscribe("/Gateway_CMD_Topic", 10, &Control_Gateway_Callback);    
   ros::Subscriber Elevator_Sub = n.subscribe("/Elevator_Floor_Topic", 10, &Control_Elevator_Callback);    //电梯楼层
   ros::Subscriber Update_IOT_Sub = n.subscribe("/Update_IOT_Topic", 10, &Update_IOT_Callback);     	//求情更新设备数据
//    ros::Subscriber Update_IOT_Sub = n.subscribe("/broadcast/a19EYxYfg19/IOTmsg", 10, &Update_IOT_Callback);     	//求情更新设备数据

    node.param<std::string>("product_key_param", product_key_param, "a19EYxYfg19");	
    node.param<std::string>("device_name_param", device_name_param, "HG-PcHost");	
    node.param<std::string>("device_secret_param", device_secret_param, "81c9dcad43b2468e373e5a4464b3844d");	
    node.param<std::string>("host_param", host_param, "a19EYxYfg19.iot-as-mqtt.cn-shanghai.aliyuncs.com");	
    node.param<std::string>("sub_topic_param", sub_topic_param, "/broadcast/a19EYxYfg19/IOTmsg");	

//发布IOT模块的相关话题
        Curtain_pub 	= n.advertise<std_msgs::Int32MultiArray>("/Curtain_State", 10);
        Lighting_pub 	= n.advertise<std_msgs::Int32MultiArray>("/Lighting_State", 10);
        Door_pub 	= n.advertise<std_msgs::Int32>("/Door_State", 10);
        Warehouse_pub 	= n.advertise<std_msgs::Int32>("/Warehouse_State", 10);
        Gateway_pub 	= n.advertise<std_msgs::Int32>("/Gateway_State", 10);
	Elevator_floor_pub = n.advertise<std_msgs::Int32>("/Elevator_floor_State", 10);		
	// 窗帘复位
	ros::Publisher Trashcan_Reset_pub = n.advertise<std_msgs::Int32>("/Trashcan_RESET_Topic", 10);		



    int32_t     res = STATE_SUCCESS;

    int8_t      public_instance = 1;  /* 用公共实例, 该参数要设置为1. 若用独享实例, 要将该参数设置为0 */
    char       *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com"; 
   
     //    std::string 转char类型
   const  char *product_key  =  product_key_param.c_str();
   const  char *device_name =  device_name_param.c_str();
   const   char *device_secret  =  device_secret_param.c_str();
   const   char  *host_data =  host_param.c_str();
   char  *sub_topic =  const_cast<char *>(sub_topic_param.c_str());

    // mod
    char        host[100] = {*host_data}; 
    uint16_t    port = 443;      
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */
    signal(SIGINT,signal_handler);
    /*设备的三元组 */
    // mod
//     char *product_key       = "a19EYxYfg19";
//     char *device_name       = "HG-PcHost";
//     char *device_secret     = "81c9dcad43b2468e373e5a4464b3844d";

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出 */
    aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = ali_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) 
    {
        printf("aiot_mqtt_init failed\n");
        return -1;
    }

    /* 不注释, 则会用TCP而不是TLS连接云平台 */
    /*
    {
        memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
        cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;
    }
    */
    if (1 == public_instance) 
    {
        snprintf(host, 100, "%s.%s", product_key, url);
    } else 
    {
        snprintf(host, 100, "%s", url);
    }

    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)host);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备productKey */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备deviceSecret */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);

    /* 与服务器建立MQTT连接 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS)
     {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_connect failed: -0x%04X\n", -res);
        return -1;
    }

  	/* MQTT 订阅topic功能示例, 请根据自己的业务需求进行使用 */
    {
        //char *sub_topic = "/broadcast/a19EYxYfg19/IOTmsg";

        res = aiot_mqtt_sub(mqtt_handle, sub_topic, NULL, 1, NULL);
        if (res < 0)
         {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            return -1;
        }
    }

    /* 创建一个单独的线程, 专用于执行aiot_mqtt_process, 自动发送心跳保活, 以及重发QoS1的未应答报文 */
    g_mqtt_process_thread_running = 1;
    res = pthread_create(&g_mqtt_process_thread, NULL, demo_mqtt_process_thread, mqtt_handle);
    if (res < 0) 
    {
        printf("创建发送心跳包线程失败%d\n", res);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_recv, 循环收取服务器下发的MQTT消息, 并在断线时自动重连 */
    g_mqtt_recv_thread_running = 1;
    res = pthread_create(&g_mqtt_recv_thread, NULL, demo_mqtt_recv_thread, mqtt_handle);
    if (res < 0) 
    {
        printf("创建接收线程失败: %d\n", res);
        return -1;
    }

//发布话题每个物联网模块的状态信息
// g_mqtt_recv_thread_running = 1;
 //   res = pthread_create(&publish_iot_state, NULL, Update_Iot_State, NULL);
//  if (res < 0) 
//  {
//        printf("创建发布物联网模块状态线程失败: %d\n", res);
//       return -1;
//  }

  // ros::Rate loop_rate(200);
    int cnt = 0;
    
    // 默认开机复位窗帘
    Trashcan_Reset_data.data = 1;
    Trashcan_Reset_pub.publish(Trashcan_Reset_data);

    while (ros::ok())
    {
	if(cnt%3 == 0)
       		Update_Iot_State(NULL);  //发布话题每个物联网模块的状态信息
                          
       	sleep(1);
      	cnt++;	
      //loop_rate.sleep();	              //循环等待回调函数
      ros::spinOnce();                          //循环等待回调函数
    }

//    pthread_join(g_mqtt_process_thread, NULL);
//    pthread_join(g_mqtt_recv_thread, NULL);
    return 0;
}

