
#include <cstdlib>
#include <cstring>
#include <vector>

// Utility things
namespace Util {
    struct String {
        char* data;
        size_t len;
    };

    // Makes a string, calls strlen, uses the raw data
    String MakeString(char* data);
    // Makes a string, calls strlen, copies the raw data
    String MakeStringCopy(char* data);
    
    void FreeString(String s);
    String GetEmptyString();
    
    template<typename T>
    T* VectorToPtr(std::vector<T>* vect);

    bool StringIsNullOrEmpty(char* s);

    // Used to navigate from "current" to "relative". 
    String GetRelativeURL(String current, String relative);
}

