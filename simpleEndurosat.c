#include "simpleEndurosat.h"

#define S_X_BAND_TRNSM_TX_RETRY               (10)                /* Number of times to reply any packet */
#define S_X_BAND_TRNSM_READ_FILE_CHUNCK_SIZE  (1472)              /* Size of data read by one packet */
#define S_X_BAND_TRNSM_MAX_READ_BUFFER        (S_X_BAND_TRNSM_READ_FILE_CHUNCK_SIZE + S_X_BAND_TRNSM_TX_RETRY)   /* Maximum size of read data */
#define S_X_BAND_STACK_RETR_SUB(condition,stackEntry)   if (condition) (stackEntry)->retries--
#define S_X_BAND_STACK_RETR_COND(stat)        (S_X_BAND_TRNSM_STAT_BUSY == (stat) || S_X_BAND_TRNSM_STAT_NACK == (stat))

/* Descriptor of all commands: Gives the Value of the command and it's maximum size */
static const S_X_BAND_TRNSM_CMD_INFO_TxInfo_struct TxCMD_Descriptor[S_X_BAND_TRNSM_CMD_NUMBER] =
{
/* X_BAND_TRNSM_CMD_GET,          */ { S_X_BAND_TRNSM_CMD_VAL_GET           , 0                             ,12                             },
/* X_BAND_TRNSM_CMD_SET,          */ { S_X_BAND_TRNSM_CMD_VAL_SET           , 112                           ,1                              },
/* X_BAND_TRNSM_CMD_DIR_F,        */ { S_X_BAND_TRNSM_CMD_VAL_DIR_F         , 0                             ,S_X_BAND_TRNSM_MAX_READ_BUFFER   },
/* X_BAND_TRNSM_CMD_DIR_NEXT_F,   */ { S_X_BAND_TRNSM_CMD_VAL_DIR_NEXT_F    , 0                             ,S_X_BAND_TRNSM_MAX_READ_BUFFER   },
/* X_BAND_TRNSM_CMD_DELL_F,       */ { S_X_BAND_TRNSM_CMD_VAL_DELL_F        , 31                            ,1                              },
/* X_BAND_TRNSM_CMD_DELL_ALL_F,   */ { S_X_BAND_TRNSM_CMD_VAL_DELL_ALL_F    , 0                             ,1                              },
/* X_BAND_TRNSM_CMD_CREATE_F,     */ { S_X_BAND_TRNSM_CMD_VAL_CREATE_F      , 31                            ,1                              },
/* X_BAND_TRNSM_CMD_WRITE_F,      */ { S_X_BAND_TRNSM_CMD_VAL_WRITE_F       , S_X_BAND_TRNSM_MAX_READ_BUFFER  ,1                              },
/* X_BAND_TRNSM_CMD_OPEN_F,       */ { S_X_BAND_TRNSM_CMD_VAL_OPEN_F        , 31                            ,1                              },
/* X_BAND_TRNSM_CMD_READ_F,       */ { S_X_BAND_TRNSM_CMD_VAL_READ_F        , 4                             ,S_X_BAND_TRNSM_MAX_READ_BUFFER   },
/* X_BAND_TRNSM_CMD_SEND_F,       */ { S_X_BAND_TRNSM_CMD_VAL_SEND_F        , 31                            ,1                              },
/* X_BAND_TRNSM_CMD_TX_MODE,      */ { S_X_BAND_TRNSM_CMD_VAL_TX_MODE       , 0                             ,1                              },
/* X_BAND_TRNSM_CMD_LOAD_MODE,    */ { S_X_BAND_TRNSM_CMD_VAL_LOAD_MODE     , 0                             ,1                              },
/* X_BAND_TRNSM_CMD_UPDATE_FW,    */ { S_X_BAND_TRNSM_CMD_VAL_UPDATE_FW     , 31                            ,1                              },
/* X_BAND_TRNSM_CMD_SAFE_SHUTDOWN,*/ { S_X_BAND_TRNSM_CMD_VAL_SAFE_SHUTDOWN , 0                             ,1                              },
/* X_BAND_TRNSM_CMD_GET_RES,      */ { S_X_BAND_TRNSM_CMD_VAL_GET_RESULT    , 2                             ,S_X_BAND_TRNSM_MAX_READ_BUFFER   }
};

