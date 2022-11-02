/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @file X_Band_Trnsm.c
* @brief Implements the very minimum commands to transmit by S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @author            Vassil Milev
* @version           1.0.0
* @date              2019.08.15
*
* @copyright         (C) Copyright Endurosat
*
*                    Contents and presentations are protected world-wide.
*                    Any kind of using, copying etc. is prohibited without prior permission.
*                    All rights - incl. industrial property rights - are reserved.
*
* @history
* @revision{         1.0.0  , 2019.10.18, author Vassil Milev, Initial revision }
* @revision{         1.0.1  , 2019.12.02, author Vassil Milev, Added Read and Write commands for all Attenuations }
* @endhistory
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* INCLUDES
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
#include "S_X_Band_Trnsm.h"
#include "es_crc32.h"
#include "string.h"
#include "stm32f4xx_it.h"
#include "fatfs.h"

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* INTERNAL DEFINES
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
#define X_BAND_TRNSM_DEFAULT_DEV_ID       (0x1003)            /* ID of the device that the OBC is communicating with */
#define S_BAND_TRNSM_DEFAULT_DEV_ID       (0x2002)            /* ID of the device that the OBC is communicating with */

#define S_X_BAND_TRNSM_TX_RETRY               (10)                /* Number of times to reply any packet */
#define S_X_BAND_TRNSM_CMD_RETRY              (6)                 /* Number of times to reply a whole request/Command */
#define S_X_BAND_TRNSM_BUSY_TIMEOUT           ((uint16_t)21000)   /* Maximum timeout to allowed the S-band to stay in busy state */
#define S_X_BAND_TRNSM_HEADER                 (0x50555345)        /* Header of every packet */
#define S_X_BAND_TRNSM_READ_FILE_CHUNCK_SIZE  (1472)              /* Size of data read by one packet */
#define S_X_BAND_TRNSM_MAX_READ_BUFFER        (S_X_BAND_TRNSM_READ_FILE_CHUNCK_SIZE + S_X_BAND_TRNSM_TX_RETRY)   /* Maximum size of read data */
#define S_X_BAND_TRNSM_DEFAULT_BAUDRATE       (4)                 /* Default speed of the USART that is connected to the S-band transmitter */ //#define S_BAND_DEFAULT_BAUDRATE ((uint32_t)(3000000))
#define S_X_BAND_STACK_SIZE                   (3)                 /* X-Band stack size */
#define S_X_BAND_STACK_DELAY                  (2)                 /* X-Band stack delay between protocol states (in ms) */
// Retry condition
#define S_X_BAND_STACK_RETR_COND(stat)        (S_X_BAND_TRNSM_STAT_BUSY == (stat) || S_X_BAND_TRNSM_STAT_NACK == (stat))
// Explicit macro for protocol retries reduction on condition
#define S_X_BAND_STACK_RETR_SUB(condition,stackEntry)   if (condition) (stackEntry)->retries--
// Debugging the X-Band stack
#define S_X_BAND_STACK_DEBUG                  (1)
// X-Band stack safety enabled or not
#define S_X_BAND_STACK_SAFETY                 (1)

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* INTERNAL TYPES DEFINITION
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/*
 * Protocol stack machine-related
 */
// Commands NB states
typedef enum S_X_BAND_TRNSM_CMD_State {
    S_X_BAND_TRNSM_CMD_STATE_EMPTY = 0x00,
    S_X_BAND_TRNSM_CMD_STATE_CMD,
    S_X_BAND_TRNSM_CMD_STATE_CMD_RES,
    S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT,
    S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES,
    S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED
} S_X_BAND_TRNSM_CMD_State;
// Stack node/entry structure
typedef struct X_BAND_CMD_StackEntry {
    uint16_t devID;
    S_X_BAND_TRNSM_CMD_State state;
    uint8_t ESTTC_request, ESTTC_request_type, ESTTC_data_size;
    uint8_t ESTTC_data[S_X_BAND_TRNSM_RX_BUFF_SIZE];
    S_X_BAND_TRNSM_CMD_enum command;
    uint8_t isFileCommand;
    uint16_t type;
    S_X_BAND_TRNSM_Response_enum response;
    S_X_BAND_TRNSM_RetRes_enum result;
    FILE *console;
    uint32_t fileHandler, fileSize;
    uint32_t timestamp, minDelay;
    uint16_t retries;
    uint8_t expectSlaveCommand;
    struct X_BAND_CMD_StackEntry *parentEntry;
} S_X_BAND_CMD_StackEntry;
// Callbacks type
typedef uint8_t (*S_X_BAND_TRNSM_CMD_callbacks_t)(S_X_BAND_CMD_StackEntry *);

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* EXTERNAL VARIABLES DEFINITION
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint16_t S_X_Band_Index = 0;                                              /* Number of received bytes */
uint8_t  S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_RX_BUFF_SIZE];           /* Buffer for row data from the S-band transmitter - includes header, information about the packet and the data itself */
uint8_t  S_X_BAND_TRNSM_Result_Rx_Buffer[S_X_BAND_TRNSM_RX_BUFF_SIZE];      /* The received data that has been requested */

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* INTERNAL (STATIC) VARIABLES DEFINITION/DECLARATION 
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t  TxDataBuffer[S_X_BAND_TRNSM_TX_BUFF_SIZE];                /* Buffer for the data that is going to be transmitted */
static uint32_t S_X_BAND_TRNSM_S_Current_Baudrate;                        /* Currently used baudrate */
static uint32_t S_X_BAND_LastExecCmd_Tick;                                /* How much ticks has passed from the last executed command */

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

/* look up table with appropriate timeouts according to the command */
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

/* look up table with number of retries to acheeve the command timeout of 21 seconds */
//  static const uint16_t TxCMD_BusyRetry[S_X_BAND_TRNSM_CMD_NUMBER] =
//  {
//  #ifdef ENABLE_SX_BAND_TESTBOARD
//  /* X_BAND_TRNSM_CMD_PD_RF_OUT_ENB_DIS,          */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A)),
//  /* X_BAND_TRNSM_CMD_PD_CLEAR_WARNINGS,          */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A)),
//  /* X_BAND_TRNSM_CMD_PD_CALIBRATION_A,           */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A)),
//  /* X_BAND_TRNSM_CMD_PD_REWRITE_CONFIG,          */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A)),
//  /* X_BAND_TRNSM_CMD_PD_RESET,                   */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A)),
//  /* X_BAND_TRNSM_CMD_PD_ADAPTATION_ENB_DIS,      */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A)),
//  #endif /* ENABLE_SX_BAND_TESTBOARD */
//  /* S_X_BAND_TRNSM_CMD_GET,          */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_SET,          */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A)) ,
//  /* S_X_BAND_TRNSM_CMD_DIR_F,        */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_DIR_NEXT_F,   */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_DELL_F,       */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_DELL_ALL_F,   */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_CREATE_F,     */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_WRITE_F,      */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01*3)),
//  /* S_X_BAND_TRNSM_CMD_OPEN_F,       */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_READ_F,       */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_SEND_F,       */ ((15*60*1000)/(0xFF*3))              , /* 15 minutes max */
//  /* S_X_BAND_TRNSM_CMD_TX_MODE,      */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0xFF)) ,
//  /* S_X_BAND_TRNSM_CMD_LOAD_MODE,    */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x0A*3)),
//  /* S_X_BAND_TRNSM_CMD_UPDATE_FW,    */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0xFF)) ,
//  /* S_X_BAND_TRNSM_CMD_SAFE_SHUTDOWN,*/ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01)) ,
//  /* S_X_BAND_TRNSM_CMD_GET_RES,      */ ((S_X_BAND_TRNSM_BUSY_TIMEOUT)/(0x01))
//  };

/* Supported baudrates = x*10 */
static const uint16_t BaudRateArra[] = {
        25,
        50,
        100,
        200,
        300,
//        500,
//        900,
//        1000,
//        1500,
//        1800,
//        2250
};

/* Stack-related */
// Stack structure
static S_X_BAND_CMD_StackEntry XSBandStack[S_X_BAND_STACK_SIZE], XSBandStack_Slave;

