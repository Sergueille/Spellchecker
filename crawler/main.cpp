
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "util.cpp"
#include "getfile.cpp"
#include "htmlparse.cpp"

int main() {
    // Seeded for tests
    char* entryPoint = (char*)"https://en.wikipedia.org/wiki/French_fries";
    int seed = time(NULL);

    printf("\nCrawler is ready, make sure you have access to the internet!\n");
    printf("To stop, hold ^C for at least 2 seconds\n\n");
    printf("Words,\tLinks,\tURL\n");

    srand(seed);

    String currentURL = Util::MakeStringCopy(entryPoint);
    String previousURL = Util::MakeStringCopy((char*)"");
    while (true) {
        printf("FETCHING %.100s", currentURL.data);
        fflush(stdout);

        char* txt = CallCurl(currentURL.data);

        printf("\rPARSING %.100s", currentURL.data);
        fflush(stdout);

        String s = Util::MakeString(txt);
        HTMLParse::PageResults res = HTMLParse::ParseHTML(s);

        printf("\r                                                                   "); // Make sure the line is cleaned properly
        printf("\r%d,\t%d,\t%.100s\n", res.wordCount, res.urlCount, currentURL.data);

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
    }
}
