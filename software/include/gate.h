/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/include/gate.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: gate.h,v $
**  Revision 1.13  2007/07/12 20:35:44  tomasz
**  - poprawiona opcja gdy w czasie jazdy ramie jest przepchniete w przeciwym kierunku. (MOTOR_DIR_CHANGED)
**  - poprawione gdy nastepuje wylamanie bramki i na powrocie wchodzi nastepny
**
**  Revision 1.12  2007/03/30 16:07:41  tomasz
**  Zbiorczy update po testach
**
**  Revision 1.11  2007/03/22 23:21:38  tomasz
**  - poprawiony tryb korygowania po³o¿enie
**  - dodana zw³oka w czytaniu wejœæ
**
**  Revision 1.10  2007/03/22 12:33:12  tomasz
**  usuniety blad synchoronizacji podczas zamykania bramki
**  dodana funkcja korygowania polozenia ramienia
**
**  Revision 1.9  2007/03/09 21:52:42  tomasz
**  - poprawki dla trybu synchornicznego
**
**  Revision 1.8  2007/03/09 19:09:00  tomasz
**  - poprawiony alarm, czytanie wejsc
**
**  Revision 1.7  2007/03/07 22:26:43  tomasz
**  *** empty log message ***
**
**  Revision 1.6  2007/03/06 11:10:24  tomasz
**  - dodana pozycja C
**
**  Revision 1.5  2007/03/04 13:18:04  tomasz
**  - dodana obsluga POWER LED
**
**  Revision 1.4  2007/01/10 22:34:03  tomasz
**  lots of changes for presentation
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

#ifndef __GATE_H__
#define __GATE_H__

#include "global.h"

#define SIGNALED 1
#define RELEASED 0

#define COUNT_UP      0
#define COUNT_DOWN    1
#define COUNT_NEUTRAL 2

typedef enum {
   SHAFT_MOVING,
   SHAFT_INITIAL,
   MOTOR_STOPPED,
   MOTOR_DIR_LEFT,
   MOTOR_DIR_RIGHT,
   MOTOR_DIR_CHANGED,
   SHAFT_STOPPED,
   SHAFT_OK,
   SHAFT_ATTEMPTS_ERROR,
   SHAFT_INPUT_MASK,
   SHAFT_WRONG_POSITION,
   SHAFT_ERROR,
   SHAFT_DELAY_EMERGENCY,
   SHAFT_ABORTED,
   SYNCHRO_OK,
   SYNCHRO_ERROR,
   SHAFT_PUSHED,
   TUNE_MODE,
   NEW_POSITION_TUNED,
   TUNE_INITIAL
} TGateErrorEnum;

typedef struct {
      unsigned char programmed;
      signed int position_a;
      signed int position_b;
      signed int position_c;
      signed int position_emergency;
      unsigned int open_time;
      unsigned char power_led;
      unsigned short crc;      
} TEEpromSettings;

#define EEPROM_STRUCT_SIZE 14
#define EEPROM_STRUCT_SIZE_NO_CRC 12

typedef enum _POSTIONS {
   POSTION_A_PROGRAMMED = 0xA0,
   POSTION_B_PROGRAMMED = 0x1A,
   POSTION_C_PROGRAMMED = 0x0A,
   POSTION_EMG_PROGRAMMED = 0xA1,
   POSITIONS_PROGRAMMED = 0xAA,
   POSITIONS_DEFAULT = 0xF0
} TPositions;

typedef enum _SHAFT_POSTION {
   INITIAL_POSITION_NONE = 0,
   FIND_INITIAL_POSITION = 0x1,
   FOUND_INITIAL_POSITION = 0x2,
   ALREADY_INITIAL_POSITION = 0x3
} TShaftPosition;

typedef enum _SHAFT_STATES {
   STATE_00       = 0x0,
   STATE_01       = 0x1,
   STATE_10       = 0x2,
   STATE_11       = 0x3,
   STATE_INITIAL  = 0x4
} TShaftStates;

typedef enum _CHECK_PORTS {
   NO_PORT_CHECKING,
   START_CHECKING
} TCheckPorts;

typedef struct _SHATF_STATE_MACHINE {
      char current;
      char left;
      char right;
} TStateMachine;

typedef struct _MOVE_DATA {
      signed int source;
      signed int destination;
      signed int real_destination;
} TMoveData;

typedef enum _STATES {
   STATE_NONE,
   STATE_ZERO,
   STATE_WAIT_MOVE,
   STATE_STOPPED,
   STATE_ALARM,
   STATE_WAIT_INPUTS,
   STATE_CHANGE_DIR,
   STATE_POS_A,
   STATE_POS_B,
   STATE_POS_C,
   STATE_EMERGENCY,
   STATE_FIND_INITIAL_POSITION,
   STATE_INITIAL_ALARM,
   STATE_WAIT_SLOW_DOWN,
   STATE_ALARM_PUSHED,
   STATE_SYCHRO_WAIT,
   STATE_EMERGENCY_CLEAR
} TStates;

//Gate status word bites
#define GSW_LEFT       0x1
#define DOUBLE_SIDE    0x2

#endif
