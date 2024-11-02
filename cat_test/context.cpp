
#include <cstdlib>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "database.cpp"

struct WordWithUse {
    int id;
    int useCount;
};

int compareCounts(const void* a, const void* b) {
    return ((WordWithUse*)b)->useCount - ((WordWithUse*)a)->useCount;
}

WordWithUse* getContextWords(int wId, int range, Database db, bool sort) {
    WordWithUse* uses = (WordWithUse*)malloc(db.wordCount * sizeof(WordWithUse));

    for (int i = 0; i < db.wordCount; i++) {
        uses[i] = WordWithUse {
            .id = i,
            .useCount = 0,
        };
    }

    for (int i = 0; i < db.idCount; i++) {
        if (db.ids[i] == wId) {
            for (int delta = -range; delta <= range; delta++) {
                if (i + delta >= 0 && i + delta < db.idCount) {
                    uses[db.ids[i + delta]].useCount++;
                }
            }
        }

    }

    if (sort) {
        qsort(uses, db.wordCount, sizeof(WordWithUse), compareCounts);
    }

    return uses;
}

int getOccurrences(int id, Database db) {
    int res = 0;

    for (int i = 0; i < db.idCount; i++) {
        if (db.ids[i] == id) {
            res++;
        }
    }

    return res;
}

int getWordId(char* w, Database db) {
    for (int i = 0; i < db.wordCount; i++) {
        if (strcmp(db.words[i], w) == 0) {
            return i;
        }
    }

    return -1;
}

// Returns an array of pointers all pointing on the same block, so you must only free the result AND the first pointer of the result, not the individual strings 
char** getWordsFromFile(char* path, int* resWordCount) {
    FILE* f = fopen(path, "r");

    // Count words
    int wordCount = 0;
    char c = ' ';
    int length = 0;
    while (true)
    {
        c = fgetc(f);

        if (c == EOF) {
            break;
        }
        else if (c == '\0') {
            wordCount++;
        }

        length++;
    }

    wordCount--; // Ignore last 0

    char** res = (char**)malloc(wordCount * sizeof(char*));
    char* resStr = (char*)malloc(length * sizeof(char));
    res[0] = resStr;

    // Back to start of the file
    rewind(f);

    int i = 0;
    int currentWord = 1;
    while (true)
    {
        c = fgetc(f);

        if (c == EOF) {
            break;
        }
        else if (c == '\0') {
            resStr[i] = c;

            if (currentWord >= wordCount) {
                break;
            }

            res[currentWord] = &resStr[i + 1];
            currentWord++;
        }
        else {
            resStr[i] = c;
        }

        i++;
    }

    *resWordCount = wordCount;
    return res;
}

void context(int argc, const char** argv, Database db) {
    if (argc <= 3) {
        printf("Enter a word!\n");
        return;
    }

    int id = getWordId((char*)argv[3], db);
    printf("Word id: %d\n", id);

    // Get occurrences in database
    int count = 0;
    for (int i = 0; i < db.idCount; i++) {
        if (db.ids[i] - 1 == id) {
            count++;
        }
    } 

    printf("Total occurrences of word: %d\n", count);

    int ranges[] = { 1, 5, 10, 15, 20 };
    for (int r = 0; r < 5; r++) {
        printf("---------------------- Range %d:\n", ranges[r]);

        WordWithUse* list = getContextWords(id, ranges[r], db, true);

        for (int i = 0; i < 100; i++) {
            if (list[i].useCount == 0) break;

            printf("%s (%d)  ", db.words[list[i].id], list[i].useCount);

            if (i % 10 == 0) printf("\n");
        }

        printf("\n");

        free(list);
    }
}

void contextTest(int argc, const char** argv, Database db) {
    if (argc <= 4) {
        printf("Expected [id] [sentence]\n");
        return;
    }

    uint32_t* sentenceIds = (uint32_t*)malloc(100 * sizeof(uint32_t));
    int idCount = 0;

    char* targetWord;

    char* acc = (char*)malloc(50 * sizeof(char));
    int accSize = 0;
    int sentenceLength = strlen(argv[4]);
    int targetWordPosition = atoi(argv[3]);

    for (int i = 0; i < sentenceLength; i++) {
        char c = argv[4][i];

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c < 0) {
            acc[accSize] = c;
            accSize++;
        }
        else {
            acc[accSize] = '\0';
            int id = getWordId(acc, db);

            if (idCount == targetWordPosition) {
                targetWord = (char*)malloc((accSize + 1) * sizeof(char));
                strcpy(targetWord, acc);
            }

            sentenceIds[idCount] = id;
            idCount++;

            accSize = 0;
        }
    }

    // Get suggestions from rust
    char* command = (char*)malloc(200 * sizeof(char));
    sprintf(command, "cd ../rust_test; cargo r --release -- file %s", targetWord);
    system(command);
    free(command);

    int wordCount;
    char** words = getWordsFromFile((char*)"../rust_test/.out", &wordCount);

    int ranges[] = { 1, 5, 10, 15, 20 };
    for (int r = 0; r < 5; r++) {
        int range = ranges[r];
        printf("\nRange %d:\n", range);

        for (int i = 0; i < wordCount; i++) {

            int wordId = getWordId(words[i], db);
            if (wordId != -1) {
                WordWithUse* list = getContextWords(wordId, range, db, false);

                float prob = 1;
                int contextWordUsed = 0;
                for (int delta = -range; delta < range; delta++) {
                    if (delta + targetWordPosition >= 0 && delta + targetWordPosition < idCount) {
                        int contextId = sentenceIds[targetWordPosition + delta];

                        if (contextId != -1) {
                            float p = (float)list[contextId].useCount / (float)getOccurrences(contextId, db);

                            if (p != 0) {
                                contextWordUsed++;
                                prob *= p;
                            }
                        }
                    }
                }

                printf(" %s: %f using %d context words\n", words[i], prob, contextWordUsed);

                free(list);
            }
            else {
                printf(" %s: Never found in database :(\n", words[i]);
            }
        }
    }

    free(words[0]);
    free(words);

    free(sentenceIds);
    free(acc);
    free(targetWord);
}

