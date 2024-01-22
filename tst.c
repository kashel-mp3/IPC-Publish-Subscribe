#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

char* parseInput(char* input, int* parsedInt) {
    // Find the first space in the input
    char* spacePosition = strchr(input, ' ');
    char* output;
    if (spacePosition != NULL) {
        // Calculate the length of the word
        size_t wordLength = spacePosition - input;

        // Allocate memory for the parsed word and copy it
        output = (char*)malloc(wordLength + 1);
        strncpy(output, input, wordLength);
        (output)[wordLength] = '\0'; // Null-terminate the parsed word

        // Check if there is an integer value after the space
        if (*(spacePosition + 1) != '\0') {
            *parsedInt = atoi(spacePosition + 1);
        } else {
            *parsedInt = -1; // Set to 0 if no integer is present
        }
    } else {
        output = input;// No space found, the entire input is the word
        *parsedInt = -1; // Set to 0 as there is no integer
    }
    return output;
}

int main(void) {
	char* essa = "ololololool 231";
	int e;
	char* output = parseInput(essa, &e);
	printf("%d, %s", e, output);
	return 0;
}