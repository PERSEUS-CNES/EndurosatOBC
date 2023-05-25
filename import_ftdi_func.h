#include "import_ftdi_lib.h"

// fonctions importées
void* libHandle;
FT_OpenFunc ftOpen;
FT_WriteFunc ftWrite;
FT_ReadFunc ftRead;
FT_ResetDeviceFunc ftResetDevice;
FT_SetBaudRateFunc ftSetBaudRate;
FT_SetDataCharacteristicsFunc ftSetDataCharacteristics;
FT_SetDtrFunc ftSetDtr;
FT_SetFlowControlFunc ftSetFlowControl;
FT_SetTimeoutsFunc ftSetTimeouts;
FT_PurgeFunc ftPurge;
FT_GetQueueStatusFunc ftGetQueueStatus;
FT_OpenExFunc ftOpenEx;