static const uint8_t TxCMD_BusyTimePeriod[S_X_BAND_TRNSM_CMD_NUMBER] =
{
/* S_X_BAND_TRNSM_CMD_GET,          */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_SET,          */ 0x0A ,
/* S_X_BAND_TRNSM_CMD_DIR_F,        */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_DIR_NEXT_F,   */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_DELL_F,       */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_DELL_ALL_F,   */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_CREATE_F,     */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_WRITE_F,      */ 0x00 ,
/* S_X_BAND_TRNSM_CMD_OPEN_F,       */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_READ_F,       */ 0x01 ,
/* S_X_BAND_TRNSM_CMD_SEND_F,       */ 0xFF ,
/* S_X_BAND_TRNSM_CMD_TX_MODE,      */ 0xFF ,
/* S_X_BAND_TRNSM_CMD_LOAD_MODE,    */ 0x0A ,
/* S_X_BAND_TRNSM_CMD_UPDATE_FW,    */ 0x0A ,
/* S_X_BAND_TRNSM_CMD_SAFE_SHUTDOWN,*/ 0x01 ,
/* S_X_BAND_TRNSM_CMD_GET_RES,      */ 0x01
};

void S_X_BAND_TRNSM_Send_Ack ( S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type);



uint8_t  S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_RX_BUFF_SIZE];           /* Buffer for row data from the S-band transmitter - includes header, information about the packet and the data itself */
static uint8_t  TxDataBuffer[S_X_BAND_TRNSM_TX_BUFF_SIZE];                /* Buffer for the data that is going to be transmitted */
S_X_BAND_CMD_StackEntry stackEntry;
FT_HANDLE  ftHandle = NULL;
static uint32_t S_X_BAND_LastExecCmd_Tick;    
uint8_t  S_X_BAND_TRNSM_Result_Rx_Buffer[S_X_BAND_TRNSM_RX_BUFF_SIZE];      /* The received data that has been requested */



void readPackStruct(S_X_BAND_TRNSM_Pack_struct * RxPackStruct, uint8_t RxBuffer[])
{
	memcpy(&RxPackStruct->Header, 	RxBuffer																				   , sizeof(uint32_t));
	memcpy(&RxPackStruct->ModuleID, RxBuffer + sizeof(uint32_t)																   , sizeof(uint16_t));
	memcpy(&RxPackStruct->Length, 	RxBuffer + sizeof(uint32_t) + sizeof(uint16_t)											   , sizeof(uint16_t));
	// printf("Valeur Lenght en decimal %d", &RxPackStruct->Length);								                           
	memcpy(&RxPackStruct->Response, RxBuffer + sizeof(uint32_t) + (sizeof(uint16_t)*2)										   , sizeof(uint16_t));
	memcpy(&RxPackStruct->CMD, 		RxBuffer + sizeof(uint32_t) + (sizeof(uint16_t)*3)										   , sizeof(uint16_t));
	memcpy(&RxPackStruct->Type, 	RxBuffer + sizeof(uint32_t) + (sizeof(uint16_t)*4)										   , sizeof(uint16_t));
    /*if(RxPackStruct->Length > 0)								                                                               
	{	
		if (RxPackStruct->Data != NULL) {								                                                       
			free(RxPackStruct->Data);								                                                           
		}								                                                                                       
		RxPackStruct->Data = (uint8_t*) malloc(RxPackStruct->Length * sizeof(uint8_t));								           
		memcpy(RxPackStruct->Data, RxBuffer + sizeof(uint32_t) + (sizeof(uint16_t)*5)										   , RxPackStruct->Length * sizeof(uint8_t));
		printf("READ DATA\n");
		for(int i = 0; i < RxPackStruct->Length ; i++){
			printf("%.2X",RxPackStruct->Data[i]);
		}
		printf("\n");
	}
	memcpy(&RxPackStruct->CRC32, 	RxBuffer + sizeof(uint32_t) + (sizeof(uint16_t)*5) + (RxPackStruct->Length*sizeof(uint8_t)), sizeof(uint32_t));
	printf("RxPackStruct->CRC32 : %.4X\n", RxPackStruct->CRC32);*/
}


