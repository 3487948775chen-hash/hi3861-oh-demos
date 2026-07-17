#include <unistd.h>
#include "stdio.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"

#define LED_TEST_GPIO 9 // Pegasus核心板板载LED1连接的引脚

void *LedTask(const char *arg)
{
    // 1. 初始化GPIO外设
    IoTGpioInit(LED_TEST_GPIO);

    // 2. 设置引脚方向为 输出模式(OUTPUT)
    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);

    (void) arg;
    while (1)
    {
        // 3. 根据你上传的驱动框架手册，Hi3861的电平输出函数为 IoTGpioSetOutputVal
        // 输出低电平，板载LED灯亮
        IoTGpioSetOutputVal(LED_TEST_GPIO, IOT_GPIO_VALUE0);
        usleep(300000); // 延时 300 毫秒

        // 输出高电平，板载LED灯灭
        IoTGpioSetOutputVal(LED_TEST_GPIO, IOT_GPIO_VALUE1);
        usleep(300000); // 延时 300 毫秒
    }

    return NULL;
} 

void led_demo(void)
{
    osThreadAttr_t attr;

    attr.name = "LedTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024; // 适当提升栈大小，防止程序崩溃
    attr.priority = 26;

    if (osThreadNew((osThreadFunc_t)LedTask, NULL, &attr) == NULL) {
        printf("[LedExample] Failed to create LedTask!\n");
    }
} 

// 让系统在启动时自动运行该模块
SYS_RUN(led_demo);