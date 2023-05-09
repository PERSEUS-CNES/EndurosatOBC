#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "emitter_config.h"
#include "emitter_reading.h"
#include "emitter_sending.h"
#include "emitter_writing.h"

#include "fils_emitter.h"

emitter_host_state host_state = emitter_initialisation;
initialisation_state init_state = ftdi_port;

void fils_emitter()
{
    uint8_t status = 0;//boolen
    switch (host_state)
    {
    case emitter_initialisation:
        switch (init_state)
        {
        case ftdi_port:
            int port = 0;
            int baud = 3000000;
            FT_STATUS status_ft = initialize_FTDI(baud, port); 

            init_state = set_parameters; 
            break;
        case set_parameters:
            struct configuration param;
            param.symbol_rate = 5;
            param.transmit_power = 27; 
            param.MODCOD = 1;
            param.roll_off = 0;
            param.pilot_signal = 1;
            param.FEC_frame_size = 0;
            param.pretransmission_delay = 3000;
            param.center_frequency = 2450.0000;

            status = set_emitter_config(&param, all_parameters);
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
            init_state = set_transmit_mode;
            break;
        case init_finished:
            host_state = get_obc_data;
            break;

        default:
            break;
        }
        break;

    case get_obc_data:
        break;

    case creating_file:
        break;
    
    case delete_files:
        break;
    
    case writing:
        break;

    case sending_file:
        break;

    case waiting:
        break;
    
    default:
        break;
    }
}