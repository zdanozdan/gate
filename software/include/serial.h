/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/include/serial.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: serial.h,v $
**  Revision 1.1  2006/12/14 00:11:03  tomasz
**  *** empty log message ***
**
**  Revision 1.1  2003/11/08 12:00:19  tomasz
**  first release
**
**  $Id: serial.h,v 1.1 2006/12/14 00:11:03 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/
#ifndef __SERIAL_H__
#define __SERIAL_H__

void rs_init ( void );
void put_serial ( const char* message);

#endif
