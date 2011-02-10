/**************************************************************************
**                                                                        
**  FILE : $Source: /cvs/cvsrepo/projects/gate/software/src/flash.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: flash.c,v $
**  Revision 1.1  2006/12/14 00:11:03  tomasz
**  *** empty log message ***
**
**
**  $Id: flash.c,v 1.1 2006/12/14 00:11:03 tomasz Exp $       
**  
**  COPYRIGHT   :  2006 TZ                                
**************************************************************************/

#include "89c51ac2.h"
#include "global.h"

#define MSK_AUXR1_ENBOOT	0x20
#define DPTR_MASK               0x01
#define MAP_BOOT 		AUXR1 |= MSK_AUXR1_ENBOOT;
#define UNMAP_BOOT		AUXR1 &= ~MSK_AUXR1_ENBOOT;
#define SET_DPTR0               AUXR1 &= ~DPTR_MASK;
#define SET_DPTR1               AUXR1 |= DPTR_MASK;

const code void (*write)(void)  = (const void(code*)(void))0xFFF0;
#define __API_JMP_BOOTLOADER (*(const void(code*)(void)) 0xFFF0)

unsigned int code at 0xFFF0 dummy = 0xFFF0;

// unsigned char read_manufacturer(void)
// {
//    EA=0;
//    MAP_BOOT
//    _asm;
//    mov r1,#0
//    mov dptr,#1
//    mov acc,#02
//    _endasm;
//    write();
//    _asm;
//    _endasm;
//    UNMAP_BOOT
//    EA=1;

//    return ACC;
// }

// unsigned char flash_write_byte(unsigned char value, unsigned int address)
// {
//    DPL = (unsigned char)address&0xFF;
//    DPH = (unsigned char)(address >> 8)&0xFF;
//    EA=0;

//    MAP_BOOT
//    _asm;
//    mov	a,#dummyret$
//    push	acc
//    mov	a,#(dummyret$ >> 8)
//    push	acc
//    push _write
//    push (_write + 1)
//    mov r1,#02

//    _endasm;
//    A=value;
//    _asm;
//    ret

// dummyret$:
//    _endasm;
//    UNMAP_BOOT
//    EA=1;
   
//    return ACC;
// }

unsigned char flash_write_page(unsigned char count, unsigned int xram, unsigned int flash) critical
{
   //   DISABLE_INTERRUPTS;
   DPL = (unsigned char)flash&0xFF;
   DPH = (unsigned char)(flash >> 8)&0xFF;
   SET_DPTR1
   DPL = (unsigned char)xram&0xFF;
   DPH = (unsigned char)(xram >> 8)&0xFF;
   SET_DPTR0

   MAP_BOOT
   _asm;
   mov	a,#dummyret_f$
   push	acc
   mov	a,#(dummyret_f$ >> 8)   
   push	acc
   push _write
   push (_write + 1)
   mov r1,#09
   _endasm;
   A=count;
   _asm;
   
   ret

dummyret_f$:

   _endasm;
   UNMAP_BOOT
   //   ENABLE_INTERRUPTS;

   return ACC;
}


