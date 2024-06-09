
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <unistd.h>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <cstddef>
#include <curses.h>

#include "util.cpp"
#include "getfile.cpp"
#include "htmlparse.cpp"

constexpr int MAX_DATA = 1000 * 1000;
constexpr int MAX_WORD_COUNT = 500 * 1000;

int main(int argc, const char** argv) {
    printf("\nCrawler is ready, make sure you have internet access!\n");
    printf("Press ENTER to stop and write file.\n");
    printf("See README.txt for usage.\n");

    char* outputFileName = (char*)"out.dat";
    char* entryPoint = (char*)"https://en.wikipedia.org/wiki/French_fries";

    if (argc > 4) {
        printf("\nToo many arguments! Expected at most 2.\n");
    }

    if (argc >= 2) {
        entryPoint = (char*)argv[1];
    }

    if (argc >= 3) {
        outputFileName = (char*)argv[2];
    }

    printf("\nWords,\tLinks,\tURL\n");

    std::map<std::string, uint32_t> map{}; // Map of (word, wordID)
    char** wordTable = (char**)malloc(MAX_WORD_COUNT * sizeof(char*)); // Table of words
    uint32_t nextID = 1; // The next id that will be given to a new word
    uint32_t* data = (uint32_t*)malloc(MAX_DATA * sizeof(uint32_t)); // Lists of words id
    int dataPos = 0;

    std::set<std::string> visitedURLs{}; // URS that are visited OR pushed in URLsToVisit
    std::queue<std::string> URLsToVisit{};
    URLsToVisit.push(std::string(entryPoint));
    visitedURLs.insert(std::string(entryPoint));

    while (true) {
        std::string current = URLsToVisit.front();
        URLsToVisit.pop();

        printf("FETCHING %.100s", current.c_str());
        fflush(stdout);

        char* txt = CallCurl((char*)current.c_str());

        printf("\rPARSING %.100s", current.c_str());
        fflush(stdout);

        std::string s = std::string(txt);
        free(txt);
        HTMLParse::PageResults res = HTMLParse::ParseHTML(s, current);

        printf("\r                                                                   "); // Make sure the line is cleaned properly
        printf("\r%d,\t%d,\t%.100s\n", (int)res.words.size(), (int)res.urls.size(), current.c_str());
        fflush(stdout);

        // Insert in database
        for (int i = 0; i < res.words.size(); i++) {
            std::string word = res.words[i];

            if (map.count(word) == 0) {
                wordTable[nextID] = CopyStringBuffer(word);
                nextID++;

                map.insert({word, nextID});
            }

            uint32_t ID = map.at(word);

            data[dataPos] = ID;
            dataPos++;
        }

        // Collect urls
        for (int i = 0; i < res.urls.size(); i++) {
            if (visitedURLs.count(res.urls[i]) > 0) continue; // Ignore if already visited

            URLsToVisit.push(res.urls[i]);
            visitedURLs.insert(res.urls[i]);
        }

        HTMLParse::FreePageResults(res);

        usleep(10000);

        // Check if a character is typed

        initscr();
        timeout(0);
        if (getch() != -1) {
            endwin();
            break;
        }
        endwin();
    }

    printf(" \n\nWRITING FILE...");
    fflush(stdout);

    // Now write the file
    FILE* outFile = fopen(outputFileName, "w");

    if (outFile == NULL) {
        printf("\rfailed to open file %s. Make sure you have the right permissions.", outputFileName);

    }
    else {
        // INT32: number of words
        uint32_t wordCount = nextID - 1;
        fwrite(&wordCount, 1, sizeof(uint32_t), outFile);

        // INT32: number of words indexes
        uint32_t dataCount = dataPos;
        fwrite(&dataCount, 1, sizeof(uint32_t), outFile);

        // wordCount times: a word string then \0
        for (int i = 0; i < wordCount; i++) {
            fprintf(outFile, "%s", wordTable[i + 1]);

            uint8_t zero = 0;
            fwrite(&zero, 1, sizeof(uint8_t), outFile);
        }

        // Another 0 to separate
        uint8_t zero = 0;
        fwrite(&zero, 1, sizeof(uint8_t), outFile);

        // dataCount times: an INT32
        for (int i = 0; i < dataCount; i++) {
            fwrite(&(data[i]), 1, sizeof(uint32_t), outFile);
        }

        printf("\rFinished! Outputted data in %s.\n", outputFileName);
        printf("%u words in total (%u different)\n", dataCount, wordCount);

        fclose(outFile);
    }

    for (int i = 1; i < nextID; i++) {
        free(wordTable[i]);
    }   

    free(data);
    free(wordTable);
}
