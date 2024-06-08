
Visite des sites au hasard, analyse le HTML, et récupère du texte.

Usage:
    ./a.out <URL> <FICHIER> <SEED>

    où
        <URL> est l'url de départ (par défaut https://en.wikipedia.org/wiki/French_fries)
        <FICHIER> est le nom du ficher a écrire en sortie (par défaut out.dat)
        <SEED> la seed (un entier) pour déterminer les liens au hasard (aléatoire par default)


Format de fichier de sortie:

- Un uint32: le nombre de mots N
- Un uint32: le nombre d’occurrences mots M

- N fois:
    - un mot (chaîne de caractère)
    - un octet nul

- un octet nul 
    (donc deux à la suite en tout, si on compte celui d'avant)

- M fois:
    - un uint32: l'identifiant du mot (en comptant a partir de 1) OU 0
        (0 signifie ques les mots ne sont pas côte-à-côte, c'est un séparateur)



