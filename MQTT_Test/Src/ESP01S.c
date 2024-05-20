#include "main.h"
#include "ESP01S.h"
#include "usart.h"
#include "lcd.h"

// #define Reply_ON
#define ESP_Open_Receive HAL_UARTEx_ReceiveToIdle_DMA(&huart1, USART1_RX_BUF, USART_REC_LEN)
#define ESP_Close_Receive HAL_UART_AbortReceive(&huart1)
#define ESP_Transmit(_cmd) printf(_cmd)
// #define ESP_Transmit_long(_cmd) HAL_UART_Transmit(&huart1, (unsigned char *)_cmd, strlen(_cmd), 0xFFFF);

#define wifi_SSID "CLcongut"
#define wifi_PSWD "qwertyui"

#define mqtt_USERNAME "dht11&a15ZWaMgl9e"
#define mqtt_PASSWORD "4FF383A5FD3E9567CF41EF11A84DA9E6E852B606"
#define mqtt_CLIENTID "30383|securemode=3\\,signmethod=hmacsha1|"
#define mqtt_HOST_URL "a15ZWaMgl9e.iot-as-mqtt.cn-shanghai.aliyuncs.com"

#define mqtt_pub_TOPIC "/sys/a15ZWaMgl9e/${deviceName}/thing/event/property/post"
#define mqtt_sub_TOPIC "/sys/a15ZWaMgl9e/${deviceName}/thing/service/property/set"

#define cmd_RST "AT+RST\r\n"
#define cmd_REPLY "ATE0\r\n"
#define cmd_CWQAP "AT+CWQAP\r\n"
#define cmd_CWMODE "AT+CWMODE=1\r\n"
#define cmd_AUTOCONN "AT+CWAUTOCONN=0\r\n"
#define cmd_CIPSNTPCFG "AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"\r\n"
/*ESP01S模块供电不足容易导致无法连接AP*/
#define cmd_CWJAP "AT+CWJAP=\"" wifi_SSID "\",\"" wifi_PSWD "\"\r\n"
#define cmd_MQTTUSERCFG "AT+MQTTUSERCFG=0,1,\"NULL\",\"" mqtt_USERNAME "\",\"" mqtt_PASSWORD "\",0,0,\"\"\r\n"
#define cmd_MQTTCLIENTID "AT+MQTTCLIENTID=0,\"" mqtt_CLIENTID "\"\r\n"
#define cmd_MQTTCONN "AT+MQTTCONN=0,\"" mqtt_HOST_URL "\",1883,1\r\n"

void gui_dp_info(const uint8_t *_info)
{
    LCD_Fill(0, 0, 128, 12, WHITE);
    LCD_ShowString(0, 0, _info, BLACK, WHITE, 12, 0);
}

void ESP_JudgeWIFI(void)
{
    uint8_t data[25];
    /*全局结构体成员，使用也要创个变量来赋值，不可直接使用，更不能强制转换*/
    uint8_t *data_rx;
    while (1)
    {
        data_rx = RX_DS.rx_data;
        /*使用 %*[^\r\n] 来跳过内容时，必须在\r\n前还有内容，否则会卡住，不如直接使用以下方式*/
        if (data_rx[0] == 'W')
        {
            sscanf((const char *)data_rx, "WIFI %1[^ ]", data);
            if (memcmp((const char *)data, "C", strlen("C")) == 0) // WIFI CONNECTED
            {
                gui_dp_info("WIFI CONNECTED");
                break;
            }
            else if (memcmp((const char *)data, "G", strlen("G")) == 0) // WIFI GOT IP
            {
                gui_dp_info("WIFI CONNECTED");
                break;
            }
        }
        else if (data_rx[0] == '+')
        {
            gui_dp_info("WIFI NOT FOUND");
            /*重新开始流程*/
            ESP_Init();
        }
    }
}

void ESP_Init(void)
{
#if 1
    /**
     * @brief 延时稳定，关闭串口接收，准备
     *
     */
    HAL_Delay(1500);
    ESP_Close_Receive;
    gui_dp_info("READY FOR ESP01");

#ifdef Reply_ON
    /**
     * @brief 关闭回显
     *
     */
    ESP_Transmit(cmd_REPLY);
    HAL_Delay(1500);
    gui_dp_info("CLOSE REPLY");
#endif

    /**
     * @brief 断开AP连接
     *
     */
    ESP_Transmit(cmd_CWQAP);
    HAL_Delay(1500);
    gui_dp_info("WIFI DISCONN");

    /**
     * @brief 重启ESP01S
     *
     */
    ESP_Transmit(cmd_RST);
    gui_dp_info("RESET ESP01");
    HAL_Delay(3000);

#ifdef Reply_ON
    /**
     * @brief 关闭回显
     *
     */
    ESP_Transmit(cmd_REPLY);
    HAL_Delay(1500);
    ESP_Judge("OK");
    gui_dp_info("CLOSE REPLY");
#endif

    /**
     * @brief 切换模式 1 station模式
     *
     */
    ESP_Transmit(cmd_CWMODE);
    HAL_Delay(1500);
    gui_dp_info("SET ESP01 MODE");

    /**
     * @brief 关闭上电自动连接AP
     *
     */
    ESP_Transmit(cmd_AUTOCONN);
    HAL_Delay(1500);
    gui_dp_info("CLOSE AUTO CONN");

    /**
     * @brief 设置时区服务器
     *
     */
    ESP_Transmit(cmd_CIPSNTPCFG);
    HAL_Delay(1500);
    gui_dp_info("SET TIME ZONE");

    /**
     * @brief 设置AP名字和密码，并且连接
     *
     */
    ESP_Transmit(cmd_CWJAP);
    ESP_Open_Receive;
    HAL_Delay(1500);
    gui_dp_info("WIFI SET DONE");
    ESP_JudgeWIFI();
#endif

    // ESP_Transmit(cmd_MQTTUSERCFG);
    // HAL_Delay(1500);
    // ESP_Judge("OK");
    // gui_dp_info("SET MQTT USERCFG");

    // ESP_Transmit(cmd_MQTTCLIENTID);
    // HAL_Delay(1500);
    // ESP_Judge("OK");
    // gui_dp_info("SET MQTT CLINTID");

    // ESP_Transmit(cmd_MQTTCONN);
    // HAL_Delay(1500);
    // ESP_Judge("OK");
    // gui_dp_info("SET MQTT URL");
}