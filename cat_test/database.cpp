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
        .shift = shift,
    };
}

bool IsInBlock(uint32_t w, Block b) {
    for (int i = 0; i < b.size; i++) {
        if (b.ids[i] == w) return true;
    }

    return false;
}


#endif
