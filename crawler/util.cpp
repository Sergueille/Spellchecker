
#ifndef UTIL_DEFINED
#define UTIL_DEFINED

#include "util.hpp"
#include "libs/yuarel.c"
#include "libs/yuarel.h"

constexpr int MAX_URL_SIZE = 500;

namespace Util {    

    char* CopyStringBuffer(std::string source) {
        char* res = (char*)malloc((source.size() + 1) * sizeof(char));

        for (int i = 0; i < source.size(); i++) {
            res[i] = source[i];
        }

        res[source.size()] =  '\0';

        return res;
    }

    template<typename T>
    T* VectorToBuffer(std::vector<T>* vect) {
        T* res = (T*)malloc(vect->size() * sizeof(T));

        for (int i = 0; i < vect->size(); i++) {
            res[i] = (*vect)[i];
        }

        return res;
    }

    bool StringIsNullOrEmpty(char* s) {
        return s == NULL || s[0] == '\0';
    }

    // OPTI: may be better to store the struct directly
    std::string GetRelativeURL(std::string current, std::string relative) {
        yuarel parsedCurrent;
        yuarel parsedRelative;
        yuarel_parse(&parsedCurrent, (char*)current.c_str());
        yuarel_parse(&parsedRelative, (char*)relative.c_str());

        // Replace current values by relative ones if they exist

        if (!StringIsNullOrEmpty(parsedRelative.scheme)) parsedCurrent.scheme = parsedRelative.scheme;
        if (!StringIsNullOrEmpty(parsedRelative.host)) parsedCurrent.host = parsedRelative.host;
        if (!StringIsNullOrEmpty(parsedRelative.path)) parsedCurrent.path = parsedRelative.path;
        parsedCurrent.query = parsedRelative.query;
        parsedCurrent.fragment = parsedRelative.fragment;

        // Reconstruct URL
        char* resData = (char*)malloc(MAX_URL_SIZE * sizeof(char));
        
        int pos = snprintf(resData, MAX_URL_SIZE, "%s://%s/%s", 
            parsedCurrent.scheme, parsedCurrent.host,parsedCurrent.path);

        /*
        if (!StringIsNullOrEmpty(parsedCurrent.query)) {
            pos += snprintf(&resData[pos], MAX_URL_SIZE - pos, "?%s", parsedCurrent.query);
        }

        if (!StringIsNullOrEmpty(parsedCurrent.fragment)) {
            pos += snprintf(&resData[pos], MAX_URL_SIZE - pos, "#%s", parsedCurrent.fragment);
        }
        */

        std::string res = std::string(resData);
        free(resData);
        return res;
    }

    bool IsURLInteresting(std::string current, std::string newURL) {
        char* currentCopy = CopyStringBuffer(current);
        char* newUrlCopy = CopyStringBuffer(newURL);

        yuarel parsedCurrent;
        yuarel parsedNew;
        yuarel_parse(&parsedCurrent, currentCopy);
        yuarel_parse(&parsedNew, newUrlCopy);

        bool res = true;

        if (parsedNew.scheme != NULL
         && strcmp(parsedNew.scheme, "https") 
         && strcmp(parsedNew.scheme, "http")) res = false; // Probably a link for mail or printing

        // Probably a link to the same page
        if ((parsedNew.path == NULL || !strcmp(parsedCurrent.path, parsedNew.path))
         && (parsedNew.host == NULL || !strcmp(parsedCurrent.host, parsedNew.host))) res = false; 

        free(currentCopy);
        free(newUrlCopy);

        return res; // Otherwise, is seems okay!
    }
}


#endif
