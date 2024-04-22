#include "grep.h"

regex_t *finished_sequence(regex_t *sequence_str, char **templates,
                           int string_counter, int cmarks, int *regerr_flag) {
  sequence_str = (regex_t *)malloc(string_counter * sizeof(regex_t));

  if (sequence_str) {
    for (int i = 0; i < string_counter; i++) {
      *regerr_flag = regcomp(sequence_str + i, templates[i], cmarks);
    }
  } else {
    *regerr_flag = -1;
  }
  return sequence_str;
}

char **sequence_templates(char **templates, char *new_line, int *string_counter,
                          int *reg_getting_status) {
  char **temp =
      (char **)realloc(templates, (*string_counter + 1) * sizeof(char *));

  if (temp) {
    templates = temp;
    templates[*string_counter] = (char *)malloc(strlen(new_line) + 1);
    if (templates[*string_counter]) {
      strcpy(templates[*string_counter], new_line);
      *string_counter = *string_counter + 1;
    } else {
      *reg_getting_status = -1;
    }
  } else {
    *reg_getting_status = -1;
  }

  return templates;
}

void get_regex_from_file(char ***templates, char *regex_file_name,
                         int *string_counter, int *reg_getting_status) {
  FILE *this_file_with_regex = fopen(regex_file_name, "r");
  char str[2048];

  if (this_file_with_regex != NULL) {
    while (fgets(str, 2048, this_file_with_regex) != NULL) {
      if (strchr(str, '\n')) *(strchr(str, '\n')) = '\0';
      *templates = sequence_templates(*templates, str, string_counter,
                                      reg_getting_status);
    }
    fclose(this_file_with_regex);
  } else {
    *reg_getting_status = -1;
  }
}

int syntax_analyzer(int argc, char *argv[], marks_for_grep *marks,
                    char ***templates, int *string_counter) {
  int currenr_flag;
  int err = 0;

  while ((currenr_flag = getopt_long(argc, argv, "e:f:ivcnlhso", 0, NULL)) !=
         -1) {
    switch (currenr_flag) {
      case 'e':
        marks->e = 1;
        *templates =
            sequence_templates(*templates, optarg, string_counter, &err);
        if (err == -1) {
          fprintf(stderr, "\nНепредвиденная ошибка!\n");
        }
        break;
      case 'i':
        marks->i = 1;
        break;
      case 'v':
        marks->v = 1;
        break;
      case 'c':
        marks->c = 1;
        break;
      case 'n':
        marks->n = 1;
        break;
      case 'l':
        marks->l = 1;
        break;
      case 'h':
        marks->h = 1;
        break;
      case 's':
        marks->s = 1;
        break;
      case 'f':
        marks->f = 1;
        get_regex_from_file(templates, optarg, string_counter, &err);
        if (err == -1) {
          fprintf(stderr, "%s: %s: No such file or directory", argv[0], optarg);
        }
        break;
      case 'o':
        marks->o = 1;
        break;
      default:
        usage_error();
        err = -1;
    }
  }
  return err;
}

void print_line(marks_for_grep *marks, char *line, int *string_index) {
  if (marks->n) {
    printf("%d:%s\n", *string_index, line);
  } else {
    printf("%s\n", line);
  }
}

void print_line_with_file_name(char *actual_filename, marks_for_grep *marks,
                               char *line, int *string_index) {
  if (marks->n) {
    printf("%s:%d:%s\n", actual_filename, *string_index, line);
  } else {
    printf("%s:%s\n", actual_filename, line);
  }
}

void other_optoins_to_invert(int argc, char *actual_filename,
                             marks_for_grep *marks, char *line,
                             int *string_index, int *matches_count,
                             int *required_file_name) {
  if ((!marks->c && !marks->l && !marks->o) ||
      (!marks->c && !marks->l && marks->o && marks->v)) {
    if ((argc - optind == 1) || marks->h) {
      print_line(marks, line, string_index);
    } else {
      print_line_with_file_name(actual_filename, marks, line, string_index);
    }
  } else if (marks->c && marks->l) {
    *matches_count = 1;
    *required_file_name = 1;
  } else if (marks->c && !marks->l) {
    *matches_count = *matches_count + 1;
  } else if (!marks->c && marks->l) {
    *required_file_name = 1;
  }
}

