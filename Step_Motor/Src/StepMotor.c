#include "main.h"
#include "StepMotor.h"

#define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr) *((volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))
// IO口地址映射
#define GPIOA_ODR_Addr (GPIOA_BASE + 12) // 0x4001080C
#define GPIOB_ODR_Addr (GPIOB_BASE + 12) // 0x40010C0C
#define GPIOC_ODR_Addr (GPIOC_BASE + 12) // 0x4001100C
#define GPIOA_IDR_Addr (GPIOA_BASE + 8)  // 0x40010808
#define GPIOB_IDR_Addr (GPIOB_BASE + 8)  // 0x40010C08
#define GPIOC_IDR_Addr (GPIOC_BASE + 8)  // 0x40011008

// 单独操作GPIO的某一个IO口，n(0～16),n表示具体是哪一个IO口
#define PAout(n) BIT_ADDR(GPIOA_ODR_Addr, n) // 输出
#define PAin(n) BIT_ADDR(GPIOA_IDR_Addr, n)  // 输入
#define PBout(n) BIT_ADDR(GPIOB_ODR_Addr, n) // 输出
#define PBin(n) BIT_ADDR(GPIOB_IDR_Addr, n)  // 输入
#define PCout(n) BIT_ADDR(GPIOC_ODR_Addr, n) // 输出
#define PCin(n) BIT_ADDR(GPIOC_IDR_Addr, n)  // 输入

#define LA PBout(3) /* A相 */
#define LB PBout(4) /* B相 */
#define LC PBout(5) /* C相 */
#define LD PBout(6) /* D相 */

#define CPU_FREQUENCY_MHZ 72 // STM32时钟主频
void delay_us(__IO uint32_t delay)
{
    int last, curr, val;
    int temp;

    while (delay != 0)
    {
        temp = delay > 900 ? 900 : delay;
        last = SysTick->VAL;
        curr = last - CPU_FREQUENCY_MHZ * temp;
        if (curr >= 0)
        {
            do
            {
                val = SysTick->VAL;
            } while ((val < last) && (val >= curr));
        }
        else
        {
            curr += CPU_FREQUENCY_MHZ * 1000;
            do
            {
                val = SysTick->VAL;
            } while ((val <= last) || (val > curr));
        }
        delay -= temp;
    }
}

static uint8_t steps[8] = {0x01, 0x03, 0x02, 0x6, 0x04, 0x0c, 0x08, 0x09};

/**
 * @name: Step_Motor_CW
 * @description: 电机正转函数
 * @param {uint32_t} nms
 * @return {*}
 */
void Step_Motor_CW(uint32_t nus)
{
    volatile uint8_t i;
    uint8_t temp = 0;
    for (i = 0; i < 8; i++)
    {
        temp = steps[i];
        LA = (uint8_t)((temp & PLA) >> 0);
        LB = (uint8_t)((temp & PLB) >> 1);
        LC = (uint8_t)((temp & PLC) >> 2);
        LD = (uint8_t)((temp & PLD) >> 3);

        delay_us(nus);
    }
    Step_Motor_Stop();
}

/**
 * @name: Step_Motor_CCW
 * @description: 电机反转函数
 * @param {uint32_t} nms
 * @return {*}
 */
void Step_Motor_CCW(uint32_t nus)
{
    int i;
    for (i = 8; i > 0; i--)
    {
        LA = (uint8_t)((steps[i - 1] & PLA) >> 0);
        LB = (uint8_t)((steps[i - 1] & PLB) >> 1);
        LC = (uint8_t)((steps[i - 1] & PLC) >> 2);
        LD = (uint8_t)((steps[i - 1] & PLD) >> 3);

        delay_us(nus);
    }
    Step_Motor_Stop();
}

/**
 * @name: Step_Motor_Stop
 * @description: 电机停止
 * @param {*}
 * @return {*}
 */
void Step_Motor_Stop(void)
{
    LA = 0;
    LB = 0;
    LC = 0;
    LD = 0;
}
