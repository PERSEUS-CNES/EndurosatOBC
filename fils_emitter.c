#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ftdi.h"

#include "emitter_config.h"
#include "emitter_reading.h"
#include "emitter_sending.h"
#include "emitter_writing.h"

#include "fils_emitter.h"

emitter_host_state host_state = emitter_initialisation;
initialisation_state init_state = ftdi_port;

const uint32_t MAX_DATA_BUFFER_LENGHT = 10000;
uint8_t * data_buffer;
uint32_t data_buffer_lenght;

uint8_t * emitter_file_name;//[]; = "Log_000000.txt";
int file_number = 0;
uint8_t fileHandle[4];

uint8_t debug = 0;

int port = 0;
int baud = 3000000;
int status_ft;
struct configuration param;
uint8_t * temp_buffer;
uint32_t temp_lenght;

void fils_emitter()
{
    uint8_t status = 0;//boolen
    //usleep(5000000);
    while(1){
        
        switch (host_state)
        {
        case emitter_initialisation:
            switch (init_state)
            {
            case ftdi_port:
                
                status_ft = initialize_FTDI(baud, port); 
                #ifdef DEBUG_FILS
                printf("ftdi init done \n");
                #endif
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
                    #ifdef DEBUG_FILS
                    printf("transmit mode done\n");
                    #endif
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
                emitter_file_name = malloc(10);
                memcpy(emitter_file_name,"Log_1.txt",10);
                host_state = initialise_data_buffer;
                #ifdef DEBUG_FILS
                printf("initialisation done\n");
                #endif
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
            incr_file_name(&emitter_file_name);
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
            #ifdef DEBUG_FILS
                printf("sending file %s done\n", emitter_file_name);
            #endif
            
            if(status)
            {
            #ifdef DEBUG_FILS
            printf("%s sent\n", emitter_file_name);
            #endif
                host_state = initialise_data_buffer;
            }
            break;

        case waiting:
            break;

        
        
        default:
            break;
        }
    }
}

void incr_file_name(uint8_t * * s)
{
   //Log_xxx.txt0
	int len = strlen(*s);
	
	int head = 3;
	int nb = len - 5;
	char d = 1;

	while(d)
	{
		if(nb == head)
		{
			*(*s + nb + 1 ) = '1';
			len++;
			*s = realloc(*s,sizeof(uint8_t)*(len + 1));
			*(*s + len) = 0;
			*(*s + len - 1) = 't';
			*(*s + len - 2) = 'x';
			*(*s + len - 3) = 't';
			*(*s + len - 4) = '.';
			*(*s + len - 5) = '0';
			len++;
			d = 0;
		}
		else if(*(*s + nb) == '9')
		{
			*(*s + nb) = '0';
			nb--;
		}
		else
		{
			d = 0;
			(*(*s + nb))++;
		}
	}

}


//fonction temporarire pour tester l'envoi de data par le fils
void temp_get_obc_data(uint8_t * * data_target, uint32_t * data_lenght)
{
    /*Message mess;
    mess = receptionEmetteur();

    *data_lenght = sizeof(Message);
    *data_target = malloc(sizeof(Message));
    memcpy(*data_target,&mess,sizeof(Message));*/
   

    *data_target = malloc(100);
    for(uint32_t i = 0; i < 100; i++)
    {
        *(*data_target + i) = i;
    }
    *data_lenght = 100;

}