
#include "filereader.hpp"
#include <cstdio>

bool ReadFile(char* path, Database* res) {
    FILE* f = fopen(path, "r");

    uint32_t wordCount;
    uint32_t dataCount;
    fread(&wordCount, sizeof(uint32_t), 1, f);
    fread(&dataCount, sizeof(uint32_t), 1, f);

    char** resWords = (char**)malloc(wordCount * sizeof(char*));
    uint32_t* resData = (uint32_t*)malloc(dataCount * sizeof(uint32_t));

    for (int i = 0; i < wordCount; i++) {
        resWords[i] = ReadString(f);
    }

    char c;
    fread(&c, sizeof(uint8_t), 1, f); // Read the null char
    if (c != '\0') {
        return false;
    }

    for (int i = 0; i < dataCount; i++) {
        fread(&(resData[i]), sizeof(uint32_t), 1, f);
    }

    fread(&c, sizeof(uint8_t), 1, f); // Make sure it's EOF;
    if (c != EOF && c != '\0') {
        return false;
    }
    
    *res = (Database) {
        .words = resWords,
        .wordCount = wordCount,
        .ids = resData,
        .idCount = dataCount,
    };

    fclose(f);

    return true;
}

char* ReadString(FILE* f) {
    long int startPos = ftell(f);
    int len = -1;

    uint8_t c = 1;
    while (c != 0) {
        fread(&c, sizeof(uint8_t), 1, f);
        len++;
    }

    fseek(f, startPos, 0);

    char* res = (char*)malloc((len + 1) * sizeof(char));
    for (int i = 0; i < len; i++) {
        fread(&(res[i]), sizeof(uint8_t), 1, f);
    }

    fread(&c, sizeof(uint8_t), 1, f); // Read the null char
    res[len] = '\0';

    return res;
}

