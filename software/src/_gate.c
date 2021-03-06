/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/src/_gate.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: _gate.c,v $
**  Revision 1.30  2010/05/04 21:26:48  tomasz
**  fixed problem with COUNT_NEUTRAL when foto switch is activated during close
**
**  Revision 1.29  2010/03/29 09:56:36  tomasz
**  comment added, play with NO/NC
**
**  Revision 1.28  2008/05/19 18:47:21  tomasz
**  poprawka bledu przy alarmie i synchronizacji
**
**  Revision 1.27  2007/10/28 13:49:15  tomasz
**  zabezpiecznie
**
**  Revision 1.26  2007/10/09 12:06:34  tomasz
**  dodany CR jako #define (CR_08_10_2007). Neguje dzia�anie port�w IN_0, IN_2
**
**  Revision 1.25  2007/08/02 13:27:47  tomasz
**  - poprawiony b��d przy programowaniu pozycji na nowym chipie
**
**  Revision 1.24  2007/07/12 20:35:45  tomasz
**  - poprawiona opcja gdy w czasie jazdy ramie jest przepchniete w przeciwym kierunku. (MOTOR_DIR_CHANGED)
**  - poprawione gdy nastepuje wylamanie bramki i na powrocie wchodzi nastepny
**
**  Revision 1.23  2007/06/28 16:55:43  tomasz
**  - poprawiony blad kasownia pozycji emergency
**
**  Revision 1.22  2007/06/28 16:11:02  tomasz
**  poprawki po wizycie on-site
**
**  Revision 1.21  2007/05/17 21:30:36  tomasz
**  zmieniony czas alarmu
**
**  Revision 1.20  2007/04/15 18:25:31  tomasz
**  *** empty log message ***
**
**  Revision 1.19  2007/04/05 13:29:49  tomasz
**  - zatrzymanie bramki i wylaczenie alarmu w trybie programowania
**
**  Revision 1.18  2007/03/30 16:07:41  tomasz
**  - Zbiorczy update po testach
**
**  Revision 1.17  2007/03/22 23:21:38  tomasz
**  - poprawiony tryb korygowania po�o�enie
**  - dodana zw�oka w czytaniu wej��
**
**  Revision 1.16  2007/03/22 12:33:12  tomasz
**  - usuniety blad synchoronizacji podczas zamykania bramki
**  - dodana funkcja korygowania polozenia ramienia
**
**  Revision 1.15  2007/03/13 21:45:58  tomasz
**  - usuniecie nadmiernego kodu
**
**  Revision 1.14  2007/03/13 21:40:57  tomasz
**  - zmiana pr�dko�ci pocz�tkowej
**
**  Revision 1.13  2007/03/10 21:54:28  tomasz
**  -zmiany algorytmu hamowania
**
**  Revision 1.12  2007/03/09 21:59:38  tomasz
**  - brak czytania wejscia recznego
**
**  Revision 1.11  2007/03/09 21:52:42  tomasz
**  - poprawki dla trybu synchornicznego
**
**  Revision 1.10  2007/03/09 19:09:00  tomasz
**  - poprawiony alarm, czytanie wejsc
**
**  Revision 1.9  2007/03/08 10:24:41  tomasz
**  -poprawiony alarm
**
**  Revision 1.8  2007/03/07 22:26:29  tomasz
**  - poprawione slow_down przy wejsciu bistabilnym
**
**  Revision 1.7  2007/03/06 11:09:00  tomasz
**  - zgaszanie diody po ustapieniu alarmu
**
**  Revision 1.6  2007/03/04 13:16:56  tomasz
**  - dodana obsluga POWER LED
**  - poprawiona obsluga trybu programowania
**
**  Revision 1.5  2007/01/10 22:33:38  tomasz
**  lots of changes - ready made for presentation
**
**  Revision 1.4  2007/01/05 14:37:50  tomasz
**  *** empty log message ***
**
**  Revision 1.3  2006/12/19 17:28:11  tomasz
**  -fixed crc checking on eeprom
**  -added moving gate procedure
**
**  Revision 1.2  2006/12/14 16:40:05  tomasz
**  programming eeprom ready
**
**  Revision 1.1  2006/12/14 00:11:03  tomasz
**  *** empty log message ***
**
**  
**  COPYRIGHT   :  2006 TZ                                
**************************************************************************/


#define OUT
#define TEST_MODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "89c51ac2.h"
#include "serial.h"
#include "gate.h"
#include "global.h"
#include "eeprom.h"
#include "crc.h"

static code char* GATE_SOFTWARE_VERSION = "GATE_VERSION: $Id: _gate.c,v 1.30 2010/05/04 21:26:48 tomasz Exp $";

#define FULL_SPEED        0xFF
#define QUARTER_SPEED     0x3F
#define HALF_SPEED        0x7F
#define MIN_SPEED         0x20
#define MIN_SPEED_SLOW_DOWN   0x90
#define PRC_SPEED_10      0x19

#define FULL_OPEN_RIGHT   -4000
#define FULL_OPEN_LEFT    4000

#define SLOW_DOWN_MARK_500       500
#define SLOW_DOWN_MARK_200       500

#define MOVEMENT_STEPS_TOLERANCE 20
#define TUNE_STEPS_TOLERANCE 150
#define CHECK_MOVEMENT_TOLERANCE 5
#define PUSHED_STEPS_TOLERANCE   150
#define POSITION_PROGRAMMING_TOLERANCE 200

#define FAST_BLINK_LED    4
#define MEDIUM_BLINK_LED  10
#define SLOW_BLINK_LED    17
#define NO_BLINK          0
#define NO_ALARM          0

#define ALARM_PULSE_PERIOD 30
#define BLINK_AND_BEEP_PERIOD 15

#define DEFAULT_OPEN_TIME 3

#define GATE_CLOSED 1
#define GATE_READY_TO_CLOSE   0

volatile static unsigned char ints_counter = 0;
volatile static unsigned char check_movement = 0;
volatile static bool slow_down_stop = false;
volatile static bool slow_down_enabled = false;
volatile static unsigned char interrupt_counter = 0;
volatile static unsigned char seconds_counter = 0;
volatile static unsigned char blink_timer = 0;
volatile static unsigned char power_blink_timer = 0;
volatile static unsigned char alarm_timer = 0;
volatile static unsigned char beep_and_blink_timer = 0;
volatile static signed int check_movement_steps = 0;
volatile static char movement_alarm = SHAFT_INITIAL;
volatile static char motor_direction = MOTOR_STOPPED;
volatile static char find_initial_position = INITIAL_POSITION_NONE;
volatile static char previous_state = STATE_INITIAL;
volatile static signed int step_count = 0;
volatile static signed int set_position_steps = 0;
volatile static bool emergency_pressed = false;
volatile static unsigned char count_direction = COUNT_NEUTRAL;


///////////////////////////////////////////////
// Global data definitions

const TStateMachine code state_machine[] = {
   { STATE_00, STATE_01, STATE_10 },
   { STATE_01, STATE_11, STATE_00 },
   { STATE_10, STATE_00, STATE_11 },
   { STATE_11, STATE_10, STATE_01 }
};

// EEprom setting are hold in xram and eeprom space
// EEprom space is choosen by setting eeprom bit MSK_EECON_EEE in EECON register
#define EEPROM_PAGE_0 0x0
xdata at EEPROM_PAGE_0 TEEpromSettings eeprom_settings;
xdata TEEpromSettings xram_settings;

