
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <unistd.h>
#include <string>
#include <map>
#include <cstddef>
#include <curses.h>

#include "util.cpp"
#include "getfile.cpp"
#include "htmlparse.cpp"

constexpr int MAX_DATA = 1000 * 1000;
constexpr int MAX_WORD_COUNT = 50 * 1000;

int main(int argc, const char** argv) {
    printf("\nCrawler is ready, make sure you have internet access!\n");
    printf("Press ENTER to stop and write file.\n");
    printf("See README.txt for usage.\n");

    char* outputFileName = (char*)"out.dat";
    char* entryPoint = (char*)"https://en.wikipedia.org/wiki/French_fries";
    int seed = time(NULL);

    if (argc > 5) {
        printf("\nToo many arguments! Expected at most 2.\n");
    }

    if (argc >= 2) {
        entryPoint = (char*)argv[1];
    }

    if (argc >= 3) {
        outputFileName = (char*)argv[2];
    }

    if (argc >= 4) {
        seed = atoi(argv[3]);
    }

    printf("\nWords,\tLinks,\tURL\n");

    srand(seed);

    std::map<std::string, uint32_t> map; // Map of (word, wordID)
    std::string* wordTable = (std::string*)malloc(MAX_WORD_COUNT * sizeof(std::string)); // Table of words
    uint32_t nextID = 1; // The next id that will be given to a new word
    uint32_t* data = (uint32_t*)malloc(MAX_DATA * sizeof(uint32_t)); // Lists of words id
    int dataPos = 0;

    String currentURL = Util::MakeStringCopy(entryPoint);
    String previousURL = Util::MakeStringCopy((char*)"");
    while (true) {
        printf("FETCHING %.100s", currentURL.data);
        fflush(stdout);

        char* txt = CallCurl(currentURL.data);

        printf("\rPARSING %.100s", currentURL.data);
        fflush(stdout);

        String s = Util::MakeString(txt);
        HTMLParse::PageResults res = HTMLParse::ParseHTML(s, currentURL);

        printf("\r                                                                   "); // Make sure the line is cleaned properly
        printf("\r%d,\t%d,\t%.100s\n", res.wordCount, res.urlCount, currentURL.data);
        fflush(stdout);

        // Insert in database
        for (int i = 0; i < res.wordCount; i++) {
            std::string word = std::string(res.words[i].data);

            if (map.count(word) == 0) {
                map.insert({word, nextID});
                wordTable[nextID] = word;
                nextID++;
            }

            uint32_t ID = map.at(word);

            data[dataPos] = ID;
            dataPos++;
        }

        if (res.urlCount > 0) {
            // Get a random URL
            int randomId = rand() % res.urlCount;

            String newURL = Util::GetRelativeURL(Util::MakeStringCopy(currentURL.data), res.urls[randomId]);
            Util::FreeString(previousURL);
            previousURL = currentURL;
            currentURL = newURL;
        }
        else {
            Util::FreeString(currentURL);
            currentURL = Util::MakeStringCopy(previousURL.data);
        }

        HTMLParse::FreePageResults(res);
        Util::FreeString(s);

        usleep(10000);

        // Check if ca character is typed

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
        for (int i = 1; i < wordCount; i++) {
            fprintf(outFile, "%s", wordTable[i].c_str());

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

    free(data);
    free(wordTable);
}
