#include "main.h"
#include "ESP01S.h"
#include "usart.h"
#include "lcd.h"
#include "DHT11.h"

/*回显开关*/
// #define REPLAY_ON

/*调试，只重复MQTT部分*/
// #define MQTT_ONLY

/********************************************************************************
 * 移植需求更改
 * 开始 >>
 */
/*开启串口DMA接收*/
#define ESP_Open_Receive HAL_UARTEx_ReceiveToIdle_DMA(&huart1, USART1_RX_BUF, USART_REC_LEN)
/*关闭串口DMA接收*/
#define ESP_Close_Receive HAL_UART_AbortReceive(&huart1)
/*串口发送*/
#define ESP_Transmit(_cmd) printf(_cmd)
/********************************************************************************
 * << 结束
 */

/********************************************************************************
 * 根据不同用户需要修改的部分
 * 开始 >>
 */
/*要让ESP01S连接的WIFI信息*/
#define wifi_SSID "CLcongut" // WIFI名字，不可中文
#define wifi_PSWD "qwertyui" // WIFI密码，不可中文，最好 8 位

/*设备名称*/
#define DeviceName "dht11"

/*MQTT三元组*/
#define mqtt_USERNAME "dht11&a15ZWaMgl9e"
#define mqtt_PASSWORD "4FF383A5FD3E9567CF41EF11A84DA9E6E852B606"
#define mqtt_CLIENTID "30383|securemode=3\\,signmethod=hmacsha1|" // 逗号前需要加两个反斜杠转义

/*MQTT域名*/
#define mqtt_HOST_URL "a15ZWaMgl9e.iot-as-mqtt.cn-shanghai.aliyuncs.com"

/*发布主题*/
#define mqtt_pub_TOPIC "/sys/a15ZWaMgl9e/" DeviceName "/thing/event/property/post"
/*订阅主题*/
#define mqtt_sub_TOPIC "/sys/a15ZWaMgl9e/" DeviceName "/thing/service/property/set"
/********************************************************************************
 * << 结束
 */

/********************************************************************************
 * 根据应用需求修改的部分，皆为AT命令
 * 开始 >>
 */
#define cmd_RST "AT+RST\r\n"                                                         // 重启
#define cmd_REPLY "ATE0\r\n"                                                         // 关闭回显
#define cmd_CWQAP "AT+CWQAP\r\n"                                                     // 关闭AP连接
#define cmd_MQTTCLEAN "AT+MQTTCLEAN=0\r\n"                                           // 关闭MQTT连接
#define cmd_CWMODE "AT+CWMODE=1\r\n"                                                 // 切换模式1：STATION模式
#define cmd_AUTOCONN "AT+CWAUTOCONN=0\r\n"                                           // 关闭上电自动连接AP
#define cmd_CIPSNTPCFG "AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"\r\n" // 设置时区服务器
/*ESP01S模块供电不足容易导致无法连接AP*/
#define cmd_CWJAP "AT+CWJAP=\"" wifi_SSID "\",\"" wifi_PSWD "\"\r\n"                                           // 连接AP
#define cmd_MQTTUSERCFG "AT+MQTTUSERCFG=0,1,\"NULL\",\"" mqtt_USERNAME "\",\"" mqtt_PASSWORD "\",0,0,\"\"\r\n" // 设置MQTT用户名，密码
#define cmd_MQTTCLIENTID "AT+MQTTCLIENTID=0,\"" mqtt_CLIENTID "\"\r\n"                                         // 设置MQTT CLIENT ID
#define cmd_MQTTCONN "AT+MQTTCONN=0,\"" mqtt_HOST_URL "\",1883,1\r\n"                                          // 连接MQTT域名
/********************************************************************************
 * << 结束
 */

/**
 * @brief WIFI是否连接标志位
 *
 */
uint8_t MQTT_CONN_Flag = 0;

uint8_t tah_data[5];

/**
 * @brief 内部函数，使用屏幕显示提示内容
 *
 * @param _info 需要显示的内容
 */
void gui_dp_info(const uint8_t *_info)
{
    LCD_Fill(0, 0, 128, 12, WHITE);
    LCD_ShowString(0, 0, _info, BLACK, WHITE, 12, 0);
}

void ESP_RXBUF_Clear(void)
{
    memset(USART1_RX_BUF, 0x00, USART_REC_LEN);
}

/**
 * @brief 判断WIFI是否连接成功，若超时无法找到或连接WIFI，则重新尝试连接
 *
 */
// void ESP_JudgeWIFI(void)
// {
//     uint8_t data[25];
//     /*全局结构体成员，使用也要创个变量来赋值，不可直接使用，更不能强制转换*/
//     uint8_t *data_rx;
//     while (1)
//     {
//         data_rx = RX_DS.rx_data;
//         /*使用 %*[^\r\n] 来跳过内容时，必须在\r\n前还有内容，否则会卡住，不如直接使用以下方式*/
//         if (data_rx[0] == 'W')
//         {
//             sscanf((const char *)data_rx, "WIFI %1[^ ]", data);
//             if (memcmp((const char *)data, "C", strlen("C")) == 0) // WIFI CONNECTED
//             {
//                 gui_dp_info("WIFI CONNECTED");
//                 break;
//             }
//             else if (memcmp((const char *)data, "G", strlen("G")) == 0) // WIFI GOT IP
//             {
//                 gui_dp_info("WIFI CONNECTED");
//                 break;
//             }
//         }
//         else if (data_rx[0] == '+') //+CWJAP
//         {
//             gui_dp_info("WIFI NOT FOUND");
//             /*重新开始流程*/
//             ESP_Init();
//         }
//     }
// }