//
///////////////////////////////////////////////

void init_pwm(void)
{
   //To configure a pin for its alternate function, set the bit in the Px register. When the latch
   //is set, the "alternate output function" signal controls the output level
   MOTOR_LEFT = 1;
   MOTOR_RIGHT = 1;

   //Set to select 12 clock periods per peripheral clock cycle.
   CKCON = X2 | T0X2;

   // CMOD (S:D9h)
   // PCA Counter Mode Register
   // CPS1 CPS0 Clock source
   // 1 0 Timer 0 overflow
   CMOD |= CPS1;

   CCAPM1 |= PWM | ECOM;
   CCAP1H = 0;

   CCAPM2 |= PWM | ECOM;
   CCAP2H = 0;
   
   // We also need to config timer 0 as source for PCA module
   // Timer 0 Mode Select Bit
   // M10 M00 Operating mode
   // 1 0 Mode 2: 8-bit auto-reload Timer/Counter (TL0)
   TMOD |= M1_0;
   motor_direction = MOTOR_STOPPED;
}

void init_position_pin(void)
{
   //configure shaft zero as input for alternate funtion
   SHAFT_ZERO_INPUT = 1;
   //interrupt generated by positive edge on shaft '0' output
   CCAPM0 = ECCF | CAPP;
   //clear interrupt flag if set
   CCF0 = 0;
   ENABLE_PCA_INTERRUPT;
}

void init_incremental_interrupt_pins(void)
{
   //configure encoder A and B as input for alternate funtion
   SHAFT_A_INPUT = 1;
   SHAFT_B_INPUT = 1;
   //interrupt generated by positive or negative edge on shaft 'A' or 'B' output
   CCAPM3 = ECCF | CAPP | CAPN;
   CCAPM4 = ECCF | CAPP | CAPN;
   //clear interrupt flags if set
   CCF3 = 0;
   CCF4 = 0;
   ENABLE_PCA_INTERRUPT;
}

unsigned char get_count_direction()
{
   unsigned char retval;

   DISABLE_PCA_INTERRUPT;
   retval = count_direction;
   ENABLE_PCA_INTERRUPT;

   return retval;
}

TGateErrorEnum check_direction_changed()
{
   TGateErrorEnum retval = SHAFT_INITIAL;
   static unsigned char prev_direction = COUNT_NEUTRAL;
   unsigned char direction;

   direction = get_count_direction();

   if (direction != COUNT_NEUTRAL)
   {
      if (prev_direction == COUNT_NEUTRAL)
      {
         prev_direction = direction;
      }      
      if (direction != prev_direction)
      {
         retval = MOTOR_DIR_CHANGED;
      }
   }

   prev_direction = direction;

   return retval;
}


signed int get_step_count()
{
   signed int retval;

   DISABLE_PCA_INTERRUPT;
   retval = step_count;
   ENABLE_PCA_INTERRUPT;

   return retval;
}

void start_pwm(void) 
{
   PCA_TIMER_RUN;
   TIMER_0_RUN;
}

void stop_pwm(void)
{
   PCA_TIMER_STOP;
   TIMER_0_STOP;
}

/*
 * Timer 1 Mode Select Bits
 * M11 M01 Operating mode
 * 0   1   Mode 1: 16-bit Timer/Counter.
 */
void init_timers(void)
{
   TMOD |= M0_1;
   ENABLE_T1_INTERRUPT;
   TIMER_1_RUN;
}

/**
 * To use a pin for general-purpose input, set the bit in the Px register to
 * turn off the output driver FET.
 *
 * @return null
 */

void init_inputs(void)
{
   INPUT_0 = 1;
   INPUT_1 = 1;
   INPUT_2 = 1;
   INPUT_3 = 1;
   INPUT_4 = 1;
   INPUT_5 = 1;

   CONFIG_INPUT_0 = 1;
   CONFIG_INPUT_1 = 1;
   CONFIG_INPUT_2 = 1;
   CONFIG_INPUT_3 = 1;
   CONFIG_INPUT_4 = 1;
   CONFIG_INPUT_5 = 1;
   CONFIG_INPUT_6 = 1;  
}

void init_seconds_counter()
{
   DISABLE_T1_INTERRUPT;
   ints_counter = 0;
   seconds_counter = 0;
   ENABLE_T1_INTERRUPT;
}

// 12 = 200 ms which should be just enough 
#define INPUT_VALIDATION_COUNT 255

//
//Change inputs IN_0 and IN_2 from NO to NC. 
//Enable/disable define to have that
//

//#define CR_08_10_2007
#ifdef CR_08_10_2007
unsigned char read_inputs()
{
   unsigned char retval = 0xFF;
   unsigned char port = 0;
   unsigned char validation_count = 0;
   bool exit = false;

   if (INPUT_0 == 1 || INPUT_2 == 1)
   {
      if (INPUT_0 == 1)
      {
         retval = 0xFE;
         port = 0;
      }
      if (INPUT_2 == 1)
      {
         retval = 0xFB;
         port = 2;
      }

      validation_count = 0;
      while ( ++validation_count < INPUT_VALIDATION_COUNT &&
              exit == false )
      {
         SERVICE_WATCHDOG;

         switch (port)
         {
            case 0:
               if (INPUT_0 == 0)
               {
                  exit = true;
                  retval = 0xFF;
               }
               break;

            case 2:
               if (INPUT_2 == 0)
               {
                  exit = true;
                  retval = 0xFF;
               }
               break;             
         }
      }
   }
   
   return retval;
}
#else
unsigned char read_inputs()
{
   unsigned char retval = 0xFF;
   unsigned char inputs = INPUTS_PORT;
   unsigned char validation_count = 0;
   bool exit = false;
   
   //we only use inputs IN0 - IN5 (P0_0 - P0_4) so mask is 0xE0;
   inputs |= 0xE0;

   if (inputs != 0xFF)
   {
      validation_count = 0;
      while ( ++validation_count < INPUT_VALIDATION_COUNT && exit == false )
      {
         inputs = INPUTS_PORT | 0xE0;
         SERVICE_WATCHDOG;
         // as long as we read "0" do nothing and wait for loop to finish
         // otherwise quit the loop and mark input stimulus as invlid 
         if (inputs != 0xFF)
         {
            retval = inputs;
         }
         else
         {
            exit = true;
            retval = 0xFF;
         }
      }
   }
   
   return retval;
}
#endif

/**
 * Check if emergency input is active
 *
 * @return true if emegrency pin = 0 false otherwise
 */
bool check_emergency_input(void)
{
   unsigned char tmp = read_inputs();
   bool retval = false;

   if (!(tmp & INPUT_4_MASK))
   {
      emergency_pressed = true;
      retval = true;
   }
 
   return retval;
}

/**
 * Check if emergency input is active
 *
 * @return SHAFT_PUSHED if gate pushed out from 
 * it's current position in either direction, SHAFT_OK otherwise
 */
TGateErrorEnum check_gate_pushed(signed int position)
{
   TGateErrorEnum retval = SHAFT_OK;
   signed int sc = get_step_count();

   int move = abs(position - sc);

   if (move > PUSHED_STEPS_TOLERANCE)
      retval = SHAFT_PUSHED;

   return retval;
}

#define DELAY_125_MS 7
#define DELAY_250_MS 14

