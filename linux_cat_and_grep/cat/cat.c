#include "cat.h"

int main(int argc, char *argv[]) {
  marks_for_cat marks_for_cat;
  char **files;
  int files_quantity = 0;

  flags_search(&marks_for_cat, argc, argv, &files_quantity, &files);

  if (marks_for_cat.mistake == 0)
    run_text_processing(marks_for_cat, files_quantity, files);
  else {
    printf("Флаги введены некорректно.");
  }

  for (int i = 0; i < files_quantity; i++) free(files[i]);
  free(files);
  return 0;
}

int flags_search(marks_for_cat *marks_for_cat, int argc, char *argv[],
                 int *files_quantity, char ***files) {
  marks_for_cat->mistake = 0;
  marks_for_cat->b = 0;
  marks_for_cat->e = 0;
  marks_for_cat->n = 0;
  marks_for_cat->s = 0;
  marks_for_cat->t = 0;
  marks_for_cat->v = 0;
  char short_opt[] = "beEnstTv";

  static struct option long_opt[] = {
      {"number-nonblank", no_argument, NULL, 'b'},
      {"number", no_argument, NULL, 'n'},
      {"squeeze-blank", no_argument, NULL, 's'},
      {NULL, 0, NULL, 0}};

  int c;
  while ((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
    switch (c) {
      case 'b':
        marks_for_cat->b = 1;
        break;
      case 'e':
        marks_for_cat->e = 1;
        marks_for_cat->v = 1;
        break;
      case 'E':
        marks_for_cat->e = 1;
        break;
      case 'n':
        marks_for_cat->n = 1;
        break;
      case 's':
        marks_for_cat->s = 1;
        break;
      case 't':
        marks_for_cat->t = 1;
        marks_for_cat->v = 1;
        break;
      case 'T':
        marks_for_cat->t = 1;
        break;
      case 'v':
        marks_for_cat->v = 1;
        break;
      case '?':
        marks_for_cat->mistake++;
    }
  }

  if (optind < argc) {
    *files_quantity = argc - optind;
    *files = malloc((*files_quantity) * sizeof(char *));

    int i = 0;
    do {
      (*files)[i] = malloc((strlen(argv[optind]) + 1) * sizeof(char));
      strcpy((*files)[i], argv[optind]);
      i++;
    } while (++optind < argc);
  } else {
    *files_quantity = 0;
  }
  return 0;
}

void mark_e_handling(char *actual_string, int *quantity) {
  actual_string[*quantity - 1] = DOLLAR_SYMBOL;
  actual_string[*quantity] = '\n';
  actual_string[*quantity + 1] = 0;
  (*quantity)++;
}

void mark_v_handling(char *actual_string, int *quantity) {
  for (int i = 0; i < *quantity; i++) {
    if ((actual_string[i] >= 0) && (actual_string[i] < 32) &&
        (actual_string[i] != 9) && (actual_string[i] != 10)) {
      for (int j = *quantity + 1; j > i + 1; j--)
        actual_string[j] = actual_string[j - 1];
      actual_string[i + 1] = actual_string[i] + 64;
      actual_string[i] = '^';
      (*quantity)++;
    }
    if (actual_string[i] == 127) {
      for (int j = (*quantity) + 1; j > i + 1; j--)
        actual_string[j] = actual_string[j - 1];
      actual_string[i + 1] = '?';
      actual_string[i] = '^';
      (*quantity)++;
    }
    if (((unsigned char)actual_string[i] >= 128) &&
        ((unsigned char)actual_string[i] < 160)) {
      for (int j = (*quantity) + 3; j > i + 1; j--)
        actual_string[j] = actual_string[j - 3];
      actual_string[i + 3] = actual_string[i] - 64;
      actual_string[i + 2] = '^';
      actual_string[i + 1] = '-';
      actual_string[i] = 'M';
      (*quantity) += 3;
    }
#ifdef __APPLE__
    if ((unsigned char)actual_string[i] >= 160) {
      for (int j = (*quantity) + 2; j > i + 1; j--)
        actual_string[j] = actual_string[j - 2];
      actual_string[i + 2] = actual_string[i] - 128;
      actual_string[i + 1] = '-';
      actual_string[i] = 'M';
      (*quantity) += 2;
    }
#endif
  }
}

void mark_t_handling(char *actual_string, int *quantity) {
  for (int i = 0; i < *quantity; i++) {
    if (actual_string[i] == '\t') {
      for (int j = *quantity + 1; j > i + 1; j--)
        actual_string[j] = actual_string[j - 1];
      actual_string[i] = '^';
      actual_string[i + 1] = 'I';
      (*quantity)++;
    }
  }
}

void missing_file_issue(char **files, int i) {
  if (i != 0) printf("\n");
  printf("cat: %s: No such file or directory\n", files[i]);
}

int run_text_processing(marks_for_cat marks_for_cat, int files_quantity,
                        char **files) {
  FILE *file;
  char actual_string[4096];
  int string_index = 0;
  int last_line_space = 0;
  int final_character_in_string = 1;

  for (int i = 0; i < files_quantity; i++) {
    file = fopen(files[i], "r");

    if (file == NULL) {
      missing_file_issue(files, i);
    } else {
      int quantity;
      while ((quantity = text_read(actual_string, file)) != -1) {
        int space = 0;
        if (strcmp(actual_string, "\n") == 0) space = 1;
        if (marks_for_cat.t) mark_t_handling(actual_string, &quantity);
        if ((marks_for_cat.e) && (actual_string[quantity - 1] == '\n'))
          mark_e_handling(actual_string, &quantity);
        if (marks_for_cat.v) mark_v_handling(actual_string, &quantity);

        text_output(marks_for_cat, &last_line_space, &string_index,
                    &final_character_in_string, actual_string, quantity, space);
      }
      fclose(file);
#ifdef __APPLE__
      string_index = 0;
#endif
    }
  }
  return 0;
}

int text_read(char *str, FILE *file) {
  int i = 0;
  while (1) {
    str[i] = fgetc(file);
    if (feof(file) != 0) break;
    i++;
    if (str[i - 1] == '\n') break;
  }
  str[i] = '\0';
  if (i == 0) i = -1;
  return i;
}

int text_output(marks_for_cat marks_for_cat, int *last_line_space,
                int *string_index, int *final_character_in_string, char *str,
                int quantity, int space) {
  if (!((marks_for_cat.s) && (*last_line_space == 1) && (space == 1))) {
#ifndef __APPLE__
    if (*final_character_in_string == 1) {
#endif
      if (((marks_for_cat.n) && (!marks_for_cat.b)) ||
          ((marks_for_cat.b) && (space == 0))) {
        (*string_index)++;
        printf("%6d\t", *string_index);
      }
#ifndef __APPLE__
    }
#endif
    for (int i = 0; i < quantity; i++) {
      fputc(str[i], stdout);
    }

    *final_character_in_string = (str[quantity - 1] == '\n') ? 1 : 0;
    *last_line_space = (space == 1) ? 1 : 0;
  }
  return 0;
}