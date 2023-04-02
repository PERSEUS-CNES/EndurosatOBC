#include <stdint.h>
#include <stdlib.h>


extern uint8_t debug;

FT_HANDLE  ftHandle = NULL;

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

uint8_t set_emitter_config(struct configuration * parametres);
