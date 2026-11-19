# Rapport de tests – DEAD CELLS NUMWORKS EDITION

Tests **réellement exécutés** sur PC (gcc 13, C99, `-fsanitize=address,undefined`) : `make test`.
Résultat final : **6 663 vérifications, 0 échec**, aucune erreur ASan/UBSan.

## Ce qui a été testé (et comment)
| # | Sujet | Contrôle |
|---|---|---|
| T01 | Salles | 14 modèles : dimensions, sol continu, bords ouverts, repères (départ, porte, arène, PNJ) |
| T02 | Niveaux | 5 zones × 40 graines : largeur, porte unique, boss + arène, ennemis posés au sol et hors des murs, départ valide |
| T03 | Physique | gravité, saut (≈ 48–52 px), saut court, double saut, vitesse de course, mur, plateformes traversables, chute en fosse (dégâts, mort) |
| T04 | Armes | dégâts de base des 10 armes (léger/lourd), niveau I→XV, ordre des qualités rien < + < ++ < S, dague dans le dos ×2, rapière, **faux‑serres : critique en alternant**, changement d'arme |
| T05 | Bouclier | garde (80 % absorbés), **parade** (0 dégât, attaquant étourdi), coup dans le dos, **renvoi de projectile**, garde‑bouclier ennemi (bloque, se brise sur coup lourd, vulnérable pendant son coup, perce par l'arbalète) |
| T06 | Objets | chiffres romains, étiquettes (`XII++`, `XVL`), bornes des niveaux, distribution des qualités |
| T07 | Ennemis | chacun des 9 types : se déplace/attaque, blesse le joueur, meurt, butin (pièces, cellules, cœurs, schémas), élites (≈ 5–10 %), nuage du putride |
| T08 | Boss | verrouillage de l'arène, 3 attaques de chaque boss, ondes/crachats, mort → porte ouverte, arme légendaire L, victoire |
| T09 | Passage | cellules mises en lieu sûr, schémas livrés, 2 PNJ, achat d'amélioration/schéma/niveau d'arme, refus faute de moyens, Oracle (3 mutations distinctes, choix, une de plus au passage suivant), changement de zone |
| T10 | Mutations | revanche, carapace, instinct, gourmand, saignement, enchaînement, adrénaline, chance |
| T11 | Sauvegarde | écriture/relecture fidèle, détection de corruption (CRC), 1ʳᵉ exécution, options |
| T12 | Rendu | 9 scènes + 5 zones non vides ; **aucun débordement** de la bande de 16 lignes (tampon de garde) |
| T13 | Stabilité | **300 000 images** de touches aléatoires sur 12 parties, invariants vérifiés à chaque image |
| T14 | Parties complètes | robot invulnérable : **40/40 parties terminées** (5 zones, 4 passages, 2 boss, victoire), ≈ 3,2 min de jeu chacune ; robot naïf sans invulnérabilité : voir journal |
| T15 | Mesures | temps de mise à jour/rendu (PC, indicatif), tailles mémoire |
| – | Boucle NumWorks | `main_eadk.c` assemblé en **un seul fichier** et exécuté contre un `eadk.h` **factice** : 1 654 images complètes, 15 bandes de 16 lignes envoyées par image, sortie propre sur la touche Accueil |

## Ce qui n'a PAS été testé (à ne pas considérer comme vérifié)
* La **compilation ARM** et le fichier **.nwa** (pas de compilateur ARM ni de réseau dans l'environnement de création).
* Le comportement sur une **vraie calculatrice** : vitesse (30 images/s non mesurés), clavier réel (noms de touches EADK d'après la documentation publique), mémoire réelle disponible.
* La **sauvegarde persistante** sur calculatrice (désactivée par défaut, voir `INSTALL.md`).
* L'équilibrage fin : le robot naïf (qui ne sait ni esquiver ni parer) meurt dans 25 parties sur 40 en moyenne en zone 2 ; un joueur humain a l'esquive, la parade et les mutations.

## Mesures
* Données de sprites : 248 200 octets (235 images) ; polices et salles : ≈ 12 Ko ; code (x86‑64 `-Os`) ≈ 40 Ko, donc ≈ 30 Ko attendus en ARM Thumb‑2 → **`.nwa` estimé ≈ 0,3 Mo**.
* RAM : `Game` = 7 080 octets, sauvegarde = 36 octets, bande de rendu = 10 240 octets.

## Journal brut de la dernière exécution
```
[T01 salles (14 modeles)]
[T02 generation semi-aleatoire (5 zones x 40 graines)]
    ennemis par niveau : 3 a 23 ; elites : 180 sur 2427 (7.4 %)
[T03 physique : gravite, saut, double saut, mur, plateformes]
[T04 armes : degats de base, niveau I..XV, qualites + ++ S L]
    epee           leger  12   lourd  26
    dague          leger   7   lourd  15
    rapiere        leger   9   lourd  22
    marteau        leger  30   lourd  48
    bouclier       leger   6   lourd   0
    arc            leger   9   lourd  24
    arbalete       leger  22   lourd  44
    baton de feu   leger  23   lourd  33
    sceptre        leger  14   lourd  10
    faux-serres    leger  16   lourd  32
    epee : niveau I = 12, niveau XV = 22, V+S = 19
    faux-serres : A=16, B apres A=18 (critique), B apres B=9
[T05 bouclier : garde, parade, renvoi de projectile, garde-bouclier ennemi]
[T06 objets : niveaux I..XV, qualites, etiquettes]
    qualites tirees (amelioration niveau 3) : rien 907, + 1798, ++ 862, S 433 (L jamais au hasard)
[T07 ennemis : detection, deplacement, attaque, mort, butin]
    geolier          degats subis  9 fois, projectiles en vol   0 images
    arbaletrier      degats subis  5 fois, projectiles en vol 127 images
    rat              degats subis  7 fois, projectiles en vol   0 images
    brute            degats subis  4 fois, projectiles en vol   0 images
    slime            degats subis 12 fois, projectiles en vol   0 images
    artificier       degats subis  4 fois, projectiles en vol 124 images
    garde-bouclier   degats subis  7 fois, projectiles en vol   0 images
    putride          degats subis  6 fois, projectiles en vol   0 images
    chauve-souris    degats subis  4 fois, projectiles en vol   0 images
    butin sur 400 ennemis : 213 pieces, 59 cellules, 49 coeurs, 5 schemas
[T08 boss : arene, attaques, mort, butin legendaire, victoire]
    Gobeur : attaques vues 3/3, ondes 1323, crachats 15
    Roi Fele : attaques vues 3/3, ondes 994, crachats 0
[T09 passage : cellules, Passeur (ameliorations, schemas), Oracle (mutations), changement de zone]
[T10 mutations (valables pour la partie)]
[T11 sauvegarde : ecriture, relecture, corruption, parametres]
    taille de la sauvegarde : 36 octets
[T12 rendu : toutes les scenes, sans debordement (ASan) et non vides]
[T13 stabilite : 300 000 images de touches aleatoires (invariants verifies a chaque image)]
    300000 images simulees sans plantage
[T14 robot : parties completes (5 zones, 2 boss, 4 passages) puis parties "normales"]
    40 / 40 parties finies (zone 1 -> victoire) ; duree moyenne 5703 images (3.2 min de jeu), pire 6089 ; 160 passages et 80 combats de boss traverses
    robot NAIF sans invulnerabilite (ne sait ni esquiver ni parer) : 25 / 40 morts, zone moyenne de la mort 2.0, 0 victoires
[T15 mesures (PC, a titre indicatif)]
    mise a jour : 0.8 us/image ; rendu plein ecran : 487 us/image (PC, avec ASan : sans valeur absolue)
    sizeof(Game) = 7080 octets ; sizeof(Save) = 36 ; bande de rendu = 10240 octets ; donnees de sprites = 248200 octets

RESULTAT : 6663 verifications, 0 echecs
```
