#include <inttypes.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <mqueue.h>
#include <stdlib.h>


#include "./Librairies/Structure.h"
#include "./Librairies/FilsSauvegarde.h"
#include "./Librairies/ReceptionDataFileMessage.h"
#include "./Librairies/VariableGlobale.h"

void FilsEnergie(){
  printf("je suis le fils energie\n");
    
   SYSENERGIE sysenergie;
  
  while(true){
   sysenergie.tension_batt_1  = 2      ;         //tension batt 1
   sysenergie.tension_batt_2 = 3       ;         //tension batt 2
   sysenergie.courant_batt_1  = 4      ;         //courant batt 1
   sysenergie.courant_batt_2  = 5      ;         //courant batt 2
   sysenergie.status_energie = 1 ; //status energie
   
   Envoie_data_Energie(sysenergie, 1, SAUVEGARDE);
   Envoie_data_Energie(sysenergie, 1, EMETTEUR);
   }

}