void purgeBuffer()
{
	int ftStatus = FT_OK;
	ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX); // Purge both Rx and Tx buffers
	if(ftStatus == FT_OK)
	{
		printf("FT_Purge OK\n");
	}
	else
	{
		printf("FT_Purge failed\n");
	}
}

void lenghtQueue(DWORD* RxBytes)
{
	int count;
	usleep(100);
	printf("\n Queue status 1111111111111111111111: %i\n", *RxBytes);	
	count = 5000;
	*RxBytes=0;	
	while(count > 0)
	{
		usleep(100);	
		FT_GetQueueStatus(ftHandle,RxBytes);
		//printf("\n Queue status 22 : %i\n", *RxBytes);
		if(*RxBytes !=0 && *RxBytes%16 == 0)
		{
			break;
		}
		count--;
	}
	printf("\n Queue status 22 : %i\n", *RxBytes);

	if(!(*RxBytes !=0 && *RxBytes%16 == 0))
	{
		*RxBytes=0;
	}
	printf("\n Queue status111 : %i\n", *RxBytes);

}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Create a file in the SD card of the S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @return            retStat - a value from enumeration X_BAND_TRNSM_CrateStatus_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint8_t S_X_BAND_TRNSM_CreateFile (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;

    // Check for available retries
    if (!stackEntry->retries) {
        if (stackEntry->state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
					printf("\n res541 \n");		
            retStat = S_X_BAND_TRNSM_PARAMS_COMM_ERR;
        }
        return retStat;
    }
    // Check for command state
    switch (stackEntry->state) {
        case S_X_BAND_TRNSM_CMD_STATE_CMD:
        case S_X_BAND_TRNSM_CMD_STATE_CMD_RES:
            // Send command
			printf("\n ici2  \n");
            if (!stackEntry->fileSize) {
                retStat = 0x01;
                break;
            }
            // Should insert FileName + FileSize like that
            memcpy (&(stackEntry->ESTTC_data[++stackEntry->ESTTC_data_size]), &stackEntry->fileSize, sizeof (stackEntry->fileSize));
            stackEntry->ESTTC_data_size += sizeof (stackEntry->fileSize);
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->command, stackEntry->type, stackEntry->ESTTC_data, stackEntry->ESTTC_data_size);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                //stackEntry->timestamp = xTaskGetTickCount();
						printf("\n res1 \n");
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    //if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                    //    // Communication at all baud rates has failed
                    //    stackEntry->retries = 0;
                    //    break;
                    //}
                }
                // Failed to send the command
                // retStat = 0x01;
                // Subtract the retry count
                // stackEntry->retries--;
                S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, stackEntry);
            }
            retStat = 0x00;
            break;
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT:
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES:
            // Check whether the protocol delay has passed
            //if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            //stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
			// retStat = S_X_BAND_TRNSM_GetResult (S_X_BAND_TRNSM_CMD_enum CMD, S_X_BAND_TRNSM_RetRes_enum CMD_Type, uint8_t * RxData, uint16_t * RxDataLenght);
			// decom
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
					printf("\n res3 \n");
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (RxDataLenght == sizeof (retStat) + sizeof (uint32_t)) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_CREATE_OK || retStat == S_X_BAND_TRNSM_CREATE_ERR) {
                        // Return data
                        if (retStat == S_X_BAND_TRNSM_CREATE_OK) {
                            stackEntry->fileHandler = *((uint32_t *)&S_X_BAND_TRNSM_Result_Rx_Buffer[1]);
                            //stackEntry->fileHandler = ((uint32_t)S_X_BAND_TRNSM_Result_Rx_Buffer[4] << 24) + ((uint32_t)S_X_BAND_TRNSM_Result_Rx_Buffer[3] << 16) + ((uint32_t)S_X_BAND_TRNSM_Result_Rx_Buffer[2] << 8) + S_X_BAND_TRNSM_Result_Rx_Buffer[1];
                        }
                        // Update the state
                        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                        break;
                    }
                } else {
                    // Wrong answer
                    // retStat = 0x01;
                }
            } else {
                // Failed to send the command
                // retStat = 0x01;
                if (S_X_BAND_STACK_RETR_COND(retStat)) retStat = 0x00;
            }
            // Subtract the retry count
            stackEntry->retries--;
            break;
        default:
            break;
    }

    return retStat;

}


