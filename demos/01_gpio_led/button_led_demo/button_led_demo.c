#include <unistd.h>
#include "stdio.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
// 【新增】引入海思特有的引脚控制头文件，用于配置上拉电阻
#include "hi_io.h" 

#define LED_GPIO 9      
#define BUTTON_GPIO 5   

void *ButtonLedTask(const char *arg)
{
    IotGpioValue btnValue = IOT_GPIO_VALUE1;

    // 1. 初始化 GPIO
    IoTGpioInit(LED_GPIO);
    IoTGpioInit(BUTTON_GPIO);

    // 2. 设置 LED 引脚为输出模式
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);

    // 3. 设置按键引脚为输入模式
    IoTGpioSetDir(BUTTON_GPIO, IOT_GPIO_DIR_IN);

    // 【核心新增】将 GPIO 05 管脚复用功能设为普通GPIO，并开启内部上拉电阻（HI_IO_PULL_UP）
    // 这样未按下时引脚才能稳定保持高电平，按下时才能被拉低到低电平
    hi_io_set_func(HI_IO_NAME_GPIO_5, HI_IO_FUNC_GPIO_5_GPIO);
    hi_io_set_pull(HI_IO_NAME_GPIO_5, HI_IO_PULL_UP);

    (void) arg;
    while (1)
    {
        // 4. 读取按键状态
        IoTGpioGetInputVal(BUTTON_GPIO, &btnValue);

        // 5. 判断按键状态
        if (btnValue == IOT_GPIO_VALUE0) 
        {
            // 按键按下（低电平）：LED 亮
            IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0);
            printf("HI_GPIO_VALUE_1\n"); 
        } 
        else 
        {
            // 按键抬起（高电平）：LED 灭
            IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1);
            printf("HI_GPIO_VALUE_0\n");
        }

        usleep(100000); 
    }

    return NULL;
}

void ButtonLedDemo(void)
{
    osThreadAttr_t attr;
    attr.name = "ButtonLedTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024; 
    attr.priority = 25;

    if (osThreadNew((osThreadFunc_t)ButtonLedTask, NULL, &attr) == NULL) {
        printf("[ButtonLedDemo] Failed to create ButtonLedTask!\n");
    }
}

SYS_RUN(ButtonLedDemo);