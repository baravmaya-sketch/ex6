#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

//gets string from user
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
int getIntInternal(const char* prompt, int* outVal) {
    if (prompt) printf("%s", prompt);

    int temp;
    // scanf returns 1 if it successfully read one integer
    if (scanf("%d", &temp) != 1) {
        return 0; // EOF or Bad Input
    }

    // Clear buffer
    int ch = getchar();
    while (ch != '\n' && ch != EOF) {
        ch = getchar();
    }

    // Check if EOF happened during clear
    if (ch == EOF) return 0;

    *outVal = temp; // Set the value
    return 1; // Success
}