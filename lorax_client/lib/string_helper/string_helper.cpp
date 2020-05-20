#include <stdio.h>
#include <stdlib.h>

const char *two_digits(int number, char *number_string) {
  if (number < 10) {
    sprintf(number_string, "0%d", number);
  } else {
    sprintf(number_string, "%d", number);
  }
  return number_string;
}

void substring(char *source_string, char *sub_string, int sub_string_start, int sub_string_length) {
    int char_index = 0;

    while (char_index < sub_string_length) {
        sub_string[char_index] = source_string[sub_string_start + char_index - 1];
        char_index++;
    }
    sub_string[char_index] = '\0';
}