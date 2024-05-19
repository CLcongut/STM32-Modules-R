#ifndef STEPMOTOR_H
#define STEPMOTOR_H

typedef enum _PIN_BIT
{
    PLA = 0x01,
    PLB = 0x02,
    PLC = 0x04,
    PLD = 0x08,
} Pin_Bit;

void Step_Motor_CW(uint32_t nus);
void Step_Motor_CCW(uint32_t nus);
void Step_Motor_Stop(void);

#endif