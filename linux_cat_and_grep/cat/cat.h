#ifndef CAT_H
#define CAT_H
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOLLAR_SYMBOL '$'

typedef struct {
  int mistake;
  bool b;
  bool e;
  bool n;
  bool s;
  bool t;
  bool v;
} marks_for_cat;

int flags_search(marks_for_cat *p, int argc, char *argv[], int *fquantity,
                 char ***files);
void missing_file_issue(char **files, int i);
int text_read(char *str, FILE *fp);
int run_text_processing(marks_for_cat p, int fquantity, char **files);
void mark_t_handling(char *actual_string, int *quantity);
void mark_e_handling(char *actual_string, int *quantity);
void mark_v_handling(char *actual_string, int *quantity);
int text_output(marks_for_cat p, int *last_line_space, int *string_index,
                int *final_character_in_string, char *str, int quantity,
                int space);

#endif