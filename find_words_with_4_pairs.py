
# Find words with 4 time the same pair of letter

# None found in french_words.txt
# In english_words.txt:
# ethylenediaminetetraacetate
# ethylenediaminetetraacetates
# expressionlessnesses
# nonconfrontation
# nonconfrontational
# nonconfrontations
# resistlessnesses
# restlessnesses
# stresslessnesses


words = open("english_words.txt").readlines()

def id(c):
    res = ord(c) - ord('a')
    if res >= 0 and res < 26:
        return res
    else:
        return 26 # non ascii code

for word in words:
    word = word.lower()
    if len(word) > 2:
        table = [[0 for _ in range(27)] for _ in range(27)]
        for i in range(len(word)-2):
            table[id(word[i])][id(word[i + 1])] += 1
            if table[id(word[i])][id(word[i + 1])] == 4:
                print(word + " " + word[i:i+2])
                break
        

