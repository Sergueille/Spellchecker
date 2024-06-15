
#include "categories.hpp"
#include <cmath>
#include <time.h>
#include <cstdlib>
#include <cstdio>


Categories AssignRandomCategories(int wordCount, int categoryCount) {
    srand(time(NULL));

    int* data = (int*)malloc(wordCount * sizeof(int));
    int* sizes = (int*)malloc(categoryCount * sizeof(int));

    for (int i = 0; i < categoryCount; i++) {
        sizes[i] = 0;
    }

    for (int i = 0; i < wordCount; i++) {
        data[i] = rand() % categoryCount;
        sizes[data[i]]++;
    }

    return (Categories) {
        .data = data,
        .sizes = sizes,
        .count = categoryCount,
    };
}

void FreeCategories(Categories c) {
    free(c.data);
    free(c.sizes);
}

// Will calculate the proportion of the pair [word][word of category cat] among all appearances of word
float CalculateWordCatCorrelation(uint32_t word, int cat, Categories c, Block b) {
    int wordCount = 0;
    int wordWithCatCount = 0;

    for (int i = 0; i < b.size; i++) {
        if (b.ids[i] == word) {
            wordCount++;

            if (i > 0 && c.data[b.ids[i - 1]] == cat) {
                wordWithCatCount++;
            }
        }
    }   

    if (wordCount == 0) return NAN;

    return (float)wordWithCatCount / (float)wordCount;
}

float* CalculateAllWordCatCorrelation(Categories c, Block b, Database db) {
    float* correlations = (float*)malloc(db.wordCount * c.count * sizeof(float));
    int* occurrences = (int*)malloc(db.wordCount * sizeof(int));

    for (int i = 0; i < db.wordCount * c.count; i++) {
        correlations[i] = 0;
    }

    for (int i = 0; i < db.wordCount; i++) {
        occurrences[i] = 0;
    }

    for (int i = 0; i < b.size; i++) {
        uint32_t word = b.ids[i];

        occurrences[word]++;

        if (i > 0) {
            int prevCategory = c.data[b.ids[i - 1]];
            correlations[word * c.count + prevCategory]++;
        }
    }

    for (int i = 0; i < db.wordCount; i++) {
        for (int j = 0; j < c.count; j++) {
            if (occurrences[j] == 0) {
                correlations[i * c.count + j] = NAN;
            }
            else {
                correlations[i * c.count + j] /= (float)occurrences[j];
            }
        }
    }

    free(occurrences);

    return correlations;
}

float* CalculateCategoryVariances(Categories c, Database db, Block b, uint32_t* furtherWord) {
    float* correlations = CalculateAllWordCatCorrelation(c, b, db);

    float* res = (float*)malloc(c.count * sizeof(float));
    float* correlationSums = (float*)malloc(c.count * sizeof(float));

    for (int i = 0; i < c.count; i++) {
        res[i] = 0;
    }

    float furtherWordDiff = 0;

    for (int current = 0; current < c.count; current++) {
        // Reset sums
        for (int i = 0; i < c.count; i++) {
            correlationSums[i] = 0;
        }

        // Sum all correlations
        for (int i = 0; i < db.wordCount; i++) {
            float correlation = correlations[i * c.count + current];
            if (!std::isnan(correlation))
                correlationSums[c.data[i]] += correlation;
        }

        // Compute averages
        for (int i = 0; i < c.count; i++) {
            correlationSums[i] /= (float)c.sizes[i];
        }

        // Sum the differences
        for (int i = 0; i < db.wordCount; i++) {
            int wordCategory = c.data[i];
            float correlation = correlations[i * c.count + current];
            if (!std::isnan(correlation)) {
                float diff = fabs(correlation - correlationSums[wordCategory]);

                if (diff > furtherWordDiff) { // TODO: check on all categories
                    furtherWordDiff =diff;
                    *furtherWord = i;
                }

                res[wordCategory] += diff;
            }
        }
    }

    for (int i = 0; i < c.count; i++) {
        res[i] /= (float)c.sizes[i];
    }

    free(correlations);
    free(correlationSums);

    return res;
}


