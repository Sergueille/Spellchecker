
#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include "filereader.cpp"
#include "database.cpp"
#include "categories.cpp"

int main(int argc, const char** argv) {
    if (argc <= 1) {
        printf("Expected filename.\n");
        return EXIT_FAILURE;
    }

    printf("Reading file...");
    fflush(stdout);

    Database db;
    bool success = ReadFile((char*)argv[1], &db);

    Block testBlock = CreateBlock(db, 10000, 0);
    
    if (!success) {
        printf("\rFailed to read/parse the file.\n");
        return EXIT_FAILURE;
    }

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
    FreeDatabase(db);

    return EXIT_SUCCESS;
}
