#ifndef INCL_DATABASE
#define INCL_DATABASE

#include <cstdlib>

#include "database.hpp"

void FreeDatabase(Database db) {
    for (int i = 0; i < db.wordCount; i++) {
        free(db.words[i]);
    }

    free(db.words);
    free(db.ids);
}

Block CreateBlock(Database db, int size, int shift) {
    return (Block) {
        .ids = &(db.ids[shift]),
        .size = size,
    };
}

#endif
