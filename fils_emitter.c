#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dlfcn.h>

#include "emitter_config.h"
#include "emitter_reading.h"
#include "emitter_sending.h"
#include "emitter_writing.h"

#include "ftd2xx.h"
#include "import_ftdi_lib.h"
#include "import_ftdi_func_extern.h"
#include "fils_emitter.h"
//#include "./Librairies/Structure.h"
//#include "./Librairies/ReceptionDataFileMessage.h"

emitter_host_state host_state = emitter_initialisation;
initialisation_state init_state = ftdi_port;

FT_HANDLE ftHandle;
const uint32_t MAX_DATA_BUFFER_LENGHT = 100000;
uint8_t * data_buffer;
uint32_t data_buffer_lenght;
uint8_t debug = 0;
uint8_t emitter_file_name[] = "Log_000000.txt";
int file_number = 0;
uint8_t fileHandle[4];

int port = 0;
int baud = 3000000;
FT_STATUS status_ft;
struct configuration param;
uint8_t * temp_buffer;
uint32_t temp_lenght;




void fils_emitter()
{   
    usleep(5000000);
    printf("fils emitter \n");
    system("sudo rmmod ftdi_sio");
    printf("dl de la lib\n");
    uint8_t function_loaded = load_func();
    if(!function_loaded)
        return 1;
    printf("lib dl done ");
    uint8_t status = 0;//boolen
    while(1){
    switch (host_state)
    {
    case emitter_initialisation:
        switch (init_state)
        {
        case ftdi_port:
            printf("init ftdi port\n");
            status_ft = initialize_FTDI(baud, port); 
            printf("init ftdi port 2\n");
            init_state = set_parameters; 
            break;
        case set_parameters:
            
            param.symbol_rate = 5;
            param.transmit_power = 27; 
            param.MODCOD = 1;
            param.roll_off = 0;
            param.pilot_signal = 1;
            param.FEC_frame_size = 0;
            param.pretransmission_delay = 3000;
            param.center_frequency = 2450.0000;

            status = set_emitter_config(&param, all_parameters);
            usleep(5000);
            if(status)
            {
                init_state = set_transmit_mode;
            }
            else
            {
                init_state = set_parameters;
            }
            break;
        case set_transmit_mode:
            status = transmit_mode(1);
            usleep(5000);
            if(status)
            {
                init_state = init_finished;
            }
            else
            {
                init_state = restart_transmit_mode;
            }
            break;
        case restart_transmit_mode:
            status = transmit_mode(0);
            usleep(5000);
            init_state = set_transmit_mode;
            break;
        case init_finished:
            host_state = initialise_data_buffer;
            break;

        default:
            break;
        }
        break;

    case initialise_data_buffer:
        data_buffer_lenght = 0;
        free(data_buffer);
        data_buffer = malloc(1);

        host_state = get_obc_data;
        break;
    case get_obc_data:
        
        temp_get_obc_data(&temp_buffer,&temp_lenght);
        data_buffer_lenght += temp_lenght;
        data_buffer = realloc(data_buffer,data_buffer_lenght);
        memcpy(data_buffer + data_buffer_lenght - temp_lenght,temp_buffer,temp_lenght);
        free(temp_buffer);
        if(data_buffer_lenght > MAX_DATA_BUFFER_LENGHT)
        {
            
            host_state = delete_files;
        }
        break;

    case creating_file:
        status = createFile(emitter_file_name,MAX_DATA_BUFFER_LENGHT,fileHandle);
        if(status)
        {
            host_state = writing;
        }
        else
        {
            host_state = delete_files;
        }
        break;
    
    case delete_files:
        status = deleteAllFiles();
        usleep(5000);
        if(status)
        {
            host_state = creating_file;
        }
        else
        {
            host_state = delete_files;
        }
        break;
    
    case writing:
        status = writeMultiple(fileHandle,data_buffer,data_buffer_lenght);
        usleep(5000);
        if(status)
        {
            host_state = sending_file;
        }
        else
        {
            host_state = delete_files;
        }
        break;

    case sending_file:
        status = sendFile(emitter_file_name);
        usleep(5000);
        if(status)
        {
            host_state = get_obc_data;
        }
        break;

    case waiting:
        break;

    
    
    default:
        break;
    }
    }
}

void incr_file_name(uint8_t * file_name, int file_number)
{
   

}

//fonction temporarire pour tester l'envoi de data par le fils
void temp_get_obc_data(uint8_t * * data_target, uint32_t * data_lenght)
{
    /*Message mess;
    mess = receptionEmetteur();

    *data_lenght = sizeof(Message);
    *data_target = malloc(sizeof(Message));
    memcpy(*data_target,&mess,sizeof(Message));*/
   

    data_target = malloc(100);
    for(uint32_t i = 0; i < 100; i++)
    {
        *(*data_target + i) = i;
    }
    *data_lenght = 100;

}