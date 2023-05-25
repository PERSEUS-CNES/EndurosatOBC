#include "import_ftdi_lib.h"

extern void* libHandle;
extern FT_OpenFunc ftOpen;
extern FT_WriteFunc ftWrite;
extern FT_ReadFunc ftRead;
extern FT_ResetDeviceFunc ftResetDevice;
extern FT_SetBaudRateFunc ftSetBaudRate;
extern FT_SetDataCharacteristicsFunc ftSetDataCharacteristics;
extern FT_SetDtrFunc ftSetDtr;
extern FT_SetFlowControlFunc ftSetFlowControl;
extern FT_SetTimeoutsFunc ftSetTimeouts;
extern FT_PurgeFunc ftPurge;
extern FT_GetQueueStatusFunc ftGetQueueStatus;
extern FT_OpenExFunc ftOpenEx;