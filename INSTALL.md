# Guide d'installation – DEAD_CELLS_NUMWORKS.nwa

## 0. Ce qu'il faut savoir
* Le jeu est écrit pour le système d'applications externes de la NumWorks (EADK). D'après le README de NumPlay, cela concerne les modèles **N0110, N0115 et N0120** avec Epsilon **23.2 ou plus récent**. Vérifie le tien dans *Paramètres → À propos*.
* Le fichier `.nwa` doit être **compilé** (on ne peut pas renommer un autre fichier). Il n'a pas pu l'être dans l'environnement de création : choisis une des 3 méthodes ci‑dessous.
* Avec le mode examen activé, les applications externes peuvent être bloquées (non vérifié).

## 1. Obtenir le .nwa

### Méthode A – GitHub, sans rien installer (possible depuis un téléphone)
1. Crée un compte et un dépôt vide sur github.com.
2. Envoie tout le contenu du zip dans le dépôt (bouton *Add file → Upload files*, en gardant le dossier `.github`).
3. Onglet **Actions → « Compiler le .nwa » → Run workflow**.
4. À la fin (≈ 3 min), ouvre l'exécution et télécharge l'artefact **DEAD_CELLS_NUMWORKS** : c'est le `.nwa`.

### Méthode B – Sur un ordinateur (Linux, macOS, Windows avec WSL)
Installer : `git`, `node`/`npm`, `make`, `python3` et `gcc-arm-none-eabi` (Ubuntu : `sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi`). Puis :
```
./build_nwa.sh
```
Le script assemble le jeu en un seul fichier, le greffe sur l'exemple **officiel** NumWorks, compile, copie le résultat en `DEAD_CELLS_NUMWORKS.nwa` et refuse de continuer s'il dépasse 6 000 Ko.

### Méthode C – Me demander de le compiler
Dans une **nouvelle conversation** (l'outil m'a indiqué que celle‑ci n'a pas de réseau, mais qu'une nouvelle en aurait), joins ce zip et écris « compile le .nwa ». Je lancerais le même script.

## 2. Vérifier
* Taille : `ls -l DEAD_CELLS_NUMWORKS.nwa` → attendu ≈ 300 Ko (max 6 000 Ko ; l'espace réel de la calculatrice est d'environ 1,9 Mo pour *toutes* les applications).
* Ne renomme jamais un autre fichier en `.nwa`.

## 3. Installer sur la calculatrice
Utilise l'installateur officiel de la NumWorks pour les applications (page « Applications » de `my.numworks.com`, navigateur compatible WebUSB, calculatrice branchée en USB), ou l'outil indiqué dans le README de NumPlay. La procédure exacte peut évoluer : suis la page officielle à jour.

## 4. Sauvegarde
L'API officielle de la NumWorks n'offre pas d'écriture permanente : **par défaut la progression est conservée seulement tant que l'application reste ouverte**. Pour une vraie sauvegarde, ajoute la bibliothèque tierce *numworks‑extapp‑storage* (`storage.c`/`storage.h`) à `src/` et compile avec `-DDC_SAVE_EXTAPP` (non testé sur calculatrice).

## 5. Dépannage
* `nwlink` / `eadk-cflags` introuvable : l'exemple officiel a changé de version ; le script corrige l'option connue, sinon regarde les lignes `NWLINK` affichées.
* Touches qui ne répondent pas : les noms de touches EADK (`eadk_key_backspace`, `eadk_key_exe`, `eadk_key_toolbox`…) n'ont pas pu être vérifiés contre l'en‑tête réel ; ajuste `src/main_eadk.c` (fonction `poll_keys`).
* Trop lent : désactive les **effets** dans *Options*.