/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Open a file in the SD card of the S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @return            retStat - a value from enumeration X_BAND_TRNSM_OpenStatus_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint8_t S_X_BAND_TRNSM_OpenFile (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
    uint8_t *FileName = (uint8_t *)stackEntry->ESTTC_data;
    uint8_t FileNameLength = strlen ((char *)FileName) + 1;
    // uint32_t FileSize = *((uint32_t *)&FileName[FileNameLength]);

    // Check for available retries
    if (!stackEntry->retries) {
        if (stackEntry->state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
            retStat = S_X_BAND_TRNSM_PARAMS_COMM_ERR;
        }
        return retStat;
    }
    // Check for command state
    switch (stackEntry->state) {
        case S_X_BAND_TRNSM_CMD_STATE_CMD:
        case S_X_BAND_TRNSM_CMD_STATE_CMD_RES:
            // Send command
            // Should insert FileName + FileSize like that
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->command, stackEntry->type, stackEntry->ESTTC_data, stackEntry->ESTTC_data_size);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                // stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                   // if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    
                }
                // Failed to send the command
                // retStat = 0x01;
                // Subtract the retry count
                // stackEntry->retries--;
                S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, stackEntry);
            }
            retStat = 0x00;
            break;
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT:
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES:
            // Check whether the protocol delay has passed
            // if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            // stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult ( stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght) ;
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (RxDataLenght == sizeof(retStat) + sizeof(uint32_t) + sizeof(uint32_t)) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_OPEN_OK || retStat == S_X_BAND_TRNSM_OPEN_ERR) {
                        // Return data
                        if (retStat == S_X_BAND_TRNSM_OPEN_OK) {
                            stackEntry->fileHandler = *((uint32_t *)&S_X_BAND_TRNSM_Result_Rx_Buffer[1]);
                            stackEntry->fileSize = *((uint32_t *)&S_X_BAND_TRNSM_Result_Rx_Buffer[5]);
                        }
                        // Update the state
                        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                        break;
                    }
                } else {
                    // Wrong answer
                    // retStat = 0x01;
                }
            } else {
                // Failed to send the command
                // retStat = 0x01;
                if (S_X_BAND_STACK_RETR_COND(retStat)) retStat = 0x00;
            }
            // Subtract the retry count
            stackEntry->retries--;
            break;
        default:
            break;
    }

    return retStat;
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Gets the result of a command that have been transmitted recently
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      Identifier - ID of the used S or X band transmitter
* @param[input]      CMD - command number from enumeration X_BAND_TRNSM_CMD_enum, that was already transmitted
* @param[input]      CMD_Type - value from enumeration X_BAND_TRNSM_RetRes_enum
* @param[output]     RxData - Data that is going to be received. If the command does not require RxData can be = NULL
* @param[output]     RxDataLenght - Number of bytes to be received
* @return            retStat - a value from enumeration S_X_BAND_TRNSM_Response_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
// static S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_GetResult (CMD, CMD_Type, * RxData, * RxDataLenght)
S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_GetResult( S_X_BAND_TRNSM_CMD_enum CMD, S_X_BAND_TRNSM_RetRes_enum CMD_Type, uint8_t * RxData, uint16_t * RxDataLenght) {
    uint8_t  retries;
//    uint16_t busy_retries = 0;
    S_X_BAND_TRNSM_Pack_struct * RxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)S_X_BAND_TRNSM_X_Band_Data;
    S_X_BAND_TRNSM_Pack_struct * TxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)TxDataBuffer;
    uint32_t RxCRC;
    uint32_t * pRxCRC;
    uint32_t TxPackCRC = 0;
	FT_STATUS  ftStatus = FT_OK;
	DWORD bytesToWrite = 0;
	DWORD bytesWritten = 0;
	DWORD RxBytes = 0;
	DWORD BytesReceived = 0;
	uint8_t RxBuffer[256] = {0};
	printf("RESULT\n");
    if (!RxDataLenght) {
        /* Invalid data pointer */
        return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
    } else {
        /* default value - 0 byes have been read */
        *RxDataLenght = 0;
    }
    if (!RxData) {
        /* Invalid data pointer */
        return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
    }
    TxPackStruct->Header = S_X_BAND_TRNSM_HEADER;
    TxPackStruct->ModuleID = S_BAND_TRNSM_DEFAULT_DEV_ID;
    TxPackStruct->Response = 0;
    TxPackStruct->CMD = TxCMD_Descriptor[S_X_BAND_TRNSM_CMD_GET_RES].CMD;
    TxPackStruct->Type = TxCMD_Descriptor[CMD].CMD;
    if ((CMD == S_X_BAND_TRNSM_CMD_GET) ||
        (CMD == S_X_BAND_TRNSM_CMD_SET)  ||
        (CMD == S_X_BAND_TRNSM_CMD_SEND_F)) {
        /* copy the type of the command */
        TxPackStruct->Length = 2;
        TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE] = (uint8_t)CMD_Type;
        TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE+1] = (uint8_t)(CMD_Type>>8);
    } else {
		printf("No length\n");
        TxPackStruct->Length = 0;
    }

    /* copy the structure of the packet to the transmit buffer */
    TxPackCRC = crc32 (0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length);
    /* copy the CRC to the transmit buffer after the structure of the packet */
    memcpy (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length], &TxPackCRC, sizeof (TxPackCRC));
    for (retries = 0; retries < S_X_BAND_TRNSM_TX_RETRY ;) {
        if (((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) > sizeof (TxDataBuffer)) {
            /* check if the transmit buffer is insufficient */
            return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
        }
        memset (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)], 0, ((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) - (S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)));
		bytesToWrite = (((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4;
		purgeBuffer();

		printf("Write\n");
		for(int i = 0; i < bytesToWrite; i++){
			printf("%.2X",TxDataBuffer[i]);
		}		
		printf("\n");
		ftStatus = FT_Write(ftHandle, 
							TxDataBuffer, 
							bytesToWrite,
							&bytesWritten);								
		if (ftStatus == FT_OK) {
			lenghtQueue(&RxBytes);
			ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived); 
			printf("READ GET\n");
			for(int i = 0; i < BytesReceived ; i++){
				printf("%.2X",RxBuffer[i]);
			}
			printf("\n");
			if (ftStatus == FT_OK && BytesReceived == RxBytes) {
				//readPackStruct(RxPackStruct, RxBuffer);
				memcpy(S_X_BAND_TRNSM_X_Band_Data, RxBuffer, BytesReceived);
				// check for end of pack and verify the packet
				if (
					(RxPackStruct->Header == TxPackStruct->Header) &&
					(RxPackStruct->ModuleID == TxPackStruct->ModuleID)) 
				{
					printf("fdgfdgmmmm\n");
					RxCRC = crc32 (0, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_STRCT_SIZE + RxPackStruct->Length);
					printf("RxCRC %.8X\n", RxCRC);
					pRxCRC = (uint32_t *)&S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE + RxPackStruct->Length];
					printf("pRxCRC %.8X\n", *pRxCRC);
					if (*pRxCRC == RxCRC) {
						printf("ICICI/n");
						if (RxPackStruct->Response == S_X_BAND_TRNSM_STAT_BUSY) {
							sleep(TxCMD_BusyTimePeriod[CMD]);
							break;
						} else {
							retries = S_X_BAND_TRNSM_TX_RETRY; // exit the loop
							if (RxPackStruct->Response == S_X_BAND_TRNSM_STAT_GET_RESULT) {
								// Copy the length of data
								*RxDataLenght = RxPackStruct->Length;
								//copy the data
								memcpy (RxData, &S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE], RxPackStruct->Length);
								S_X_BAND_TRNSM_Send_Ack (CMD, CMD_Type);
								usleep(1000);//1ms timeout
							}
						}
					}
				} else {
					// retry - send the packet again
				}
			}
			else
			{
				// retry - send the packet again
				printf("read error\n");
			}
		} else {
			printf("write error\n");
			// retry - send the packet again
		}
        if (retries < S_X_BAND_TRNSM_TX_RETRY) {  // If maximum number of retries is not reached
            retries ++;  // Count one more retry
            if (retries == S_X_BAND_TRNSM_TX_RETRY) { // If last retry is done
                RxPackStruct->Response = S_X_BAND_TRNSM_STAT_NACK;
            } else {
                // Retry
            }
        }
	}
    return (S_X_BAND_TRNSM_Response_enum)RxPackStruct->Response;
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Transmit Acknowledge packet after response of a successful command
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      Identifier - ID of the used S or X band transmitter
* @param[input]      CMD - command number from enumeration X_BAND_TRNSM_CMD_enum, that was already transmitted
* @param[input]      CMD_Type - Some commands have different types, the rest don't have. Use values as follow:
*                       + X_BAND_TRNSM_NULL_TYPE - for commands without type
*                       + value form X_BAND_TRNSM_GET_types_enum - for Command GET (X_BAND_TRNSM_CMD_GET)
*                       + value from X_BAND_TRNSM_SET_types_enum - for Command SET (X_BAND_TRNSM_CMD_SET)
*                       + value from X_BAND_TRNSM_SendFile_types_enum - for Command SEND FILE (X_BAND_TRNSM_CMD_SEND_F)
*                       + value from X_BAND_TRNSM_SendFile_types_enum - for Command SEND FILE (X_BAND_TRNSM_CMD_SEND_F)
* @param[output]     none
* @return            none
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void  S_X_BAND_TRNSM_Send_Ack(S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type) {
    uint32_t TxPackCRC = 0;
	uint8_t  retries;
	FT_STATUS  ftStatus = FT_OK;
	DWORD bytesToWrite = 0;
	DWORD bytesWritten = 0;
	DWORD RxBytes = 0;
	DWORD BytesReceived = 0;
	uint8_t RxBuffer[256] = {0};
	uint32_t RxCRC;
    uint32_t * pRxCRC;
    S_X_BAND_TRNSM_Pack_struct * TxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)TxDataBuffer;
	S_X_BAND_TRNSM_Pack_struct * RxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)S_X_BAND_TRNSM_X_Band_Data;

    TxPackStruct->Header = S_X_BAND_TRNSM_HEADER;
    TxPackStruct->ModuleID = S_BAND_TRNSM_DEFAULT_DEV_ID;
    TxPackStruct->Length = 0;
    TxPackStruct->Response = S_X_BAND_TRNSM_STAT_ACK;
    TxPackStruct->CMD = TxCMD_Descriptor[CMD].CMD;
    TxPackStruct->Type = CMD_Type;

    /* copy the structure of the packet to the transmit buffer */
    TxPackCRC = crc32(0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE);

    // Clear the extra bytes that will be set after the packet to complete to divisible to 16
    memset(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC)],0,((((S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC))>>4)+1)<<4) - (S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC)));
	
    /* copy the CRC to the transmit buffer after the structure of the packet */
    memcpy(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length], &TxPackCRC, sizeof(TxPackCRC));
	bytesToWrite=(((S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC))>>4)+1)<<4;
	
}


