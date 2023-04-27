#include <stdint.h>
#include <stdlib.h>
#include "ftd2xx.h"


extern uint8_t debug;

extern FT_HANDLE  ftHandle;

typedef enum  {all_parameters,
                        parm_symbol_rate, 
                        parm_transmit_power, 
                        parm_MODCOD, 
                        parm_roll_off,
                        parm_pilot_signal,
                        parm_FEC_frame_size,
                        parm_pretransmission_delay,
                        parm_center_frequency} parameters;


struct configuration {
    uint8_t symbol_rate;
    uint8_t transmit_power;
    uint8_t MODCOD;
    uint8_t roll_off;
    uint8_t pilot_signal;
    uint8_t FEC_frame_size;
    uint16_t pretransmission_delay;
    float center_frequency;
};

FT_STATUS initialize_FTDI(int baudRate, int portNum);

uint8_t set_emitter_config(struct configuration * parametres, parameters config_changes);

uint8_t get_emitter_config(struct configuration * parametres, parameters config_changes);

uint8_t safe_shutdown();