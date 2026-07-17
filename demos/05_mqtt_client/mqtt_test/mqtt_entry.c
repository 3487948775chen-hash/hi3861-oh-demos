#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_wifi_api.h"
#include "mqtt_test.h" // 引入你的头文件

#include <hi_wifi_api.h>

// 【新增这一行】：引入底层 IP 网络和 DHCP 接口
#include "lwip/netifapi.h"
#include "lwip/netif.h" // 【新增这一行】专门用来定义 struct netif 结构体

/// 初始化 Wi-Fi 并连接
int hi_wifi_start_connect(void)
{
    int ret;
    hi_wifi_assoc_request assoc_req = {0};

    /* 0. 开启 STA 模式 (已修正函数名和参数) */
    char ifname[16] = {0}; // 用来存放网卡名字的缓冲区
    int len = sizeof(ifname);
    ret = hi_wifi_sta_start(ifname, &len);
    if (ret != 0) { // 0 代表成功 (HISI_OK)
        printf("[WIFI] Start STA failed! ret=%d\n", ret);
        return -1;
    }

    /* 1. 设置 WiFi SSID (账号) */
    strcpy(assoc_req.ssid, "cyrmagic"); 
    
    /* 2. 设置加密方式 */
    assoc_req.auth = HI_WIFI_SECURITY_WPA2PSK;

    /* 3. 设置 WiFi 密码 */
    strcpy(assoc_req.key, "87654321");

    /* 4. 发起连接 */
    ret = hi_wifi_sta_connect(&assoc_req);
    if (ret != 0) {
        printf("[WIFI] Connect failed! ret=%d\n", ret);
        return -1;
    }
    printf("[WIFI] Connect Success!\n");
    return 0;
}
// 主任务入口

static void MqttMainTask(void *arg)
{
    (void)arg;
    
    // 1. 先连接 WiFi
    if (hi_wifi_start_connect() == 0) {
        printf("[WIFI] Connected to AP! Starting DHCP...\n");
        
        // 【新增代码开始】：启动 DHCP 客户端去获取 IP
        struct netif *netif = netifapi_netif_find("wlan0"); // 找到无线网卡
        if (netif) {
            netifapi_dhcp_start(netif); // 告诉网卡去向热点索要 IP
            
            // 必须多等一会儿，给手机/路由器分配 IP 的时间
            osDelay(3000); 
            
            // （可选）打印出拿到的 IP 地址，成功的话就能在串口看到了！
            printf("[WIFI] Got IP: %s\n", ipaddr_ntoa(&netif->ip_addr));
        }
        // 【新增代码结束】

        // 2. 拿到 IP 后，再启动 MQTT 连接测试
        mqtt_connect(); 
    }
}

// 注册入口函数
static void MqttTestEntry(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "MqttMainTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; // 分配足够的栈空间
    attr.priority = osPriorityNormal;

    if (osThreadNew(MqttMainTask, NULL, &attr) == NULL) {
        printf("[MQTT] Failed to create MqttMainTask!\n");
    }
}

SYS_RUN(MqttTestEntry);