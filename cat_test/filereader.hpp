
#include<string>
#include<cinttypes>

#include "database.cpp"

// Reads file provided by the crawler

bool ReadFile(char* path, Database* res) ;

char* ReadString(FILE* f);

void FreeDatabase(Database db);


