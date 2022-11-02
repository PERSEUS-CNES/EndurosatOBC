#include <stdio.h>
#include <assert.h>
#include "ftd2xx.h"
#include <string.h>
#include <unistd.h>
#include "es_crc32.h"
#include <stdint.h>
#include <stdlib.h>

#define S_X_BAND_TRNSM_STRCT_SIZE         (14)     /* size of one packet without the data and the CRC */

#define S_X_BAND_TRNSM_TX_BUFF_SIZE       (1536)
#define S_X_BAND_TRNSM_RX_BUFF_SIZE       (1536)

#define S_X_BAND_TRNSM_HEADER (0x50555345)        /* Header of every packet */
#define S_BAND_TRNSM_DEFAULT_DEV_ID (0x0720)            /* ID of the device that the OBC is communicating with */
#define S_X_BAND_TRNSM_CMD_RETRY              (6)                 /* Number of times to reply a whole request/Command */
#define S_X_BAND_STACK_DELAY                  (2)                 /* X-Band stack delay between protocol states (in ms) */

typedef struct{
    uint32_t Header;
    uint16_t ModuleID;
    uint16_t Length;
    uint16_t Response;
    uint16_t CMD;
    uint16_t Type;
}__attribute__((__packed__)) S_X_BAND_TRNSM_Pack_struct;


typedef enum {
    S_X_BAND_TRNSM_CMD_GET,               /* Get status */
    S_X_BAND_TRNSM_CMD_SET,               /* Set status */
    S_X_BAND_TRNSM_CMD_DIR_F,             /* Dir list of files in the SD Card */
    S_X_BAND_TRNSM_CMD_DIR_NEXT_F,        /* Get next page from the list of file after cmd DIR */
    S_X_BAND_TRNSM_CMD_DELL_F,            /* Delete a file */
    S_X_BAND_TRNSM_CMD_DELL_ALL_F,        /* Delete all files */
    S_X_BAND_TRNSM_CMD_CREATE_F,          /* Create a file */
    S_X_BAND_TRNSM_CMD_WRITE_F,           /* Write data to a file */
    S_X_BAND_TRNSM_CMD_OPEN_F,            /* Open a file */
    S_X_BAND_TRNSM_CMD_READ_F,            /* Read data from a file */
    S_X_BAND_TRNSM_CMD_SEND_F,            /* Send a file by the S-Band RF transmitter */
    S_X_BAND_TRNSM_CMD_TX_MODE,           /* Enter Transmit mode (High power consumption) */
    S_X_BAND_TRNSM_CMD_LOAD_MODE,         /* Exit Transmit mode (Low power consumption) */
    S_X_BAND_TRNSM_CMD_UPDATE_FW,         /* Update the firmware from a file */
    S_X_BAND_TRNSM_CMD_SAFE_SHUTDOWN,     /* Terminate all pending operations and prepare for shut down (Shut down must be done 1 second after that command) */
    S_X_BAND_TRNSM_CMD_GET_RES,           /* Pool the result from the last command */
    S_X_BAND_TRNSM_CMD_NUMBER             /* Get the number of all command */
} S_X_BAND_TRNSM_CMD_enum;

/* Values of all commands */
typedef enum {
    S_X_BAND_TRNSM_CMD_VAL_GET           = 0x0100,
    S_X_BAND_TRNSM_CMD_VAL_SET           = 0x0101,
    S_X_BAND_TRNSM_CMD_VAL_DIR_F         = 0x0102,
    S_X_BAND_TRNSM_CMD_VAL_DIR_NEXT_F    = 0x0103,
    S_X_BAND_TRNSM_CMD_VAL_DELL_F        = 0x0104,
    S_X_BAND_TRNSM_CMD_VAL_DELL_ALL_F    = 0x0105,
    S_X_BAND_TRNSM_CMD_VAL_CREATE_F      = 0x0106,
    S_X_BAND_TRNSM_CMD_VAL_WRITE_F       = 0x0107,
    S_X_BAND_TRNSM_CMD_VAL_OPEN_F        = 0x0108,
    S_X_BAND_TRNSM_CMD_VAL_READ_F        = 0x0109,
    S_X_BAND_TRNSM_CMD_VAL_SEND_F        = 0x010A,
    S_X_BAND_TRNSM_CMD_VAL_TX_MODE       = 0x0110,
    S_X_BAND_TRNSM_CMD_VAL_LOAD_MODE     = 0x0111,
    S_X_BAND_TRNSM_CMD_VAL_UPDATE_FW     = 0x0112,
    S_X_BAND_TRNSM_CMD_VAL_SAFE_SHUTDOWN = 0x0113,
    S_X_BAND_TRNSM_CMD_VAL_GET_RESULT    = 0x0114,
    S_X_BAND_TRNSM_CMD_VAL_NUMBER
} S_X_BAND_TRNSM_CMD_VAL_enum;


