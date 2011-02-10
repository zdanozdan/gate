/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/include/eeprom.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: eeprom.h,v $
**  Revision 1.2  2006/12/19 17:28:11  tomasz
**  -fixed crc checking on eeprom
**  -added moving gate procedure
**
**  Revision 1.1  2006/12/14 00:11:03  tomasz
**  *** empty log message ***
**
**
**  $Id: eeprom.h,v 1.2 2006/12/19 17:28:11 tomasz Exp $       
**  
**  COPYRIGHT   :  2006 TZ                                
**************************************************************************/

#ifndef __EEPROM_H__
#define __EEPROM_H__

unsigned char eeprom_busy_check(void);
unsigned char eeprom_read_byte(unsigned char *address) critical;
unsigned char eeprom_write_byte(unsigned char *address, unsigned char value) critical;
signed int eeprom_read_int(int *address) critical;
signed int eeprom_write_int(int *address, signed int value) critical;
void eeprom_write_buffer(xdata char *destination, const xdata char *source, const unsigned char size) critical;
void eeprom_read_buffer(xdata char *destination, const xdata char *source, const unsigned char size) critical;

#endif
