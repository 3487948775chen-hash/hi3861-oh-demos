#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include <hi_types_base.h>
#include <hi_early_debug.h>
#include <hi_pwm.h>
// 新增的硬件控制头文件
#include <hi_gpio.h>
#include <hi_io.h>

hi_void pwm_0_demo(hi_u16 duty)
{
    static int initialized = 0;
    hi_u16 freq = 4000; 

    if (!initialized) {
        hi_gpio_init();
        hi_io_set_func(9,5); 
        hi_gpio_set_dir(9, HI_GPIO_DIR_OUT);
        
        hi_pwm_init(HI_PWM_PORT_PWM0);
        hi_pwm_set_clock(PWM_CLK_160M);
        
        initialized = 1;
    }

    hi_pwm_start(HI_PWM_PORT_PWM0, duty, freq);
}

hi_void app_demo_pwm(hi_void)
{
    static int direction = 1;  
    static hi_u16 duty_val = 3900; 

    if (direction == 1) {
        duty_val -= 100; 
        if (duty_val <= 100) {
            direction = 0; 
        }
    } else {
        duty_val += 100; 
        if (duty_val >= 3900) {
            direction = 1; 
        }
    }

    pwm_0_demo(duty_val);
}

void *PWM_Task(const char *arg)
{
    arg = arg;

    while(1)
    {
        app_demo_pwm();
        usleep(20000); // 丝滑呼吸的关键延时
    }
}

void pwm_demo(void)
{
    osThreadAttr_t attr;

    attr.name = "PWM_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = 26;

    if (osThreadNew((osThreadFunc_t)PWM_Task, NULL, &attr) == NULL) {
        printf("[PWM_Task] Falied to create PWM_Task!\n");
    }
}

SYS_RUN(pwm_demo);