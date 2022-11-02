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

#define CRC32_POLY                  ((DWORD)0x04C11DB7)                  /* where 104C11DB7 is a common CRC32 generator polynomial*/
#define CRC32_INIT_REMAINDER        ((DWORD)0x00000000)
#define CRC32_FINAL_XOR_VALUE       ((DWORD)0xFFFFFFFF)

#include "integer.h"
#include "User_types.h"

extern   DWORD crc32_tab[];

DWORD crc32(DWORD crc, BYTE *buf, DWORD size);
         
static __inline DWORD crc32_inline(DWORD crc, BYTE buf)
{
    crc = ~crc;
	return ~( crc32_tab[ (BYTE)crc ^ buf ] ^ ( crc >> 8) );
}

#endif
