# Casse_Brique

**Presentation :**

Implementation in C from scratch of a break out game.

Helped with live and youtube 'tutorials' of Dan Zaidan

Twitch : https://www.twitch.tv/danzaidantutoriaux
YouTube : https://www.youtube.com/channel/UCkG6WU8eYvNYOgMhnpAUf4Q

Sprites and animations done under Pixaki apps - iOs

**Commands :**

* Mouse : Move the player

* F5 : Change resolution

* Escape : Close the game

* Left arrow / Right arrow : Change level

**Build and run :**

Move the "Sprites" and "Sounds" directories in the OS directory that fits the environnement who want to play the game ("Windows", "Unix" or "MacOS" directory  (the sound will be soon developed for these two last OSs))

1. "Windows" directory to build and play the game under Window OS (tested under Windowd 10)
   
   -Run win_platform32.exe in the "build" directory 
   
   -If you want to modify the code, build the build.bat file from a developer command prompt for VS2017 in the "Code" directory.

2. "Unix" or "MacOS" directory to build and play the game under Unix OS (tested under Ubuntu 16.04) or MacOS (no tested)
   
   -Need the installation of the SDL2 library : sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
   
   -Run ./build.sh in a prompt (may need a chmod)
   
   -Run unix_platform.out

========================================================================================================================================================================================

**Présentation :**

Implémentation d'un casse_brique en C 'from scratch' 

Aidé avec les 'tutoriaux' live et youtube de Dan Zaidan :

Twitch : https://www.twitch.tv/danzaidan
YouTube : https://www.youtube.com/channel/UCkG6WU8eYvNYOgMhnpAUf4Q

Sprites et animations créées sous Pixaki - iOs

**Commandes :**

* Souris :                        Déplacer joueur

* F5 :                            Changer la résolution 

* Echape :                        Quitter le jeu

* Flèche gauche / flèche droite : Changer de niveaux

**Compilation et exécution :**

Déplacer le dossier "Sprites" dans le dossier OS correspondant à l'environnement où vous souhaitez lancer le jeu (dossier "Windows" ou "Unix")

1. Dossier "Windows" pour compiler et lancer le jeu sous Windows (testé sous Windows 10)

   -Exécutez win_platform32.exe dans le dossier "build".

   -Si vous souhaitez modifier le code, compilez avec un invité de commande développer pour VS2017 le fichier build.bat dans le dossier "Code".

2. Dossier "Unix" ou "MacOS" pour compiler et lancer le jeu sous Unix (testé sous Ubuntu 16.04) ou MacOS (non testé)

   -Nécessite l'installation de la librairie SDL2 : sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

   -Lancez ./build.sh depuis un terminal (peut nécessiter un chmod)

   -Lancez unix_platform.out
