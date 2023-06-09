typedef enum {
    emitter_initialisation, 
    delete_files, 
    creating_file, 
    writing, 
    sending_file, 
    waiting, 
    get_obc_data,
    initialise_data_buffer //
}emitter_host_state;

typedef enum {
    ftdi_port,
    set_parameters,
    set_transmit_mode,
    restart_transmit_mode,
    init_finished
} initialisation_state;

extern emitter_host_state host_state;

//fonction a lancer dans le fils de l'emetteur
void fils_emitter();

//incremente le nom du fichier Log_XX.txt
void incr_file_name(uint8_t * * s);

//permet de tester l'envoi de data
void temp_get_obc_data(uint8_t * * data_target, uint32_t * data_lenght);