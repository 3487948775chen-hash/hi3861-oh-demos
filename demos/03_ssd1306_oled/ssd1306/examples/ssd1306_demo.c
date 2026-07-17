/*
 * Copyright (c) 2020, HiHope Community.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_pwm.h"
#include "iot_i2c.h"
#include "iot_errno.h"

#include "ssd1306.h"
#include "ssd1306_tests.h"

#include "hi_io.h"

#define OLED_I2C_BAUDRATE 400*1000

void TestGetTick(void)
{
    for (int i = 0; i < 20; i++) {
        usleep(10*1000);
        printf("HAL_GetTick(): %d\r\n", HAL_GetTick());
    }

    for (int i = 0; i < 20; i++) {
        HAL_Delay(25);
        printf(" HAL_GetTick(): %d\r\n", HAL_GetTick());
    }
}


/**
 * 汉字字模在线： https://www.23bei.com/tool-223.html
 * 数据排列：从左到右从上到下
 * 取模方式：横向8位左高位
**/
void TestDrawChinese1(void)
{
    const uint32_t W = 16, H = 16;
    uint8_t fonts[][32] = {
        {0x00,0x40,0x78,0x40,0x48,0x40,0x57,0xFE,0x50,0x80,0x61,0x20,0x51,0x20,0x4A,0x20,0x4B,0xFC,0x48,0x20,0x69,0x28,0x51,0x24,0x42,0x22,0x44,0x22,0x40,0xA0,0x40,0x40},/*"陈",0*/
{0x08,0x80,0x08,0x40,0x08,0x40,0x17,0xFE,0x10,0x80,0x30,0x80,0x31,0x40,0x51,0x44,0x93,0x48,0x15,0x30,0x19,0x20,0x11,0x10,0x11,0x08,0x11,0x44,0x11,0x82,0x11,0x00},/*"依",1*/
{0x10,0x20,0x10,0x28,0x1E,0x24,0x22,0x20,0x33,0xFE,0x4A,0x20,0xA4,0x50,0x14,0x50,0x08,0x88,0x10,0x88,0x21,0x04,0x42,0x02,0x00,0x00,0x48,0x88,0x44,0x44,0x84,0x44},/*"然",2*/

};




    ssd1306_Fill(Black);
    for (size_t i = 0; i < sizeof(fonts)/sizeof(fonts[0]); i++) {
        ssd1306_DrawRegion(i * W, 0, W, H, fonts[i], sizeof(fonts[0]), W);
    }
    ssd1306_UpdateScreen();
    sleep(3);
}

void TestDrawChinese2(void)
{
    ssd1306_Fill(Black);
    
    // 设置在 X=15, Y=0 开始写，刚好在屏幕中央一行放下
    ssd1306_SetCursor(15, 0); 
    ssd1306_DrawString("37120242203893", Font_7x10, White);
    
    ssd1306_UpdateScreen();
    sleep(3);
}

void Ssd1306TestTask(void* arg)
{
    (void) arg;
    IoTGpioInit(HI_IO_NAME_GPIO_13);
    IoTGpioInit(HI_IO_NAME_GPIO_14);

    hi_io_set_func(HI_IO_NAME_GPIO_13, HI_IO_FUNC_GPIO_13_I2C0_SDA);
    hi_io_set_func(HI_IO_NAME_GPIO_14, HI_IO_FUNC_GPIO_14_I2C0_SCL);
    
    IoTI2cInit(0, OLED_I2C_BAUDRATE);

    //WatchDogDisable();

    usleep(20*1000);
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("Hello HarmonyOS!", Font_7x10, White);

    uint32_t start = HAL_GetTick();
    ssd1306_UpdateScreen();
    uint32_t end = HAL_GetTick();
    printf("ssd1306_UpdateScreen time cost: %d ms.\r\n", end - start);

    TestDrawChinese1();
    TestDrawChinese2();

    TestGetTick();
    while (1) {
        ssd1306_TestAll();
        usleep(10000);
    }
}

void Ssd1306TestDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "Ssd1306Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(Ssd1306TestTask, NULL, &attr) == NULL) {
        printf("[Ssd1306TestDemo] Falied to create Ssd1306TestTask!\n");
    }
}
APP_FEATURE_INIT(Ssd1306TestDemo);
