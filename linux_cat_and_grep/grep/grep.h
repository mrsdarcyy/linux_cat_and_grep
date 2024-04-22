#ifndef GREP_H
#define GREP_H

#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  bool e;
  bool v;
  bool i;
  bool l;
  bool c;
  bool n;
  bool h;
  bool s;
  bool f;
  bool o;
} marks_for_grep;

regex_t *finished_sequence(regex_t *sequence_str, char **templates,
                           int string_counter, int cmarks, int *regerr_flag);
char **sequence_templates(char **templates, char *template_line,
                          int *string_counter, int *reg_getting_status);
void get_regex_from_file(char ***templates, char *regex_file_name,
                         int *string_counter, int *reg_getting_status);
int syntax_analyzer(int argc, char *argv[], marks_for_grep *marks,
                    char ***templates, int *string_counter);
void print_line(marks_for_grep *marks, char *line, int *string_index);
void print_line_with_file_name(char *actual_filename, marks_for_grep *marks,
                               char *line, int *string_index);
void property_output(int argc, char *actual_filename, marks_for_grep *marks,
                     int matches_count, int required_file_name);
void other_optoins_to_invert(int argc, char *actual_filename,
                             marks_for_grep *marks, char *line,
                             int *string_index, int *matches_count,
                             int *required_file_name);
void run_process(int argc, char *argv[], regex_t **sequence_str,
                 char **templates, int string_counter, marks_for_grep *marks);
void freeArrays(char **templates, regex_t *sequence_str, int string_counter);
void usage_error();

#endif
