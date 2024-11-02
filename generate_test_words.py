# Generate words with errors (0 to 4 errors)

import random

WORD_COUNT = 100

def apply_random_err(word):
    rand = random.randint(0, 3)

    if rand == 0: # add
        pos = random.randint(0, len(word))
        rand_char = chr(random.randint(ord('a'), ord('z')))

        return word[:pos] + rand_char + word[pos:]
    elif rand == 1: # remove
        pos = random.randint(0, len(word) - 1)
        return word[:pos] + word[pos+1:]
    elif rand == 2: # exchange
        pos = random.randint(0, len(word) - 1)
        rand_char = chr(random.randint(ord('a'), ord('z')))

        return word[:pos] + rand_char + word[pos+1:]
    else: # swap
        pos = random.randint(0, len(word) - 2)
        return word[:pos] + word[pos + 1] + word[pos] + word[pos+2:]


words = open("french_words.txt").readlines()
output = open("test_words.txt", "w")

for _ in range(WORD_COUNT):
    rand = random.randint(0, len(words) - 1)
    word = words[rand][:-1]

    for err in range(0, 5):
        cpy = word

        for _ in range(err):
            cpy = apply_random_err(cpy)

        output.write(cpy + "\n")
