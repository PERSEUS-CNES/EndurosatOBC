#include "endurosat_manager.h"

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
//S_X_BAND_CMD_StackEntry stackEntry;
static uint32_t S_X_BAND_LastExecCmd_Tick;    
uint8_t  S_X_BAND_TRNSM_Result_Rx_Buffer[S_X_BAND_TRNSM_RX_BUFF_SIZE];      /* The received data that has been requested */
S_X_BAND_TRNSM_WriteFile_struct S_X_BAND_TRNSM_FileDataBuffer;
uint32_t S_X_BAND_TRNSM_UploadToXSBand_FPos;

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
	printf("Create FILE !!!!!!!!!!!\n");
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
            if (!stackEntry->fileSize) {
                retStat = S_X_BAND_TRNSM_STAT_COMM_ERR;
                break;
            }
            // Should insert FileName + FileSize like that
            memcpy (&(stackEntry->ESTTC_data[stackEntry->ESTTC_data_size]), &stackEntry->fileSize, sizeof (stackEntry->fileSize));
            stackEntry->ESTTC_data_size += sizeof (stackEntry->fileSize);
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->command, stackEntry->type, stackEntry->ESTTC_data, stackEntry->ESTTC_data_size);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                //stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    //if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                    //    // Communication at all baud rates has failed
                    stackEntry->retries = 0;
                    break;
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
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
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
							// Update the state
							stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
						}
						else
						{
							stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD;
						}
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
* @brief Delete a file from the SD card of the S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_DellStatus_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint8_t S_X_BAND_TRNSM_DelFile (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
	printf("DEL FILE !!!!!!!!!!!\n");
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
            if (stackEntry->command == S_X_BAND_TRNSM_CMD_DELL_F) {
                uint8_t *FileName = (uint8_t *)stackEntry->ESTTC_data;
                uint8_t FileNameLength = strlen ((char *)FileName) + 1;
                retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->command, stackEntry->type, FileName, FileNameLength);
            } else {
				stackEntry->command == S_X_BAND_TRNSM_CMD_DELL_ALL_F;
                retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->command, stackEntry->type, 0, 0);
            }
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                //stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
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
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (RxDataLenght == sizeof (retStat)) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_DELL_OK || retStat == S_X_BAND_TRNSM_DELL_NOT_FOUND) {
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
* @brief Lists all available files in the SD card
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      Identifier - ID of the used S or X band transmitter
* @param[input]      FileListBuffer - FileList from functions X_BAND_TRNSM_CMD_Dir() and X_BAND_TRNSM_CMD_DirNext()
* @param[output]     File - a buffer where will be written the found filename, the length of the name, and the size of the file
* @return            pdTRUE - the name of the file has been found, pdFALSE - Error
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint8_t S_X_BAND_TRNSM_FileNameParser (uint8_t * FileListBuffer, S_X_BAND_TRNSM_FileInfo_struct * File) {
    uint32_t * Size;

    File->NameLength = strlen((char *)FileListBuffer)+1;  //Get the length of the name of the file + the Null character after it

    if( (File->NameLength > 0 ) && (File->NameLength <= 31 ) )  // Verify the length
    {
        /* Get the size of the file */
        Size = (uint32_t *)&FileListBuffer[File->NameLength];
        File->Size = *Size;

        // Copy the name of the file
        memcpy( File->FileName, FileListBuffer, File->NameLength);

        return 0x01; //OK
    }

    return 0x00;   //File name is NOT found
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Lists all available files in the SD card
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_Dir_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint8_t S_X_BAND_TRNSM_CMD_Dir (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
    S_X_BAND_TRNSM_FileList_struct *DirData = (S_X_BAND_TRNSM_FileList_struct *)stackEntry->ESTTC_data;

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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->command, stackEntry->type, 0, 0);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
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
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (RxDataLenght >= 4) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_DIR_OK || retStat == S_X_BAND_TRNSM_DIR_NO_FILES) {
                        // Update the state
                        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                        // Copy the data
                        if (retStat == S_X_BAND_TRNSM_DIR_OK) {
                            DirData->DirNextAvailable = S_X_BAND_TRNSM_Result_Rx_Buffer[1];
                            DirData->NumberFiles = (S_X_BAND_TRNSM_Result_Rx_Buffer[3] << 8) + S_X_BAND_TRNSM_Result_Rx_Buffer[2];
                            //DirData->FileList = &stackEntry->ESTTC_data[3];
                            //if (DirData->FileList != NULL && RxDataLenght > 4) {
                                // stackEntry->ESTTC_data_size = RxDataLenght - 4;
                                //memcpy (DirData->FileList, &X_BAND_TRNSM_Result_Rx_Buffer[4], RxDataLenght - 4);
                                memcpy (&stackEntry->ESTTC_data[sizeof (DirData->DirNextAvailable) + sizeof (DirData->NumberFiles)], &S_X_BAND_TRNSM_Result_Rx_Buffer[4], RxDataLenght - 4);
                                stackEntry->ESTTC_data_size = RxDataLenght - 1;
                            //}
                        } else {
                            DirData->DirNextAvailable = DirData->NumberFiles = 0;
                        }
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
* @brief Lists all available files in the SD card
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_Dir_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint8_t S_X_BAND_TRNSM_CMD_Dir_Extended (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    S_X_BAND_TRNSM_FileList_struct DirData;
    S_X_BAND_TRNSM_FileInfo_struct FileName;
    static uint8_t S_X_BAND_TRNSM_CMD_Dir_i = 0, S_X_BAND_TRNSM_CMD_Dir_index = 0;

    // Do da dew
    //retStat = S_X_BAND_TRNSM_CMD_Dir (&XSBandStack_Slave);
    if (retStat) {
        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
        return retStat;
    } else if (stackEntry->state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
        // Check the status
        if (stackEntry->ESTTC_data_size >= (sizeof (uint8_t) + sizeof (uint16_t))) {
            memcpy (&DirData, stackEntry->ESTTC_data, sizeof (uint8_t) + sizeof (uint16_t));
            //DirData.FileList = &XBandStack_Slave.ESTTC_data[3];
            DirData.FileList = (uint8_t *)&stackEntry->ESTTC_data[sizeof (uint8_t) + sizeof (uint16_t)];
        }
        if (!S_X_BAND_TRNSM_CMD_Dir_i++) {
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD;
            printf ( "Page 0, Found files = %d \r\n", DirData.NumberFiles);
            for (uint8_t i = 0; i < DirData.NumberFiles; i++) {
                if (S_X_BAND_TRNSM_FileNameParser (&DirData.FileList[S_X_BAND_TRNSM_CMD_Dir_index], &FileName)) {
                    S_X_BAND_TRNSM_CMD_Dir_index += FileName.NameLength + 4;
                    printf ( "%02d Name: %s ", i, FileName.FileName);
                    printf ( " [Size: %d]\r\n", (int)FileName.Size);
                } else {
                    printf ( " File name parser Err \r\n");
                    retStat = 0x01;
                    DirData.DirNextAvailable = 0;
                    break;
                }
            }
        } else {
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD;
            // Continunu
            printf ( "\r\nPage %d, Found files = %d \r\n", S_X_BAND_TRNSM_CMD_Dir_i, DirData.NumberFiles);
            for (uint8_t i = 0; i < DirData.NumberFiles; i++) {
                if (S_X_BAND_TRNSM_FileNameParser (&DirData.FileList[S_X_BAND_TRNSM_CMD_Dir_index], &FileName)) {
                    S_X_BAND_TRNSM_CMD_Dir_index += FileName.NameLength + 4;
                    printf ( "%02d Name: %s ", i, FileName.FileName);
                    printf ( " [Size: %d]\r\n", (int)FileName.Size);
                } else {
                    printf ( " File name parser Err \r\n");
                    retStat = 0x01;
                    DirData.DirNextAvailable = 0;
                    break;
                }
            }
            // Check the counter, if the limit is reached - stop printing the DIR data
            if (S_X_BAND_TRNSM_CMD_Dir_i == 50) {
                DirData.DirNextAvailable = 0;
            }
        }
        // X_BAND_TRNSM_CMD_Dir_i++;
        // Reset the slave command
        stackEntry->command = S_X_BAND_TRNSM_CMD_DIR_NEXT_F;
        stackEntry->ESTTC_data_size = 0;
        S_X_BAND_TRNSM_CMD_Dir_index = 0;
        // Check for more data
        if (!DirData.DirNextAvailable) {
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
        }
    }

    return retStat;
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
uint8_t S_X_BAND_TRNSM_WriteFile (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
	printf("WRITE FILE !!!!!!!!!!!\n");
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
			memcpy(stackEntry->ESTTC_data, &S_X_BAND_TRNSM_FileDataBuffer.Size, sizeof(uint16_t));
			memcpy(&stackEntry->ESTTC_data[2], &stackEntry->fileHandler, sizeof(uint32_t)); //Data Packet Length
			memcpy(&stackEntry->ESTTC_data[6], &S_X_BAND_TRNSM_FileDataBuffer.PacketNumber, sizeof(uint32_t)); //Data Packet Length + File Handle
			memcpy(&stackEntry->ESTTC_data[10], S_X_BAND_TRNSM_FileDataBuffer.Data, sizeof(uint8_t) * S_X_BAND_TRNSM_FileDataBuffer.Size); //Data Packet Length + File Handle + Packet Number
            retStat = S_X_BAND_TRNSM_SendCMD (S_X_BAND_TRNSM_CMD_WRITE_F, S_X_BAND_TRNSM_NULL_TYPE, stackEntry->ESTTC_data, S_X_BAND_TRNSM_FileDataBuffer.Size + 10);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->minDelay = 1;
                stackEntry->command = S_X_BAND_TRNSM_CMD_WRITE_F;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                retStat = 0x00;
            } else {
				printf ("\r\n WRITE_F CMD ERROR = %d (retry No %d)\n", retStat, stackEntry->retries);
                // Failed to send the command
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                        stackEntry->retries = 0;
                        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                        return S_X_BAND_TRNSM_PARAMS_COMM_ERR;
                }
                S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, stackEntry);
            }
			break;
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT:
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES:
			retStat = S_X_BAND_TRNSM_GetResult ( stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght) ;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                retStat = S_X_BAND_TRNSM_SEND_CARD_ERR; //0xF1;
                // Verify the answer
                if (RxDataLenght == sizeof (retStat)) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (!retStat) {
                        // On success move to the next packet
                        S_X_BAND_TRNSM_FileDataBuffer.PacketNumber++;
                        /* Set new position */
                        S_X_BAND_TRNSM_UploadToXSBand_FPos += S_X_BAND_TRNSM_FileDataBuffer.Size;
                        // Update the state
                        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
						return retStat;
                    } else {
                        retStat = 0xF3;
                    }
                } else {
                    retStat = 0xF5;
                }
            } else {
                if (S_X_BAND_STACK_RETR_COND(retStat)) retStat = 0x00;
                else {
                    retStat = 0xF6;
                }
            }
			if (retStat) {
                printf ("\r\n WRITE_F GETRESULT ERROR = %.2X (retry No %d)\n", retStat, stackEntry->retries);
				retStat = 0x00;
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
        TxPackStruct->Length = 0;
    }
	printf("S_X_BAND_TRNSM_GetResult write\n");
    /* copy the structure of the packet to the transmit buffer */
    TxPackCRC = crc32 (0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length);
    /* copy the CRC to the transmit buffer after the structure of the packet */
    memcpy (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length], &TxPackCRC, sizeof (TxPackCRC));
    if (((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) > sizeof (TxDataBuffer)) {
        /* check if the transmit buffer is insufficient */
        return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
    }
    memset (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)], 0, ((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) - (S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)));
	bytesToWrite = (((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4;
    for (retries = 0; retries < S_X_BAND_TRNSM_TX_RETRY ;) {
		purgeBuffer();
		ftStatus = FT_Write(ftHandle, 
							TxDataBuffer, 
							bytesToWrite,
							&bytesWritten);								
		if (ftStatus == FT_OK) {
			lenghtQueue(&RxBytes);
			ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived); 
			if (ftStatus == FT_OK && BytesReceived == RxBytes) {
				//readPackStruct(RxPackStruct, RxBuffer);
				memcpy(S_X_BAND_TRNSM_X_Band_Data, RxBuffer, BytesReceived);
				// check for end of pack and verify the packet
				if (
					(RxPackStruct->Header == TxPackStruct->Header) &&
					(RxPackStruct->ModuleID == TxPackStruct->ModuleID)) 
				{
					printf("S_X_BAND_TRNSM_GetResult read\n");
					RxCRC = crc32 (0, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_STRCT_SIZE + RxPackStruct->Length);
					pRxCRC = (uint32_t *)&S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE + RxPackStruct->Length];
					if (*pRxCRC == RxCRC) {
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
	
	printf("S_X_BAND_TRNSM_Send_Ack write\n");
    /* copy the structure of the packet to the transmit buffer */
    TxPackCRC = crc32(0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE);

    // Clear the extra bytes that will be set after the packet to complete to divisible to 16
    memset(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC)],0,((((S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC))>>4)+1)<<4) - (S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC)));
	
    /* copy the CRC to the transmit buffer after the structure of the packet */
    memcpy(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length], &TxPackCRC, sizeof(TxPackCRC));
	bytesToWrite=(((S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC))>>4)+1)<<4;
	
	purgeBuffer();
	ftStatus = FT_Write(ftHandle, 
						TxDataBuffer, 
						bytesToWrite,
						&bytesWritten);		
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
	printf("S_X_BAND_TRNSM_SendCMD write\n");
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
			if(RxBytes > 0)
			{
				ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived); 
				if (ftStatus == FT_OK && BytesReceived == RxBytes) {
					memcpy(S_X_BAND_TRNSM_X_Band_Data, RxBuffer, BytesReceived);
					// check for end of pack and verify the packet
					if (
						(RxPackStruct->Header == TxPackStruct->Header) &&
						(RxPackStruct->ModuleID == TxPackStruct->ModuleID)){
						printf("S_X_BAND_TRNSM_SendCMD Read\n");
						RxCRC = crc32 (0, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_STRCT_SIZE);
						pRxCRC = (uint32_t *)&S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE];
						if (*pRxCRC == RxCRC) {
							if (RxPackStruct->Response == S_X_BAND_TRNSM_STAT_ACK) {
								retries = S_X_BAND_TRNSM_TX_RETRY;
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
						printf("No read\n");
						RxPackStruct->Response = S_X_BAND_TRNSM_STAT_COMM_ERR;
						// retry - send the packet again
					}
				}
				else
				{
					printf("read error\n");
					RxPackStruct->Response = S_X_BAND_TRNSM_STAT_COMM_ERR;
				}
			}
			else
			{
				printf("no read\n");
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
	
    return (S_X_BAND_TRNSM_Response_enum)RxPackStruct->Response;

}
