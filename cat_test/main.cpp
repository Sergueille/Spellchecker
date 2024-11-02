
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "filereader.cpp"
#include "database.cpp"
#include "categories.cpp"

#include "context.cpp"


void categories(int argc, const char** argv, Database db) {
    printf("Reading file...");
    fflush(stdout);

    Block testBlock = CreateBlock(db, 10000, 0);

    printf("\rRunning...                  \n");

    for (int i = 0; i < testBlock.size; i++) {
        printf("%s ", db.words[db.ids[i]]);
    }
    printf("\n\n");

    int CAT_COUNT = 10; // Absolutely random... 

    Categories c = AssignRandomCategories(db.idCount, CAT_COUNT);

    uint32_t furtherWord;
    float* variances = CalculateCategoryVariances(c, db, testBlock, &furtherWord);
    float avg = Average(variances, c.count);

    for (int step = 0; step < 10000; step++) {

        furtherWord = testBlock.ids[rand() % testBlock.size]; // TEST: pick random

        int initialCategory = c.data[furtherWord];
        bool success = false;

        printf("%d: Moving %s\n", step, db.words[furtherWord]);
        
        for (int cat = 0; cat < c.count; cat++) {
            if (cat == initialCategory) continue;

            ChangeCategoryOfWord(furtherWord, cat, c);

            uint32_t newFurtherWord;
            float* newVariances = CalculateCategoryVariances(c, db, testBlock, &newFurtherWord);
            float newAvg = Average(newVariances, c.count);
            free(newVariances);

            if (newAvg < avg) {
                furtherWord = newFurtherWord;
                avg = newAvg;
                success = true;

                printf("Average variance: %.10f\n", avg);
                break;
            }
        }

        if (!success) {
            ChangeCategoryOfWord(furtherWord, initialCategory, c);
            /*
            printf("Worst word already in best category. Exiting...\n");
            break;
            */
        }
    }

    for (int i = 0; i < c.count; i++) {
        for (int j = 0; j < db.wordCount; j++) {
            if (c.data[j] == i && IsInBlock(j, testBlock)) {
                printf("%s, ", db.words[j]);
            }
        }

        printf("\n--------------\n");
    }

    free(variances);

    FreeCategories(c);
}

int main(int argc, const char** argv) {
    if (argc <= 2) {
        printf("Usage: keyword(cat, ctx) database-path\n");
        return EXIT_FAILURE;
    }

    Database db;
    bool success = ReadFile((char*)argv[2], &db);
    
    if (!success) {
        printf("\rFailed to read/parse the file.\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "cat") == 0) {
        categories(argc, argv, db);
    }
    else if (strcmp(argv[1], "ctx") == 0) {
        context(argc, argv, db);
    }
    else if (strcmp(argv[1], "cxt") == 0) {
        contextTest(argc, argv, db);
    }
    else {
        printf("Invalid keyword. Possible ones are: cat, ctx.\n");
        return EXIT_FAILURE;
    }

    FreeDatabase(db);

    return EXIT_SUCCESS;
}