/* Driver device type */
//static S_X_BAND_TRNSM_DEVTYPE XSBandDevType;

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* INTERNAL (STATIC) ROUTINES DECLARATION
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void S_X_BAND_TRNSM_Rx_Tx_State (uint8_t state);
static void S_X_BAND_TRNSM_Send_Ack (uint16_t Identifier, S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type);
static uint8_t S_X_BAND_TRNSM_CMD_GET_Param (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_CMD_SET_Param (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_CMD_Dir (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_CMD_Dir_Extended (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_DelFile (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_CreateFile (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_UploadFileToSBandTrnsm (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_DownloadFileFromSBandTrnsm (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_OpenFile (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_SendFile (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_StartTxMode (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_StartLoadMode (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_FW_Update (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_ShutDownMode (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_CMD_SET_Param_Baudrate (S_X_BAND_CMD_StackEntry *stackEntry);
static void XSBandTryFinish (uint8_t retRes, S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t XSBandExtractCommandAndType (S_X_BAND_CMD_StackEntry *stackEntry);
static uint8_t S_X_BAND_TRNSM_AutoSearchBaudrate (uint16_t Identifier);
static uint8_t S_X_BAND_TRNSM_FileNameParser (uint16_t Identifier, uint8_t * FileListBuffer, S_X_BAND_TRNSM_FileInfo_struct * File);
/* Low level functions */
static S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_SendCMD(uint16_t Identifier, S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type, uint8_t * TxData, uint16_t TxDataLenght);
static S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_GetResult(uint16_t Identifier, S_X_BAND_TRNSM_CMD_enum CMD, S_X_BAND_TRNSM_RetRes_enum CMD_Type, uint8_t * RxData, uint16_t * RxDataLenght);
/* End of Low level functions */
// Stack callback table
static const S_X_BAND_TRNSM_CMD_callbacks_t XSBandCallbacks[S_X_BAND_TRNSM_CMD_NUMBER] = {
    /* S_X_BAND_TRNSM_CMD_GET */                      S_X_BAND_TRNSM_CMD_GET_Param,
    /* S_X_BAND_TRNSM_CMD_SET */                      S_X_BAND_TRNSM_CMD_SET_Param,
    /* S_X_BAND_TRNSM_CMD_DIR_F */                    S_X_BAND_TRNSM_CMD_Dir_Extended,
    /* S_X_BAND_TRNSM_CMD_DIR_NEXT_F */               S_X_BAND_TRNSM_CMD_Dir_Extended,
    /* S_X_BAND_TRNSM_CMD_DELL_F */                   S_X_BAND_TRNSM_DelFile,
    /* S_X_BAND_TRNSM_CMD_DELL_ALL_F */               S_X_BAND_TRNSM_DelFile,
    /* S_X_BAND_TRNSM_CMD_CREATE_F */                 S_X_BAND_TRNSM_UploadFileToSBandTrnsm,
    /* S_S_X_BAND_TRNSM_CMD_WRITE_F */                S_X_BAND_TRNSM_UploadFileToSBandTrnsm,
    /* S_X_BAND_TRNSM_CMD_OPEN_F */                   S_X_BAND_TRNSM_DownloadFileFromSBandTrnsm,
    /* S_X_BAND_TRNSM_CMD_READ_F */                   S_X_BAND_TRNSM_DownloadFileFromSBandTrnsm,
    /* S_X_BAND_TRNSM_CMD_SEND_F */                   S_X_BAND_TRNSM_SendFile,
    /* S_X_BAND_TRNSM_CMD_TX_MODE */                  S_X_BAND_TRNSM_StartTxMode,
    /* S_X_BAND_TRNSM_CMD_LOAD_MODE */                S_X_BAND_TRNSM_StartLoadMode,
    /* S_X_BAND_TRNSM_CMD_UPDATE_FW */                S_X_BAND_TRNSM_FW_Update,
    /* S_X_BAND_TRNSM_CMD_SAFE_SHUTDOWN */            S_X_BAND_TRNSM_ShutDownMode,
    /* S_X_BAND_TRNSM_CMD_GET_RES */                  NULL
};

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* EXTERNAL (NONE STATIC) ROUTINES DEFINITION
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Init routine for the S_X_BAND_TRNSMITTER component
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      none
* @param[output]     none
* @return            none
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void S_X_BAND_TRNSM_Init (void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

//  XSBandDevType = devType;    //VMI_TODO - why is this for
    /* Reset the in-memory structures */
    memset (XSBandStack, 0x00, sizeof (XSBandStack));
    memset (&XSBandStack_Slave, 0x00, sizeof (XSBandStack_Slave));
    S_X_BAND_LastExecCmd_Tick = 0;
    /* Initialize the Current baudrate to default */
    S_X_BAND_TRNSM_S_Current_Baudrate = S_X_BAND_TRNSM_DEFAULT_BAUDRATE;    //Default settings 4 -> BaudRateArra[4] =

    HAL_UART_DMAStop((UART_HandleTypeDef*)COM_SBAND);
    HAL_UART_DeInit((UART_HandleTypeDef*)COM_SBAND);

    __HAL_RCC_GPIOF_CLK_ENABLE();

    MX_DMA_USART7_Init();

    /* UART7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(UART7_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART7_IRQn);

    /* Set the speed of the USART */
    MX_UART7_Init (S_X_BAND_TRNSM_GetBaudrate());

    memset(TxDataBuffer, 0, S_X_BAND_TRNSM_TX_BUFF_SIZE);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(RS485_TX_GPIO_Port, RS485_TXPin, GPIO_PIN_RESET);

    /*Configure GPIO pin : RS485 Transmit enable pin */
    GPIO_InitStruct.Pin = RS485_TXPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RS485_TX_GPIO_Port, &GPIO_InitStruct);

    /* Initialise the variable as none command has been requested yet */
    // X_BAND_TRNSM_CMD_ESTTC_request = 0;
    // X_BAND_TRNSM_CMD_ESTTC_console = COMM;

    /* Default ID of the S-Band Transmitter */
    // X_BAND_TRNSM_S_ID = X_BAND_TRNSM_DEFAULT_SBAND_ID;

    /* Release the USART interface */
    S_X_DeSelect();
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Deinit routine for the S_X_BAND_TRNSMITTER component
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      none
* @param[output]     none
* @return            none
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void S_X_BAND_TRNSM_DeInit(void)
{
    HAL_UART_DMAStop((UART_HandleTypeDef*)COM_SBAND);
    HAL_UART_DeInit((UART_HandleTypeDef*)COM_SBAND);

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = SX_BAND_RX_Pin|SX_BAND_TX_Pin|RS485_TXPin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Returns the currently used baudrate
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      none
* @param[output]     none
* @return            none
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
uint32_t S_X_BAND_TRNSM_GetBaudrate(void) {
    return BaudRateArra[S_X_BAND_TRNSM_S_Current_Baudrate] * 10000;
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
static S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_SendCMD (uint16_t Identifier, S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type, uint8_t * TxData, uint16_t TxDataLenght) {
    uint8_t retries;
    S_X_BAND_TRNSM_Pack_struct * RxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)S_X_BAND_TRNSM_X_Band_Data;
    S_X_BAND_TRNSM_Pack_struct * TxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)TxDataBuffer;
    uint32_t RxCRC;
    uint32_t * pRxCRC;
    uint32_t TxPackCRC = 0;

    if (CMD >= S_X_BAND_TRNSM_CMD_GET_RES) {
        /* allow all commands but Get result */
        return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
    }
    if ((TxDataLenght + sizeof (S_X_BAND_TRNSM_Pack_struct) + sizeof(RxCRC) ) > S_X_BAND_TRNSM_TX_BUFF_SIZE) {
        /* if CPU stops here - please increase the size of the buffer -> X_BAND_TRNSM_TX_BUFF_SIZE */
        return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
    }
    // Stack timeout
    if ((xTaskGetTickCount() - S_X_BAND_LastExecCmd_Tick) < (1 * S_X_BAND_STACK_DELAY)) {
#if S_X_BAND_STACK_DEBUG
        // fprintf (COMM, "Timeout: X_BAND_TRNSM_SendCMD %d %d => %d - %d = %d\r\n", (int)CMD, (int)CMD_Type, (int)xTaskGetTickCount(), (int)X_BAND_LastExecCmd_Tick, (int)xTaskGetTickCount() - (int)X_BAND_LastExecCmd_Tick);
#endif
        return S_X_BAND_TRNSM_STAT_GET_RESULT;
    }
    TxPackStruct->Header = S_X_BAND_TRNSM_HEADER;
    TxPackStruct->ModuleID = Identifier;
    if (TxCMD_Descriptor[CMD].tx_data_max_size > 0) {
        if (TxCMD_Descriptor[CMD].tx_data_max_size >= TxDataLenght) {
            TxPackStruct->Length = TxDataLenght;
        } else {
            return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
        }
    } else {
        TxPackStruct->Length = 0;
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
    } else {
        TxPackCRC = crc32(0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE);
        /* copy the CRC to the transmit buffer after the structure of the packet */
        memcpy(&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE], &TxPackCRC, sizeof(TxPackCRC));
    }
    for (retries = 0; retries < S_X_BAND_TRNSM_TX_RETRY ;) {
        if (((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) > sizeof (TxDataBuffer)) {
            S_X_BAND_LastExecCmd_Tick = xTaskGetTickCount();
            /* check if the transmit buffer is insufficient */
            return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
        }
        /* Reserve the USART interface for X-band */
        if (pdFALSE == S_X_Select (SX_SELECT_X_BAND_TRANSCEIVER)) {
            S_X_BAND_LastExecCmd_Tick = xTaskGetTickCount();
            return S_X_BAND_TRNSM_STAT_COMM_ERR;
        }
        HAL_UART_Receive_DMA ((UART_HandleTypeDef *)COM_SBAND, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_RX_BUFF_SIZE);
        // Clear the extra bytes that will be set after the packet to complete to divisible to 16
        memset (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)], 0, ((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) - (S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)));
        S_X_BAND_TRNSM_Rx_Tx_State(1);
        HAL_StatusTypeDef UsartStat;
        //          ------------------------------------------------------------------------------     send number of bytes divisible to 16     ||     ||
        //          ------------------------------------------------------------------------------                                              \/     \/
        UsartStat = HAL_UART_Transmit ((UART_HandleTypeDef *)COM_SBAND, TxDataBuffer, (((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4, MCU_WWDG_MAX_CYCLING_TIME);
        if (UsartStat == HAL_TIMEOUT) {
           HAL_UART_ErrorCallback ((UART_HandleTypeDef *)COMM);
        }
        S_X_BAND_TRNSM_Rx_Tx_State (0);
        osDelay(2);//1ms timeout
        /* calculate the number of received bytes */
        S_X_Band_Index = S_X_BAND_TRNSM_RX_BUFF_SIZE - hdma_uart7_rx.Instance->NDTR;
        HAL_UART_AbortReceive((UART_HandleTypeDef *)COM_SBAND); // the packet is received, no need to continued receiving (the DMA buffer is bigger then the packet and will never be filled with one packet)
        /* Release the USART interface */
        S_X_DeSelect();
        if (S_X_Band_Index > 0) {
            S_X_Band_Index = 0;
            // check for end of pack and verify the packet
            if (
                 (RxPackStruct->Header == TxPackStruct->Header) &&
                 (RxPackStruct->ModuleID == TxPackStruct->ModuleID)) {
                RxCRC = crc32 (0, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_STRCT_SIZE);
                pRxCRC = (uint32_t *)&S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE];
                if (*pRxCRC == RxCRC) {
                    if (RxPackStruct->Response == S_X_BAND_TRNSM_STAT_ACK) {
                        retries = S_X_BAND_TRNSM_TX_RETRY; // exit the loop
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
        } else {
            // none bytes are received
            RxPackStruct->Response = S_X_BAND_TRNSM_STAT_COMM_ERR;
            // retry - send the packet again - due to timeout with on answer
        }
        if (retries < S_X_BAND_TRNSM_TX_RETRY) {
            retries ++;
            if (retries == S_X_BAND_TRNSM_TX_RETRY) {
                //RxPackStruct->Response = X_BAND_TRNSM_STAT_NACK;
            } else {
                // wait some time between retries
                //osDelay(1);
            }
        }
    }
    S_X_BAND_LastExecCmd_Tick = xTaskGetTickCount();

    return (S_X_BAND_TRNSM_Response_enum)RxPackStruct->Response;
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
static S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_GetResult (uint16_t Identifier, S_X_BAND_TRNSM_CMD_enum CMD, S_X_BAND_TRNSM_RetRes_enum CMD_Type, uint8_t * RxData, uint16_t * RxDataLenght) {
    uint8_t  retries;
//    uint16_t busy_retries = 0;
    S_X_BAND_TRNSM_Pack_struct * RxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)S_X_BAND_TRNSM_X_Band_Data;
    S_X_BAND_TRNSM_Pack_struct * TxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)TxDataBuffer;
    uint32_t RxCRC;
    uint32_t * pRxCRC;
    uint32_t TxPackCRC = 0;

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
    TxPackStruct->ModuleID = Identifier;
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

    /* copy the structure of the packet to the transmit buffer */
    TxPackCRC = crc32 (0, (BYTE *)TxPackStruct, S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length);
    /* copy the CRC to the transmit buffer after the structure of the packet */
    memcpy (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length], &TxPackCRC, sizeof (TxPackCRC));
    for (retries = 0; retries < S_X_BAND_TRNSM_TX_RETRY ;) {
        if (((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) > sizeof (TxDataBuffer)) {
            /* check if the transmit buffer is insufficient */
            return S_X_BAND_TRNSM_STAT_WRONG_PARAM;
        }
        /* Reserve the USART interface for X-band */
        if (pdFALSE == S_X_Select (SX_SELECT_X_BAND_TRANSCEIVER)) {
            return S_X_BAND_TRNSM_STAT_COMM_ERR;
        }
        HAL_UART_Receive_DMA ((UART_HandleTypeDef *)COM_SBAND, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_RX_BUFF_SIZE);
        // Clear the extra bytes that will be set after the packet to complete to divisible to 16
        memset (&TxDataBuffer[S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)], 0, ((((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4) - (S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC)));
        S_X_BAND_TRNSM_Rx_Tx_State(1);
        HAL_StatusTypeDef UsartStat;
        //          ------------------------------------------------------------------------------     send number of bytes divisible to 16     ||     ||
        //          ------------------------------------------------------------------------------                                              \/     \/
        UsartStat = HAL_UART_Transmit ((UART_HandleTypeDef *)COM_SBAND, TxDataBuffer, (((S_X_BAND_TRNSM_STRCT_SIZE + TxPackStruct->Length + sizeof (TxPackCRC) - 1) >> 4) + 1) << 4, MCU_WWDG_MAX_CYCLING_TIME);
        if (UsartStat == HAL_TIMEOUT) {
           HAL_UART_ErrorCallback((UART_HandleTypeDef *)COMM);
        }
        S_X_BAND_TRNSM_Rx_Tx_State (0);
        osDelay (2 + ((S_X_BAND_TRNSM_STRCT_SIZE + 4 + TxCMD_Descriptor[CMD].rx_data_max_size + (BaudRateArra[S_X_BAND_TRNSM_S_Current_Baudrate] / 2)) / BaudRateArra[S_X_BAND_TRNSM_S_Current_Baudrate]));//1ms timeout + 1 ms for every 300 bytes (at speed 3MB/sec) and round up
        // osDelay (5);
        /* calculate the number of received bytes */
        S_X_Band_Index = S_X_BAND_TRNSM_RX_BUFF_SIZE - hdma_uart7_rx.Instance->NDTR;
        HAL_UART_AbortReceive((UART_HandleTypeDef *)COM_SBAND); // the packet is received, no need to continued receiving (the DMA buffer is bigger then the packet and will never be filled with one packet)
        /* Release the USART interface */
        S_X_DeSelect();
        if (S_X_Band_Index > 0) {
            S_X_Band_Index = 0;
            // check for end of pack and verify the packet
            if (
                 (RxPackStruct->Header == TxPackStruct->Header) &&
                 (RxPackStruct->ModuleID == TxPackStruct->ModuleID)
               ) {
                RxCRC = crc32 (0, S_X_BAND_TRNSM_X_Band_Data, S_X_BAND_TRNSM_STRCT_SIZE + RxPackStruct->Length);
                pRxCRC = (uint32_t *)&S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE + RxPackStruct->Length];
                if (*pRxCRC == RxCRC) {
                    if (RxPackStruct->Response == S_X_BAND_TRNSM_STAT_BUSY) {
                        osDelay (TxCMD_BusyTimePeriod[CMD]);
                        break;
                    } else {
                        retries = S_X_BAND_TRNSM_TX_RETRY; // exit the loop
                        if (RxPackStruct->Response == S_X_BAND_TRNSM_STAT_GET_RESULT) {
                            // Copy the length of data
                            *RxDataLenght = RxPackStruct->Length;
                            //copy the data
                            memcpy (RxData, &S_X_BAND_TRNSM_X_Band_Data[S_X_BAND_TRNSM_STRCT_SIZE], RxPackStruct->Length);
                            S_X_BAND_TRNSM_Send_Ack (Identifier, CMD, CMD_Type);
                            //osDelay(1);//1ms timeout
                        }
                    }
                }
            } else {
                // retry - send the packet again
            }
        } else {
            // retry - send the packet again - due to timeout with on answer
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
* @brief Change the Setting: Baudrate
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_SetGetParams_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_CMD_SET_Param_Baudrate (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t  retStat = 0x00;
    uint16_t RxDataLenght;
    static uint32_t Current_Baudrate_Temp;
    uint8_t pBaudrate = *((uint8_t *)stackEntry->ESTTC_data);

    // Check the requested baud rate
    if (pBaudrate >= sizeof (BaudRateArra) / sizeof (BaudRateArra[0])) {
        retStat = S_X_BAND_TRNSM_PARAMS_PARAMS_ERR;
        return retStat;
    }
    // Check for available retries
    if (!stackEntry->retries) {
        if (stackEntry->state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
            retStat = 0x01;
        }
        return retStat;
    }
    // Check for command state
    switch (stackEntry->state) {
        case S_X_BAND_TRNSM_CMD_STATE_CMD:
        case S_X_BAND_TRNSM_CMD_STATE_CMD_RES:
            Current_Baudrate_Temp = S_X_BAND_TRNSM_S_Current_Baudrate;
            S_X_BAND_TRNSM_S_Current_Baudrate = pBaudrate;
            // Send command
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, &pBaudrate, sizeof (pBaudrate));
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
                }
                // Failed to send the command
                // retStat = 0x01;
                // Subtract the retry count
                S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, stackEntry);
                // stackEntry->retries--;
            }
            retStat = 0x00;
            break;
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT:
        case  S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES:
            // Check whether the protocol delay has passed
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Change to the new baudrate
            MX_UART7_Init (BaudRateArra[pBaudrate] * 10000);
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (sizeof (retStat) == RxDataLenght) {
                    // Update the state
                    stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    if (retStat == S_X_BAND_TRNSM_PARAMS_OK || retStat == S_X_BAND_TRNSM_PARAMS_PARAMS_ERR) {
                        if (retStat == S_X_BAND_TRNSM_PARAMS_OK) {
                            S_X_BAND_TRNSM_S_Current_Baudrate = pBaudrate;
                        }
                        break;
                    }
                } else {
                    // Wrong answer (return the baudrate)
                    // retStat = 0x01;

                    // Go back to the previous baud rate
                    S_X_BAND_TRNSM_S_Current_Baudrate = Current_Baudrate_Temp; //(return the baudrate)
					MX_UART7_Init (S_X_BAND_TRNSM_GetBaudrate());
                }
            } else {
                // Failed to send the command (return the baudrate)
                // retStat = 0x01;
                if (S_X_BAND_STACK_RETR_COND(retStat)){
                    retStat = 0x00;

                    if( stackEntry->retries <= 1 )  // on last try to get results change the baudrate back to normal
                    {
                		// Go back to the previous baud rate
                		S_X_BAND_TRNSM_S_Current_Baudrate = Current_Baudrate_Temp;
                		MX_UART7_Init (S_X_BAND_TRNSM_GetBaudrate());
                    }
                }
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
static uint8_t S_X_BAND_TRNSM_CMD_Dir (S_X_BAND_CMD_StackEntry *stackEntry) {
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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, 0, 0);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
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
static uint8_t S_X_BAND_TRNSM_CMD_Dir_Extended (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    S_X_BAND_TRNSM_FileList_struct DirData;
    S_X_BAND_TRNSM_FileInfo_struct FileName;
    static uint8_t S_X_BAND_TRNSM_CMD_Dir_i, S_X_BAND_TRNSM_CMD_Dir_index;

    // Start the complex command sequence
    if (stackEntry->expectSlaveCommand) {
        memcpy (&XSBandStack_Slave, stackEntry, sizeof (S_X_BAND_CMD_StackEntry));
        XSBandStack_Slave.expectSlaveCommand = stackEntry->expectSlaveCommand = 0;
        S_X_BAND_TRNSM_CMD_Dir_i = S_X_BAND_TRNSM_CMD_Dir_index = 0;
        XSBandStack_Slave.parentEntry = stackEntry;
    }
    // Do da dew
    retStat = S_X_BAND_TRNSM_CMD_Dir (&XSBandStack_Slave);
    if (retStat) {
        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
        return retStat;
    } else if (XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
        // Check the status
        if (XSBandStack_Slave.ESTTC_data_size >= (sizeof (uint8_t) + sizeof (uint16_t))) {
            memcpy (&DirData, XSBandStack_Slave.ESTTC_data, sizeof (uint8_t) + sizeof (uint16_t));
            //DirData.FileList = &XBandStack_Slave.ESTTC_data[3];
            DirData.FileList = (uint8_t *)&XSBandStack_Slave.ESTTC_data[sizeof (uint8_t) + sizeof (uint16_t)];
        }
        if (!S_X_BAND_TRNSM_CMD_Dir_i++) {
            XSBandStack_Slave.timestamp = xTaskGetTickCount();
            XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
            fprintf (XSBandStack_Slave.console, "Page 0, Found files = %d \r\n", DirData.NumberFiles);
            for (uint8_t i = 0; i < DirData.NumberFiles; i++) {
                if (pdTRUE == S_X_BAND_TRNSM_FileNameParser (XSBandStack_Slave.devID, &DirData.FileList[S_X_BAND_TRNSM_CMD_Dir_index], &FileName)) {
                    S_X_BAND_TRNSM_CMD_Dir_index += FileName.NameLength + 4;
                    fprintf (XSBandStack_Slave.console, "%02d Name: %s ", i, FileName.FileName);
                    fprintf (XSBandStack_Slave.console, " [Size: %d]\r\n", (int)FileName.Size);
                } else {
                    fprintf (XSBandStack_Slave.console, " File name parser Err \r\n");
                    retStat = 0x01;
                    DirData.DirNextAvailable = 0;
                    break;
                }
            }
        } else {
            // Check whether the protocol delay has passed
            /* if (XBandStack_Slave.minDelay > (xTaskGetTickCount() - XBandStack_Slave.timestamp)) {
                return retStat;
            } */
            XSBandStack_Slave.timestamp = xTaskGetTickCount();
            XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
            // Continunu
            fprintf (XSBandStack_Slave.console, "\r\nPage %d, Found files = %d \r\n", S_X_BAND_TRNSM_CMD_Dir_i, DirData.NumberFiles);
            for (uint8_t i = 0; i < DirData.NumberFiles; i++) {
                if (pdTRUE == S_X_BAND_TRNSM_FileNameParser (XSBandStack_Slave.devID, &DirData.FileList[S_X_BAND_TRNSM_CMD_Dir_index], &FileName)) {
                    S_X_BAND_TRNSM_CMD_Dir_index += FileName.NameLength + 4;
                    fprintf (XSBandStack_Slave.console, "%02d Name: %s ", i, FileName.FileName);
                    fprintf (XSBandStack_Slave.console, " [Size: %d]\r\n", (int)FileName.Size);
                } else {
                    fprintf (XSBandStack_Slave.console, " File name parser Err \r\n");
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
        XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_DIR_NEXT_F;
        XSBandStack_Slave.ESTTC_data_size = 0;
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
* @brief Lists all available files in the SD card
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      Identifier - ID of the used S or X band transmitter
* @param[input]      FileListBuffer - FileList from functions X_BAND_TRNSM_CMD_Dir() and X_BAND_TRNSM_CMD_DirNext()
* @param[output]     File - a buffer where will be written the found filename, the length of the name, and the size of the file
* @return            pdTRUE - the name of the file has been found, pdFALSE - Error
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_FileNameParser (uint16_t Identifier, uint8_t * FileListBuffer, S_X_BAND_TRNSM_FileInfo_struct * File) {
    uint32_t * Size;

    File->NameLength = strlen((char *)FileListBuffer)+1;  //Get the length of the name of the file + the Null character after it

    if( (File->NameLength > 0 ) && (File->NameLength <= 31 ) )  // Verify the length
    {
        /* Get the size of the file */
        Size = (uint32_t *)&FileListBuffer[File->NameLength];
        File->Size = *Size;

        // Copy the name of the file
        memcpy( File->FileName, FileListBuffer, File->NameLength);

        return pdTRUE; //OK
    }

    return pdFALSE;   //File name is NOT found
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
static uint8_t S_X_BAND_TRNSM_DelFile (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;

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
                retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, FileName, FileNameLength);
            } else {
                retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, 0, 0);
            }
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
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
* @brief Create a file in the SD card of the S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @return            retStat - a value from enumeration X_BAND_TRNSM_CrateStatus_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_CreateFile (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
    // uint8_t *FileName = (uint8_t *)stackEntry->ESTTC_data;
    // uint8_t FileNameLength = strlen ((char *)FileName) + 1;
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
            if (!stackEntry->fileSize) {
                retStat = 0x01;
                break;
            }
            // Should insert FileName + FileSize like that
            memcpy (&(stackEntry->ESTTC_data[++stackEntry->ESTTC_data_size]), &stackEntry->fileSize, sizeof (stackEntry->fileSize));
            stackEntry->ESTTC_data_size += sizeof (stackEntry->fileSize);
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, stackEntry->ESTTC_data, stackEntry->ESTTC_data_size);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
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
* @brief Create a file in the SD card of the S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @return            retStat - a value from enumeration X_BAND_TRNSM_OpenStatus_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_OpenFile (S_X_BAND_CMD_StackEntry *stackEntry) {
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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, FileName, FileNameLength);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
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
* @brief Transmit a file from the SD card of the S-Band transmitter trough the radio
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_SendStatus_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_SendFile (S_X_BAND_CMD_StackEntry *stackEntry) {
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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, FileName, FileNameLength);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (RxDataLenght == sizeof(retStat)) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_SEND_OK) {
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
* @brief Switch the S-Band to transmit mode (Attention: High power consumption)
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_ChangeMode_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_StartTxMode (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;

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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, 0, 0);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
                }
                // Failed to send the command
                // retStat = 0x01;
                // Subtract the retry count
                // stackEntry->retries--;
                S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, stackEntry);
            }
            retStat = 0x00;
            break;
        case S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT:
        case S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES:
            // Check whether the protocol delay has passed
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (RxDataLenght == sizeof(retStat)) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_CHANG_MODE_OK) {
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
* @brief Switch the S-Band to standby mode (Low power consumption)
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_ChangeMode_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_StartLoadMode (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;

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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, 0, 0);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (RxDataLenght == sizeof(retStat)) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_CHANG_MODE_OK) {
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
* @brief Change firmware using a file in the SD card of the S-Band transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_FW_update_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_FW_Update (S_X_BAND_CMD_StackEntry *stackEntry) {
    typedef enum X_BAND_TRNSM_FW_Update_State_t {
        S_X_BAND_TRNSM_FW_Update_State_DIR = 0x00,
        S_X_BAND_TRNSM_FW_Update_State_REPORT,
        S_X_BAND_TRNSM_FW_Update_State_FWUPD,
        S_X_BAND_TRNSM_FW_Update_State_REPORT2,
        S_X_BAND_TRNSM_FW_Update_State_DELAY
    } S_X_BAND_TRNSM_FW_Update_State_t;

    uint8_t retStat = 0x00;
    static S_X_BAND_TRNSM_FW_Update_State_t S_X_BAND_TRNSM_FW_Update_State;
    static X_BAND_TRNSM_GET_FullReport_struct FullReportData_Temp[2];
    static S_BAND_TRNSM_GET_SimpleReport_struct SimpleReportData_Temp[2];

    // Check for available retries
    if (!stackEntry->retries) {
        if (stackEntry->state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
            retStat = S_X_BAND_TRNSM_PARAMS_COMM_ERR;
        }
        return retStat;
    }
    // Start the complex command sequence
    if (stackEntry->expectSlaveCommand) {
        memset (FullReportData_Temp, 0x00, sizeof (FullReportData_Temp));
        memcpy (&XSBandStack_Slave, stackEntry, sizeof (S_X_BAND_CMD_StackEntry));
        XSBandStack_Slave.expectSlaveCommand = stackEntry->expectSlaveCommand = 0;
        XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_DIR_F;
        XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
        S_X_BAND_TRNSM_FW_Update_State = S_X_BAND_TRNSM_FW_Update_State_DIR;
        XSBandStack_Slave.parentEntry = stackEntry;
    }
    // Check the global state of the complex command
    switch (S_X_BAND_TRNSM_FW_Update_State) {
        case S_X_BAND_TRNSM_FW_Update_State_DIR:
            retStat = S_X_BAND_TRNSM_CMD_Dir (&XSBandStack_Slave);
            if (!retStat && XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
                XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_GET;
                if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                    XSBandStack_Slave.type = X_BAND_TRNSM_GET_TYPE_FULL_REPORT;
                } else {
                    XSBandStack_Slave.type = S_X_BAND_TRNSM_GET_TYPE_SIMPLE_REPORT;
                }
                XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                S_X_BAND_TRNSM_FW_Update_State = S_X_BAND_TRNSM_FW_Update_State_REPORT;
            }
            break;
        case S_X_BAND_TRNSM_FW_Update_State_REPORT:
            retStat = S_X_BAND_TRNSM_CMD_GET_Param (&XSBandStack_Slave); // X_BAND_TRNSM_CMD_GET_FullReport (&XBandStack_Slave);
            if (!retStat && XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
                XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_UPDATE_FW;
                XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
                XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                S_X_BAND_TRNSM_FW_Update_State = S_X_BAND_TRNSM_FW_Update_State_FWUPD;
                if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                    memcpy (&FullReportData_Temp[0], XSBandStack_Slave.ESTTC_data, XSBandStack_Slave.ESTTC_data_size);
                } else {
                    memcpy (&SimpleReportData_Temp[0], XSBandStack_Slave.ESTTC_data, XSBandStack_Slave.ESTTC_data_size);
                }
            }
            break;
        case S_X_BAND_TRNSM_FW_Update_State_FWUPD:
            retStat = S_X_BAND_TRNSM_SendCMD (XSBandStack_Slave.devID, S_X_BAND_TRNSM_CMD_UPDATE_FW, S_X_BAND_TRNSM_NULL_TYPE, (uint8_t *)((char *)stackEntry->ESTTC_data), (uint16_t)(strlen ((char *)stackEntry->ESTTC_data) + 1));
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                XSBandStack_Slave.timestamp = stackEntry->timestamp = xTaskGetTickCount();
                XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_GET;
                if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                    XSBandStack_Slave.type = X_BAND_TRNSM_GET_TYPE_FULL_REPORT;
                } else {
                    XSBandStack_Slave.type = S_X_BAND_TRNSM_GET_TYPE_SIMPLE_REPORT;
                }
                XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                S_X_BAND_TRNSM_FW_Update_State = S_X_BAND_TRNSM_FW_Update_State_REPORT2;
                retStat = 0x00;
            } else {
                // Failed to send the command
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                        return S_X_BAND_TRNSM_PARAMS_COMM_ERR;
                    }
                }
                S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, &XSBandStack_Slave);
            }
            break;
        case S_X_BAND_TRNSM_FW_Update_State_REPORT2:
            // Check whether the protocol delay has passed
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Do da dew
            retStat = S_X_BAND_TRNSM_CMD_GET_Param (&XSBandStack_Slave); // X_BAND_TRNSM_CMD_GET_FullReport (&XBandStack_Slave);
            if (!retStat && XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                S_X_BAND_TRNSM_FW_Update_State = S_X_BAND_TRNSM_FW_Update_State_DELAY;
                XSBandStack_Slave.timestamp = xTaskGetTickCount();
                XSBandStack_Slave.minDelay = 500;
                if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                    memcpy (&FullReportData_Temp[1], XSBandStack_Slave.ESTTC_data, XSBandStack_Slave.ESTTC_data_size);
                    // Check the FW version
                    if (FullReportData_Temp[0].FW_Version != FullReportData_Temp[1].FW_Version) {
                        return S_X_BAND_TRNSM_FW_UPDATE_OK;
                    } else {
                        return S_X_BAND_TRNSM_FW_UPDATE_UPDATE_FAILD;
                    }
                } else {
                    memcpy (&SimpleReportData_Temp[1], XSBandStack_Slave.ESTTC_data, XSBandStack_Slave.ESTTC_data_size);
                    // Check the FW version
                    if (SimpleReportData_Temp[0].FW_Version != SimpleReportData_Temp[1].FW_Version) {
                        return S_X_BAND_TRNSM_FW_UPDATE_OK;
                    } else {
                        return S_X_BAND_TRNSM_FW_UPDATE_UPDATE_FAILD;
                    }
                }
            }
            break;
        case S_X_BAND_TRNSM_FW_Update_State_DELAY:
            // Check whether the protocol delay has passed
            if (XSBandStack_Slave.minDelay > (xTaskGetTickCount() - XSBandStack_Slave.timestamp)) break;
            // Do da dew
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
            return 0x00;
    }
    // Retry on error
    if (retStat || !XSBandStack_Slave.retries) {
        stackEntry->retries--;
        memset (FullReportData_Temp, 0x00, sizeof (FullReportData_Temp));
        memset (SimpleReportData_Temp, 0x00, sizeof (SimpleReportData_Temp));
        memcpy (&XSBandStack_Slave, stackEntry, sizeof (S_X_BAND_CMD_StackEntry));
        XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
        XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_DIR_F;
        XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
        S_X_BAND_TRNSM_FW_Update_State = S_X_BAND_TRNSM_FW_Update_State_DIR;
        XSBandStack_Slave.parentEntry = stackEntry;
        return 0x00;
    // Or gAt out of hell!
    } else if (XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
        if (S_X_BAND_TRNSM_FW_Update_State++ == S_X_BAND_TRNSM_FW_Update_State_DELAY) {
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
            return retStat;
        }
        XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
    }

    return retStat;
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Prepare S-Band transmitter to be turned off using the power supply
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_ChangeMode_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_ShutDownMode (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;

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
            stackEntry->timestamp = xTaskGetTickCount();
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, 0, 0);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                // if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
                }
                // Failed to send the command
                // retStat = 0x01;
                // Subtract the retry count
                // stackEntry->retries--;
                S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, stackEntry);
            }
            retStat = 0x00;
            break;
        default:
            break;
    }

    return retStat;
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Copy a file from the SD card of the X-Band Transmitter to the SD card of the OBC
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            uint8_t:
*                      + X_BAND_TRNSM_STAT_WRONG_PARAM - file not found
*                      + X_BAND_TRNSM_DellStatus_enum  - delete Error
* @note              if such a file is already existing it will be overwritten
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_DownloadFileFromSBandTrnsm (S_X_BAND_CMD_StackEntry *stackEntry) {
    typedef enum X_BAND_TRNSM_DownloadFromXSBand_State_t {
        S_X_BAND_TRNSM_DownloadFromXSBand_State_OPENFILE = 0x00,
        S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE,
        S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE_GETRESULT
    } S_X_BAND_TRNSM_DownloadFromXSBand_State_t;

    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
    S_X_BAND_TRNSM_ReadFile_struct FileDataBuffer, FileDataBufferTmp;
    static S_X_BAND_TRNSM_DownloadFromXSBand_State_t S_X_BAND_TRNSM_DownloadFromXSBand_State;
    static FIL S_X_BAND_TRNSM_DownloadFromXSBand_file_descr;
    static uint32_t S_X_BAND_TRNSM_DownloadFromXSBand_PacketNumber, S_X_BAND_TRNSM_DownloadFromXSBand_FPos;

    // Check for available retries
    if (!stackEntry->retries) {
#if S_X_BAND_STACK_DEBUG
        fprintf (stackEntry->console, "X_BAND_TRNSM_DownloadFileFromSBandTrnsm() FINISHED ERR, time =  %d\r\n", (int)xTaskGetTickCount());
#endif
        if (stackEntry->state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
            retStat = S_X_BAND_TRNSM_PARAMS_COMM_ERR;
        }
        return retStat;
    }
    // Start the complex command sequence
    if (stackEntry->expectSlaveCommand) {
#if S_X_BAND_STACK_DEBUG
        fprintf (stackEntry->console, "X_BAND_TRNSM_DownloadFileFromSBandTrnsm() START time =  %d\r\n", (int)xTaskGetTickCount());
#endif
        memcpy (&XSBandStack_Slave, stackEntry, sizeof (S_X_BAND_CMD_StackEntry));
        memset (&FileDataBuffer, 0x00, sizeof (FileDataBuffer));
        XSBandStack_Slave.expectSlaveCommand = stackEntry->expectSlaveCommand = 0;
        XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_OPEN_F;
        XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
        XSBandStack_Slave.fileHandler = -1;
        S_X_BAND_TRNSM_DownloadFromXSBand_State = S_X_BAND_TRNSM_DownloadFromXSBand_State_OPENFILE;
        XSBandStack_Slave.parentEntry = stackEntry;
    }
    // Check the global state of the complex command
    switch (S_X_BAND_TRNSM_DownloadFromXSBand_State) {
        case S_X_BAND_TRNSM_DownloadFromXSBand_State_OPENFILE:
            retStat = S_X_BAND_TRNSM_OpenFile (&XSBandStack_Slave);
            if (!retStat && XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
                if (XSBandStack_Slave.fileHandler != -1 || XSBandStack_Slave.fileSize) {
                    FRESULT fr = f_open (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr, (char *)stackEntry->ESTTC_data, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
                    retStat = fr;
                    if (fr != FR_OK) {
                        // on disk error - try to reinitialise the SD card
                        if (fr == FR_DISK_ERR || fr == FR_LOCKED) {
                            InitCdCard();
#if S_X_BAND_STACK_DEBUG
                            fprintf (stackEntry->console, "X_BAND_TRNSM_DownloadFileFromSBandTrnsm() SD card. ERR = %d\r\n", fr);
#endif
                        }
                        // Close the file - regardless if the copy procedure is successful or not
                        f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                    } else {
                        // Go west
                        // XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_READ_F;
                        // XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
                        // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                        XSBandStack_Slave.retries = stackEntry->retries; // * 20;
                        XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                        S_X_BAND_TRNSM_DownloadFromXSBand_State = S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE;
                        S_X_BAND_TRNSM_DownloadFromXSBand_PacketNumber = S_X_BAND_TRNSM_DownloadFromXSBand_FPos = 0;
                    }
                } else {
                    retStat = 0xFA;
                }
            }
            break;
        case S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE:
            if (S_X_BAND_TRNSM_DownloadFromXSBand_FPos >= XSBandStack_Slave.fileSize) {
                /*
                FILINFO  file_info;
                FRESULT fr = f_stat ((char *)stackEntry->ESTTC_data, &file_info);
                retStat = fr;
                if (fr == FR_OK) {
                    if (X_BAND_TRNSM_DownloadFromXSBand_FPos == file_info.fsize) {
                        X_BAND_TRNSM_DownloadFromXSBand_State = X_BAND_TRNSM_DownloadFromXSBand_State_READFILE_GETRESULT;
                        XBandStack_Slave.state = X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                        break;
                    }
                }
                */
                if (S_X_BAND_TRNSM_DownloadFromXSBand_FPos == XSBandStack_Slave.fileSize) {
                    retStat = 0x00;
                } else {
                    retStat = 0x01;
                }
                f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                S_X_BAND_TRNSM_DownloadFromXSBand_State = S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE_GETRESULT;
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
            } else {
                // XBandStack_Slave.ESTTC_data_size = sizeof (XBandStack_Slave.fileHandler);
                // memcpy (&XBandStack_Slave.ESTTC_data, XBandStack_Slave.fileHandler, XBandStack_Slave.ESTTC_data_size);
                retStat = S_X_BAND_TRNSM_SendCMD (XSBandStack_Slave.devID, S_X_BAND_TRNSM_CMD_READ_F, S_X_BAND_TRNSM_NULL_TYPE, (uint8_t *)&XSBandStack_Slave.fileHandler, sizeof (XSBandStack_Slave.fileHandler));
                if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                    XSBandStack_Slave.minDelay = 1;
                    XSBandStack_Slave.timestamp = stackEntry->timestamp = xTaskGetTickCount();
                    XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_READ_F;
                    XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
                    // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                    XSBandStack_Slave.retries = stackEntry->retries; // * 200;
                    XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                    S_X_BAND_TRNSM_DownloadFromXSBand_State = S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE_GETRESULT;
                    retStat = 0x00;
                } else {
                    // Failed to send the command
                    if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                        // If there is no communication with the Device try to reconfigure the speed
                        if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                            // Communication at all baud rates has failed
                            stackEntry->retries = 0;
                            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                            return S_X_BAND_TRNSM_PARAMS_COMM_ERR;
                        }
                    }
                    S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, &XSBandStack_Slave);
                }
            }
            break;
        case S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE_GETRESULT:
            // Check whether the protocol delay has passed
            if (XSBandStack_Slave.minDelay > (xTaskGetTickCount() - XSBandStack_Slave.timestamp)) break;
            XSBandStack_Slave.timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (XSBandStack_Slave.devID, S_X_BAND_TRNSM_CMD_READ_F, S_X_BAND_TRNSM_NULL_TYPE, &FileDataBuffer.Status, &RxDataLenght);
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                retStat = S_X_BAND_TRNSM_SEND_CARD_ERR; //0xF1;
                // Verify the answer
                if (RxDataLenght > 0) {
                    if (FileDataBuffer.PacketNumber == S_X_BAND_TRNSM_DownloadFromXSBand_PacketNumber) {
                        // Get the status of the request
                        retStat = FileDataBuffer.Status;
                        // If the status is "OK"
                        if (!retStat) {
//fprintf (stackEntry->console, "f_lseek() START time =  %d (retry No %d)\r\n", (int)xTaskGetTickCount(), stackEntry->retries);
                            // Set the cursor to the end of the file
                            FRESULT fr = f_lseek (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr, S_X_BAND_TRNSM_DownloadFromXSBand_FPos);
                            if (fr == FR_OK) {
                                uint32_t NumberReadBytes;

                                // Read a piece of file in the OBC
                                fr = f_write (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr, &FileDataBuffer.Data, FileDataBuffer.Length, (UINT*)&NumberReadBytes);
//fprintf (stackEntry->console, "f_write() STOP time =  %d (retry No %d)\r\n", (int)xTaskGetTickCount(), stackEntry->retries);
                                if (fr != FR_OK || FileDataBuffer.Length != NumberReadBytes) {
                                    // Wrong answer
                                    // retStat = 0x01;
                                    f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                                    InitCdCard();
                                    fr = f_open (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr, (char *)stackEntry->ESTTC_data, FA_READ | FA_WRITE | FA_OPEN_EXISTING);
                                    if (fr != FR_OK) {
                                        retStat = S_X_BAND_TRNSM_SEND_CARD_ERR;
#if S_X_BAND_STACK_DEBUG
                                        fprintf (stackEntry->console, "\r\n READ_F CMD f_open() error = %d (retry No %d)", fr, stackEntry->retries);
#endif
                                    } else {
                                        retStat = 0x00;
                                    }
                                    stackEntry->retries--;
                                    // retStat = 0x01;
                                } else {
                                    S_X_BAND_TRNSM_DownloadFromXSBand_PacketNumber++;
                                    S_X_BAND_TRNSM_DownloadFromXSBand_FPos += FileDataBuffer.Length;
                                    // Update the state
                                    S_X_BAND_TRNSM_DownloadFromXSBand_State = S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE;
                                    XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                                    // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                                    XSBandStack_Slave.retries = stackEntry->retries;
                                    memcpy (&FileDataBufferTmp, &FileDataBuffer, sizeof (FileDataBuffer));

                                }
                            } else {
                                f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                                InitCdCard();
                                fr = f_open (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr, (char *)stackEntry->ESTTC_data, FA_READ | FA_WRITE | FA_OPEN_EXISTING);
                                if (fr != FR_OK) {
                                    retStat = S_X_BAND_TRNSM_SEND_CARD_ERR;
#if S_X_BAND_STACK_DEBUG
                                    fprintf (stackEntry->console, "\r\n READ_F CMD f_open() error = %d (retry No %d)", fr, stackEntry->retries);
#endif
                                } else {
                                    retStat = 0x00;
                                }
                                // stackEntry->retries--;
                                // retStat = 0xF2;
                                // Wrong answer
                                // retStat = 0x01;
                            }
                        } else {
                            f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                            retStat = 0xF3;
                            // Wrong answer
                            // retStat = 0x01;
                        }
                    } else {
                        f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                        retStat = 0xF4;
                        // Wrong answer
                        // retStat = 0x01;
                    }
                } else {
                    f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                    retStat = 0xF5;
                    // Wrong answer
                    // retStat = 0x01;
                }
            } else {
                if (S_X_BAND_STACK_RETR_COND(retStat)) retStat = 0x00;
                else {
                    f_close (&S_X_BAND_TRNSM_DownloadFromXSBand_file_descr);
                    retStat = 0xF6;
                }
                // Failed to send the command
                // retStat = 0x01;
            }
            break;
    }
    // Retry on error
    if (retStat || !XSBandStack_Slave.retries) {
#if S_X_BAND_STACK_DEBUG
        fprintf (stackEntry->console, "\r\n <DOWNLOAD> Error on state = %d", S_X_BAND_TRNSM_DownloadFromXSBand_State);
        fprintf (stackEntry->console, "\r\n <DOWNLOAD> Retrying on retStat = %d (retry No %d) (local retry %d)", retStat, stackEntry->retries - 1, XSBandStack_Slave.retries);
#endif
        stackEntry->retries--;
        memcpy (&XSBandStack_Slave, stackEntry, sizeof (S_X_BAND_CMD_StackEntry));
        // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
        XSBandStack_Slave.retries = stackEntry->retries;
        XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_OPEN_F;
        XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
        XSBandStack_Slave.fileHandler = -1;
        S_X_BAND_TRNSM_DownloadFromXSBand_State = S_X_BAND_TRNSM_DownloadFromXSBand_State_OPENFILE;
        XSBandStack_Slave.parentEntry = stackEntry;
        return 0x00;
    // Or gAt out of hell!
    } else if (XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
        if (S_X_BAND_TRNSM_DownloadFromXSBand_State == S_X_BAND_TRNSM_DownloadFromXSBand_State_READFILE_GETRESULT) {
#if S_X_BAND_STACK_DEBUG
            fprintf (stackEntry->console, "X_BAND_TRNSM_DownloadFileFromSBandTrnsm() FINISHED OK, time =  %d\r\n", (int)xTaskGetTickCount());
#endif
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
            return retStat;
        }
        // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
        XSBandStack_Slave.retries = stackEntry->retries;
    }

    return retStat;
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Copy a file from the SD card of the OBC to the SD card of the S-Band Transmitter
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            uint8_t:
*                      + X_BAND_TRNSM_STAT_WRONG_PARAM - file not found
*                      + X_BAND_TRNSM_DellStatus_enum  - delete Error
* @note              if such a file is already existing it will be overwritten
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_UploadFileToSBandTrnsm (S_X_BAND_CMD_StackEntry *stackEntry) {
    typedef enum S_X_BAND_TRNSM_UploadToXSBand_State_t {
        S_X_BAND_TRNSM_UploadToXSBand_State_OPEN_AND_DEL_FILE = 0x00,
        S_X_BAND_TRNSM_UploadToXSBand_State_CREATEFILE,
        S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE,
        S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE_GETRESULT
    } S_X_BAND_TRNSM_UploadToXSBand_State_t;

    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
    FRESULT fr;
    static FILINFO S_X_BAND_TRNSM_UploadToXSBand_file_info;
    static S_X_BAND_TRNSM_UploadToXSBand_State_t S_X_BAND_TRNSM_UploadToXSBand_State;
    static FIL S_X_BAND_TRNSM_UploadToXSBand_file_descr;
    static uint32_t S_X_BAND_TRNSM_UploadToXSBand_FPos, S_X_BAND_TRNSM_UploadToXSBand_ReadLen;
    static S_X_BAND_TRNSM_WriteFile_struct S_X_BAND_TRNSM_FileDataBuffer;

    // Check for available retries
    if (!stackEntry->retries) {
#if S_X_BAND_STACK_DEBUG
        fprintf (stackEntry->console, "X_BAND_TRNSM_DownloadFileFromSBandTrnsm() FINISHED ERR, time =  %d\r\n", (int)xTaskGetTickCount());
#endif
        if (stackEntry->state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
            retStat = S_X_BAND_TRNSM_PARAMS_COMM_ERR;
        }
        return retStat;
    }
    // Start the complex command sequence
    if (stackEntry->expectSlaveCommand) {
#if S_X_BAND_STACK_DEBUG
        fprintf (stackEntry->console, "X_BAND_TRNSM_UploadFileToSBandTrnsm() START time =  %d\r\n", (int)xTaskGetTickCount());
#endif
        memcpy (&XSBandStack_Slave, stackEntry, sizeof (S_X_BAND_CMD_StackEntry));
        XSBandStack_Slave.expectSlaveCommand = stackEntry->expectSlaveCommand = 0;
        XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_DELL_F;
        XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
        XSBandStack_Slave.fileHandler = -1;
        S_X_BAND_TRNSM_UploadToXSBand_State = S_X_BAND_TRNSM_UploadToXSBand_State_OPEN_AND_DEL_FILE;
        XSBandStack_Slave.parentEntry = stackEntry;
    }
    // Check the global state of the complex command
    switch (S_X_BAND_TRNSM_UploadToXSBand_State) {
        case S_X_BAND_TRNSM_UploadToXSBand_State_OPEN_AND_DEL_FILE:
            fr = f_open (&S_X_BAND_TRNSM_UploadToXSBand_file_descr, (char *)stackEntry->ESTTC_data, FA_READ | FA_OPEN_EXISTING);
            if (fr != FR_OK) {
                if (fr == FR_DISK_ERR || fr == FR_LOCKED) {
                    // on disk error - try to reinitialise the SD card
                    InitCdCard();
#if S_X_BAND_STACK_DEBUG
                    fprintf (stackEntry->console, "X_BAND_TRNSM_UploadFileToSBandTrnsm() Error f_open(). ERR = %d\r\n", fr);
#endif
                }
                retStat = 0x01;
                break;
            }
            fr = f_stat ((char *)stackEntry->ESTTC_data, &S_X_BAND_TRNSM_UploadToXSBand_file_info);
            if (fr != FR_OK) {
                f_close (&S_X_BAND_TRNSM_UploadToXSBand_file_descr);
                if (fr == FR_DISK_ERR || fr == FR_LOCKED) {
                    // on disk error - try to reinitialise the SD card
                    InitCdCard();
#if S_X_BAND_STACK_DEBUG
                    fprintf (stackEntry->console, "X_BAND_TRNSM_UploadFileToSBandTrnsm() Error SD card. ERR = %d\r\n", fr);
#endif
                }
                retStat = 0x02;
                break;
            }
            retStat = S_X_BAND_TRNSM_DelFile (&XSBandStack_Slave);
            if ((!retStat || retStat == S_X_BAND_TRNSM_DELL_NOT_FOUND) && XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
                retStat = 0x00;
                XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_CREATE_F;
                XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
                // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                XSBandStack_Slave.retries = stackEntry->retries; //12 * S_X_BAND_TRNSM_CMD_RETRY;
                // XSBandStack_Slave.minDelay = 1;
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                S_X_BAND_TRNSM_UploadToXSBand_State = S_X_BAND_TRNSM_UploadToXSBand_State_CREATEFILE;
            } else if (retStat) {
#if S_X_BAND_STACK_DEBUG
                fprintf (stackEntry->console, "\r\n S_X_BAND_TRNSM_UploadToXSBand_State_OPEN_AND_DEL_FILE ERROR = %d (retry No %d) (local retry %d)", retStat, stackEntry->retries - 1, XSBandStack_Slave.retries);
#endif
                f_close (&S_X_BAND_TRNSM_UploadToXSBand_file_descr);
            }
            break;
        case S_X_BAND_TRNSM_UploadToXSBand_State_CREATEFILE:
            XSBandStack_Slave.fileSize = S_X_BAND_TRNSM_UploadToXSBand_file_info.fsize;
            retStat = S_X_BAND_TRNSM_CreateFile (&XSBandStack_Slave);
            if (!retStat && XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
                // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                XSBandStack_Slave.retries = stackEntry->retries;
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                S_X_BAND_TRNSM_UploadToXSBand_State = S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE;
                memset (&S_X_BAND_TRNSM_FileDataBuffer, 0x00, sizeof (S_X_BAND_TRNSM_FileDataBuffer));
                S_X_BAND_TRNSM_FileDataBuffer.FileHandler = XSBandStack_Slave.fileHandler;
                S_X_BAND_TRNSM_UploadToXSBand_FPos = 0;
            } else if (retStat && XSBandStack_Slave.fileHandler == -1) {
#if S_X_BAND_STACK_DEBUG
                fprintf (stackEntry->console, "\r\n S_X_BAND_TRNSM_UploadToXSBand_State_CREATEFILE ERROR = %d (retry No %d) (local retry %d)", retStat, stackEntry->retries - 1, XSBandStack_Slave.retries);
#endif
                retStat = 0x03;
                f_close (&S_X_BAND_TRNSM_UploadToXSBand_file_descr);
            }
            break;
        case S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE:
            if (S_X_BAND_TRNSM_UploadToXSBand_FPos >= XSBandStack_Slave.fileSize) {
                if (S_X_BAND_TRNSM_UploadToXSBand_FPos == XSBandStack_Slave.fileSize) {
                    retStat = 0x00;
                } else {
                    retStat = 0x01;
                }
                f_close (&S_X_BAND_TRNSM_UploadToXSBand_file_descr);
                S_X_BAND_TRNSM_UploadToXSBand_State = S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE_GETRESULT;
                XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
            } else {
                uint32_t NumberReadBytes;

                // if the end of the file is almost reached check if the chunk is bigger the the rest of the file
                if ((S_X_BAND_TRNSM_UploadToXSBand_FPos + S_X_BAND_TRNSM_READ_FILE_CHUNCK_SIZE) <= XSBandStack_Slave.fileSize) {
                    /* there is more data in the file then the size of the chunk */
                    S_X_BAND_TRNSM_UploadToXSBand_ReadLen = S_X_BAND_TRNSM_READ_FILE_CHUNCK_SIZE;
                } else {
                    /* Read the last part of the file that have left */
                    S_X_BAND_TRNSM_UploadToXSBand_ReadLen = XSBandStack_Slave.fileSize - S_X_BAND_TRNSM_UploadToXSBand_FPos;
                }
                S_X_BAND_TRNSM_FileDataBuffer.Size = S_X_BAND_TRNSM_UploadToXSBand_ReadLen;
//fprintf (stackEntry->console, "f_lseek() START time =  %d (retry No %d)\r\n", (int)xTaskGetTickCount(), stackEntry->retries);
                // Set the cursor to the end of the file
                fr = f_lseek (&S_X_BAND_TRNSM_UploadToXSBand_file_descr, S_X_BAND_TRNSM_UploadToXSBand_FPos);
                if (fr == FR_OK) {
                    // Read a piece of the file in the OBC
                    fr = f_read (&S_X_BAND_TRNSM_UploadToXSBand_file_descr, &S_X_BAND_TRNSM_FileDataBuffer.Data, S_X_BAND_TRNSM_UploadToXSBand_ReadLen, (UINT*)&NumberReadBytes);
//fprintf (stackEntry->console, "f_read() STOP time =  %d (retry No %d)\r\n", (int)xTaskGetTickCount(), stackEntry->retries);
                    if (fr != FR_OK || S_X_BAND_TRNSM_UploadToXSBand_ReadLen != NumberReadBytes) {
#if S_X_BAND_STACK_DEBUG
                        fprintf (stackEntry->console, "\r\n WRITE_F CMD f_read() error = %d (retry No %d)", fr, stackEntry->retries);
#endif
                        // retStat = X_BAND_TRNSM_SEND_CARD_ERR;
                        InitCdCard();
                        fr = f_open (&S_X_BAND_TRNSM_UploadToXSBand_file_descr, (char *)stackEntry->ESTTC_data, FA_READ | FA_OPEN_EXISTING);
                        if (fr != FR_OK) {
                            retStat = S_X_BAND_TRNSM_SEND_CARD_ERR;
#if S_X_BAND_STACK_DEBUG
                            fprintf (stackEntry->console, "\r\n WRITE_F CMD f_open() error = %d (retry No %d)", fr, stackEntry->retries);
#endif
                        } else {
                            retStat = 0x00;
                        }
                        stackEntry->retries--;
                        break;
                    }
                } else {
#if S_X_BAND_STACK_DEBUG
                    fprintf (stackEntry->console, "\r\n WRITE_F CMD f_lseek() error = %d (retry No %d)", fr, stackEntry->retries);
#endif
                    // retStat = X_BAND_TRNSM_SEND_CARD_ERR;
                    InitCdCard();
                    fr = f_open (&S_X_BAND_TRNSM_UploadToXSBand_file_descr, (char *)stackEntry->ESTTC_data, FA_READ | FA_OPEN_EXISTING);
                    if (fr != FR_OK) {
                        retStat = S_X_BAND_TRNSM_SEND_CARD_ERR;
#if S_X_BAND_STACK_DEBUG
                        fprintf (stackEntry->console, "\r\n WRITE_F CMD f_open() error = %d (retry No %d)", fr, stackEntry->retries);
#endif
                    } else {
                        retStat = 0x00;
                    }
                    stackEntry->retries--;
                    break;
                }
                S_X_BAND_TRNSM_FileDataBuffer.Size = S_X_BAND_TRNSM_UploadToXSBand_ReadLen;
                retStat = S_X_BAND_TRNSM_SendCMD (XSBandStack_Slave.devID, S_X_BAND_TRNSM_CMD_WRITE_F, S_X_BAND_TRNSM_NULL_TYPE, (uint8_t *)&S_X_BAND_TRNSM_FileDataBuffer.Size, S_X_BAND_TRNSM_UploadToXSBand_ReadLen + 10);
                if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                    XSBandStack_Slave.minDelay = 1;
                    XSBandStack_Slave.timestamp = stackEntry->timestamp = xTaskGetTickCount();
                    XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_WRITE_F;
                    XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
                    // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                    XSBandStack_Slave.retries = stackEntry->retries;
                    XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                    S_X_BAND_TRNSM_UploadToXSBand_State = S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE_GETRESULT;
                    retStat = 0x00;
                } else {
#if S_X_BAND_STACK_DEBUG
                    if (retStat) {
                        fprintf (stackEntry->console, "\r\n WRITE_F CMD ERROR = %d (retry No %d)", retStat, stackEntry->retries);
                    }
#endif
                    // Failed to send the command
                    if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                        // If there is no communication with the Device try to reconfigure the speed
                        if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                            // Communication at all baud rates has failed
                            stackEntry->retries = 0;
                            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                            return S_X_BAND_TRNSM_PARAMS_COMM_ERR;
                        }
                    }
                    S_X_BAND_STACK_RETR_SUB (retStat != S_X_BAND_TRNSM_STAT_ACK && retStat != S_X_BAND_TRNSM_STAT_GET_RESULT, &XSBandStack_Slave);
                }
            }
            break;
        case S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE_GETRESULT:
            // Check whether the protocol delay has passed
            if (XSBandStack_Slave.minDelay > (xTaskGetTickCount() - XSBandStack_Slave.timestamp)) break;
            XSBandStack_Slave.timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (XSBandStack_Slave.devID, S_X_BAND_TRNSM_CMD_WRITE_F, S_X_BAND_TRNSM_NULL_TYPE, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
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
                        S_X_BAND_TRNSM_UploadToXSBand_FPos += S_X_BAND_TRNSM_UploadToXSBand_ReadLen;
                        // Update the state
                        S_X_BAND_TRNSM_UploadToXSBand_State = S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE;
                        XSBandStack_Slave.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
                        // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
                        XSBandStack_Slave.retries = stackEntry->retries;
                    } else {
                        retStat = 0xF3;
                        f_close (&S_X_BAND_TRNSM_UploadToXSBand_file_descr);
                        // Wrong answer
                        // retStat = 0x01;
                    }
                } else {
                    retStat = 0xF5;
                    f_close (&S_X_BAND_TRNSM_UploadToXSBand_file_descr);
                    // Wrong answer
                    // retStat = 0x01;
                }
            } else {
                if (S_X_BAND_STACK_RETR_COND(retStat)) retStat = 0x00;
                else {
#if S_X_BAND_STACK_DEBUG
                    if (retStat) {
                        fprintf (stackEntry->console, "\r\n WRITE_F GETRESULT LOCAL ERR = %d (retry No %d)", retStat, XSBandStack_Slave.retries);
                    }
#endif
                    retStat = 0xF6;
                    f_close (&S_X_BAND_TRNSM_UploadToXSBand_file_descr);
                }
                // Failed to send the command
                // retStat = 0x01;
            }
#if S_X_BAND_STACK_DEBUG
            if (retStat) {
                fprintf (stackEntry->console, "\r\n WRITE_F GETRESULT ERROR = %d (retry No %d)", retStat, stackEntry->retries);
            }
#endif
            break;
    }
    // Retry on error
    if (retStat || !XSBandStack_Slave.retries) {
#if S_X_BAND_STACK_DEBUG
        fprintf (stackEntry->console, "\r\n Error on state = %d", S_X_BAND_TRNSM_UploadToXSBand_State);
        fprintf (stackEntry->console, "\r\n Retrying on retStat = %d (retry No %d) (local retry %d)", retStat, stackEntry->retries - 1, XSBandStack_Slave.retries);
#endif
        retStat = 0x00;
        stackEntry->retries--;
        memcpy (&XSBandStack_Slave, stackEntry, sizeof (S_X_BAND_CMD_StackEntry));
        // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
        XSBandStack_Slave.retries = stackEntry->retries;
        XSBandStack_Slave.command = S_X_BAND_TRNSM_CMD_DELL_F;
        XSBandStack_Slave.type = S_X_BAND_TRNSM_NULL_TYPE;
        XSBandStack_Slave.fileHandler = -1;
        S_X_BAND_TRNSM_UploadToXSBand_State = S_X_BAND_TRNSM_UploadToXSBand_State_OPEN_AND_DEL_FILE;
        XSBandStack_Slave.parentEntry = stackEntry;
        return 0x00;
    // Or gAt out of hell!
    } else if (XSBandStack_Slave.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
        if (S_X_BAND_TRNSM_UploadToXSBand_State == S_X_BAND_TRNSM_UploadToXSBand_State_WRITEFILE_GETRESULT) {
#if S_X_BAND_STACK_DEBUG
            fprintf (stackEntry->console, "X_BAND_TRNSM_DownloadFileFromSBandTrnsm() FINISHED OK, time =  %d\r\n", (int)xTaskGetTickCount());
#endif
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
            return retStat;
        }
        // XSBandStack_Slave.retries = S_X_BAND_TRNSM_CMD_RETRY;
        XSBandStack_Slave.retries = stackEntry->retries;
    }

    return retStat;
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief A Task that serves commands through ESTTC protocol
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      argument - unused, a template is followed
* @param[output]     none
* @return            none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void S_X_BAND_Task (void const * argument) {
    uint8_t i, j;

    osDelay (1500);  // Wait the other tasks to be initialised

    for (;;) {

        // Process with the state machine
        for (i = j = 0; i < S_X_BAND_STACK_SIZE; ++i) {
            // Empty slot - continue
            if (XSBandStack[i].state == S_X_BAND_TRNSM_CMD_STATE_EMPTY) {
                ++j;
                continue;
            }
            // Processing state - process the data and induce a new state if needed
            if (XSBandCallbacks[XSBandStack[i].command]) {
                XSBandTryFinish (XSBandCallbacks[XSBandStack[i].command] (&XSBandStack[i]), &XSBandStack[i]);
            }
        }
        // Check if all the stack was empty
        if (j == S_X_BAND_STACK_SIZE) {
            osDelay (200);
        } else {
            // ~5 clocks for branch and nop on ~100Mhz => 20 ns per iteration => x50 = 1 us
            // TODO !!! replace this with proper timer !!!
            for (i = 0; i < 50; ++i) __NOP();
            // osDelay (2);
        }
    }
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Request a command from ESTTC gateway
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      commInterface - Pointer to the serial (USART) interface
* @param[input]      Identifier - ID of the used S or X band transmitter
* @param[input]      Cmd - Command number that is requested to be executed
*                     - 0x01    Symbol Rate
*                     - 0x02    Tx Power
*                     - 0x03    Central Frequency
*                     - 0x04    MODCOD
*                     - 0x05    Roll-Off
*                     - 0x06    Pilot Signal On/Off
*                     - 0x07    FEC Frame size
*                     - 0x08    Pretransmission Staffing Delay
*                     - 0x09    Change All Parameters
*                     - 0x0A    Simple Report
*                     - 0x0B    Full Report
*                     - 0x0C    Change Baudrate
*                     - 0x30    TxMode
*                     - 0x31    Load Mode
*                     - 0x32    Prepare to ShutDown
*                     - 0x35    FW update
*                     - 0x40    Dir
*                     - 0x50    Delete
*                     - 0x55    Delete all
*                     - 0x60    Upload a file
*                     - 0x61    Download a file
*                     - 0x70    Send a file
* @param[input]      Type - accepts only: 'R' - Read command, 'W' - Write commands
* @param[input]      size - size of the input data if used (if pData == NULL, this parameter is ignored)
* @param[input]      pData - pointer to the input data (if not needed set to NULL. This can be settings, file name etc.)
* @param[output]     none
* @return            none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void S_X_BAND_TRNSM_ESTTC_StartCmd (FILE *commInterface, uint16_t Identifier, uint8_t Cmd, uint8_t Type, uint8_t size, uint8_t * pData) {
    uint8_t i, j;

    // Check if any command is currently in progress
    // Should we add a new command to the stack ???
    // First, check whether this is attempted consecutive file commands execution
#if S_X_BAND_STACK_SAFETY
    j = (Cmd >= 0x35 && Cmd < 0x80) ? 1 : 0;
    for (i = 0; i < S_X_BAND_STACK_SIZE; ++i) {
        if (XSBandStack[i].state != S_X_BAND_TRNSM_CMD_STATE_EMPTY) {
            if (j && XSBandStack[i].isFileCommand) {
                fprintf (XSBandStack[i].console, "Error - Cannot execute several filesystem commands simultaneously \r\n");
#ifdef S_X_BAND_STACK_DEBUG
                for (i = 0; i < S_X_BAND_STACK_SIZE; ++i) {
                    fprintf (XSBandStack[i].console, "Stack lvl [%d] - STATUS: %d RETR: %d CMD: %d !!!\r\n", i, XSBandStack[i].state, XSBandStack[i].retries, XSBandStack[i].command);
                }
                if (XSBandStack_Slave.parentEntry != NULL) {
                    fprintf (XSBandStack[i].console, "Stack lvl [SLAVE] {PARENT: %d} - STATUS: %d RETR: %d CMD: %d !!!\r\n", XSBandStack_Slave.parentEntry->command, XSBandStack_Slave.state, XSBandStack_Slave.retries, XSBandStack_Slave.command);
                }
#endif
                return;
            } else if (XSBandStack[i].ESTTC_request == Cmd && XSBandStack[i].ESTTC_request_type == Type) {
                fprintf (XSBandStack[i].console, "Error - Cannot execute duplicate commands simultaneously \r\n");
#ifdef S_X_BAND_STACK_DEBUG
                for (i = 0; i < S_X_BAND_STACK_SIZE; ++i) {
                    fprintf (XSBandStack[i].console, "Stack lvl [%d] - STATUS: %d RETR: %d CMD: %d !!!\r\n", i, XSBandStack[i].state, XSBandStack[i].retries, XSBandStack[i].command);
                }
                if (XSBandStack_Slave.parentEntry != NULL) {
                    fprintf (XSBandStack[i].console, "Stack lvl [SLAVE] {PARENT: %d} - STATUS: %d RETR: %d CMD: %d !!!\r\n", XSBandStack_Slave.parentEntry->command, XSBandStack_Slave.state, XSBandStack_Slave.retries, XSBandStack_Slave.command);
                }
#endif
                return;
            }
        }
    }
#endif /* S_X_BAND_STACK_SAFETY */
    // Try to add
    for (i = 0; i < S_X_BAND_STACK_SIZE; ++i) {
        if (XSBandStack[i].state == S_X_BAND_TRNSM_CMD_STATE_EMPTY) {
            memset (&XSBandStack[i], 0x00, sizeof (S_X_BAND_CMD_StackEntry));
            XSBandStack[i].devID = Identifier;
            XSBandStack[i].ESTTC_request = Cmd;
            XSBandStack[i].isFileCommand = j;
            XSBandStack[i].ESTTC_request_type = Type;
            XSBandStack[i].ESTTC_data_size = size;
            if (XSBandStack[i].ESTTC_data_size > 0 && pData) {
                memcpy (XSBandStack[i].ESTTC_data, pData, XSBandStack[i].ESTTC_data_size);
            }
            XSBandStack[i].console = commInterface; // X_BAND_TRNSM_CMD_ESTTC_console;
            XSBandStack[i].retries = S_X_BAND_TRNSM_CMD_RETRY;
            XSBandStack[i].timestamp = 0; //xTaskGetTickCount(); // Not eligible for microseconds....
            XSBandStack[i].minDelay = S_X_BAND_STACK_DELAY;
            if (XSBandExtractCommandAndType (&XSBandStack[i])) {
                fprintf (XSBandStack[i].console, "Error - Unknown command %d \r\n", XSBandStack[i].command);
                memset (&XSBandStack[i], 0x00, sizeof (XSBandStack[i]));
                return;
            }
            XSBandStack[i].state = S_X_BAND_TRNSM_CMD_STATE_CMD;
            break;
        }
    }
    if (i == S_X_BAND_STACK_SIZE) {
        fprintf (COMM, "Error - Cannot add command to the stack !!!\r\n");
#ifdef S_X_BAND_STACK_DEBUG
        for (i = 0; i < S_X_BAND_STACK_SIZE; ++i) {
            fprintf (COMM, "Stack lvl [%d] - STATUS: %d RETR: %d CMD: %d !!!\r\n", i, XSBandStack[i].state, XSBandStack[i].retries, XSBandStack[i].command);
        }
        if (XSBandStack_Slave.parentEntry != NULL) {
            fprintf (COMM, "Stack lvl [SLAVE] {PARENT: %d} - STATUS: %d RETR: %d CMD: %d !!!\r\n", XSBandStack_Slave.parentEntry->command, XSBandStack_Slave.state, XSBandStack_Slave.retries, XSBandStack_Slave.command);
        }
#endif
    }
}

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* INTERNAL (STATIC) ROUTINES DEFINITION
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Fill up the stack according to the command
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      none
* @param[output]     stackEntry - current stack entry
* @return            uint8_t:
*                      + 0 - OK
*                      + 1 - cmd is not found
* @note              if such a file is already existing it will be overwritten
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t XSBandExtractCommandAndType (S_X_BAND_CMD_StackEntry *stackEntry) {
    if (stackEntry->ESTTC_request_type == 'R') {
        switch (stackEntry->ESTTC_request) {
            case 0x01:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_SYMBOL_RATE;
                break;
            case 0x02:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_TX_POWER;
                break;
            case 0x03:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_CENTRAL_FREQ;
                break;
            case 0x04:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_MODCOD;
                break;
            case 0x05:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_ROLL_OFF;
                break;
            case 0x06:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_PILOT_SIGNAL;
                break;
            case 0x07:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_FEC_FRAME_SIZE;
                break;
            case 0x08:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_PRETRASIT_DELAY;
                break;
            case 0x09:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_ALL_CHANGE_MODE;
                break;
            case 0x0A:
                stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_SIMPLE_REPORT;
                break;
            case 0x0B:
                if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                    stackEntry->command = S_X_BAND_TRNSM_CMD_GET;
                    stackEntry->type = X_BAND_TRNSM_GET_TYPE_FULL_REPORT;
                } else {
                    return 0x01;
                }
                break;
            case 0x40:
                stackEntry->command = S_X_BAND_TRNSM_CMD_DIR_F;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                stackEntry->expectSlaveCommand = 1;
                stackEntry->minDelay = 250;
                break;
            default:
                return 0x01;
        }
    } else if (stackEntry->ESTTC_request_type == 'W') {
        switch (stackEntry->ESTTC_request) {
            case 0x01:
                stackEntry->ESTTC_data[0] = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                stackEntry->ESTTC_data_size = sizeof (uint8_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_SYMBOL_RATE;
                break;
            case 0x02:
                stackEntry->ESTTC_data[0] = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                stackEntry->ESTTC_data_size = sizeof (uint8_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_TX_POWER;
                break;
            case 0x03:
            {
                float centralFreq = (stackEntry->ESTTC_data[0] * 1000) + (stackEntry->ESTTC_data[1] * 100) + (stackEntry->ESTTC_data[2] * 10) + stackEntry->ESTTC_data[3];
                float decPart = (stackEntry->ESTTC_data[5] * 100) + (stackEntry->ESTTC_data[6] * 10) + stackEntry->ESTTC_data[7];
                decPart /= 1000;
                centralFreq += decPart;
                memcpy (stackEntry->ESTTC_data, &centralFreq, sizeof (centralFreq));
                stackEntry->ESTTC_data_size = sizeof (centralFreq);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_CENTRAL_FREQ;
                break;
            }
            case 0x04:
                stackEntry->ESTTC_data[0] = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                stackEntry->ESTTC_data_size = sizeof (uint8_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_MODCOD;
                break;
            case 0x05:
                stackEntry->ESTTC_data[0] = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                stackEntry->ESTTC_data_size = sizeof (uint8_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_ROLL_OFF;
                break;
            case 0x06:
                stackEntry->ESTTC_data[0] = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                stackEntry->ESTTC_data_size = sizeof (uint8_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_PILOT_SIGNAL;
                break;
            case 0x07:
                stackEntry->ESTTC_data[0] = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                stackEntry->ESTTC_data_size = sizeof (uint8_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_FEC_FRAME_SIZE;
                break;
            case 0x08:
            {
                uint16_t preTransDelay = (stackEntry->ESTTC_data[0] * 1000) + (stackEntry->ESTTC_data[1] * 100) + (stackEntry->ESTTC_data[2] * 10) + stackEntry->ESTTC_data[3];
                memcpy (stackEntry->ESTTC_data, &preTransDelay, sizeof (preTransDelay));
                stackEntry->ESTTC_data_size = sizeof (uint16_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_PRETRASIT_DELAY;
                break;
            }
            case 0x09:
            {
                S_X_BAND_TRNSM_GET_AllParams_struct AllParamsData;
                AllParamsData.SymbolRate         = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                AllParamsData.TxPower            = (stackEntry->ESTTC_data[2] * 10) + stackEntry->ESTTC_data[3];
                AllParamsData.MODCOD             = (stackEntry->ESTTC_data[4] * 10) + stackEntry->ESTTC_data[5];
                AllParamsData.RollOff            = (stackEntry->ESTTC_data[6] * 10) + stackEntry->ESTTC_data[7];
                AllParamsData.PilotSygnal        = (stackEntry->ESTTC_data[8] * 10) + stackEntry->ESTTC_data[9];
                AllParamsData.FEC_Frame_Size     = (stackEntry->ESTTC_data[10] * 10) + stackEntry->ESTTC_data[11];
                AllParamsData.PreTxStaffingDelay = (stackEntry->ESTTC_data[12] * 1000) + (stackEntry->ESTTC_data[13] * 100) + (stackEntry->ESTTC_data[14] * 10) + (stackEntry->ESTTC_data[15] * 1);
                AllParamsData.CentralFreq        = (stackEntry->ESTTC_data[16] * 1000) + (stackEntry->ESTTC_data[17] * 100) + (stackEntry->ESTTC_data[18] * 10) + (stackEntry->ESTTC_data[19] * 1);
                float decimal_part               = (stackEntry->ESTTC_data[21] * 100) + (stackEntry->ESTTC_data[22] * 10) + (stackEntry->ESTTC_data[23] * 1);
                decimal_part = decimal_part / 1000;
                AllParamsData.CentralFreq += decimal_part;
                memcpy (stackEntry->ESTTC_data, &AllParamsData, sizeof (S_X_BAND_TRNSM_GET_AllParams_struct));
                stackEntry->ESTTC_data_size = sizeof (S_X_BAND_TRNSM_GET_AllParams_struct);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_GET_TYPE_ALL_CHANGE_MODE;
                break;
            }
            case 0x0C:
                stackEntry->ESTTC_data[0] = (stackEntry->ESTTC_data[0] * 10) + stackEntry->ESTTC_data[1];
                stackEntry->ESTTC_data_size = sizeof (uint8_t);
                stackEntry->command = S_X_BAND_TRNSM_CMD_SET;
                stackEntry->type = S_X_BAND_TRNSM_SET_TYPE_BAUDRATE;
                break;
            case 0x30:
                fprintf (stackEntry->console, "Please wait ... \r\n");
                stackEntry->command = S_X_BAND_TRNSM_CMD_TX_MODE;
                stackEntry->retries = 240;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                break;
            case 0x31:
                stackEntry->command = S_X_BAND_TRNSM_CMD_LOAD_MODE;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                break;
            case 0x32:
                stackEntry->command = S_X_BAND_TRNSM_CMD_SAFE_SHUTDOWN;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                stackEntry->minDelay = 650;
                break;
            case 0x35:
                fprintf (stackEntry->console, "Updating ... \r\n");
                stackEntry->command = S_X_BAND_TRNSM_CMD_UPDATE_FW;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                stackEntry->expectSlaveCommand = 1;
                stackEntry->minDelay = 3000;
                break;
            case 0x50:
                // Data implicitly set as string
                stackEntry->command = S_X_BAND_TRNSM_CMD_DELL_F;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                break;
            case 0x55:
                stackEntry->command = S_X_BAND_TRNSM_CMD_DELL_ALL_F;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                break;
            case 0x60:
                fprintf (stackEntry->console, "Please wait ... \r\n");
                // Data implicitly set as string
                stackEntry->retries = 60;
                stackEntry->command = S_X_BAND_TRNSM_CMD_WRITE_F;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                stackEntry->expectSlaveCommand = 1;
                break;
            case 0x61:
                fprintf (stackEntry->console, "Please wait ... \r\n");
                // Data implicitly set as string
                stackEntry->retries = 60;
                stackEntry->command = S_X_BAND_TRNSM_CMD_READ_F;
                stackEntry->type = S_X_BAND_TRNSM_NULL_TYPE;
                stackEntry->expectSlaveCommand = 1;
                break;
            case 0x70:
                fprintf (stackEntry->console, "Please wait ... \r\n");
                // Data implicitly set as string
                stackEntry->minDelay = 50;
                stackEntry->retries = 6000;
                stackEntry->command = S_X_BAND_TRNSM_CMD_SEND_F;
                stackEntry->type = S_X_BAND_TRNSM_SEND_F_TYPE_PREDIS_AND_RF;
                break;
            default:
                return 0x01;
        }
    }

    return 0;
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief A Task that serves commands through ESTTC protocol
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      retRes - execution status
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void XSBandTryFinish (uint8_t retRes, S_X_BAND_CMD_StackEntry *stackEntry) {
    // Exit condition - command hasn't been finished yet
    if (stackEntry->state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED && !retRes) {
        return;
    }
    // Reset the command stack state
    stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_EMPTY;
    if (XSBandStack_Slave.parentEntry == stackEntry) {
        memset (&XSBandStack_Slave, 0x00, sizeof (S_X_BAND_CMD_StackEntry));
    }
    // Process the stack command result
    if (stackEntry->ESTTC_request_type == 'R') {
        switch (stackEntry->ESTTC_request) {
            case 0x01: // Symbol rate
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "Symbol Rate = %d \r\n", *stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x02: // TX Power
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "Tx Power = %d \r\n", *stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x03:   // Central frequency
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "Central Frequency = %4.3f \r\n", *((float *)stackEntry->ESTTC_data));
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x04:   // Modcod
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "MODCOD = %d \r\n", *stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x05:   // Roll-Off
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "Roll-Off = %d \r\n", *stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x06:   // Pilot Signal On/Off
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "Pilot Signal = %d \r\n", *stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x07:   // FEC Frame size
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "FEC Frame size = %d \r\n", *stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x08:   // Pretransmission Staf Delay
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    fprintf (stackEntry->console, "Pretransmission Staf Delay = %d \r\n", *((uint16_t *)stackEntry->ESTTC_data));
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x09:   // All parameters
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    S_X_BAND_TRNSM_GET_AllParams_struct *AllParamsData = (S_X_BAND_TRNSM_GET_AllParams_struct *)stackEntry->ESTTC_data;
                    fprintf (stackEntry->console, "           Symbol Rate = %d \r\n"   , AllParamsData->SymbolRate);
                    fprintf (stackEntry->console, "              Tx Power = %d \r\n"   , AllParamsData->TxPower);
                    fprintf (stackEntry->console, "                MODCOD = %d \r\n"   , AllParamsData->MODCOD);
                    fprintf (stackEntry->console, "              Roll Off = %d \r\n"   , AllParamsData->RollOff);
                    fprintf (stackEntry->console, "          Pylot Sygnal = %d \r\n"   , AllParamsData->PilotSygnal);
                    fprintf (stackEntry->console, "   Pre. FEC Frame Size = %d \r\n"   , AllParamsData->FEC_Frame_Size);
                    fprintf (stackEntry->console, "Pre. Tx Staffing delay = %d \r\n"   , AllParamsData->PreTxStaffingDelay);
                    fprintf (stackEntry->console, "     Central Frequency = %4.3f \r\n", AllParamsData->CentralFreq);
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x0A:   // Simple Report
                if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                    if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                        X_BAND_TRNSM_GET_SimpleReport_struct *SimpleReportData = (X_BAND_TRNSM_GET_SimpleReport_struct *)stackEntry->ESTTC_data;
                        fprintf (stackEntry->console, "          System State = %d \r\n", SimpleReportData->SystemState);
                        fprintf (stackEntry->console, "           Status Flag = %d \r\n", SimpleReportData->StatusFlag);
                        fprintf (stackEntry->console, "        PA Temperature = %d \r\n", SimpleReportData->PA_Temp);
                        fprintf (stackEntry->console, "              Tx Power = %d \r\n", SimpleReportData->TxPower_m);
                    } else {
                        S_BAND_TRNSM_GET_SimpleReport_struct *SimpleReportData = (S_BAND_TRNSM_GET_SimpleReport_struct *)stackEntry->ESTTC_data;
                        fprintf(stackEntry->console, "          System State = %d \r\n",    SimpleReportData->SystemState);
                        fprintf(stackEntry->console, "           Status Flag = %d \r\n",    SimpleReportData->StatusFlag);
                        fprintf(stackEntry->console, "     CPT Temperature = %3.1f \r\n",   SimpleReportData->CPU_Temp);
                        fprintf(stackEntry->console, "           FW version = %d \r\n",     (int)SimpleReportData->FW_Version);
                    }
                } else {
                    fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                }
                break;
            case 0x0B:   // Full Report
                if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        X_BAND_TRNSM_GET_FullReport_struct *FullReportData = (X_BAND_TRNSM_GET_FullReport_struct *)stackEntry->ESTTC_data;
                        fprintf (stackEntry->console, "                  System State         = %d \r\n", FullReportData->SystemState);
                        fprintf (stackEntry->console, "                   Status Flag         = %d \r\n", FullReportData->StatusFlag);
                        fprintf (stackEntry->console, "                PA Temperature         = %d \r\n", FullReportData->PA_Temp);
                        fprintf (stackEntry->console, "                      Tx Power         = %d \r\n", FullReportData->TxPower_m);
                        fprintf (stackEntry->console, "          Predistortion Status         = %d \r\n", FullReportData->PredistortionStat);
                        fprintf (stackEntry->console, "           Predistortion Error         = %d \r\n", FullReportData->PredistortionErr);
                        fprintf (stackEntry->console, "         Predistortion Warning         = %d \r\n", FullReportData->PredistortionWarning);
                        fprintf (stackEntry->console, "        Predistortion Frequency Range  = %d \r\n", FullReportData->PredistortionFreqRange);
                        fprintf (stackEntry->console, "        Predistortion Adaptation Mode  = %d \r\n", FullReportData->PredistortionAdaptMode);
                        fprintf (stackEntry->console, "        Predistortion Adaptation State = %d \r\n", FullReportData->PredistortionAdaptState);
                        fprintf (stackEntry->console, "           Predistortion Output Status = %d \r\n", FullReportData->PredistortionOutputStat);
                        fprintf (stackEntry->console, "    Predistortion Normalization Factor = %d \r\n", FullReportData->PredistortionNomrFact);
                        fprintf (stackEntry->console, "                Predistortion RFIN_AGC = %d \r\n", FullReportData->PredistortionRFIN_AGC);
                        fprintf (stackEntry->console, "                Predistortion RFRB_AGC = %d \r\n", FullReportData->PredistortionRFRB_AGC);
                        fprintf (stackEntry->console, "Predistortion Unnormilized Coefficient = %d \r\n", FullReportData->PredistortionUnnormCoeff);
                        fprintf (stackEntry->console, "    Predistortion Internal Temperature = %d \r\n", FullReportData->PredistortionInternTemp);
                        fprintf (stackEntry->console, "  Predistortion Minimum Frequency Scan = %f \r\n", FullReportData->PredisMinFreqScan);
                        fprintf (stackEntry->console, "  Predistortion Maximum Frequency Scan = %f \r\n", FullReportData->PredisMaxFreqScan);
                        fprintf (stackEntry->console, "        Predistortion Signal Bandwidth = %f \r\n", FullReportData->PredisSignalBandwidth);
                        fprintf (stackEntry->console, "       Predistortion Central Frequency = %f \r\n", FullReportData->PredisCentralFreq);
                        fprintf (stackEntry->console, "                       CPU Temperature = %f \r\n", FullReportData->CPU_Temperature);
                        fprintf (stackEntry->console, "                            FW version = %d \r\n", (int)FullReportData->FW_Version);
                        // fprintf (stackEntry->console, "+");
                    } else {
                        fprintf (stackEntry->console, "Reading: Error %d \r\n", retRes);
                        // fprintf (stackEntry->console, "-");
                    }
                } else {
                    fprintf (stackEntry->console, "Error: Not ALLOWED \r\n");
                }
                break;
            case 0x40:   // Get a list of all files
                if (S_X_BAND_TRNSM_DIR_OK != retRes) {
                    fprintf (stackEntry->console, " Err %d \r\n", retRes);
                }
                break;
            default:
                fprintf (stackEntry->console, "Err Unknown CMD 0x%X\r\n", stackEntry->ESTTC_request);
                break;
        }
    } else if (stackEntry->ESTTC_request_type == 'W') {
        switch (stackEntry->ESTTC_request) {
            case 0x01: // Symbol rate
                if (stackEntry->ESTTC_data_size == sizeof (uint8_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "Symbol Rate = %d \r\n", stackEntry->ESTTC_data[0]);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x02: // TX Power
                if (stackEntry->ESTTC_data_size == sizeof (uint8_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "Tx Power = %d \r\n", stackEntry->ESTTC_data[0]);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x03:   // Central frequency
                if (stackEntry->ESTTC_data_size == sizeof (float)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "Central Frequency = %4.3f \r\n", *((float *)stackEntry->ESTTC_data));
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x04:   // Modcod
                if (stackEntry->ESTTC_data_size == sizeof (uint8_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "MODCOD = %d \r\n", stackEntry->ESTTC_data[0]);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x05:   // Roll-Off
                if (stackEntry->ESTTC_data_size == sizeof (uint8_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "Roll-Off = %d \r\n", stackEntry->ESTTC_data[0]);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x06:   // Pilot Signal On/Off
                if (stackEntry->ESTTC_data_size == sizeof (uint8_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "Pilot Signal = %d \r\n", stackEntry->ESTTC_data[0]);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x07:   // FEC Frame size
                if (stackEntry->ESTTC_data_size == sizeof (uint8_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "FEC Frame size = %d \r\n", stackEntry->ESTTC_data[0]);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x08:   // Pretransmission Staf Delay
                if (stackEntry->ESTTC_data_size == sizeof (uint16_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "Pretransmission Staf Delay = %d \r\n", *((uint16_t *)stackEntry->ESTTC_data));
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x09:   // All parameters
                if (stackEntry->ESTTC_data_size == sizeof (S_X_BAND_TRNSM_GET_AllParams_struct)) {
                    S_X_BAND_TRNSM_GET_AllParams_struct AllParamsData;
                    memcpy (&AllParamsData, stackEntry->ESTTC_data, sizeof (S_X_BAND_TRNSM_GET_AllParams_struct));
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "           Symbol Rate = %d \r\n"   , AllParamsData.SymbolRate);
                        fprintf (stackEntry->console, "              Tx Power = %d \r\n"   , AllParamsData.TxPower);
                        fprintf (stackEntry->console, "                MODCOD = %d \r\n"   , AllParamsData.MODCOD);
                        fprintf (stackEntry->console, "              Roll Off = %d \r\n"   , AllParamsData.RollOff);
                        fprintf (stackEntry->console, "          Pylot Sygnal = %d \r\n"   , AllParamsData.PilotSygnal);
                        fprintf (stackEntry->console, "   Pre. FEC Frame Size = %d \r\n"   , AllParamsData.FEC_Frame_Size);
                        fprintf (stackEntry->console, "Pre. Tx Staffing delay = %d \r\n"   , AllParamsData.PreTxStaffingDelay);
                        fprintf (stackEntry->console, "     Central Frequency = %4.3f \r\n", AllParamsData.CentralFreq);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x0C:   // Change Baudrate
                if (stackEntry->ESTTC_data_size == sizeof (uint8_t)) {
                    if (S_X_BAND_TRNSM_PARAMS_OK == retRes) {
                        fprintf (stackEntry->console, "Baud rate = %d \r\n", stackEntry->ESTTC_data[0]);
                    } else {
                        fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                    }
                } else {
                    fprintf (stackEntry->console, "Err - Size %d\r\n", stackEntry->ESTTC_data_size);
                }
                break;
            case 0x30:  // Start transmit mode of the S-band transmitter
                if (S_X_BAND_TRNSM_CHANG_MODE_OK == retRes) {
                    fprintf (stackEntry->console, "Tx mode started \r\n");
                } else {
                    fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                }
                break;
            case 0x31:  // Start Low power mode
                if (S_X_BAND_TRNSM_CHANG_MODE_OK == retRes) {
                    fprintf (stackEntry->console, "Load mode started \r\n");
                } else {
                    fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                }
                break;
            case 0x32:  // Start IDLE mode of the S-band
                if (S_X_BAND_TRNSM_CHANG_MODE_OK == retRes) {
                    fprintf (stackEntry->console, "Ready to Shut Down \r\n");
                } else {
                    fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                }
                break;
            case 0x35:  // Start a firmware update
                if (S_X_BAND_TRNSM_CHANG_MODE_OK == retRes) {
                    fprintf (stackEntry->console, "FW Updated using  \"%s\" \r\n", (char *)stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                }
                break;
            case 0x50:  // Delete one files
                if (S_X_BAND_TRNSM_DELL_OK == retRes) {
                    fprintf (stackEntry->console, "File \"%s\" deleted \r\n", (char *)stackEntry->ESTTC_data);
                } else {
                    fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                }
                break;
            case 0x55:  // Delete all files
                if (S_X_BAND_TRNSM_DELL_OK == retRes) {
                    fprintf (stackEntry->console, "All files deleted \r\n");
                } else {
                    fprintf (stackEntry->console, "Set Parameter: Error %d \r\n", retRes);
                }
                break;
            case 0x60:  // Copy a file from the OBC to the X-Band
                if (!retRes) {
                    fprintf (stackEntry->console, "File \"%s\" Uploaded\r\n", (char *)stackEntry->ESTTC_data);
                } else if (S_X_BAND_TRNSM_STAT_WRONG_PARAM == retRes) {
                    fprintf (stackEntry->console, "Copy Err - Parameter/File name \r\n");
                } else {
                    fprintf (stackEntry->console, "Copy Err %d \r\n", retRes);
                }
                break;
            case 0x61:  // Copy a file from X-Band to the OB
                if (!retRes) {
                    fprintf (stackEntry->console, "File \"%s\" Downloaded\r\n", (char *)stackEntry->ESTTC_data);
                } else if (S_X_BAND_TRNSM_STAT_WRONG_PARAM == retRes) {
                    fprintf (stackEntry->console, "Copy Err - Parameter/File name \r\n");
                } else {
                    fprintf (stackEntry->console, "Copy Err %d \r\n", retRes);
                }
                break;
            case 0x70:  // Start transmitting a file
                if (!retRes) {
                    fprintf (stackEntry->console, "File \"%s\" Sent\r\n", (char *)stackEntry->ESTTC_data);
                } else if (S_X_BAND_TRNSM_STAT_WRONG_PARAM == retRes) {
                    fprintf (stackEntry->console, "Sending Err - Parameter/File name \r\n");
                } else {
                    fprintf (stackEntry->console, "Sending Err %d \r\n", retRes);
                }
                break;
            default:
                fprintf (stackEntry->console, "Err Unknown CMD 0x%X\r\n", stackEntry->ESTTC_request);
                break;
        }
    } else {
        fprintf (stackEntry->console, "Unknown request type '%c' \r\n", stackEntry->ESTTC_request_type);
    }
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Change the mode of the RS485
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      state:
*                       1 - Transmit
*                       0 - Receive
* @param[output]     none
* @return            none
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void S_X_BAND_TRNSM_Rx_Tx_State(uint8_t state)
{
    if( state == 0 )
    {
        /* Receive mode */
        HAL_GPIO_WritePin(RS485_TX_GPIO_Port, RS485_TXPin, GPIO_PIN_RESET);
    }else{
        /* Transmit mode */
        HAL_GPIO_WritePin(RS485_TX_GPIO_Port, RS485_TXPin, GPIO_PIN_SET);
    }
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
static void S_X_BAND_TRNSM_Send_Ack(uint16_t Identifier, S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type) {
    uint32_t TxPackCRC = 0;
    S_X_BAND_TRNSM_Pack_struct * TxPackStruct = (S_X_BAND_TRNSM_Pack_struct *)TxDataBuffer;

    TxPackStruct->Header = S_X_BAND_TRNSM_HEADER;
    TxPackStruct->ModuleID = Identifier;
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

    S_X_BAND_TRNSM_Rx_Tx_State(1);

    HAL_StatusTypeDef UsartStat;
    //          -------------------------------------------------------     send number of bytes divisible to 16     ||     ||
    //          -------------------------------------------------------                                              \/     \/
    UsartStat = HAL_UART_Transmit((UART_HandleTypeDef *)COM_SBAND, TxDataBuffer, (((S_X_BAND_TRNSM_STRCT_SIZE + sizeof(TxPackCRC))>>4)+1)<<4, MCU_WWDG_MAX_CYCLING_TIME);
    if ( UsartStat == HAL_TIMEOUT )
    {
       HAL_UART_ErrorCallback((UART_HandleTypeDef *)COMM);
    }

    S_X_BAND_TRNSM_Rx_Tx_State (0);
}

/*!
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @brief Writes all settings of the S-Band transmitter (without the baud-rate)
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_SetGetParams_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_CMD_SET_Param (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;

    // Check for baudrate
    if (stackEntry->type == S_X_BAND_TRNSM_SET_TYPE_BAUDRATE) {
        return S_X_BAND_TRNSM_CMD_SET_Param_Baudrate (stackEntry);
    }
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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, stackEntry->ESTTC_data, stackEntry->ESTTC_data_size);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Verify the answer
                if (sizeof (retStat) == RxDataLenght) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_PARAMS_OK || retStat == S_X_BAND_TRNSM_PARAMS_PARAMS_ERR) {
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
#if S_X_BAND_STACK_DELAY
                else fprintf (COMM, "X_BAND_TRNSM_CMD_SET_Param GETRESULT error %d\r\n", (int)retStat);
#endif
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
* @brief Reads one settings of the S-Band transmitter (without the baud-rate)
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      stackEntry - current stack entry
* @param[output]     none
* @return            retStat - a value from enumeration X_BAND_TRNSM_SetGetParams_enum
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_CMD_GET_Param (S_X_BAND_CMD_StackEntry *stackEntry) {
    uint8_t retStat = 0x00;
    uint16_t RxDataLenght;
    uint32_t expectedSize = 0;

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
            retStat = S_X_BAND_TRNSM_SendCMD (stackEntry->devID, stackEntry->command, stackEntry->type, 0, 0);
            if (S_X_BAND_TRNSM_STAT_ACK == retStat) {
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT;
                stackEntry->timestamp = xTaskGetTickCount();
            } else {
                // Failed to send the command
                stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_RES;
                if (S_X_BAND_TRNSM_STAT_COMM_ERR == retStat) {
                    // If there is no communication with the Device try to reconfigure the speed
                    if (pdFALSE == S_X_BAND_TRNSM_AutoSearchBaudrate (stackEntry->devID)) {
                        // Communication at all baud rates has failed
                        stackEntry->retries = 0;
                        break;
                    }
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
            if (stackEntry->minDelay > (xTaskGetTickCount() - stackEntry->timestamp)) break;
            stackEntry->timestamp = xTaskGetTickCount();
            // Send GetResult command
            retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
            // Update the state
            stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
            if (S_X_BAND_TRNSM_STAT_GET_RESULT == retStat) {
                // Identify the expected size
                switch (stackEntry->type) {
                    case S_X_BAND_TRNSM_GET_TYPE_SIMPLE_REPORT:
                        if (stackEntry->devID >= X_BAND_ID_RANGE_MIN && stackEntry->devID <= X_BAND_ID_RANGE_MAX) {
                            expectedSize = sizeof (X_BAND_TRNSM_GET_SimpleReport_struct);
                        } else {
                            expectedSize = sizeof (S_BAND_TRNSM_GET_SimpleReport_struct);
                        }
                        break;
                    case X_BAND_TRNSM_GET_TYPE_FULL_REPORT:
                        expectedSize = sizeof (X_BAND_TRNSM_GET_FullReport_struct);
                        break;

                    default:
                        expectedSize = sizeof (S_X_BAND_TRNSM_GET_AllParams_struct);
                        break;
                }
                // Verify the answer
                if (++expectedSize >= RxDataLenght && RxDataLenght > 0) {
                    // Get the status of the request
                    retStat = S_X_BAND_TRNSM_Result_Rx_Buffer[0];
                    // If the status is "OK"
                    if (retStat == S_X_BAND_TRNSM_PARAMS_OK) {
                        // Update the state
                        stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED;
                        // Copy the data
                        stackEntry->ESTTC_data_size = RxDataLenght - 1;
                        memcpy (stackEntry->ESTTC_data, &S_X_BAND_TRNSM_Result_Rx_Buffer[1], stackEntry->ESTTC_data_size);
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
#if S_X_BAND_STACK_DELAY
                else fprintf (COMM, "X_BAND_TRNSM_CMD_GET_Param GETRESULT error %d\r\n", (int)retStat);
#endif
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
* @brief Check for successful communication at all available baudrates
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* @param[input]      Identifier - ID of the used S or X band transmitter
* @param[output]     none
* @return            pdTRUE - done, pdFALSE - failed to establish communication
* @note              none
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static uint8_t S_X_BAND_TRNSM_AutoSearchBaudrate (uint16_t Identifier) {
    uint8_t retStat;
    uint8_t Baudrate;
    uint32_t  Current_Baudrate_Temp = S_X_BAND_TRNSM_S_Current_Baudrate;
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOF_CLK_ENABLE();

    HAL_UART_DMAStop((UART_HandleTypeDef*)COM_SBAND);
    HAL_UART_DeInit((UART_HandleTypeDef*)COM_SBAND);

    //Reinitialise the periphery
    MX_DMA_USART7_Init();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(RS485_TX_GPIO_Port, RS485_TXPin, GPIO_PIN_RESET);

    /*Configure GPIO pin : RS485 Transmit enable pin */
    GPIO_InitStruct.Pin = RS485_TXPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RS485_TX_GPIO_Port, &GPIO_InitStruct);

    for( Baudrate = 0; Baudrate < (sizeof(BaudRateArra)/sizeof(BaudRateArra[0])); Baudrate ++ )
    {
        S_X_BAND_TRNSM_S_Current_Baudrate = Baudrate;

        MX_UART7_Init(BaudRateArra[Baudrate]*10000);

        retStat = S_X_BAND_TRNSM_SendCMD(Identifier, S_X_BAND_TRNSM_CMD_GET, S_X_BAND_TRNSM_GET_TYPE_SYMBOL_RATE, 0, 0);
        if (S_X_BAND_TRNSM_STAT_ACK == retStat)
        {
            uint16_t RxDataLenght;
            S_X_BAND_TRNSM_GetResult(Identifier, S_X_BAND_TRNSM_CMD_GET, S_X_BAND_TRNSM_GET_TYPE_SYMBOL_RATE, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);    //Pool the result of the command to clean it up from the X-band Transmitter
            retStat = pdTRUE;
            break;
        }else{
            /* restore the baudrate if all other baudrates are not successful */
            S_X_BAND_TRNSM_S_Current_Baudrate = Current_Baudrate_Temp;
            retStat = pdFALSE;
            osDelay(5);
        }
    }

    if(retStat == pdFALSE)
    {
        S_X_BAND_TRNSM_S_Current_Baudrate = S_X_BAND_TRNSM_DEFAULT_BAUDRATE;
        //VMI_TODO - DTC error -> need to rest the S/X-band (exit from transmit operation mode)
    }

    return retStat;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