/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Send a command to S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      Identifier - ID of the used S or X band transmitter
* @param[input]      CMD - command number from enumeration X_BAND_TRNSM_CMD_enum
* @param[input]      CMD_Type - Some commands have different types, the rest don't have. Use values as follow:
*                       + X_BAND_TRNSM_NULL_TYPE - for commands without type
*                       + value form X_BAND_TRNSM_GET_types_enum - for Command GET (X_BAND_TRNSM_CMD_GET)
*                       + value from X_BAND_TRNSM_SET_types_enum - for Command SET (X_BAND_TRNSM_CMD_SET)
*                       + value from X_BAND_TRNSM_SendFile_types_enum - for Command SEND FILE (X_BAND_TRNSM_CMD_SEND_F)
*                       + value from X_BAND_TRNSM_SendFile_types_enum - for Command SEND FILE (X_BAND_TRNSM_CMD_SEND_F)
* @param[input]      TxData - Data that is going to be transmitted. If the command does not require TxData can be = NULL
* @param[input]      TxDataLenght - Number of bytes to be transmitted
* @param[output]     none
* @return            retStat - a value from enumeration S_X_BAND_TRNSM_Response_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_SendCMD (S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type, uint8_t * TxData, uint16_t TxDataLenght) {
    uint8_t retries;
    S_X_BAND_TRNSM_Pack_struct * RxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)S_X_BAND_TRNSM_X_Band_Data;
    S_X_BAND_TRNSM_Pack_struct * TxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)TxDataBuffer;
    uint32_t RxCRC;
    uint32_t * pRxCRC;
    uint32_t TxPackCRC = 0;
	DWORD bytesToWrite = 0;
	DWORD bytesWritten = 0;
	DWORD RxBytes = 0;
	DWORD BytesReceived = 0;
	uint8_t RxBuffer[256] = {0};
	S_X_BAND_LastExecCmd_Tick = 0;
	FT_STATUS  ftStatus = FT_OK;
	
    if (CMD >= S_X_BAND_TRNSM_CMD_GET_RES) {
        /* allow all commands but Get result */
        return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
    }
    if ((TxDataLenght + sizeof (S_X_BAND_TRNSM_Pack_struct) + sizeof(RxCRC) ) > S_X_BAND_TRNSM_TX_BUFF_SIZE) {
        /* if CPU stops here - please increase the size of the buffer -> X_BAND_TRNSM_TX_BUFF_SIZE */
        return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
    }
    // Stack timeout