void property_output(int argc, char *actual_filename, marks_for_grep *marks,
                     int matches_count, int required_file_name) {
  if ((argc - optind == 1) || marks->h) {
    if (marks->c && !marks->l) {
      printf("%d\n", matches_count);
    } else {
      if (marks->c && marks->l) {
        printf("%d\n", matches_count);
        if (required_file_name) {
          printf("%s\n", actual_filename);
        }
      } else {
        if (!marks->c && marks->l && required_file_name) {
          printf("%s\n", actual_filename);
        }
      }
    }
  }

  else {
    if (marks->c && !marks->l) {
      printf("%s:%d\n", actual_filename, matches_count);
    } else {
      if (marks->c && marks->l) {
        printf("%s:%d\n", actual_filename, matches_count);
        if (required_file_name) {
          printf("%s\n", actual_filename);
        }
      } else {
        if (!marks->c && marks->l && required_file_name) {
          printf("%s\n", actual_filename);
        }
      }
    }
  }
}

void run_process(int argc, char *argv[], regex_t **sequence_str,
                 char **templates, int string_counter, marks_for_grep *marks) {
  char current_line_from_file[2048];
  int error = 0, matches_count = 0, string_index = 0, required_file_name = 0,
      shift = 0;
  FILE *current_file;
  regmatch_t match;
  int regerr_flag = 0;

  *sequence_str = finished_sequence(
      *sequence_str, templates, string_counter,
      marks->i ? REG_ICASE | REG_EXTENDED : REG_EXTENDED, &regerr_flag);

  if (regerr_flag != 0) {
    printf("Regex error");
    return;
  }

  for (int i = optind; i < argc; i++) {
    current_file = fopen(argv[i], "r");
    if (current_file != NULL) {
      required_file_name = 0;
      string_index = 0;
      matches_count = 0;

      while (fgets(current_line_from_file, 2048, current_file) != NULL) {
        int lines_similar_flag = 0;
        shift = 0;

        if (strchr(current_line_from_file, '\n'))
          *(strchr(current_line_from_file, '\n')) = '\0';

        string_index++;
        for (int j = 0; j < string_counter; j++) {
          if ((error = regexec(*sequence_str + j, current_line_from_file, 1,
                               &match, 0)) == 0) {
            lines_similar_flag = 1;
            if (marks->o) {
              while ((error = regexec(*sequence_str + j,
                                      current_line_from_file + shift, 1, &match,
                                      0)) == 0) {
                if (!marks->l && !marks->c && !marks->v) {
                  if (((argc - optind > 1) && !marks->h) && (shift == 0)) {
                    printf("%s:", argv[i]);
                  }
                  if (marks->n && (shift == 0)) {
                    printf("%d:", string_index);
                  }
                  printf("%.*s\n", (int)(match.rm_eo - match.rm_so),
                         (current_line_from_file + shift + match.rm_so));
                }
                shift += match.rm_eo;
              }
            }
          }
        }

        if ((marks->v && !lines_similar_flag) ||
            ((!marks->v && lines_similar_flag))) {
          other_optoins_to_invert(argc, argv[i], marks, current_line_from_file,
                                  &string_index, &matches_count,
                                  &required_file_name);
        }
      }
      property_output(argc, argv[i], marks, matches_count, required_file_name);
      fclose(current_file);
    } else if (!marks->s) {
      fprintf(stderr, "%s: %s: No such file or directory\n", argv[0], argv[i]);
    }
  }
}

void freeArrays(char **templates, regex_t *sequence_str, int string_counter) {
  if (templates != NULL) {
    for (int i = 0; i < string_counter; i++) {
      free(templates[i]);
    }
    free(templates);
  }
  if (sequence_str != NULL) {
    for (int i = 0; i < string_counter; i++) {
      regfree(sequence_str + i);
    }
    free(sequence_str);
  }
}

void usage_error() {
  fprintf(stderr,
          "usage:\ngrep [-ivclnhso] [file ...]\nor grep [-options] -e [regex] "
          "[file ...]\nor grep [-options] -f [file_with_regex] [file ...]");
}

int main(int argc, char *argv[]) {
  marks_for_grep marks = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char **templates = NULL;
  int err = 0;
  int string_counter = 0;
  int reg_getting_status = 0;
  regex_t *sequence_str = NULL;

  if ((err = syntax_analyzer(argc, argv, &marks, &templates,
                             &string_counter)) == 0) {
    if ((!marks.e) && (!marks.f) && (argc != optind)) {
      templates = sequence_templates(templates, argv[optind], &string_counter,
                                     &reg_getting_status);
      optind++;
    }
    if (argc == optind) {
      usage_error();
    } else {
      run_process(argc, argv, &sequence_str, templates, string_counter, &marks);
    }
  }

  freeArrays(templates, sequence_str, string_counter);

  return 0;
}
