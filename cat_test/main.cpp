
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

    Block testBlock = CreateBlock(db, 100000, 0);
    
    if (!success) {
        printf("\rFailed to read/parse the file.\n");
        return EXIT_FAILURE;
    }

    int CAT_COUNT = 50; // Absolutely random... 

    Categories c = AssignRandomCategories(db.idCount, CAT_COUNT);

    uint32_t furtherWord;
    float* test = CalculateCategoryVariances(c, db, testBlock, &furtherWord);
    printf("\n%f\n", test[0]);
    printf("%s\n", db.words[furtherWord]);

    FreeCategories(c);
    FreeDatabase(db);

    free(test);

    return EXIT_SUCCESS;
}
