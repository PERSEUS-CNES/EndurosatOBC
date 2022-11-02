### Introduction
Repository de développement de l'OBC des fusées C'Space, intégrant la couche de communication avec l'emetteur [Endurosat](https://www.endurosat.com/cubesat-store/cubesat-communication-modules/s-band-transmitter/) de télémesure en bande-S.
### How to build 
Clone this repository and run `make`.
It will compile object files in `obj` directory, and create an executable named `obc` in `out` directory.
To execute it :
```
./out/obc
```
To clean object files and executable :
```
make clean
```
### Branches de développement
Le développement de l'**OBC** est scindé en **2 équipes** qui travaillent sur des **branches parallèles** :
- branche `dev_scheduler` intégrant :
-- la boucle principale d'acquisition des données capteurs (IMU, GPS, ...)
-- le stockage des données sur fichiers
-- l'envoie des données aux sous-systèmes intra-fusée (gestion du roulis, du parafoil, ...) par trames/socket UDP
- branche `dev_emitter` avec un développement incrémental de la **couche de communication** avec l'emetteur Endurosat via des données **factices** (bouchon/[stub](https://en.wikipedia.org/wiki/Method_stub))

Lors d'une **release** mensuelle, ces 2 branches sont mergées dans le tronc `main` pour obtenir un **OBC complet** avec gestion des I/O, stockage, communication Bord-Bord (sous-systèmes) et Sol-Bord (Télémesure). 
Le tronc commun **main** ne **DOIT** contenir que du code **COMPILABLE** et **FONCTIONNEL**.
### Stratégie de portage
Un code complet de communication avec l'emetteur existe déjà, mais uniquement fonctionnel sur microcontrolleur avec [DMA](https://en.wikipedia.org/wiki/Direct_memory_access).
L'objectif est donc de porter l'ensemble du code existant sur **Raspberry Pi4** avec l'utilisation d'un convertisseur **[FTDI USB-RS485](https://ftdichip.com/products/usb-rs485-we-1800-bt/)**,  fonction par fonction dans une logique de [Test-Driven Development](https://fr.wikipedia.org/wiki/Test_driven_development).
### Supplier's code and specification
La documentation fournisseur **Endurosat** est disponible dans le répertoire `SBAND Datasheet` :
- Manuel d'utilisation de l'emetteur Endurosat `2020.10.15 - S-Band Transmitter User Manual`
- Spécificiation du convertisseur FTDI USB-RS485 `USB-RS485-WE-1800-BT.pdf`
- Guide de programmation du FTDI USB-RS485 `D2XX_Programmers_Guide.pdf`
- Code de référence implémentant une machine à état avec toutes les fonctions **uniquement fonctionnel sur microcontrolleur** `S-band reference code`
### Remember PERSEUS's motto _**"TO INFINITY AND BEYOND"**_
![PERSEUS Logo](/index.png)