void ESP_JudgeWIFI(void)
{
    uint8_t *data_rx;
    while (1)
    {
        data_rx = RX_DS.rx_data;
        if (strstr((const char *)data_rx, "OK"))
        {
            gui_dp_info("WIFI CONNECTED");
            break;
        }
        else
        {
            gui_dp_info("WIFI NOT FOUND");
            ESP_Init();
        }
    }
    ESP_RXBUF_Clear();
}

/**
 * @brief 判断MQTT域名是否连接成功，若超时无法连接MQTT域名，则重新尝试连接
 *
 */
// void ESP_JudgeMQTT(void)
// {
//     uint8_t data[25];
//     uint8_t *data_rx;
//     while (1)
//     {
//         data_rx = RX_DS.rx_data;
//         HAL_Delay(500);

//         sscanf((const char *)data_rx, "%*[^\r\n]%s\r\n", data);
//         if (memcmp((const char *)data, "OK", strlen("OK")) == 0)
//         {
//             gui_dp_info("MQTT CONNECTED");
//             break;
//         }
//         else
//         {
//             gui_dp_info("MQTT CONN FAIL");
//             ESP_Init();
//         }
//     }
// }
void ESP_JudgeMQTT(void)
{
    uint8_t *data_rx;
    while (1)
    {
        data_rx = RX_DS.rx_data;
        HAL_Delay(4000);

        if (strstr((const char *)data_rx, "OK"))
        {
            gui_dp_info("MQTT CONNECTED");
            MQTT_CONN_Flag = 0;
            break;
        }
        else
        {
            gui_dp_info("MQTT CONN FAIL");
            MQTT_CONN_Flag = 1;
            HAL_Delay(3000);
            ESP_Init();
        }
    }
    ESP_RXBUF_Clear();
}

/**
 * @brief ESP01S初始化函数，详细内容看函数内部注释
 *
 */
void ESP_Init(void)
{
    if (!MQTT_CONN_Flag)
    {
        RX_DS.rx_dn_flag = 0;
#ifndef MQTT_ONLY
        /**
         * @brief 延时稳定，关闭串口接收，准备
         *
         */
        HAL_Delay(1500);
        ESP_Close_Receive;
        gui_dp_info("READY FOR ESP01");

#ifdef REPLAY_ON
        /**
         * @brief 关闭回显
         *
         */
        ESP_Transmit(cmd_REPLY);
        HAL_Delay(1500);
        gui_dp_info("CLOSE REPLY");
#endif

        /**
         * @brief 断开MQTT连接
         *
         */
        ESP_Transmit(cmd_MQTTCLEAN);
        HAL_Delay(1500);
        gui_dp_info("MQTT DISCONN");

        /**
         * @brief 断开WIFI连接
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

#ifdef REPLAY_ON
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
         * @brief 关闭上电自动连接WIFI
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
         * @brief 设置WIFI名字和密码，并且连接
         *
         */
        ESP_Transmit(cmd_CWJAP);
        ESP_Open_Receive;
        HAL_Delay(1500);
        gui_dp_info("WIFI SET DONE");
        ESP_JudgeWIFI();
#endif

        /**
         * @brief 设定MQTT用户名和密钥
         *
         */
        HAL_Delay(1500);
        ESP_Close_Receive;
        ESP_Transmit(cmd_MQTTUSERCFG);
        HAL_Delay(1500);
        gui_dp_info("SET MQTT USERCFG");

        /**
         * @brief 设定MQTT CLINT ID
         *
         */
        ESP_Transmit(cmd_MQTTCLIENTID);
        HAL_Delay(2000);
        gui_dp_info("SET MQTT CLINTID");
    }

    /**
     * @brief 设定并连接MQTT域名
     *
     */
    ESP_Transmit(cmd_MQTTCONN);
    HAL_Delay(4000);
    ESP_Open_Receive;
    gui_dp_info("SET MQTT URL");
    HAL_Delay(4000);
    ESP_JudgeMQTT();

    ESP_Transmit("AT+MQTTSUB=0,\"" mqtt_sub_TOPIC "\",1\r\n");
    HAL_Delay(3000);
    gui_dp_info("TOPIC SUB");
    RX_DS.rx_dn_flag = 1;
}

void ESP_MQTT_Trans_DHT11(void)
{
    float temp, humi;
    if (DHT_Read(tah_data))
    {
        LCD_ShowIntNum(00, 40, tah_data[2], 2, BLACK, WHITE, 16);
        LCD_ShowIntNum(20, 40, tah_data[3], 2, BLACK, WHITE, 16);

        LCD_ShowIntNum(40, 40, tah_data[0], 2, BLACK, WHITE, 16);
        LCD_ShowIntNum(60, 40, tah_data[1], 2, BLACK, WHITE, 16);

        temp = tah_data[2] + tah_data[3] / 100.0;
        humi = tah_data[0] + tah_data[1] / 100.0;
        printf("AT+MQTTPUB=0,\"" mqtt_pub_TOPIC "\",\"{\\\"params\\\":{\\\"temperature\\\":%f}}\",0,0\r\n", temp);
        HAL_Delay(3000);
        printf("AT+MQTTPUB=0,\"" mqtt_pub_TOPIC "\",\"{\\\"params\\\":{\\\"humidity\\\":%f}}\",0,0\r\n", humi);
        HAL_Delay(3000);
        ESP_RXBUF_Clear();
    }
}

void ESP_MQTT_Recei_LED(void)
{
    if (1)
    {
        uint8_t *data_rx;
        data_rx = RX_DS.rx_data;
        if (strstr((const char *)data_rx, "+MQTTSUBRECV:"))
        {
            // HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        }
        ESP_RXBUF_Clear();
    }
}