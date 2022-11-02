#ifndef FTDI_MANAGER_H
#define FTDI_MANAGER_H

#include "ftd2xx.h"								// USB communication
#include <assert.h>
#include <stdio.h>
#include <unistd.h>								// Defines miscellaneous symbolic constants and types and functions

extern FT_HANDLE  ftHandle;

void purgeBuffer(FT_HANDLE* ftHandle);
void lenghtQueue(FT_HANDLE* ftHandle, DWORD* RxBytes);
void initialize_FTDI();

#endif