void delay_ints(unsigned char interrupts)
{
   DISABLE_T1_INTERRUPT;
   interrupt_counter = 0;
   ENABLE_T1_INTERRUPT;
   while (interrupt_counter < interrupts)
   {      
      SERVICE_WATCHDOG;
   }
}

/**
 * Generic delay function
 *
 * @param seconds - number of seconds to wait
 *
 * @return null
 */
void delay(unsigned char seconds)
{
   init_seconds_counter();
   while (seconds_counter < seconds)
   {      
      SERVICE_WATCHDOG;
   }
}

void blink_led(unsigned char value)
{
   DISABLE_T1_INTERRUPT;
   blink_timer = value;
   if (value == NO_BLINK)
   {
      NOTIF_LED = 0;
   }
   ENABLE_T1_INTERRUPT;
}

void power_blink_led(unsigned char value)
{
   DISABLE_T1_INTERRUPT;
   power_blink_timer = value;
   if (value == NO_BLINK)
   {
      P_LED_ON = xram_settings.power_led;
      P_LED_OFF = 1;
   }
   ENABLE_T1_INTERRUPT;
}

void alarm(unsigned char value)
{
   DISABLE_T1_INTERRUPT;
   alarm_timer = value;
   if (value == NO_ALARM)
   {
      P_LED_ON = xram_settings.power_led;
      P_LED_OFF = 1;
   }
   ENABLE_T1_INTERRUPT;
}


//UWAGA ta funkcja nie dziala jesli mask jest OR (np INPUT_0_MASK | INPUT_1_MASK)
TGateErrorEnum delay_mask(unsigned char seconds, unsigned char mask)
{
   TGateErrorEnum retval = SHAFT_OK;
   unsigned char exit = false;
   unsigned char tmp;

   init_seconds_counter();

   while (exit == false)
   {
      if (seconds_counter >= seconds)
      {
         exit = true;
         retval = SHAFT_OK;
      }

      if (check_emergency_input() == true)
      {
         retval = SHAFT_DELAY_EMERGENCY;
         exit = true;
      }
      
      tmp = read_inputs();
      if (!(tmp & mask))
      {
         init_seconds_counter();
         exit = false;
      }

      SERVICE_WATCHDOG;
   }

   return retval;
}

void go_right(signed int steps, char speed)
{
   set_position_steps = steps;
   motor_direction = MOTOR_DIR_RIGHT;
   CCAP1H = 0;
   CCAP2H = speed;
}

void go_left(signed int steps, char speed)
{
   set_position_steps = steps;
   motor_direction = MOTOR_DIR_LEFT;
   CCAP1H = speed;
   CCAP2H = 0;
}

void start_movement_check(void)
{
   DISABLE_T1_INTERRUPT;
   check_movement = 0;
   check_movement_steps = get_step_count();
   movement_alarm = SHAFT_MOVING;
   ENABLE_T1_INTERRUPT;
}

void stop_movement_check(void)
{
   movement_alarm = SHAFT_INITIAL;
}

void fill_move_data(TMoveData *md, signed int *source, signed int *destination)
{
   md->source = *source;
   md->destination = *destination;
   md->real_destination = *destination;
}

void swap_move_data(TMoveData *md)
{
   signed int tmp = md->source;
   md->source = md->destination;
   md->destination = tmp;
   md->real_destination = tmp;
}

/**
 * Move gate to desired absolute position
 *
 * @param position - step position
 * @param speed - moving speed
 * @return null
 */
void gate_go(signed int position, unsigned char speed)
{
   signed int sc = get_step_count();

   int move = abs(position - sc);

   // To do - check it again !!
   if (move > MOVEMENT_STEPS_TOLERANCE)
   {
      count_direction = COUNT_NEUTRAL;
      start_movement_check();
      slow_down_enabled = true;
      move = position - sc;
      if (move < 0)
         go_right(position,speed);
      else
         go_left(position,speed);

      delay_ints(DELAY_250_MS);
   }
}

void gate_go_position(TMoveData *md, signed int *a, signed int *b, char speed)
{
   fill_move_data(md, a, b);
   gate_go(md->destination, speed);
}

void gate_swap_position(TMoveData *md, char speed)
{
   swap_move_data(md);
   gate_go(md->destination, speed);
}

/**
 * Depends on configuration DP-SW we go left or right
 *
 * @return      null
 */
void go_initial()
{
   static unsigned char oposite = 0;

   if (oposite & 0x1)
   {
      if (CONFIG_INPUT_0 == 1)
         go_left(FULL_OPEN_LEFT, HALF_SPEED);
      else
         go_right(FULL_OPEN_RIGHT, HALF_SPEED);
   }
   else
   {
      if (CONFIG_INPUT_0 == 1)
         go_right(FULL_OPEN_RIGHT, HALF_SPEED);   
      else
         go_left(FULL_OPEN_LEFT, HALF_SPEED);
   }
   oposite = ~oposite;
}

void stop_motor(void)
{
   stop_movement_check();
   motor_direction = MOTOR_STOPPED;
   count_direction = COUNT_NEUTRAL;
   slow_down_enabled = false;
   DISABLE_T0_INTERRUPT;
   CCAP1H = 0;
   CCAP2H = 0;
}

void slow_down(void)
{
   ENABLE_T0_INTERRUPT;
}

void slow_down_and_stop(void)
{
   slow_down_stop = true;
   ENABLE_T0_INTERRUPT;
}

/**
 * This function is responsible for finding zero position. 
 *
 * @return      TGateErrorEnum 
 */

TGateErrorEnum find_zero_position(void)
{
   TGateErrorEnum retval = SHAFT_OK;
   char exit = false;

   find_initial_position = FIND_INITIAL_POSITION;
   start_movement_check();

   while (exit == false)
   {
      if (movement_alarm == SHAFT_STOPPED)
      {
         retval = SHAFT_STOPPED;
         exit = true;
      }
      if (find_initial_position == FOUND_INITIAL_POSITION)
      {
         retval = SHAFT_OK;
         exit = true;
      }

      SERVICE_WATCHDOG;
   }

   stop_movement_check();

   return retval;
}


void wait_eeprom(void)
{
   while (eeprom_busy_check() == 1)
   {
      SERVICE_WATCHDOG;
   }
}

void tune_left_right(void)
{
   if (P2_4 == 0)
   {
      go_left(FULL_OPEN_LEFT, QUARTER_SPEED);
      while (P2_4 == 0)
      {
         SERVICE_WATCHDOG;
      }
   }
   if (P2_5 == 0)
   {
      go_right(FULL_OPEN_LEFT, QUARTER_SPEED);
      while (P2_5 == 0)
      {
         SERVICE_WATCHDOG;
      }
   }
   stop_motor();
}

