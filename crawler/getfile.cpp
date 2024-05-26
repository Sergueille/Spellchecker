
#include <cstdio>
#include <cstdlib>
#include "getfile.hpp"

char* CallCurl(char* url) {
    // Create the command
    char command[2048];
    sprintf(command, "curl \"%s\" --location -s -o tmp", url);

    // Call it!
    system(command);

    // Read file located in "tmp"
    FILE* resFile = fopen("tmp", "r");
    if (resFile == NULL) {
        printf("\nFailed to get file!" );
        exit(1);
    }
    // Get the length of the file
    fseek(resFile, 0, SEEK_END); 
    size_t fileSize = ftell(resFile);
    fseek(resFile, 0, SEEK_SET);

    // Potential ub cause here?
    char* res = (char*)malloc((fileSize + 2) * sizeof(char));

    char c;
    int i = 0;
    do {
        c = fgetc(resFile);
        res[i] = c;
        i++;
    } while (c != EOF);
    res[i] = '\0';

    fclose(resFile);

    // Remove the file
    int err = remove("tmp");
    if (err) {
        printf(" \nFailed to delete tmp file (%d)", err);
        exit(1);
    }

    return res;
}

