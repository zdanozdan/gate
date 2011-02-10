/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/include/flash.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: flash.h,v $
**  Revision 1.1  2006/12/14 00:11:03  tomasz
**  *** empty log message ***
**
**
**  $Id: flash.h,v 1.1 2006/12/14 00:11:03 tomasz Exp $       
**  
**  COPYRIGHT   :  2006 TZ                                
**************************************************************************/

#ifndef __FLASH_H__
#define __FLASH_H__

unsigned char flash_write_byte(unsigned char value,unsigned int address);
unsigned char flash_write_page(unsigned char count, unsigned int xram, unsigned int flash);
unsigned char read_manufacturer(void);
#endif