#define OPEN_TIME_1SEC_DELAY 0x40
unsigned char open_time_programming_mode(void)
{
   unsigned char retval = 0;
   char exit = false;
   ///////////////////////////////////////////////
   //turn blinking on
   blink_led(MEDIUM_BLINK_LED);
   
   while (exit == false)
   {
      // pushing those result in open time = 0
      // and exiting programming loop
      if (P2_4 == 0 || P2_5 == 0)
      {
         retval = 0;
         exit = true;
      }
      if (P2_6 == 0)
      {
         interrupt_counter = 0;
         blink_led(NO_BLINK);
         while (P2_6 == 0)
         {
            if (interrupt_counter >= OPEN_TIME_1SEC_DELAY)
            {
               retval++;
               interrupt_counter = 0;
               NOTIF_LED = 1;
               BUZZER_ON;
               beep_and_blink_timer = BLINK_AND_BEEP_PERIOD;
            }
            SERVICE_WATCHDOG;
         }
         exit = true;
      }

      // let's check if we are still in time programming mode
      // if not abort operation
      if (CONFIG_INPUT_3 == 1)
      {
         retval = xram_settings.open_time;
         exit = true;
      }
      
      // max open time is fixed to 30 secs
      if (retval > 30)
         retval = 30;
      
      SERVICE_WATCHDOG;
   }
   
   return retval;
}

#define PROGRAMMING_BUTTON_TIMEOUT 0x7F
TGateErrorEnum manual_zero_find_mode(void)
{
   char exit = false;
   TGateErrorEnum retval = SHAFT_OK;

   blink_led(SLOW_BLINK_LED);

   while (exit == false)
   {  
      tune_left_right();

      if (CONFIG_INPUT_2 == 1)
      {
         exit = true;
      }
      
      SERVICE_WATCHDOG;
   }

   blink_led(NO_BLINK);
   
   return retval;
}

TGateErrorEnum position_programming_mode(bool override)
{
   char exit = false;
   TGateErrorEnum retval = SHAFT_OK;

   blink_led(FAST_BLINK_LED);
   // in case motor is running stop it before 
   // entering programming loop
   stop_motor();

   while (exit == false)
   {  
      tune_left_right();

      //programming mode - 5 sec push programs value into eeprom register
      if (P2_6 == 0)
      {
         interrupt_counter = 0;
         blink_led(MEDIUM_BLINK_LED);
         while (P2_6 == 0)
         {
            if (interrupt_counter >= PROGRAMMING_BUTTON_TIMEOUT)
            {
               blink_led(NO_BLINK);
               exit = true;
            }
            SERVICE_WATCHDOG;
         }
         blink_led(FAST_BLINK_LED);
      }

      // let's check if we are still in position programming mode
      // if no, abort programming operation
      // aborting is only possible if it's not error/empty eeprom situation
      // In that case always force full programming

      if (override == true)
      {
         if (CONFIG_INPUT_2 == 1)
         {
            blink_led(NO_BLINK);
            retval = SHAFT_ABORTED;
            exit = true;
         }
      }
   
      SERVICE_WATCHDOG;
   }

   return retval;
}

/**
 * set up default setting for non - programmed gate
 */

#define DEFAULT_POSITION_B   -50;
#define DEFAULT_POSITION_A   1000;
#define DEFAULT_POSITION_C   -1000;
#define DEFAULT_POSITION_EMG 1000;

void set_default_settings(void)
{
   xram_settings.position_b = DEFAULT_POSITION_B;
   xram_settings.position_a = DEFAULT_POSITION_A;
   xram_settings.position_c = DEFAULT_POSITION_C;
   xram_settings.position_emergency = DEFAULT_POSITION_EMG;
   xram_settings.open_time = DEFAULT_OPEN_TIME;
   xram_settings.power_led = 0;
   xram_settings.programmed = POSITIONS_PROGRAMMED;
}

/**
 * check if we have position already programmed into memory
 * enter programming mode if not set already
 *
 * @param override if set to true reprograms eeprom settings
 * @return null
 */
void prepare_settings(bool override)
{
   unsigned short crc;
   TGateErrorEnum retval;

   stop_motor();
   alarm(NO_ALARM);

   //clear XRAM in case of soft reset
   memset(&xram_settings,0,EEPROM_STRUCT_SIZE);

   eeprom_read_buffer((xdata char*)&xram_settings, (xdata char*)&eeprom_settings, EEPROM_STRUCT_SIZE);

   //calclate current CRC value
   crc = crc_16(&xram_settings, EEPROM_STRUCT_SIZE_NO_CRC);

   if (xram_settings.programmed != POSITIONS_PROGRAMMED || 
       xram_settings.crc != crc ||
       override == true)
   {
      //////////////////////////
      //first program position_a
      retval = position_programming_mode(override);
      if (retval == SHAFT_OK)
      {
         xram_settings.position_a = step_count;
         xram_settings.programmed = POSTION_A_PROGRAMMED;

         //////////////////////////
         //program position_b
         retval = position_programming_mode(override);
         if (retval == SHAFT_OK)
         {
            xram_settings.position_b = step_count;
            xram_settings.programmed = POSTION_B_PROGRAMMED;

            ///////////////////////////////
            //program position_emergency
            retval = position_programming_mode(override);
            if (retval == SHAFT_OK)
            {
               xram_settings.position_emergency = step_count;
               xram_settings.programmed = POSTION_EMG_PROGRAMMED;

               /////////////////////////////////
               //program position_c
               retval = position_programming_mode(override);
               if (retval == SHAFT_OK)
               {
                  xram_settings.position_c = step_count;
                  xram_settings.programmed = POSTION_C_PROGRAMMED;
               }

               // if no errors continue and store the values
               if (retval == SHAFT_OK)
               {
                  ////////////////////////////////////////////////
                  // defalult goes only if positions not
                  // programmed or CRC error
                  if (override == false)
                  {
                     xram_settings.open_time = DEFAULT_OPEN_TIME;
                     xram_settings.power_led = 0;
                     //
                     //zabezpieczenie przeciw kopiowaniu. Jesli P1_2 !=0 to robimy kiche z ustawien
                     if (P1_2 != 0)
                     {
                        xram_settings.position_b %= 2;
                     }                     
                  }
                  
                  /////////////////////////
                  // program status byte
                  xram_settings.programmed = POSITIONS_PROGRAMMED;
                  xram_settings.crc = crc_16(&xram_settings, EEPROM_STRUCT_SIZE_NO_CRC);
                  
                  ////////////////////////////////////
                  //program setting into eeprom memory
                  wait_eeprom();
                  eeprom_write_buffer((xdata char*)&eeprom_settings, (xdata char*)&xram_settings, EEPROM_STRUCT_SIZE);                     
               }               
            }            
         }
      }      
   }
}

/**
 * collect time data and program it into eeprom memory
 *
 * @return null
 */

void prepare_time(void)
{
   //////////////////////////////////////////////
   // program open time and calculate new CRC
   xram_settings.open_time = open_time_programming_mode();
   xram_settings.crc = crc_16(&xram_settings, EEPROM_STRUCT_SIZE_NO_CRC);

   ////////////////////////////////////
   //program setting into eeprom memory
   wait_eeprom();
   eeprom_write_buffer((xdata char*)&eeprom_settings, (xdata char*)&xram_settings, EEPROM_STRUCT_SIZE);
}

void prepare_led(unsigned char led_value)
{
   //////////////////////////////////////////////
   // program led configuration and calculate new CRC
   xram_settings.power_led = led_value;
   xram_settings.crc = crc_16(&xram_settings, EEPROM_STRUCT_SIZE_NO_CRC);

   ////////////////////////////////////
   //program setting into eeprom memory
   wait_eeprom();
   eeprom_write_buffer((xdata char*)&eeprom_settings, (xdata char*)&xram_settings, EEPROM_STRUCT_SIZE);
}

