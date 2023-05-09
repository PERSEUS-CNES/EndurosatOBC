typedef enum {
    emitter_initialisation, 
    delete_files, 
    creating_file, 
    writing, 
    sending_file, 
    waiting, 
    get_obc_data 
}emitter_host_state;

typedef enum {
    ftdi_port,
    set_parameters,
    set_transmit_mode,
    restart_transmit_mode,
    init_finished
} initialisation_state;

extern emitter_host_state host_state;

void fils_emitter();

void incr_file_name(uint8_t * file_name, int file_number);

void temp_get_obc_data(uint8_t * * data_target, uint32_t * data_lenght);