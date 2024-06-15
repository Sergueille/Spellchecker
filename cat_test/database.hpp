
#include <cinttypes>

struct Database {
    char** words;
    uint32_t wordCount;
    uint32_t* ids;
    uint32_t idCount;
};

struct Block {
    uint32_t* ids;
    int size;
};


void FreeDatabase(Database db);

Block CreateBlock(Database db, int size, int shift);

