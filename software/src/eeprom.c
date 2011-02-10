/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/src/eeprom.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: eeprom.c,v $
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
**  $Id: eeprom.c,v 1.3 2006/12/19 17:28:11 tomasz Exp $       
**  
**  COPYRIGHT   :  2006 TZ                                
**************************************************************************/

#include "89c51ac2.h"
#include "gate.h"

#define MSK_AUXR1_ENBOOT	0x20
#define DPTR_MASK               0x01
#define MAP_BOOT 		AUXR1 |= MSK_AUXR1_ENBOOT;
#define UNMAP_BOOT		AUXR1 &= ~MSK_AUXR1_ENBOOT;
#define SET_DPTR0               AUXR1 &= ~DPTR_MASK;
#define SET_DPTR1               AUXR1 |= DPTR_MASK;

const code void (*write)(void)  = (const void(code*)(void))0xFFF0;
#define __API_JMP_BOOTLOADER (*(const void(code*)(void)) 0xFFF0)

/**
 * Checks if eeprom memory is ready for programming
 *
 * @return 1 if eeprom is busy 0 otherwise
 */
unsigned char eeprom_busy_check(void)
{
   return EECON & MSK_EECON_EEBUSY;
}
/**
 * Reads one byte from program memory. Disables and reenables interrupts
 *
 * @param address pointer to address location in eeprom memory
 * @return read value from address
 */
unsigned char eeprom_read_byte(unsigned char *address) critical
{
   unsigned char retval;
   EECON = MSK_EECON_EEE; //mask eeprom in XRAM space
   retval = *address;
   EECON = 0;

   return retval;
}
/**
 * Write one byte to program memory. Disables and reenables interrupts
 *
 * @param address pointer to address location in eeprom memory
 * @param value to write into address location
 * @return written value
 */
unsigned char eeprom_write_byte(unsigned char *address, unsigned char value) critical
{   
   EECON = MSK_EECON_EEE; //mask eeprom in XRAM space
   *address = value;
   EECON = 0x50;
   EECON = 0xA0;

   return value;
}

/**
 * Reads one word from program memory. Disables and reenables interrupts
 *
 * @param address pointer to address location in eeprom memory
 * @return read value from address
 */
signed int eeprom_read_int(int *address) critical
{
   signed int retval;
   EECON = MSK_EECON_EEE; //mask eeprom in XRAM space
   retval = *address;
   EECON = 0;

   return retval;
}

/**
 * Write one word to program memory. Disables and reenables interrupts
 *
 * @param address pointer to address location in eeprom memory
 * @param value to write into address location
 * @return written value
 */
signed int eeprom_write_int(int *address, signed int value) critical
{   
   EECON = MSK_EECON_EEE; //mask eeprom in XRAM space
   *address = value;
   EECON = 0x50;
   EECON = 0xA0;

   return value;
}

/**
 * Write eeprom data pointed by source into destination. Disables and reenables interrupts
 * Size cannot be more than MAX_EEPROM_PAGE_WRITE and belong to one 128 
 * byte page (not checked in this function !!!)
 *
 * @param destination pointer to destination address location in eeprom memory
 * @param source pointer into fist byte of data buffer
 * @param size of data buffer
 * @return TGateErrorEnum error value
 */

void eeprom_write_buffer(xdata char *destination, const xdata char *source, const unsigned char size) critical
{
   unsigned char i, tmp;
   
   for (i=0; i<size; i++)
   {
      tmp = *source;
      EECON = MSK_EECON_EEE; //mask eeprom in XRAM space
      *destination = tmp;
      EECON = 0x0;           //map XRAM back
      destination++;
      source++;
   }
   //start eeprom write
   EECON = 0x50;
   EECON = 0xA0;
}

/**
 * Read eeprom data pointed by source into destination. Disables and reenables interrupts
 *
 * @param destination pointer to destination address location in eeprom memory
 * @param source pointer into fist byte of data buffer
 * @param size of data buffer
 * @return null
 */
void eeprom_read_buffer(xdata char *destination, const xdata char *source, const unsigned char size) critical
{
   unsigned char i,tmp;

   for (i=0; i<size; i++)
   {
      EECON = MSK_EECON_EEE; //mask eeprom in XRAM space
      tmp = *source;
      EECON = 0x0;           //map XRAM back
      *destination = tmp;
      source++;
      destination++;
   }
   EECON = 0x0;           //map XRAM back
}

