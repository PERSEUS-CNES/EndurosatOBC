/**
  ******************************************************************************
  * @file    es_crc32.h
  * @brief   This file contains the crc32 function definitions.	
	******************************************************************************
  *
  * COPYRIGHT(c) 2018 Enduro Sat
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ES_CRC32_H
#define __ES_CRC32_H

#define CRC32_POLY                  ((unsigned int)0x04C11DB7)                  /* where 104C11DB7 is a common CRC32 generator polynomial*/
#define CRC32_INIT_REMAINDER        ((unsigned int)0x00000000)
#define CRC32_FINAL_XOR_VALUE       ((unsigned int)0xFFFFFFFF)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>


//#include "integer.h"
//#include "User_types.h"

extern   unsigned int crc32_tab[];

unsigned int crc32(unsigned int crc, unsigned char *buf, unsigned int size);
         
static __inline unsigned int crc32_inline(unsigned int crc, unsigned char buf)
{
    crc = ~crc;
	return ~( crc32_tab[ (unsigned char)crc ^ buf ] ^ ( crc >> 8) );
}

#endif
