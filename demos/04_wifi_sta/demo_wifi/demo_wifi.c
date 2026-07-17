#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

// 引入海思官方 Wi-Fi 核心接口
#include <hi_wifi_api.h>

void app_demo_wifi(void)
{
    int ret;
    char ifname[64] = {0};
    int len = sizeof(ifname);

    printf("[WiFi_Demo] ========= Start WiFi Experiment =========\n");

    // (1) 开启 STA 模式
    ret = hi_wifi_sta_start(ifname, &len);
    if (ret != 0) {
        printf("[WiFi_Demo] hi_wifi_sta_start failed, error = %d\n", ret);
        return;
    }
    printf("[WiFi_Demo] (1) STA Mode started on interface: %s\n", ifname);

    // (3) 扫描附近的热点
    ret = hi_wifi_sta_scan();
    if (ret != 0) {
        printf("[WiFi_Demo] hi_wifi_sta_scan failed\n");
    }
    printf("[WiFi_Demo] (2) Scan command sent, waiting 2s...\n");
    sleep(2); 

    // 准备连接参数结构体
    hi_wifi_assoc_request req = {0};
    
    // ========================================================
    const char *ssid = "cyrmagic";   // 👈 【改成你的 2.4GHz 手机热点名】
    const char *passwd = "87654321";   // 👈 【改成你的热点密码】
    // ========================================================

    memcpy(req.ssid, ssid, strlen(ssid));
    memcpy(req.key, passwd, strlen(passwd));
    
    // (通常加密方式是：HI_WIFI_SECURITY_WPA2PSK)
    req.auth = HI_WIFI_SECURITY_WPA2PSK;

    printf("[WiFi_Demo] (3) Connecting to SSID: %s\n", req.ssid);

    // (4) 连接热点
    ret = hi_wifi_sta_connect(&req);
    if (ret != 0) {
        printf("[WiFi_Demo] hi_wifi_sta_connect failed, error = %d\n", ret);
        return;
    }

    // 因为 Wi-Fi 握手在底层是异步进行的，我们在这里在主线程等待 8 秒
    // 留出充足的时间给芯片和手机热点完成空中握手和密码校验
    printf("[WiFi_Demo] (4) Connect command success! Device is connecting in background...\n");
    printf("[WiFi_Demo] Please wait about 8 seconds to check your phone hotspot connection status.\n");
    
    sleep(8); 
    
    printf("[WiFi_Demo] ========= WiFi Experiment Done! =========\n");
}

// 包装 CMSIS 任务线程
void WiFi_Task(void *arg)
{
    (void)arg;
    app_demo_wifi();

    while(1) {
        usleep(1000000);
    }
}

// 入口注册
void wifi_demo_entry(void)
{
    osThreadAttr_t attr;
    attr.name = "WiFi_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; 
    attr.priority = 24;

    if (osThreadNew((osThreadFunc_t)WiFi_Task, NULL, &attr) == NULL) {
        printf("[WiFi_Demo] Failed to create WiFi_Task!\n");
    }
}

SYS_RUN(wifi_demo_entry);