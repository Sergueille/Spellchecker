
#ifndef UTIL_DEFINED
#define UTIL_DEFINED

#include "util.hpp"
#include "libs/yuarel.c"
#include "libs/yuarel.h"

constexpr int MAX_URL_SIZE = 500;

namespace Util {
    String MakeString(char* data) {
        return (String) {
            .data = data,
            .len = strlen(data)
        };
    }

    String MakeStringCopy(char* data) {
        size_t len = strlen(data);
        char* copy = (char*)malloc((len + 1) * sizeof(char));
        strcpy(copy, data);

        return (String) {
            .data = copy,
            .len = len
        };
    }

    void FreeString(String s) {
        free(s.data);
    }

    String GetEmptyString() {
        return (String) {
            .data = NULL,
            .len = 0,
        };
    }

    template<typename T>
    T* VectorToPtr(std::vector<T>* vect) {
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
    String GetRelativeURL(String current, String relative) {
        yuarel parsedCurrent;
        yuarel parsedRelative;
        yuarel_parse(&parsedCurrent, current.data);
        yuarel_parse(&parsedRelative, relative.data);

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

        if (!StringIsNullOrEmpty(parsedCurrent.query)) {
            pos += snprintf(&resData[pos], MAX_URL_SIZE - pos, "?%s", parsedCurrent.query);
        }

        if (!StringIsNullOrEmpty(parsedCurrent.fragment)) {
            pos += snprintf(&resData[pos], MAX_URL_SIZE - pos, "#%s", parsedCurrent.fragment);
        }

        return (String) {
            .data = resData,
            .len = strlen(resData),
        };
    }

    bool IsURLInteresting(String current, String newURL) {
        String currentCopy = MakeStringCopy(current.data);
        String newUrlCopy = MakeStringCopy(newURL.data);

        yuarel parsedCurrent;
        yuarel parsedNew;
        yuarel_parse(&parsedCurrent, currentCopy.data);
        yuarel_parse(&parsedNew, newUrlCopy.data);

        if (parsedNew.scheme != NULL
         && strcmp(parsedNew.scheme, "https") 
         && strcmp(parsedNew.scheme, "http")) return false; // Probably a link for mail or printing

        // Probably a link to the same page
        if ((parsedCurrent.path != NULL && (parsedNew.path == NULL || !strcmp(parsedCurrent.path, parsedNew.path)))
         && (parsedCurrent.host != NULL && (parsedNew.host == NULL || !strcmp(parsedCurrent.host, parsedNew.host)))) return false; 

        return true; // Otherwise, is seems okay!
    }
}


#endif
