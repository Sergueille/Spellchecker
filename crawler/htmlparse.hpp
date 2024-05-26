
#include "util.cpp"

using namespace Util;

// Parses an html file into sentences
namespace HTMLParse {

struct PageResults {
    String* words;
    int wordCount;
    String* urls;
    int urlCount;
};

struct Status {
    String text;
    int position;
};

// Reads the page! Ignores some tags such as scripts, footers (to prevent the same text from being included multiple times)...
PageResults ParseHTML(String text);

// Returns true if s->position is on the end of the text
bool IsEOF(Status* s);

// Reads a word: skip spaces returns the first char, then all alphanumerical chars after, then stops. 
String ReadWord(Status* s);

// Reads until stopChar (or EOF) is found. Places the position on stopChar. If storeResult is true, return a string with the content
String ReadUntil(Status* s, char stopChar, bool storeResult);

// Try to find compare, but ignores whitespace from source text. Places the position after the last char of compare or on the first different char.
bool TryRead(Status* s, char* compare);

// Return corresponding char if HTML entity found, \0 otherwise. Places the cursor after it if found, and does not move the cursor otherwise 
char LookForHTMLEntity(Status* s);

// Read a tag name, returns true if tag should be ignored ("script", "style", ...)
char LookForIgnoredTag(Status* s, char** outName);

// Returns true if the character is unknown ASCII, so it's probably half of an UTF8 character
bool IsHalfChar(char c);

void FreePageResults(PageResults r);

}
