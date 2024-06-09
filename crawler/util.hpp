
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

// Utility things
namespace Util {
    char* CopyStringBuffer(std::string source);

    template<typename T>
    T* VectorToBuffer(std::vector<T>* vect);

    bool StringIsNullOrEmpty(char* s);

    // Used to navigate from "current" to "relative". 
    std::string GetRelativeURL(std::string current, std::string relative);

    // Should navigate to URL?
    bool IsURLInteresting(std::string current, std::string newURL);
}

