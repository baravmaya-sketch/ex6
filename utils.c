#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

char* getString(const char* prompt) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    // Initial allocation for the first character (or null terminator placeholder)
    char* str = (char*)malloc(sizeof(char));
    if (str == NULL) {
        exit(1);
    }

    int len = 0;
    char ch;

    // Scan the first character
    if (scanf(" %c", &ch) == 1) {
        str[len] = ch;
        len++;
    }
    else {
        // If scanning failed (e.g. EOF), free memory and return NULL
        free(str);
        return NULL;
    }

    // Loop to read the rest of the line char by char
    while (scanf("%c", &ch) == 1 && ch != '\n') {
        // Reallocate exactly one byte more for the new character
        char* temp = (char*)realloc(str, (len + 1) * sizeof(char));
        if (temp == NULL) {
            free(str);
            exit(1);
        }
        str = temp;

        str[len] = ch;
        len++;
    }

    // Reallocate one last time for the null terminator
    char* temp = (char*)realloc(str, (len + 1) * sizeof(char));
    if (temp == NULL) {
        free(str);
        exit(1);
    }
    str = temp;
    str[len] = '\0';

    return str;
}

//gets integer input from user
int getInt(const char* prompt) {
    int value;

    // Display the prompt to the user if it's not NULL
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    // Loop until a valid integer is successfully read.
    // scanf returns the number of items converted, so we wait for 1.
    while (scanf("%d", &value) != 1) {
        // Input was not an integer (e.g., user typed characters)
        printf("Invalid input. Please enter a number: ");

        // Clear the input buffer: consume characters until a newline is found
        while (getchar() != '\n');
    }

    // Clear the trailing newline character (Enter key) left in the buffer.
    // This prevents skipping future input prompts.
    char ch = getchar();
    while (ch != '\n' && ch != EOF) {
        ch = getchar();
    }

    return value;
}