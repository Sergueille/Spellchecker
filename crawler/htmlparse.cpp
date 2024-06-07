

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "htmlparse.hpp"

using namespace Util;
namespace HTMLParse {

constexpr int MAX_WORD_LEN = 20; // Prevent unexpectedly long word from being parsed (such a a js expression, a token...)
constexpr int MAX_READ_LEN = 2000; // For URLs

enum ParseMode {
    Normal, InTag, TagStart, InQuotes
};

PageResults ParseHTML(String text, String currentURL) {
    ParseMode mode = Normal;

    std::vector<String> words = std::vector<String>();
    std::vector<String> urls = std::vector<String>();

    // FIXME: fails to ignore all tag if multiple tags with same name are nested (not very likely, and not that bad)
    bool isInIgnoredTag = false; // If true, words are ignored
    char* ignoredTagName = NULL;

    bool isInATag = false; // If true, will look for href

    Status s = (Status) {
        .text = text,
        .position = 0,
    };
    while (!IsEOF(&s)) {
        char current = s.text.data[s.position];

        if (mode == Normal) {
            if (TryRead(&s, (char*)"<")) { // Check for tag opening
                mode = TagStart;
            }
            else if (isInIgnoredTag) { // skip ignored tag content
                ReadUntil(&s, '<', false);
            }
            else {
                char ent = LookForHTMLEntity(&s);
                if (ent != '\0') { // Handle HTML entities such as '&lt;' for '<'
                    if (ent != ' ') {
                        char* res = (char*)malloc(2 * sizeof(char));
                        res[0] = ent;
                        res[1] = '\0';

                        words.push_back((String) {
                            .data = res,
                            .len = 1,
                        });
                    }
                }
                else {
                    String w = ReadWord(&s);
                    words.push_back(w);
                    //printf("> %s\n", w.data);
                }
            }
        }
        else if (mode == InTag || mode == TagStart) {
            if (mode == TagStart) {
                if (isInIgnoredTag) {
                    if (TryRead(&s, (char*)"/") && TryRead(&s, ignoredTagName)) {
                        isInIgnoredTag = false;
                    }
                    isInATag = false;
                }
                else {
                    isInIgnoredTag = LookForIgnoredTag(&s, &ignoredTagName);
                    isInATag = TryRead(&s, (char*)"a");
                }
            }

            while (!IsEOF(&s)) {
                current = s.text.data[s.position];
                if (current == '>') {
                    mode = Normal;
                    s.position++;
                    break;
                }
                else if (current == '"') {
                    mode = InQuotes;
                    s.position++;
                    break;
                }
                else if (isInATag && TryRead(&s, (char*)"href=")) {
                    // Found href in a tag!

                    // Make sure there is a quote
                    if (TryRead(&s, (char*)"\"")) {
                        String url = ReadUntil(&s, '"', true);

                        if (IsURLInteresting(currentURL, url)) {
                            urls.push_back(url);
                        }
                    }
                }

                s.position++;
            }
        }
        else if (mode == InQuotes) {
            ReadUntil(&s, '"', false);
            s.position++;
            mode = InTag;
        }
    }

    String* wordsArr = Util::VectorToPtr(&words);
    String* urlsArr = Util::VectorToPtr(&urls);

    return (PageResults) {
        .words = wordsArr,
        .wordCount = (int)words.size(),
        .urls = urlsArr,
        .urlCount = (int)urls.size(),
    };
}

bool IsEOF(Status* s) {
    return (size_t)s->position >= s->text.len;
}

String ReadWord(Status* s) {
    // Skip spaces
    while (!IsEOF(s) && std::isspace(s->text.data[s->position])) {
        s->position++;
    }
    
    if (IsEOF(s)) {
        return (String) {
            .data = NULL,
            .len = 0,
        };
    }

    char* wordData = (char*)malloc((MAX_WORD_LEN + 2) * sizeof(char));

    // Read actual word
    size_t wordLen = 0;

    // Letters
    if (std::isalnum(s->text.data[s->position])) {
        while (wordLen < MAX_WORD_LEN && !IsEOF(s)) {
            if (std::isalnum(s->text.data[s->position])) {
                wordData[wordLen] = s->text.data[s->position];
                wordLen++;
                s->position++;
            }
            else if (IsHalfChar(s->text.data[s->position])) {
                // Get two (half) chars!
                // printf("? %c\n", s->text.data[s->position]);
                wordData[wordLen] = s->text.data[s->position];
                wordLen++;
                s->position++;

                if (IsEOF(s)) break; // Shouldn't happen, except if invalid character at very end of file

                wordData[wordLen] = s->text.data[s->position];
                wordLen++;
                s->position++;
            }
            else break;
        }
    }
    else { // Other symbol: just read one
        wordData[0] = s->text.data[s->position];
        wordLen = 1;
        s->position++;
    }

    wordData[wordLen] = '\0';

    return (String) {
        .data = wordData,
        .len = wordLen
    };
}

String ReadUntil(Status* s, char stopChar, bool storeResult) {
    char* data;
    if (storeResult) {
        data = (char*)malloc((MAX_READ_LEN + 1) * sizeof(char));
    }

    size_t i = 0;
    while (i < MAX_READ_LEN && !IsEOF(s) && s->text.data[s->position] != stopChar) {
        if (storeResult) {
            char entity = LookForHTMLEntity(s);
            if (entity) {
                data[i] = entity;
                s->position--;
            }
            else {
                data[i] = s->text.data[s->position];
            }

            i++;
        }

        s->position++;
    }
    if (storeResult) data[i] = '\0';

    if (storeResult) {
        return (String) {
            .data = data,
            .len = i,
        };
    }
    else {
        return GetEmptyString();
    }
}

bool TryRead(Status* s, char* compare) {
    int i = 0;

    while (true) {
        if (compare[i] == '\0') return true;

        if (IsEOF(s)) return false;

        if (std::isspace(s->text.data[s->position])) {
            s->position++;
        }
        else if (s->text.data[s->position] != compare[i]) {
            return false;
        }
        else {
            s->position++;
            i++;
        }
    }
}

char LookForHTMLEntity(Status* s) {
    int initialPosition = s->position;

    if (s->text.data[s->position] == '&') {
        s->position++;

        int wordPos = s->position;

        const int ENTITY_COUNT = 12;
        const char* words[ENTITY_COUNT] = {
            "nbsp;",
            "lt;",
            "gt;",
            "amp;",
            "quot;",
            "apos;",
            "cent;",
            "pound;",
            "yen;",
            "euro;",
            "copy;",
            "reg;",
        };

        char chars[ENTITY_COUNT] = {
            ' ',
            '<',
            '>',
            '&',
            '"',
            '\'',
            '$',
            '$',
            '$',
            '$',
            '?',
            '?',
        };

        for (int i = 0; i < ENTITY_COUNT; i++) {
            bool ok = TryRead(s, (char*)words[i]);

            if (ok) {
                return chars[i];
            }
            else {
                s->position = wordPos;
            }
        }

        s->position = initialPosition;
        return '\0';
    }
    else return '\0';
}

char LookForIgnoredTag(Status* s, char** outName) {
    const int TAG_COUNT = 4;
    const char* words[TAG_COUNT] = {
        "script",
        "style",
        "header",
        "footer",
    };

    int startPos = s->position;

    for (int i = 0; i < TAG_COUNT; i++) {
        bool ok = TryRead(s, (char*)words[i]);

        if (ok) {
            *outName = (char*)words[i];
            return true;
        }
        else {
            s->position = startPos;
        }
    }

    s->position = startPos;
    return false;
}

bool IsHalfChar(char c) {
    return static_cast<unsigned char>(c) > 127;
}

void FreePageResults(PageResults r) {
    for (int i = 0; i < r.wordCount; i++) {
        Util::FreeString(r.words[i]);
    }

    for (int i = 0; i < r.urlCount; i++) {
        Util::FreeString(r.urls[i]);
    }
}

}
