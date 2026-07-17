\# hi3861-oh-demos

基于Hi3861 WiFi IoT开发套件的OpenHarmony全套基础实验代码

配套电子工艺实训完整实验报告、硬件实拍、串口日志截图，适用于通信工程课程实训。



\## 开发环境

\- 硬件：HiSpark Pegasus Hi3861V100 WiFi IoT开发板

\- 系统：OpenHarmony LiteOS-M 4.0

\- 编译工具：hb build

\- 烧录工具：HiBurn

\- 调试工具：串口Monitor



\## 实验清单（对应实训报告全部内容）

\### 01 GPIO实验（点灯+按键控制LED）

1\. LED周期闪烁：GPIO9输出高低电平翻转，300ms间隔流水闪烁

2\. 按键控制LED：GPIO5配置内部上拉输入，按下点亮、松开熄灭

代码位于 `demos/01\_gpio\_led/`



\### 02 ADC分压多按键实验

通过ADC2通道读取模拟电压，区分S1/S2/S3三个独立按键，分别控制LED亮灭、翻转。

解决单引脚扩展多路输入，资源受限场景IO复用方案。

代码位于 `demos/02\_adc\_multi\_key/`

配套日志截图：`docs/hardware\_pic/adc\_monitor.png`



\### 03 SSD1306 I2C OLED显示实验

I2C0（GPIO13-SDA、GPIO14-SCL）驱动0.96寸OLED：

\- 打印鸿蒙英文标语

\- 16\*16汉字字模显示姓名「陈依然」

\- 显示学号37120242203893

配套硬件实拍：`docs/hardware\_pic/oled\_screen1.png`、`oled\_screen2.png`

代码位于 `demos/03\_ssd1306\_oled/`



\### 04 WiFi STA联网实验

开发板切换Station模式，扫描周边WiFi，连接指定热点`cyrmagic`，获取DHCP分配IP。

配套串口日志：`docs/hardware\_pic/wifi\_log.png`

代码位于 `demos/04\_wifi\_sta/`



\### 05 MQTT物联网通信实验

完整端云通信链路：WiFi联网 → DHCP获取IP → 连接MQTT Broker

\- 订阅`name`主题接收云端下发消息

\- 发布`ID`主题上传本地学号

配套MQTT收发日志：`docs/hardware\_pic/mqtt\_log.png`

代码位于 `demos/05\_mqtt\_client/`



\## 快速编译使用步骤

1\. 将本仓库代码放入OpenHarmony源码 `applications/sample/wifi-iot/app/` 目录

2\. 修改顶层BUILD.gn，开启需要编译的demo（注释不需要的实验）

3\. 终端执行 `hb set` 选择 `wifiiot\_hispark\_pegasus`

4\. 全量编译：`hb build -f`

5\. 使用HiBurn烧录生成的镜像文件，串口Monitor查看输出



\## 配套文档说明

`docs/实验报告原文.docx`：完整实训报告，包含OpenHarmony理论、全部实验代码、现象分析、硬件小车拓展内容。



\## 关联项目

综合循迹遥控智能小车完整工程仓库：hi3861-oh-smart-car



\## License

MIT

