
Visite des sites web, analyse le HTML, et récupère du texte.
Fait un parcours en largeur: à chaque page visitée, les liens sont ajoutés à une queue, puis tirés un pr un.

Usage:
    ./a.out <URL> <FICHIER> <FILTRE>

    où
        <URL> est l'url de départ (par défaut https://en.wikipedia.org/wiki/French_fries)
        <FICHIER> est le nom du ficher a écrire en sortie (par défaut out.dat)
        <FILTRE> est une chaîne de caractères, tous les liens qui la contiennent seront ignorés


Format de fichier de sortie:

- Un uint32: le nombre de mots N
- Un uint32: le nombre d’occurrences mots M

- N fois:
    - un mot (chaîne de caractère)
    - un octet nul

- un octet nul 
    (donc deux à la suite en tout, si on compte celui d'avant)

- M fois:
    - un uint32: l'identifiant ID du mot (il apparaît donc en ID-ème dans la première parie du fichier)


TODO: find a way to see when two texts are not next to each other
TODO: prevent same text from being included many times in different pages (like headers and footers that are always the same) (already ignoring <footer> and <header>, but may de insufficient)
TODO: better filters for links and text
OPTI: avoid some string copies, and also urls are parsed multiple times (but the main bottleneck is curl for now)


