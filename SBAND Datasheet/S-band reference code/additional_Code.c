//Additional code:

int main(void)
{


...
// Create the test of S or X band driver
  osThreadDef(myS_X_BAND_Task, S_X_BAND_Task, osPriorityLow, 1, 12*128);
  osThreadCreate(osThread(myS_X_BAND_Task), NULL);

..
  S_X_BAND_TRNSM_Init();

...


 osKernelStart();


}


void ATaskXxxxX(...)
{

..


if( .. )// on request of a read command:
S_X_BAND_TRNSM_ESTTC_StartCmd(ComInterface, Identifier, cmd, 'R', 0, NULL);

if( .. )// on request of a write command:
S_X_BAND_TRNSM_ESTTC_StartCmd(ComInterface, Identifier, cmd, 'W', 0, NULL);

}