#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <hi_types_base.h>
#include <hi_io.h>
#include <hi_early_debug.h>
#include <hi_gpio.h>
#include "iot_gpio.h"
#include <hi_task.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#define APP_DEMO_ADC
#include <hi_adc.h>
#include <hi_stdlib.h>

#define KEY_EVENT_NONE   0  
#define KEY_EVENT_S1     1  
#define KEY_EVENT_S2     2  
#define KEY_EVENT_S3     3  
#define KEY_EVENT_S4     4  
#define ADC_TEST_LENGTH  64  
#define VLT_MIN          100 
#define LED_GPIO         9   // 主板LED通常连接在GPIO 9

hi_u16 g_adc_buf[ADC_TEST_LENGTH] = { 0 };
int key_status = KEY_EVENT_NONE;
char key_flg = 0;

// 获取当前按键事件并清除
int get_key_event(void)
{
    int tmp = key_status;
    key_status = KEY_EVENT_NONE;
    return tmp;
}

/* ADC 值转换为电压并判定按键 */
hi_void convert_to_voltage(hi_u32 data_len)
{
    hi_u32 i;
    float vlt_max = 0;
    float vlt_min = VLT_MIN;
    float vlt_val = 0;
    hi_u16 vlt;
    for (i = 0; i < data_len; i++) {
        vlt = g_adc_buf[i];
        float voltage = (float)vlt * 1.8 * 4 / 4096.0;
        vlt_max = (voltage > vlt_max) ? voltage : vlt_max;
        vlt_min = (voltage < vlt_min) ? voltage : vlt_min;
    }

    vlt_val = (vlt_min + vlt_max) / 2.0;

    // S1 按键判定 (OLED板按键)
    if ((vlt_val > 0.4) && (vlt_val < 0.6)) {
        if (key_flg == 0) {
            key_flg = 1;
            key_status = KEY_EVENT_S1;
        }
    }
    // S2 按键判定 (OLED板按键)
    else if ((vlt_val > 0.8) && (vlt_val < 1.1)) {
        if (key_flg == 0) {
            key_flg = 1;
            key_status = KEY_EVENT_S2;
        }
    }
    // S3 按键判定 (主板自定义按键)
    else if ((vlt_val > 0.01) && (vlt_val < 0.3)) {
        if (key_flg == 0) {
            key_flg = 1;
            key_status = KEY_EVENT_S3;
        }
    }
    // 松开按键
    else if (vlt_val > 3.0) {
        key_flg = 0;
        key_status = KEY_EVENT_NONE;
    }
}

/* 循环读取 ADC2 通道 */
void app_demo_adc_test(void)
{
    hi_u32 ret, i;
    hi_u16 data;

    memset_s(g_adc_buf, sizeof(g_adc_buf), 0x0, sizeof(g_adc_buf));

    for (i = 0; i < ADC_TEST_LENGTH; i++) {
        ret = hi_adc_read((hi_adc_channel_index)HI_ADC_CHANNEL_2, &data, HI_ADC_EQU_MODEL_1, HI_ADC_CUR_BAIS_DEFAULT, 0);
        if (ret != HI_ERR_SUCCESS) {
            printf("ADC Read Fail\n");
            return;
        }
        g_adc_buf[i] = data;
    }
    convert_to_voltage(ADC_TEST_LENGTH);
}

/* 核心控制线程 */
void my_gpio_adc_control_demo(void *arg)
{
    (void)arg;
    IotGpioValue ledValue = IOT_GPIO_VALUE1; // 默认灭

    // 1. 初始化 GPIO 并配置 LED
    IoTGpioInit(LED_GPIO);
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(LED_GPIO, ledValue); // 初始关闭 LED

    // 2. 配置复用引脚 GPIO 5 作为模拟量输入（绝对不能配置内部上拉！）
    hi_io_set_func(HI_IO_NAME_GPIO_5, HI_IO_FUNC_GPIO_5_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_5, HI_GPIO_DIR_IN);

    printf("ADC Key LED Control Task Started!\r\n");

    while (1)
    {
        // 读取并分析当前的 ADC 按键状态
        app_demo_adc_test();

        // 获取并消费按键事件（get_key_event 确保每次按键只进入一次 case 分支）
        switch (get_key_event())
        {
            case KEY_EVENT_NONE:
                break;

            case KEY_EVENT_S1: // 打开主板 LED 灯
                IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0); // 假设低电平点亮，若相反请改VALUE1
                printf("KEY_EVENT_S1 >>> LED ON\r\n");
                break;

            case KEY_EVENT_S2: // 关闭主板 LED 灯
                IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1); // 假设低电平熄灭
                printf("KEY_EVENT_S2 >>> LED OFF\r\n");
                break;

            case KEY_EVENT_S3: // 切换（反转）主板 LED 灯状态
                // 获取当前 LED 的电平状态
                IoTGpioGetInputVal(LED_GPIO, &ledValue);
                if (ledValue == IOT_GPIO_VALUE0) {
                    IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1);
                    printf("KEY_EVENT_S3 >>> LED OFF\r\n");
                } else {
                    IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0);
                    printf("KEY_EVENT_S3 >>> LED ON\r\n");
                }
                break;

            default:
                break;
        }

        // 核心修复：添加系统延时，释放 CPU，平滑消抖
        osDelay(5); // 延时约 50ms
    }
}

void key_demo(void)
{
    osThreadAttr_t attr;

    attr.name = "KeyControlTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096; 
    attr.priority = osPriorityNormal;

    // 创建并运行合并后的任务
    if (osThreadNew((osThreadFunc_t)my_gpio_adc_control_demo, NULL, &attr) == NULL) {
        printf("[key_demo] Failed to create KeyControlTask!\n");
    }
}

SYS_RUN(key_demo);