TGateErrorEnum check_position(signed int position, unsigned char tolerance)
{
   TGateErrorEnum retval = SHAFT_OK;
   signed int sc = get_step_count();

   if (abs(position - sc) > tolerance)
      retval = SHAFT_WRONG_POSITION;

   return retval;
}



TGateErrorEnum wait_slow_down()
{
   TGateErrorEnum retval = SHAFT_INITIAL;

   if(motor_direction == MOTOR_STOPPED) 
      retval = SHAFT_OK;
   
   return retval;
}


TGateErrorEnum wait_stopped()
{
   TGateErrorEnum retval = SHAFT_INITIAL;

   if(motor_direction == MOTOR_STOPPED) 
   {
      retval = SHAFT_OK;
   }
   else
   {
      if (movement_alarm == SHAFT_STOPPED)
      {
         retval = SHAFT_STOPPED;
         stop_movement_check();
      }
      else
      {
         retval = check_direction_changed();
      }
   }

   return retval;
}

// co z zakloceniami tutaj ? 
TGateErrorEnum check_synchro(unsigned char current)
{
   TGateErrorEnum retval = SYNCHRO_OK;

   if (CONFIG_INPUT_1 == 0)
   {
      if (INPUT_5 != current)
         retval = SYNCHRO_ERROR;
   }

   return retval;
}

TGateErrorEnum position_tune(unsigned char *inputs)
{
   TGateErrorEnum retval = SHAFT_OK;
   unsigned char exit = false;
   unsigned char current_pos = 0;
   unsigned char tmp = 0;
   signed int range_error = 0;

   if ( P2_4 == 0 || P2_5 == 0 )
   {
      blink_led(FAST_BLINK_LED);
      // in case motor is running stop it before 
      // entering programming loop
      stop_motor();

      while (exit == false)
      {
         SERVICE_WATCHDOG;
         tune_left_right();

         //abandon programming mode when foto switch is enabled
         *inputs = read_inputs();
         if ( !(*inputs & INPUT_0_MASK) || 
              !(*inputs & INPUT_1_MASK) ||
              !(*inputs & INPUT_2_MASK) ||
              !(*inputs & INPUT_3_MASK) )
            exit = true;

         retval = check_position(xram_settings.position_b, TUNE_STEPS_TOLERANCE);
         if (retval == SHAFT_WRONG_POSITION )
         {
            range_error = get_step_count();

            retval = check_position(xram_settings.position_a, TUNE_STEPS_TOLERANCE);
            if (retval == SHAFT_WRONG_POSITION)
            {
               range_error = get_step_count();

               retval = check_position(xram_settings.position_c, TUNE_STEPS_TOLERANCE);
               if (retval == SHAFT_OK)
               {
                  current_pos = STATE_POS_C;
               }
               else
               {
                  range_error = get_step_count();
               }
            }
            else
            {
               current_pos = STATE_POS_A;
            }
         }
         else
         {
            current_pos = STATE_POS_B;
         }

         if(retval == SHAFT_OK)
         {
            blink_led(FAST_BLINK_LED);

            //programming mode - 5 sec push programs value into eeprom register
            if (P2_6 == 0)
            {
               interrupt_counter = 0;
               blink_led(MEDIUM_BLINK_LED);

               while (P2_6 == 0)
               {
                  SERVICE_WATCHDOG;

                  if (interrupt_counter >= PROGRAMMING_BUTTON_TIMEOUT)
                  {
                     switch (current_pos)
                     {
                        case STATE_POS_A:
                           xram_settings.position_a = step_count;
                        break;

                        case STATE_POS_B:
                           xram_settings.position_b = step_count;
                        break;

                        case STATE_POS_C:
                           xram_settings.position_c = step_count;
                        break;
                     }

                     /////////////////////////
                     // calculate CRC code
                     xram_settings.crc = crc_16(&xram_settings, EEPROM_STRUCT_SIZE_NO_CRC);
                  
                     ////////////////////////////////////
                     //program setting into eeprom memory
                     wait_eeprom();
                     eeprom_write_buffer((xdata char*)&eeprom_settings, (xdata char*)&xram_settings, EEPROM_STRUCT_SIZE);                     

                     //retval = NEW_POSITION_TUNED;
                     exit = true;
                     blink_led(NO_BLINK);
                  }
               }
            }
         }
         else 
         {
            // LED stops blinking as we go out of range
            blink_led(NO_BLINK);
            NOTIF_LED = 1;
         }
      }
      retval = TUNE_INITIAL;
   }

   blink_led(NO_BLINK);
   return retval;
}

TGateErrorEnum check_inputs_and_go(unsigned char inputs, TMoveData *md)
{
   TGateErrorEnum retval = SHAFT_OK;

   if (!(inputs & INPUT_0_MASK) || !(inputs & INPUT_2_MASK))
   {
      gate_go_position(md, &xram_settings.position_b, &xram_settings.position_a, FULL_SPEED);
      retval = SHAFT_MOVING;
   }
#ifdef OUT
   if (!(inputs & INPUT_1_MASK) || !(inputs & INPUT_3_MASK))
   {
      gate_go_position(md, &xram_settings.position_b, &xram_settings.position_c, FULL_SPEED);
      retval = SHAFT_MOVING;
   }
#endif

   return retval;
}

#define DEFAULT_ALARM_MOVES 3
#define DEFAULT_INITIAL_ATTEMPTS 3
#define OPEN_TIME_5SEC_DELAY 0x7F

