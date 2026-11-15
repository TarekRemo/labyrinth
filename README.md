# Labyrinthe

Jeu de labyrinthe en C qui se joue dans le terminal.

Le joueur (`O`) part en haut à gauche, doit récupérer la clé (`%`) pour ouvrir la porte (`#`) située en bas à droite, et gagne des points en chemin (trésors, rapidité). Les labyrinthes sont générés aléatoirement et sauvegardés dans des fichiers. Deux difficultés existent :

- **Facile** : un labyrinthe « parfait » (un seul chemin possible entre deux cases), avec une clé, des trésors et des pièges.
- **Difficile** : en plus, des murs sont cassés au hasard (plusieurs chemins possibles) et des monstres se déplacent dans le labyrinthe (fantômes `F` et ogres `G`).

Les meilleurs scores de chaque labyrinthe sont enregistrés.

## Exigences système

- Linux (testé sous Ubuntu) ou tout système de type Unix
- `gcc` et `make`
- Un terminal qui gère les couleurs ANSI (le terminal d'Ubuntu convient) et assez grand pour afficher le labyrinthe : chaque case occupe 3 colonnes, donc un labyrinthe de 25 cases de large demande environ 80 colonnes

Aucune bibliothèque externe n'est nécessaire.

## Installation et compilation

```bash
git clone <adresse-du-depot>
cd labyrinth
make
```

La commande `make` crée l'exécutable `bin/labyrinth.out`. Pour tout supprimer et recompiler :

```bash
make clean
make
```

## Lancement

Le jeu doit être lancé **depuis la racine du projet**, car les sauvegardes sont écrites dans le dossier relatif `data/` :

```bash
./bin/labyrinth.out
```

## Utilisation rapide

1. Dans le menu, choisir **Créer un labyrinthe** (ou **Charger un labyrinthe** pour en reprendre un déjà sauvegardé).
2. Choisir **Jouer**.
3. Se déplacer avec les flèches du clavier ou les touches `Z` `Q` `S` `D`.
4. Ramasser la clé `%`, puis rejoindre la porte en bas à droite pour gagner.

| Touche | Action |
|---|---|
| Flèches ou `Z` `Q` `S` `D` | Se déplacer (dans le jeu) / naviguer (dans les menus) |
| `Entrée` | Valider un choix |
| `R` | Revenir au menu (la partie en cours est conservée) |

## Tests unitaires

Les tests utilisent [minunit](https://github.com/siu/minunit) (fichier `tests/minunit.h`) :

```bash
make test
```

## Organisation du projet

| Dossier | Contenu |
|---|---|
| `src/` et `include/` | Code source (`.c`) et en-têtes (`.h`) |
| `tests/` | Tests unitaires |
| `data/labyrinths/` | Labyrinthes sauvegardés (`.cfg`) |
| `data/scores/` | Meilleurs scores (`.score`) |
| `bin/` et `build/` | Exécutable et fichiers objets générés par `make` |
| `compte_rendu.docx` | Rapport avec le manuel du développeur et le manuel du joueur |
