#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"

// 【千万别用 Network.h】
// 1. 先包含底层操作系统的网络接口
#include "MQTTLiteOS.h" 
// 2. 再包含 Paho-MQTT 客户端核心
#include "MQTTClient.h"

// 引入底层 IP 网络和 DHCP 接口
#include "lwip/netifapi.h"
#include "lwip/netif.h" 

// 声明全局变量
Network n;
MQTTClient mq_client;
unsigned char *onenet_mqtt_buf;
unsigned char *onenet_mqtt_readbuf;
int buf_size;

// ===================================================
// 【新增】：消息接收回调函数 (快递代收点)
// 当电脑向开发板发布消息时，这个函数会自动被触发
// ===================================================
void messageArrived(MessageData* data)
{
    printf("\n===================================\n");
    // 1. 打印收到的主题名字
    printf("[MQTT 接收] 主题: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data);
    
    // 2. 打印收到的具体内容
    printf("[MQTT 接收] 内容: %.*s\n", data->message->payloadlen, (char*)data->message->payload);
    printf("===================================\n\n");
}
// ===================================================


int mqtt_connect(void)
{
    int rc = 0;
    
    /* 1. 初始化网络与连接服务器 */
    NetworkInit(&n);
    printf("Connecting to Network...\n");
    
    // 判断底层 TCP 是否连接成功
    rc = NetworkConnect(&n, "10.197.203.53", 1883); 
    if (rc != 0) {
        printf("NetworkConnect failed! rc = %d\n", rc);
        return -1; // 连不上就直接退出，不要往下走
    }
    printf("NetworkConnect Success!\n");

    /* 2. 分配内存缓冲区 */
    buf_size = 4096 + 1024;
    onenet_mqtt_buf = (unsigned char *) malloc(buf_size);
    onenet_mqtt_readbuf = (unsigned char *) malloc(buf_size);
    if (!(onenet_mqtt_buf && onenet_mqtt_readbuf))
    {
        printf("No memory for MQTT client buffer!\n");
        return -2;
    }

    /* 3. 初始化 MQTT 客户端 */
    MQTTClientInit(&mq_client, &n, 1000, onenet_mqtt_buf, buf_size, onenet_mqtt_readbuf, buf_size);
    
    /* 4. 配置连接参数 (账号密码) */
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.keepAliveInterval = 30;
    data.cleansession = 1;
    data.clientID.cstring = "ohos_hi3861";
    data.username.cstring = "Still";
    data.password.cstring = "123456chen";

    /* 5. 发起 MQTT 连接 */
    rc = MQTTConnect(&mq_client, &data);
    if (rc != 0) {
        printf("MQTTConnect failed, rc = %d\n", rc);
        return -1;
    }
    printf("MQTT Connected Successful!\n");

    // 只有在连接彻底成功后，才启动后台任务，完美避开看门狗复位！
    MQTTStartTask(&mq_client);

    // ===================================================
    // 【新增】：向服务器订阅主题，让板子长“耳朵”
    // ===================================================
    // 让板子订阅 "test/cmd" 主题。如果有消息，就交给 messageArrived 处理
    rc = MQTTSubscribe(&mq_client, "name", QOS0, messageArrived);
    if (rc != 0) {
        printf("MQTT Subscribe failed!\n");
    } else {
        printf("MQTT Subscribe Success! Listening ...\n");
    }
    // ===================================================

    /* 6. 发布消息 (保留你原来的大喇叭功能) */
    MQTTMessage message;
    char payload[] = "37120242203893";
    
    message.qos = QOS0;
    message.retained = 0;
    message.payload = (void *)payload;
    message.payloadlen = strlen(payload);

    rc = MQTTPublish(&mq_client, "ID", &message);
    if (rc != 0) {
        printf("MQTT Publish failed!\n");
    } else {
        printf("MQTT Publish Success!\n");
    }

    return 0;
}