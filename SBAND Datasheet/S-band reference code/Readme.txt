Please see the example of one of the most basic commands Get symoll Rate:

open the file "GetSimbolRate.logicdata"

using the program of saleae that you can download from:
https://www.saleae.com/downloads/

============================================================================================================================

Attached you can find an example code that can be used as a guideline for implementing component in a device that have to transfer data with S or X band transmitter.

It is meant to be used with a operation system (FreeRTOS).

Please, start first with the external routines:

S_X_BAND_TRNSM_Init (void)    -> Initialize the USART and the Task
S_X_BAND_TRNSM_DeInit(void)    -> Deinitialize if needed to be used at all
S_X_BAND_Task (void const * argument) -> Task handler, that manages the communication
S_X_BAND_TRNSM_GetBaudrate (void)    -> return the used baudrate (it is configurable)
S_X_BAND_TRNSM_ESTTC_StartCmd (...) -> request a communication command

Core of the protocol is implemented by the static functions

S_X_BAND_TRNSM_SendCMD( ... )

S_X_BAND_TRNSM_GetResult( ... )