//  if ((xTaskGetTickCount() - S_X_BAND_LastExecCmd_Tick) < (1 * S_X_BAND_STACK_DELAY)) {
//#if S_X_BAND_STACK_DEBUG
//        // fprintf (COMM, "Timeout: X_BAND_TRNSM_SendCMD %d %d => %d - %d = %d\r\n", (int)CMD, (int)CMD_Type, (int)xTaskGetTickCount(), (int)X_BAND_LastExecCmd_Tick, (int)xTaskGetTickCount() - (int)X_BAND_LastExecCmd_Tick);
//#endif
//        return S_X_BAND_TRNSM_STAT_GET_RESULT;
//    }
    TxPackStruct->Header = S_X_BAND_TRNSM_HEADER;
    TxPackStruct->ModuleID = S_BAND_TRNSM_DEFAULT_DEV_ID;
    if (TxCMD_Descriptor[CMD].tx_data_max_size > 0) {
        if (TxCMD_Descriptor[CMD].tx_data_max_size >= TxDataLenght) {
            TxPackStruct->Length = TxDataLenght;
        } else {
            return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
		}
        /*} else {
        TxPackStruct->Length = 0;
		}*/
	}
    TxPackStruct->Response = 0;
    TxPackStruct->CMD = TxCMD_Descriptor[CMD].CMD;
    TxPackStruct->Type = CMD_Type;
    if (TxPackStruct->Length > 0) {
        if (!TxData) {
            /* Invalid data pointer */
            return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
        }
        /* copy the data of the packet to the transmit buffer, after the structure */
        memcpy(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE], TxData, TxPackStruct->Length);

        /* copy the CRC to the transmit buffer after the data */
        TxPackCRC = crc32(0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length);
        memcpy(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE+TxPackStruct->Length], &TxPackCRC, sizeof(TxPackCRC));
		}
	else
	{
		TxPackCRC = crc32(0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE);
		/* copy the CRC to the transmit buffer after the structure of the packet */
		memcpy(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE], &TxPackCRC, sizeof(TxPackCRC));
	}
		
	// Clear the extra bytes that will be set after the packet to complete to divisible to 16
    memset (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)], 0, ((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) - (S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)));	
	bytesToWrite = (((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4;
	
	for (retries = 0; retries < S_X_BAND_TRNSM_TX_RETRY;) 
	{

		purgeBuffer();
			
		ftStatus = FT_Write(ftHandle, 
							TxDataBuffer, 
							bytesToWrite,
							&bytesWritten);								
		if (ftStatus == FT_OK) {
			lenghtQueue(&RxBytes);
			ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived); 
			printf("RFT_Read\n");
			for(int i = 0; i < BytesReceived ; i++){
				printf("%.2X",RxBuffer[i]);
			}
			printf("\n");
			if (ftStatus == FT_OK && BytesReceived == RxBytes) {
				memcpy(S_X_BAND_TRNSM_X_Band_Data, RxBuffer, BytesReceived);
				// check for end of pack and verify the packet
				if (
					(RxPackStruct->Header == TxPackStruct->Header) &&
					(RxPackStruct->ModuleID == TxPackStruct->ModuleID)){
					printf("jdajdajda \n");
					RxCRC = crc32 (0, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_STRCT_SIZE);
					printf("RxCRC SEND CMD %.8X\n", RxCRC);
					pRxCRC = (uint32_t *)&S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE];
					printf("pRxCRC SEND CMD %.8X\n", *pRxCRC);
					if (*pRxCRC == RxCRC) {
						if (RxPackStruct->Response == S_X_BAND_TRNSM_STAT_ACK) {
							retries = S_X_BAND_TRNSM_TX_RETRY;
							printf("\n LA \n");
							// exit the loop
						} else {
							RxPackStruct->Response = S_X_BAND_TRNSM_STAT_NACK;
							// retry - send the packet again
						}
					} else {
						// wrong CRC
						RxPackStruct->Response = S_X_BAND_TRNSM_STAT_COMM_ERR;
						// retry - send the packet again
					}
				} else {
					// wrong data structure
					RxPackStruct->Response = S_X_BAND_TRNSM_STAT_COMM_ERR;
					// retry - send the packet again
				}
			}
			else
			{
				printf("read error\n");
				RxPackStruct->Response = S_X_BAND_TRNSM_STAT_COMM_ERR;
			}
		} else {
			printf("write error\n");
			// none bytes are received
			RxPackStruct->Response = S_X_BAND_TRNSM_STAT_COMM_ERR;
			// retry - send the packet again - due to timeout with on answer
		}
		if (retries < S_X_BAND_TRNSM_TX_RETRY) {
			retries++;
		}
	}
	
	printf("\n 1 \n");
    return (S_X_BAND_TRNSM_Response_enum)RxPackStruct->Response;

}
