# DEAD CELLS – NUMWORKS EDITION (projet de fan)

Jeu d'action‑plateforme 2D pour calculatrice **NumWorks**, dans l'esprit des « roguelites » (mort et recommencement, armes aléatoires, améliorations permanentes, mutations).
**Tout est original** : code, sprites pixel art, niveaux, noms. Projet non officiel, sans lien avec Motion Twin / Evil Empire.

> **État** : le jeu est complet et testé sur PC (voir `TEST_REPORT.md`). Le fichier `DEAD_CELLS_NUMWORKS.nwa` n'a **pas** pu être compilé dans l'environnement où le jeu a été écrit (pas de compilateur ARM ni de réseau) : voir `INSTALL.md` pour l'obtenir en quelques minutes.
> Il n'y a volontairement **aucun son, aucune musique et aucune transition** (la NumWorks n'a pas de haut‑parleur ; on garde l'essentiel).

## Commandes
| Touche | Action |
|---|---|
| ◄ ► | se déplacer |
| ▲ | sauter (appuyer une 2ᵉ fois en l'air = double saut) |
| ▼ | traverser une plateforme |
| **OK** | attaque rapide ; **maintenu puis relâché** = attaque lourde |
| **Retour** | esquive (invincible quelques images) |
| **⌫** | changer d'arme (avec la *faux‑serres* : 2ᵉ attaque) |
| **EXE** | boire une fiole de soin |
| **Boîte à outils** | pause |
| **OK** (dans un passage) | parler à un PNJ |
| Accueil | quitter |

## Contenu
* **5 zones** : Quartiers des prisonniers, Promenade des condamnés, Égouts toxiques, Remparts, Château final. Niveaux **semi‑aléatoires** (salles assemblées au hasard), difficulté progressive.
* **9 ennemis + 2 boss** : geôlier (mêlée), arbalétrier (distance), rat (rapide), brute (résistante), slime, **artificier** (bombes en cloche), **garde‑bouclier** (garde frontale, à briser ou contourner), **putride** (zombie toxique, nuage de poison à sa mort), **chauve‑souris** (pique du ciel). Boss : le **Gobeur** (zone 3) et le **Roi Fêlé** (zone 5, plusieurs attaques et phase de rage). Certains ennemis sont des **élites** (PV ×2,2, butin ×2, schémas).
* **10 armes** : épée, dague (coup dans le dos ×2), rapière (coup de pointe), marteau (étourdit), **bouclier** (garde, **parade** juste à temps = 0 dégât + renvoi des projectiles), arc, arbalète (perce les boucliers), bâton de feu (brûlure), sceptre d'éclair (chaîne), **faux‑serres** (arme double : occupe les 2 emplacements, **critique en alternant** ses 2 attaques).
* **Niveau d'objet I à XV** (augmente avec la zone) et **qualité** : rien, **+**, **++**, **S**, **L** (légendaire, lâché par le Gobeur, avec effet spécial).
* **Passages entre zones** avec deux PNJ : le **Passeur** (améliorations permanentes et **schémas** contre des **cellules**, amélioration de l'arme contre des pièces) et l'**Oracle** (**mutations** valables pour la partie : une de plus à chaque passage, au choix, 10 possibles).
* **Cellules** (récoltées sur les ennemis, mises en lieu sûr à chaque passage ; 30 % conservés si tu meurs avant), **schémas** (ouvrent de nouvelles armes dans le tirage aléatoire), pièces.
* Écran titre, boutique, options (effets, mode facile, effacer la progression), commandes, pause, mort, victoire.
* Sauvegarde de la progression, des améliorations, des statistiques et des paramètres (voir `INSTALL.md` : à activer sur calculatrice).

## Technique
* C99 pur, entiers uniquement, aucune allocation dynamique. Rendu **par bandes de 16 lignes** (10 Ko) : pas de tampon plein écran (153 Ko) en mémoire.
* RAM : état du jeu 7 Ko + bande de rendu 10 Ko. Taille estimée du `.nwa` : **≈ 0,3 Mo** (6 000 Ko maximum exigés, 2 000 Ko visés).
* `src/` jeu · `tools/` génération des sprites/salles (Python) et assemblage en un seul fichier · `host/` banc de test PC · `assets/` aperçus.