/* Available types for CMD "Get Result" ( 0x0101 ) */
typedef enum {
    S_X_BAND_TRNSM_RET_RES_GET           = S_X_BAND_TRNSM_CMD_VAL_GET        ,
    S_X_BAND_TRNSM_RET_RES_SET           = S_X_BAND_TRNSM_CMD_VAL_SET        ,
    S_X_BAND_TRNSM_RET_RES_DIR_F         = S_X_BAND_TRNSM_CMD_VAL_DIR_F      ,
    S_X_BAND_TRNSM_RET_RES_DIR_NEXT_F    = S_X_BAND_TRNSM_CMD_VAL_DIR_NEXT_F ,
    S_X_BAND_TRNSM_RET_RES_DELL_F        = S_X_BAND_TRNSM_CMD_VAL_DELL_F     ,
    S_X_BAND_TRNSM_RET_RES_DELL_ALL_F    = S_X_BAND_TRNSM_CMD_VAL_DELL_ALL_F ,
    S_X_BAND_TRNSM_RET_RES_CREATE_F      = S_X_BAND_TRNSM_CMD_VAL_CREATE_F   ,
    S_X_BAND_TRNSM_RET_RES_WRITE_F       = S_X_BAND_TRNSM_CMD_VAL_WRITE_F    ,
    S_X_BAND_TRNSM_RET_RES_OPEN_F        = S_X_BAND_TRNSM_CMD_VAL_OPEN_F     ,
    S_X_BAND_TRNSM_RET_RES_READ_F        = S_X_BAND_TRNSM_CMD_VAL_READ_F     ,
    S_X_BAND_TRNSM_RET_RES_SEND_F        = S_X_BAND_TRNSM_CMD_VAL_SEND_F     ,
    S_X_BAND_TRNSM_RET_RES_TX_MODE       = S_X_BAND_TRNSM_CMD_VAL_TX_MODE    ,
    S_X_BAND_TRNSM_RET_RES_LOAD_MODE     = S_X_BAND_TRNSM_CMD_VAL_LOAD_MODE  ,
    S_X_BAND_TRNSM_RET_RES_UPDATE_FW     = S_X_BAND_TRNSM_CMD_VAL_UPDATE_FW  ,
    S_X_BAND_TRNSM_RET_RES_NUMBER
} S_X_BAND_TRNSM_RetRes_enum;

/* Types for all commands that don't have type */
#define S_X_BAND_TRNSM_NULL_TYPE          ((uint16_t)(0x0000))

/* Available types for CMD "GET" ( 0x0100 ) */
typedef enum {
    S_X_BAND_TRNSM_GET_TYPE_SYMBOL_RATE     = 0x0040,
    S_X_BAND_TRNSM_GET_TYPE_TX_POWER        = 0x0041,
    S_X_BAND_TRNSM_GET_TYPE_CENTRAL_FREQ    = 0x0042,
    S_X_BAND_TRNSM_GET_TYPE_MODCOD          = 0x0043,
    S_X_BAND_TRNSM_GET_TYPE_ROLL_OFF        = 0x0044,
    S_X_BAND_TRNSM_GET_TYPE_PILOT_SIGNAL    = 0x0045,
    S_X_BAND_TRNSM_GET_TYPE_FEC_FRAME_SIZE  = 0x0046,
    S_X_BAND_TRNSM_GET_TYPE_PRETRASIT_DELAY = 0x0047,
    S_X_BAND_TRNSM_GET_TYPE_ALL_CHANGE_MODE = 0x0048,
    S_X_BAND_TRNSM_GET_TYPE_SIMPLE_REPORT   = 0x0049,
    X_BAND_TRNSM_GET_TYPE_FULL_REPORT       = 0x004A, //X-Band only
    S_X_BAND_TRNSM_GET_TYPE_ATTENUATION_PAR = 0x004C,
    S_X_BAND_TRNSM_GET_TYPE_NUMBER
} X_BAND_TRNSM_GET_types_enum;

/* Available types for CMD "SET" ( 0x0101 ) */
typedef enum {
    S_X_BAND_TRNSM_SET_TYPE_SYMBOL_RATE     = 0x0040,
    S_X_BAND_TRNSM_SET_TYPE_TX_POWER        = 0x0041,
    S_X_BAND_TRNSM_SET_TYPE_CENTRAL_FREQ    = 0x0042,
    S_X_BAND_TRNSM_SET_TYPE_MODCOD          = 0x0043,
    S_X_BAND_TRNSM_SET_TYPE_ROLL_OFF        = 0x0044,
    S_X_BAND_TRNSM_SET_TYPE_PILOT_SIGNAL    = 0x0045,
    S_X_BAND_TRNSM_SET_TYPE_FEC_FRAME_SIZE  = 0x0046,
    S_X_BAND_TRNSM_SET_TYPE_PRETRASIT_DELAY = 0x0047,
    S_X_BAND_TRNSM_SET_TYPE_ALL_CHANGE_MODE = 0x0048,
    S_X_BAND_TRNSM_SET_TYPE_BAUDRATE        = 0x004B,
    S_X_BAND_TRNSM_SET_TYPE_NUMBER
} S_X_BAND_TRNSM_SET_types_enum;

