/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/include/crc.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: crc.h,v $
**  Revision 1.2  2006/12/19 17:28:11  tomasz
**  -fixed crc checking on eeprom
**  -added moving gate procedure
**
**  Revision 1.1  2006/12/15 21:35:38  tomasz
**  first release
**
**  
**  COPYRIGHT   :  2006 TZ                                
**************************************************************************/

#ifndef __CRC_H__
#define __CRC_H__

unsigned short crc_16( const unsigned char *buffer, const unsigned int buff_len );

#endif
