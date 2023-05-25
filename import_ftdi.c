#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dlfcn.h>

#include "import_ftdi_lib.h"
#include "import_ftdi_lib.h"

//charge les fonctions de la librairie en memoire pour pouvoir les utiliser
uint8_t load_func()
{
    char* dlsymError;
    libHandle = dlopen("/home/pi/release/build/libftd2xx.so.1.4.27",RTLD_LAZY);
    if (!libHandle) {
        printf( "Impossible de charger la bibliothèque : %s\n", dlerror());
        return 0;
    }

    ftOpen = (FT_OpenFunc)dlsym(libHandle, "FT_Open");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_Open : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftWrite = (FT_WriteFunc)dlsym(libHandle, "FT_Write");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_Write : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftRead = (FT_ReadFunc)dlsym(libHandle, "FT_Read");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_Read : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);


    ftResetDevice = (FT_ResetDeviceFunc)dlsym(libHandle, "FT_ResetDevice");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_ResetDevice : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftSetBaudRate = (FT_SetBaudRateFunc)dlsym(libHandle, "FT_SetBaudRate");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_SetBaudRate : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftSetDataCharacteristics = (FT_SetDataCharacteristicsFunc)dlsym(libHandle, "FT_SetDataCharacteristics");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_SetDataCharacteristics : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftSetDtr = (FT_SetDtrFunc)dlsym(libHandle, "FT_SetDtr");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_SetDtr : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftSetFlowControl = (FT_SetFlowControlFunc)dlsym(libHandle, "FT_SetFlowControl");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_SetFlowControl : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);


    ftSetTimeouts = (FT_SetTimeoutsFunc)dlsym(libHandle, "FT_SetTimeouts");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_SetTimeouts : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftPurge = (FT_PurgeFunc)dlsym(libHandle, "FT_Purge");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_Purge : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftGetQueueStatus = (FT_GetQueueStatusFunc)dlsym(libHandle, "FT_GetQueueStatus");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_GetQueueStatus : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);

    ftOpenEx = (FT_OpenExFunc)dlsym(libHandle, "FT_OpenEx");
    dlsymError = dlerror();
    if (dlsymError != NULL) {
        printf( "Impossible de trouver la fonction FT_OpenEx : %s\n", dlsymError);
        dlclose(libHandle);
        return 0;
    }
    free(dlsymError);


    return 1;
}