/* Available types for CMD "Send File" ( 0x010A ) */
typedef enum {
    S_X_BAND_TRNSM_SEND_F_TYPE_WITHOUT_ISSUES = 0x0050,
    S_X_BAND_TRNSM_SEND_F_TYPE_WITH_PREDST    = 0x0051,
    S_X_BAND_TRNSM_SEND_F_TYPE_WITH_RF_TRACT  = 0x0052,
    S_X_BAND_TRNSM_SEND_F_TYPE_PREDIS_AND_RF  = 0x0053,   /* Recommended */
    S_X_BAND_TRNSM_SEND_F_TYPE_NUMBER
} S_X_BAND_TRNSM_SendFile_types_enum;

/* Return statuses from sending command to S/X-Band transmitter */
typedef enum {
    S_X_BAND_TRNSM_STAT_GET_RESULT  = 0x00,
    S_X_BAND_TRNSM_STAT_ACK         = 0x05,
    S_X_BAND_TRNSM_STAT_NACK        = 0x06,
    S_X_BAND_TRNSM_STAT_BUSY        = 0x07,
    S_X_BAND_TRNSM_STAT_NCE         = 0x08,
    S_X_BAND_TRNSM_STAT_STACK_FULL  = 0x09,
    S_X_BAND_TRNSM_STAT_CTNA        = 0x0A,
    S_X_BAND_TRNSM_STAT_WRONG_PARAM = 0xFE,
    S_X_BAND_TRNSM_STAT_COMM_ERR    = 0xFF,
    S_X_BAND_TRNSM_STAT_NUMBER
} S_X_BAND_TRNSM_Response_enum;


/* Return statuses from CMD Delete and Delete All */
typedef enum {
    S_X_BAND_TRNSM_DELL_OK         = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_DELL_NOT_FOUND  = 0x01,    /* The file is not found */
    S_X_BAND_TRNSM_DELL_CARD_ERR   = 0x03,    /* SD card Error */
    S_X_BAND_TRNSM_DELL_PARAMS_ERR = 0xFE,    /* Parameter Error */
    S_X_BAND_TRNSM_DELL_COMM_ERR   = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_DELL_NUMBER
} S_X_BAND_TRNSM_DellStatus_enum;

/* Return statuses from CMD Create a file */
typedef enum {
    S_X_BAND_TRNSM_CREATE_OK         = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_CREATE_ERR        = 0x01,    /* The file cannot be created - maybe it is already existing (should be deleted first) */
    S_X_BAND_TRNSM_CREATE_CARD_ERR   = 0x03,    /* SD card Error */
    S_X_BAND_TRNSM_CREATE_PARAMS_ERR = 0xFE,    /* Parameter Error */
    S_X_BAND_TRNSM_CREATE_COMM_ERR   = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_CREATE_NUMBER
} S_X_BAND_TRNSM_CrateStatus_enum;

/* Return statuses from CMD Open a file */
typedef enum {
    S_X_BAND_TRNSM_OPEN_OK         = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_OPEN_ERR        = 0x01,    /* The file cannot be open - it may not exist */
    S_X_BAND_TRNSM_OPEN_CARD_ERR   = 0x03,    /* SD card Error */
    S_X_BAND_TRNSM_OPEN_PARAMS_ERR = 0xFE,    /* Parameter Error */
    S_X_BAND_TRNSM_OPEN_COMM_ERR   = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_OPEN_NUMBER
} S_X_BAND_TRNSM_OpenStatus_enum;

/* Return statuses from CMD Send a file */
typedef enum {
    S_X_BAND_TRNSM_SEND_OK              = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_SEND_ERR             = 0x01,    /* The file cannot be Send - maybe it is missing */
    S_X_BAND_TRNSM_SEND_CARD_ERR        = 0x03,    /* SD card Error */
    S_X_BAND_TRNSM_SEND_FILE_COMM_ERR   = 0x04,    /* RF error */
    S_X_BAND_TRNSM_SEND_FILE_NOT_READY  = 0x06,    /* The file is not ready to be transmitted */
    S_X_BAND_TRNSM_SEND_PARAMS_ERR      = 0xFE,    /* Parameter Error */
    S_X_BAND_TRNSM_SEND_COMM_ERR        = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_SEND_NUMBER
} S_X_BAND_TRNSM_SendStatus_enum;

