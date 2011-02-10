/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/include/global.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: global.h,v $
**  Revision 1.6  2007/03/07 22:26:43  tomasz
**  *** empty log message ***
**
**  Revision 1.5  2007/03/06 11:11:33  tomasz
**  - zmiana portu buzzera
**
**  Revision 1.4  2007/03/04 13:18:04  tomasz
**  - dodana obsluga POWER LED
**
**  Revision 1.3  2007/01/05 14:37:49  tomasz
**  *** empty log message ***
**
**  Revision 1.2  2006/12/19 17:28:11  tomasz
**  -fixed crc checking on eeprom
**  -added moving gate procedure
**
**  Revision 1.1  2006/12/14 00:11:03  tomasz
**  *** empty log message ***
**
**
**  COPYRIGHT   :  2006 TZ                                
**************************************************************************/

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "89c51ac2.h"

#ifndef NULL
#define NULL  0
#endif

#define TRUE  1
#define FALSE 0
#define true  1
#define false 0

#define ASCII_OFFSET          0x30

#define ENABLE_XRAM_1024      AUXR |= 0xC0;

//
//Define this if X2 mode bit is set in the core
//
#define X2_MODE

typedef unsigned char BOOL;
typedef unsigned char bool;
typedef unsigned char boolean;

#define ENABLE_INTERRUPTS      EA=1
#define DISABLE_INTERRUPTS     EA=0
#define ENABLE_PCA_INTERRUPT   EC=1
#define DISABLE_PCA_INTERRUPT  EC=0
#define IS_PCA_ENABLED         EC==1
#define IS_PCA_DISABLED        EC==0
#define ENABLE_EXT_INTERRUPT   EX1=1
#define DISABLE_EXT_INTERRUPT  EX1=0
#define ENABLE_T1_INTERRUPT    ET1=1
#define DISABLE_T1_INTERRUPT   ET1=0
#define ENABLE_T0_INTERRUPT    ET0=1
#define DISABLE_T0_INTERRUPT   ET0=0
#define ENABLE_T2_INTERRUPT    ET2=1
#define DISABLE_T2_INTERRUPT   ET2=0
#define ENABLE_PULL_INTERRUPT  EX0=1
#define DISABLE_PULL_INTERRUPT EX0=0
#define ENABLE_HOLO_INTERRUPT  EX1=1
#define DISABLE_HOLO_INTERRUPT EX1=0
#define HOLO_NEGATIVE_TRIGGER  IT1=1
#define HOLO_LOW_LEVEL_TRIGGER IT1=0
#define IS_HOLO_ENABLED        EX1==1
#define IS_HOLO_DISABLED       EX1==0

#define PCA_PRIORITY_BIT       0x40
#define TIMER2_PRIORITY_BIT    0x20
#define SERIAL_PRIRITY_BIT     0x10
#define TIMER1_PRIORITY_BIT    0x8
#define EXTERNAL1_PRIORITY_BIT 0x4
#define TIMER0_PRIORITY_BIT    0x2
#define EXTERNAL0_PRIORITY_BIT 0x1

#define TIMER_0_RUN            TR0=1
#define TIMER_0_STOP           TR0=0
#define TIMER_1_RUN            TR1=1
#define TIMER_1_STOP           TR1=0
#define TIMER_2_RUN            TR2=1
#define TIMER_2_STOP           TR2=0
#define PCA_TIMER_RUN          CR=1;
#define PCA_TIMER_STOP         CR=0;

#define SERVICE_WATCHDOG       WDTRST=0x1e; WDTRST=0xe1;

#define SET_MAX_WDT_DELAY      WDTPRG=0x7

#define MOTOR_RIGHT            P1_4
#define MOTOR_LEFT             P1_5                   
#define NOTIF_LED              P4_1
#define NOTIF_PORT             P4
#define NOTIF_LED_POSITION     0x2

#define P_LED_ON               P1_0
#define P_LED_OFF              P0_7

//#define BUZZER_PORT            P2
#define BUZZER                 P1_1
#define BUZZER_ON              BUZZER=1
#define BUZZER_OFF             BUZZER=0
//#define BUZZER_POSITION        0x80

#define SHAFT_ZERO_INPUT       P1_3
#define SHAFT_A_INPUT          P1_6
#define SHAFT_B_INPUT          P1_7

#define INPUTS_PORT            P0
#define INPUT_0                P0_0
#define INPUT_1                P0_1
#define INPUT_2                P0_2
#define INPUT_3                P0_3
#define INPUT_4                P0_4
#define INPUT_5                P3_4

#define OUTPUT_0               P0_5
#define OUTPUT_1               P0_6
#define OUTPUT_2               P0_7

#define CONFIG_INPUTS_PORT     P2
#define CONFIG_INPUT_0         P2_0
#define CONFIG_INPUT_1         P2_1
#define CONFIG_INPUT_2         P2_2
#define CONFIG_INPUT_3         P2_3
#define CONFIG_INPUT_4         P2_4
#define CONFIG_INPUT_5         P2_5
#define CONFIG_INPUT_6         P2_6

#define INPUT_0_MASK           0x1
#define INPUT_1_MASK           0x2
#define INPUT_2_MASK           0x4
#define INPUT_3_MASK           0x8
#define INPUT_4_MASK           0x10
#endif
