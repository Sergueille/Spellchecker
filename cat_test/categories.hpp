
#include <cinttypes>
#include "database.cpp"

struct Categories {
    int* data;
    int* sizes;
    int count;
};

// Assign random categories, categoryCount is the amount of different categories
Categories AssignRandomCategories(int wordCount, int categoryCount);

void FreeCategories(Categories c);

// Will calculate the proportion of the pair [word], [word of category cat] among all appearances of word
// Can return NaN if word not present in block
float CalculateWordCatCorrelation(uint32_t word, int cat, Categories c, Block b);

// Same as CalculateWordCatCorrelation, return an array of correlations, index with [wordId * categoryCount + categoryId] 
float* CalculateAllWordCatCorrelation(Categories c, Block b, Database db);

// Let C de the set of word contained in cat, Cs th set of all categories
// f(w, C) : CalculatePrevWordCat
// avg(A, B) : the average of f(w, B) for w \in A
//
// This function returns
// \sum_{w \in C} \sum_{D \in Cs} | f(w, D) - avg(A, D) | / |C|
//
float* CalculateCategoryVariances(Categories c, Database db, Block b, uint32_t* furtherWord);