/* Return statuses from all commands about changing the mode of the S/X-Band Transceiver */
typedef enum {
    S_X_BAND_TRNSM_CHANG_MODE_OK              = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_CHANG_MODE_SYS_ERR         = 0x01,    /* System error */
    S_X_BAND_TRNSM_CHANG_MODE_BUSY            = 0x03,    /* SD card Error */
    S_X_BAND_TRNSM_CHANG_MODE_PARAMS_ERR      = 0xFE,    /* Parameter Error */
    S_X_BAND_TRNSM_CHANG_MODE_COMM_ERR        = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_CHANG_MODE_NUMBER
} S_X_BAND_TRNSM_ChangeMode_enum;

/* Return statuses from all CMD Firmware update */
typedef enum {
    S_X_BAND_TRNSM_FW_UPDATE_OK              = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_FW_UPDATE_FW_FILE_ERR     = 0x01,    /* The file is corrupted */
    S_X_BAND_TRNSM_FW_UPDATE_OLD_SAME_FW     = 0x02,    /* Earlier or same FW version */
    S_X_BAND_TRNSM_FW_UPDATE_SD_CARD_ERR     = 0x03,    /* SD Card Error */
    S_X_BAND_TRNSM_FW_UPDATE_UPDATE_FAILD    = 0xFC,    /* Updated started, but finished with a Error */
    S_X_BAND_TRNSM_FW_UPDATE_PARAMS_ERR      = 0xFE,    /* Parameter Error */
    S_X_BAND_TRNSM_FW_UPDATE_COMM_ERR        = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_FW_UPDATE_NUMBER
} S_X_BAND_TRNSM_FW_update_enum;

/* Return statuses from CMD Set and Get parameters */
typedef enum {
    S_X_BAND_TRNSM_PARAMS_OK         = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_PARAMS_PARAMS_ERR = 0x03,    /* Parameter Error */
    S_X_BAND_TRNSM_PARAMS_COMM_ERR   = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_PARAMS_NUMBER
} S_X_BAND_TRNSM_SetGetParams_enum;

/* Return statuses from CMD Dir and Dir Next*/
typedef enum {
    S_X_BAND_TRNSM_DIR_OK         = 0x00,    /* Delete complete successful */
    S_X_BAND_TRNSM_DIR_NO_FILES   = 0x01,    /* The files cannot be found */
    S_X_BAND_TRNSM_DIR_CARD_ERR   = 0x03,    /* SD card Error */
    S_X_BAND_TRNSM_DIR_COMM_ERR   = 0xFF,    /* Communication Error with S-Band */
    S_X_BAND_TRNSM_DIR_NUMBER
} S_X_BAND_TRNSM_Dir_enum;


typedef struct {
    S_X_BAND_TRNSM_CMD_VAL_enum CMD;
    uint16_t                    tx_data_max_size;
    uint16_t                    rx_data_max_size;
}__attribute__((__packed__)) S_X_BAND_TRNSM_CMD_INFO_TxInfo_struct;

typedef enum {
    S_X_BAND_TRNSM_DEVTYPE_XBAND = 0x00,
    S_X_BAND_TRNSM_DEVTYPE_SBAND,
    S_X_BAND_TRNSM_DEVTYPE_BOTH
} S_X_BAND_TRNSM_DEVTYPE;

typedef struct {
    uint16_t  Reserved;
    uint16_t  Size;
    int32_t   FileHandler;
    uint32_t  PacketNumber;
    uint8_t   Data[S_X_BAND_TRNSM_TX_BUFF_SIZE];
} __attribute__((__packed__)) S_X_BAND_TRNSM_WriteFile_struct;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

extern FT_HANDLE  ftHandle;
extern S_X_BAND_CMD_StackEntry stackEntry;
extern uint8_t  S_X_BAND_TRNSM_Result_Rx_Buffer[S_X_BAND_TRNSM_RX_BUFF_SIZE];  /* The received data that has been requested */

uint8_t S_X_BAND_TRNSM_CreateFile (S_X_BAND_CMD_StackEntry *stackEntry);
uint8_t S_X_BAND_TRNSM_OpenFile (S_X_BAND_CMD_StackEntry *stackEntry);
S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_GetResult( S_X_BAND_TRNSM_CMD_enum CMD, S_X_BAND_TRNSM_RetRes_enum CMD_Type, uint8_t * RxData, uint16_t * RxDataLenght);
void  S_X_BAND_TRNSM_Send_Ack(S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type);
S_X_BAND_TRNSM_Response_enum S_X_BAND_TRNSM_SendCMD (S_X_BAND_TRNSM_CMD_enum CMD, uint16_t CMD_Type, uint8_t * TxData, uint16_t TxDataLenght);
void purgeBuffer();
void lenghtQueue(DWORD* RxBytes);