void main (void)  
{
   unsigned char state = STATE_NONE;
   unsigned char moves = 0;
   unsigned char inputs = 0;
   unsigned char inputs_spare = 0;
   unsigned char tmp = 0;
   unsigned char stopped = false;
   bool check_pushed = true;
   bool pushed_and_cleared = false;

   TMoveData md;
   TGateErrorEnum retval;

   SET_MAX_WDT_DELAY;   // we would have 2s at 12MHz, so using 40MHz will give 0,5s delay which should be enough

   ENABLE_INTERRUPTS;
   
   init_inputs();
   init_pwm();
   init_timers();
   init_position_pin();
   init_incremental_interrupt_pins();
   start_pwm();

   previous_state = SHAFT_A_INPUT * 2 + SHAFT_B_INPUT;

   blink_led(NO_BLINK);

   NOTIF_LED = 0;
   P_LED_ON = 0;
   BUZZER_OFF;

   if (CONFIG_INPUT_2 == 0)
   {
      manual_zero_find_mode();
   }

   //based on LEFT/RIGTH type configuration start gate move
   go_initial();
   state = STATE_FIND_INITIAL_POSITION;

   while (1)
   {
      switch (state)
      {
         case STATE_FIND_INITIAL_POSITION:
            retval = find_zero_position();
            if (retval == SHAFT_OK)
            {
               stop_motor();
               prepare_settings(false);
               // turn power LED according to settings
               P_LED_ON = xram_settings.power_led;
               delay(1);
               state = STATE_ZERO;
               blink_led(NO_BLINK);
            }
            if (retval == (TGateErrorEnum)SHAFT_STOPPED)
            {
               if (++moves == DEFAULT_INITIAL_ATTEMPTS)
               {
                  stop_motor();
                  alarm(ALARM_PULSE_PERIOD);
                  state = STATE_INITIAL_ALARM;                 
               }
               else
               {
                  stop_motor();
                  delay(1);
                  go_initial();
               }
            }
            break;

         case STATE_INITIAL_ALARM:
            OUTPUT_1 = GATE_READY_TO_CLOSE;
            tmp = read_inputs();
            if ( !(tmp & INPUT_0_MASK)  || 
                 !(tmp & INPUT_1_MASK) ||
                 !(tmp & INPUT_2_MASK) ||
                 !(tmp & INPUT_3_MASK) )
            {
               moves = 0;
               alarm(NO_ALARM);
               BUZZER_OFF;
               go_initial();
               state = STATE_FIND_INITIAL_POSITION;
            }
            break;

         case STATE_ZERO:
            moves = 0;
            gate_go_position(&md, &xram_settings.position_a, &xram_settings.position_b, FULL_SPEED);
            state = STATE_WAIT_MOVE;
            break;

         case STATE_EMERGENCY_CLEAR:
            moves = 0;
            gate_go_position(&md, &xram_settings.position_emergency, &xram_settings.position_b, FULL_SPEED);
            state = STATE_WAIT_MOVE;
            break;

         case STATE_WAIT_MOVE:               
            retval = wait_stopped();
            // first let's check it it's emergency move. If so go straight
            // to emergency position ignoring any obstacles
            if (md.destination == xram_settings.position_emergency && emergency_pressed == true)
            {
               // just blink POWER LED, no alarm is triggered
               blink_led(ALARM_PULSE_PERIOD);
               power_blink_led(ALARM_PULSE_PERIOD);

               // if somebody program POS_B = POS_EMG or POS_A = POS_EMG etc
               // there is problem of false emergency mode

               if (retval == SHAFT_OK)
               {
                  if (check_position(md.destination, MOVEMENT_STEPS_TOLERANCE) == SHAFT_OK)
                  {
                     //change current state
                     state = STATE_EMERGENCY;
                     emergency_pressed = false;
                  }
                  else
                  {
                     gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_emergency, FULL_SPEED);
                  }
               }
               if (retval == (TGateErrorEnum)SHAFT_STOPPED)
               {
                  stop_motor();
                  delay(1);
                  gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_emergency, FULL_SPEED);
               }
               if (retval == (TGateErrorEnum)MOTOR_DIR_CHANGED)
               {
                  //blink_led(FAST_BLINK_LED);               
                  stop_motor();
                  delay(1);
                  gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_emergency, FULL_SPEED);
               }
            }
            else
            {
               if (retval == SHAFT_OK)
               {
                  if (stopped == true)
                  {
                     delay(1);
                     stopped = false;
                     gate_swap_position(&md, FULL_SPEED);
                  }
                  else
                  {
                     if (md.destination == xram_settings.position_b)
                        state = STATE_POS_B;
                     
                     if (md.destination == xram_settings.position_a)
                        state = STATE_POS_A;
                     
                     if (abs(xram_settings.position_c - xram_settings.position_b) > POSITION_PROGRAMMING_TOLERANCE)
                     {
                        if (md.destination == xram_settings.position_c)
                           state = STATE_POS_C;
                     }
                  }
               }
               if (retval == (TGateErrorEnum)SHAFT_STOPPED)
               {
                  //blink_led(MEDIUM_BLINK_LED);
                  state = STATE_STOPPED;                
               }
               if (retval == (TGateErrorEnum)MOTOR_DIR_CHANGED)
               {
                  blink_led(FAST_BLINK_LED);
                  state = STATE_STOPPED;
               }
            }

            // we always check emergency input 
            // if pressed go to emegrency mode straight away                 
               if (check_emergency_input() == true && emergency_pressed == false)
               {
                  stop_motor();
                  delay(1);
                  gate_go_position(&md, &xram_settings.position_a, &xram_settings.position_emergency, FULL_SPEED);
                  state = STATE_WAIT_MOVE;
               }

            // we need to check if INPUT_0 or INPUT_1 has been activated
            // during move from A->B or C->B
            if ( md.destination == xram_settings.position_b )
            {
               inputs_spare = read_inputs();
               if ( !(inputs_spare & INPUT_0_MASK )  ||
                    !(inputs_spare & INPUT_1_MASK ) )
               {
                  if (pushed_and_cleared == true)
                  {
                     inputs = inputs_spare;
                     pushed_and_cleared = false;
                  }
                  slow_down_and_stop(); 
                  start_movement_check();
                  state = STATE_WAIT_SLOW_DOWN;
               } 
            }
            break;

         case STATE_WAIT_SLOW_DOWN:
            //wait for gate to open again in either direction
            retval = wait_stopped();
            if (retval == SHAFT_OK)
            {
               if ( !(inputs_spare & INPUT_0_MASK ) )
               {
                  gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_a, FULL_SPEED);
                  //delay(1);
                  state = STATE_WAIT_MOVE;
               }
#ifdef OUT
               if ( !(inputs_spare & INPUT_1_MASK ) )
               {
                  gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_c, FULL_SPEED);
                  //delay(1);
                  state = STATE_WAIT_MOVE;
               }
#endif
               OUTPUT_1 = GATE_CLOSED;
            }
            // check if stopped during slow down
            if (retval == (TGateErrorEnum)SHAFT_STOPPED)
               state = STATE_STOPPED;

            if (retval == (TGateErrorEnum)MOTOR_DIR_CHANGED)
            {
               //blink_led(FAST_BLINK_LED);
               state = STATE_STOPPED;
            }
            break;

         case STATE_CHANGE_DIR:
            gate_swap_position(&md, FULL_SPEED);
            state = STATE_WAIT_MOVE;
            break;

         case STATE_STOPPED:
            stopped = true;
            stop_motor();
            delay(1);
            if (++moves >= DEFAULT_ALARM_MOVES)
            {
               alarm(ALARM_PULSE_PERIOD);
               state = STATE_ALARM;
               moves = 0;
            }
            else
            {
               gate_swap_position(&md, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            break;

         case STATE_SYCHRO_WAIT:
            OUTPUT_1 = GATE_READY_TO_CLOSE;
            retval = check_synchro(GATE_READY_TO_CLOSE);            
            if (retval == SYNCHRO_OK)
            {
               gate_go(md.destination, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            //we need to check emergency input as well
            if (check_emergency_input() == true)
            {
               gate_go_position(&md, &xram_settings.position_a, &xram_settings.position_emergency, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            break;

         case STATE_POS_A:
            //clear moves counter if we able to get to destination
            //which means getting from a->b b->c etc.      
            // blink_led(NO_BLINK);
            moves = 0;
            if (!(inputs & INPUT_0_MASK) || 
                !(inputs & INPUT_1_MASK) )
            {
               if (delay_mask(xram_settings.open_time, INPUT_0_MASK) == SHAFT_DELAY_EMERGENCY)
               {
                  gate_go_position(&md, &xram_settings.position_a, &xram_settings.position_emergency, FULL_SPEED);
                  state = STATE_WAIT_MOVE;  
               }
               else
               {
                  //prepare data, if synchro mode is on state synchro just starts the gate
                  fill_move_data(&md, &xram_settings.position_a, &xram_settings.position_b);
                  state = STATE_SYCHRO_WAIT;
               }
            }
#ifdef OUT
            // INPUT_2 and INPUT_3 are bistable
            if (!(inputs & INPUT_2_MASK))
            {
               // go back when:
               // 1. monobutton is pressed or
               // 2. monobutton at oposite direction is pressed or
               // 3. emergency button is pressed
               tmp = read_inputs();
               if (!(tmp & INPUT_2_MASK))
               {
                  fill_move_data(&md, &xram_settings.position_a, &xram_settings.position_b);
                  state = STATE_SYCHRO_WAIT;
               }
               if (!(tmp & INPUT_3_MASK))
               {
                  inputs &= ~INPUT_3_MASK;
                  fill_move_data(&md, &xram_settings.position_a, &xram_settings.position_c);
                  state = STATE_SYCHRO_WAIT;
               }
            }
#endif
            //we need to check emergency input as well
            if (check_emergency_input() == true)
            {
               gate_go_position(&md, &xram_settings.position_a, &xram_settings.position_emergency, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            break;

         case STATE_POS_C:
            //blink_led(NO_BLINK);
            moves = 0;
            if ( !(inputs & INPUT_1_MASK) ||
                 !(inputs & INPUT_0_MASK) )
            {
               if (delay_mask(xram_settings.open_time, INPUT_1_MASK) == SHAFT_DELAY_EMERGENCY)
               {
                  gate_go_position(&md, &xram_settings.position_c, &xram_settings.position_emergency, FULL_SPEED);
                  state = STATE_WAIT_MOVE;  
               }
               else
               {
                  fill_move_data(&md, &xram_settings.position_c, &xram_settings.position_b);
                  state = STATE_SYCHRO_WAIT;
               }
            }
#ifdef OUT
            //INPUT_2 and INPUT_3 are bistable
            if (!(inputs & INPUT_3_MASK))
            {
               // go back when:
               // 1. monobutton is pressed or
               // 2. monobutton at oposite direction is pressed or
               // 3. emergency button is pressed
               tmp = read_inputs();
               if (!(tmp & INPUT_3_MASK))
               {
                  fill_move_data(&md, &xram_settings.position_c, &xram_settings.position_b);
                  state = STATE_SYCHRO_WAIT;
               }
               if (!(tmp & INPUT_2_MASK))
               {
                  inputs &= ~INPUT_2_MASK;
                  fill_move_data(&md, &xram_settings.position_c, &xram_settings.position_a);
                  state = STATE_SYCHRO_WAIT;
               }
            }
#endif
            //we need to check emergency input as well
            if (check_emergency_input() == true)
            {
               gate_go_position(&md, &xram_settings.position_a, &xram_settings.position_emergency, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            break;
               
         case STATE_POS_B:
            OUTPUT_1 = GATE_CLOSED;
            moves = 0;
            inputs = read_inputs();
            //check if somebody wants to tune positions
            retval = position_tune(&inputs);
            // it's ok if gate is moved manually, don't check if pushed then
            if (retval == TUNE_INITIAL)
            {
               check_pushed = false;
            }

            if (check_pushed == true)
            {
               ///////////////////////////////////////////////////////////////////
               // check if gate pushed out from B position in either direction
               retval = check_gate_pushed(xram_settings.position_b);
               if (retval == SHAFT_PUSHED)
               {
                  alarm(ALARM_PULSE_PERIOD);
                  state = STATE_ALARM_PUSHED;
               }
            }

            retval = check_inputs_and_go(inputs, &md);
            if (retval == SHAFT_MOVING)
            {
               check_pushed = true;
               state = STATE_WAIT_MOVE;
            }

            if (check_emergency_input() == true)
            {
               check_pushed = true;
               gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_emergency, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            break;

         case STATE_EMERGENCY:
            OUTPUT_1 = GATE_CLOSED;
            moves = 0;
            inputs = read_inputs();
            retval = check_inputs_and_go(inputs, &md);
            if (retval == SHAFT_MOVING)
            {
               blink_led(NO_BLINK);
               power_blink_led(NO_BLINK);
               state = STATE_WAIT_MOVE;
            }
            break;

         case STATE_ALARM:
            OUTPUT_1 = GATE_CLOSED;
            moves = 0;
            inputs = read_inputs();
            retval = check_inputs_and_go(inputs, &md);
            if (retval == SHAFT_MOVING)
            {
               alarm(NO_ALARM);
               BUZZER_OFF;
               // This is fix for the synchronization bug after alarm is cleared
               // Gate 1 (where alarm went off) goes into B position while gate 2 goes into A/C position waiting for another signal
               stopped = false;
               state = STATE_WAIT_MOVE;
            }

            if (check_emergency_input() == true)
            {
               gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_emergency, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            break;

         case STATE_ALARM_PUSHED:
            OUTPUT_1 = GATE_CLOSED;
            inputs = read_inputs();
            ////////////////////////////////////////////////////////
            // go back to B position if alarm counter expires
            if (alarm_timer == NO_ALARM)
            {
               alarm(NO_ALARM);
               BUZZER_OFF;
               pushed_and_cleared = true;
               state = STATE_EMERGENCY_CLEAR;
            }
            else
            {
               retval = check_inputs_and_go(inputs, &md);
               if (retval == SHAFT_MOVING)
               {
                  alarm(NO_ALARM);
                  BUZZER_OFF;
                  state = STATE_WAIT_MOVE;
               }
            }
            //finally emergency input need to be checked
            if (check_emergency_input() == true)
            {
               gate_go_position(&md, &xram_settings.position_b, &xram_settings.position_emergency, FULL_SPEED);
               state = STATE_WAIT_MOVE;
            }
            break;

            // in case of default just reboot
         default:
            while(1) {}
            break;

      }
                   
      SERVICE_WATCHDOG;
        
      ////////////////////////////////////////////////////
      // position programming mode
      // to enter mode 'CONFIG_INPUT_2' must be forced 0
      if (CONFIG_INPUT_2 == 0)
      {  
         blink_led(NO_BLINK);
         NOTIF_LED = 1;
         //
         //Need to stop motor in case it tries to go somewhere
         stop_motor();
         delay(1);
         prepare_settings(true);
         blink_led(NO_BLINK);
         NOTIF_LED = 1;
         while (CONFIG_INPUT_2 == 0)
         {
            SERVICE_WATCHDOG;
         }
         //now wait to reduce false pulses
         delay(1);
         NOTIF_LED = 0;
      }

      ////////////////////////////////////////////////////
      // open time programming mode
      // to enter mode 'CONFIG_INPUT_3' must be forced 0
      if (CONFIG_INPUT_3 == 0)
      {      
         stop_motor();
         blink_led(NO_BLINK);
         NOTIF_LED = 1;
         delay(1);
         prepare_time();
         blink_led(NO_BLINK);
         NOTIF_LED = 1;
         while (CONFIG_INPUT_3 == 0)
         {
            SERVICE_WATCHDOG;
         }
         //now wait to reduce false pulses
         delay(1);
         NOTIF_LED = 0;
      }

      //P2_6 power led management enabling/disabling power LED
      if (P2_6 == 0)
      {
         stop_motor();
         interrupt_counter = 0;
         blink_led(NO_BLINK);
         NOTIF_LED = 1;
         delay(1);
         while (P2_6 == 0)
         {
            if (interrupt_counter >= OPEN_TIME_5SEC_DELAY)
            {
               interrupt_counter = 0;
               P_LED_ON = !P_LED_ON;
               prepare_led(P_LED_ON);
            }
            SERVICE_WATCHDOG;
         }
         NOTIF_LED = 0;
      }
   }
}

#define SLOW_DOWN_RATIO        2
#define SLOW_DOWN_RATIO_LINEAR 75

// For Timer operation (C/Tx# = 0), the Timer register counts the divided-down peripheral
// clock. The Timer register is incremented once every peripheral cycle (6 peripheral clock
// periods). The Timer clock rate is FPER/6, i.e. FOSC/12 in standard mode or FOSC/6 in X2
// mode.
void timer0_interrupt(void) __interrupt TF0_VECTOR using 0
{
   static signed int sc = 0;
   static signed int slow_count = 0;

   if ( abs(sc - step_count) > SLOW_DOWN_RATIO )
   {
      sc = step_count;
      slow_count = 0;

      if (motor_direction == MOTOR_DIR_LEFT)
      {
         if (slow_down_stop == true)
         {
            if (CCAP1H <= MIN_SPEED_SLOW_DOWN)
            {
               DISABLE_T0_INTERRUPT;
               stop_motor();
               slow_down_stop = false;
            }
            else
            {
               CCAP1H--;
            }
         }
         else
         {
            if (CCAP1H <= MIN_SPEED)
            {
               DISABLE_T0_INTERRUPT;
            }
            else
            {
               CCAP1H--;
            }
         }
      }
      if (motor_direction == MOTOR_DIR_RIGHT)
      {
         if (slow_down_stop == true)
         {
            if (CCAP2H <= MIN_SPEED_SLOW_DOWN)
            {
               DISABLE_T0_INTERRUPT;
               stop_motor();
               slow_down_stop = false;
            }
            else
            {
               CCAP2H--;
            }
         }
         else
         {
            if (CCAP2H <= MIN_SPEED)
            {
               DISABLE_T0_INTERRUPT;
            }
            else
            {
               CCAP2H--;
            }
         }
      }
   }
}



// 24Mhz clock = 0,25 uS resolution
// 0,25 * 65535 = 16,3 per interrupt
// 1 sec delay = 61 ints
#define DELAY_1_SEC              61
#define CHECK_MOVEMENT_DELAY     45

// 61*30sec = 1830 interrupts
#define ALARM_DELAY_30_SECS      1830 
#define ALARM_DELAY_15_SECS      915 
#define ALARM_DELAY_5_SECS       305
#define ALARM_DELAY_6_SECS       365

#define ALARM_DELAY_10_SECS      310
#define ALARM_DELAY_11_SECS      340

#define ALARM_DELAY_START      310
#define ALARM_DELAY_STOP       340

void timer1_interrupt(void) __interrupt TF1_VECTOR using 0
{
   static unsigned char timer = 0;
   static unsigned char p_timer = 0;
   static unsigned char timer_1 = 0;
   static unsigned char timer_2 = 0;
   static unsigned int alarm_delay = 0;
   static unsigned int test_mode = 0;
   static signed int move;
   
   interrupt_counter++;

   if (movement_alarm != SHAFT_INITIAL)
   {
      if (++check_movement >= CHECK_MOVEMENT_DELAY)
      {
         move = abs(check_movement_steps - step_count);
         if (move < CHECK_MOVEMENT_TOLERANCE)
         {
            movement_alarm = SHAFT_STOPPED;
         }
         check_movement_steps = step_count;
         check_movement = 0;
      }
   }
   //////////////////////////////////
   // timer for blinking LED
   if (blink_timer > 0)
   {
      if (++timer >= blink_timer)
      {
         NOTIF_LED = !NOTIF_LED;
         timer = 0;
      }
   }

   //////////////////////////////////
   // timer for blinking LED
   if (power_blink_timer > 0)
   {
      P_LED_ON = 0;
      if (++p_timer >= power_blink_timer)
      {
         P_LED_OFF = !P_LED_OFF;
         p_timer = 0;
      }
   }

   ///////////////////////////////
   // timer for buzzer alarm
   if (alarm_timer > 0)
   {
      if (++timer_1 >= alarm_timer)
      {
         P_LED_ON = 0;
         BUZZER = !BUZZER;
         NOTIF_LED = !NOTIF_LED;
         P_LED_OFF = !P_LED_OFF;
         timer_1 = 0;
      }
      if (++alarm_delay >= ALARM_DELAY_10_SECS)
      {
         BUZZER_OFF;
         NOTIF_LED = 0;
         alarm_delay = 0;
         timer_1 = 0;
         alarm_timer = 0;
         P_LED_ON = xram_settings.power_led;
         P_LED_OFF = 1;
      }
   }


   //////////////////////////////////////
   // timer for blink and beep once
   if (beep_and_blink_timer > 0)
   {
      if (++timer_2 >= beep_and_blink_timer)
      {
         BUZZER_OFF;
         NOTIF_LED = 0;
         beep_and_blink_timer = 0;
         timer_2 = 0;
      }
   }

   ///////////////////////////////////
   // timer for 1 sec delay
   if (++ints_counter >= DELAY_1_SEC)
   {
      seconds_counter++;
      ints_counter = 0;
   }
#ifdef TEST_MODE
   /////////////////////////////////////////////
   //test mode, output 2 goes low periodically
   if(++test_mode > ALARM_DELAY_START)
   {
      P2_7 = 0;
      if (test_mode > ALARM_DELAY_STOP)
      {
         P2_7 = 1;
         test_mode = 0;
      }
   }
#endif
}

#define CHANGE_DIR_STEPS_SENSING 0xFF

// Incremental optical encoders generate two data signals that are electrically 90 out of
// phase with each other. The term quadrature refers to this 90 phase relationship. Since
// each full cycle contains four transitions, or edges, an encoder that generates 2500
// cycles/rev, for example, provides 10,000 edges per revolution.

void pca_interrupt(void) __interrupt PCA_VECTOR using 0
{
   static char state = STATE_00;
   static char read_zero = 1;
   static unsigned char count_up = 0;
   static unsigned char count_down = 0;

   //Test if CCF3 or CCF4 is set
   if (CCON & 0x18 > 0)
   {
      state = SHAFT_A_INPUT * 2 + SHAFT_B_INPUT;

      if (previous_state == state_machine[state].left)
      {
         step_count--;

         count_up = 0;
         count_down++;
         if (count_down == CHANGE_DIR_STEPS_SENSING)
         {
            count_direction = COUNT_DOWN;
            count_down = 0;
         }

         previous_state = state;
      }

      if (previous_state == state_machine[state].right) 
      {
         step_count++;

         count_down = 0;
         count_up++;
         if (count_up == CHANGE_DIR_STEPS_SENSING)
         {
            count_direction = COUNT_UP;
            count_up = 0;
         }

         previous_state = state;
      }

      if (slow_down_enabled == true)
      {
         if (abs(step_count - set_position_steps) <= SLOW_DOWN_MARK_500)
         {
            //FIXED problem when gate goes back coz of count direction error
            count_direction = COUNT_NEUTRAL;
            count_up = 0;
            count_down = 0;

            slow_down();
         }
      }
      if (step_count == set_position_steps)
      {
         stop_motor();
      }

      CCF3 = 0;
      CCF4 = 0;
      
      if (CCF0 == 1)
      {
         if (read_zero == 1)
         {
            step_count = 0;
            read_zero = 0;
         }

         if (find_initial_position == FIND_INITIAL_POSITION)
            find_initial_position = FOUND_INITIAL_POSITION;
         //must clear interrupt flag in software
         CCF0 = 0;
         NOTIF_LED = 1;
         beep_and_blink_timer = BLINK_AND_BEEP_PERIOD;
      }
   }
}
