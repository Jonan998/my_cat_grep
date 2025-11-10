#include <getopt.h>
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define PATTERNS_INIT_CAPACITY 10

typedef struct {
  int e;  // шаблон для поиска
  int i;  // игнорирование регистра
  int v;  // инвертирование поиска
  int c;  // вывод количества совпадений
  int l;  // вывод имен файлов с совпадениями
  int n;  // вывод номеров строк
  int h;  // скрытие имен файлов
  int s;  // подавление ошибок
  int f;  // чтение шаблонов из файла
  int o;  // вывод только совпадающих частей
  int multiple_files;  // флаг множественных файлов
} flags_t;

typedef struct {
  char **patterns;  // массив шаблонов
  int count;        // количество шаблонов
  int capacity;     // емкость массива
} patterns_t;

void patterns_init(patterns_t *p);
void patterns_add(patterns_t *p, const char *pattern);
void patterns_free(patterns_t *p);
char *read_line(FILE *file);
int patterns_load_from_file(patterns_t *p, const char *filename,
                            int suppress_errors);
int parse_arguments(int argc, char *argv[], flags_t *flags,
                    patterns_t *patterns);
regex_t *compile_patterns(patterns_t *patterns, flags_t flags, int *success);
void free_regexes(regex_t *regexes, int count);
int has_match(const char *line, regex_t *regexes, int regex_count,
              flags_t flags);
void print_only_matches(const char *line, regex_t *regexes, int regex_count,
                        const char *filename, int line_num, flags_t flags);
void print_full_line(const char *line, const char *filename, int line_num,
                     flags_t flags);
void print_count(const char *filename, flags_t flags, int count);
void print_final_results(const char *filename, flags_t flags, int match_count);
void print_line_match(const char *line, const char *filename, int line_num,
                      flags_t flags, regex_t *regexes, int regex_count);
int process_line(const char *line, const char *filename, int line_num,
                 flags_t flags, regex_t *regexes, int regex_count,
                 int *match_count);
void remove_newline(char *line);
void cleanup_file(FILE *file);
FILE *open_file(const char *filename, flags_t flags);
void process_file_content(FILE *file, const char *filename, flags_t flags,
                          regex_t *regexes, int regex_count);
void process_file(const char *filename, flags_t flags, regex_t *regexes,
                  int regex_count);
char *custom_strdup(const char *s);

char *custom_strdup(const char *s) {
  if (!s) return NULL;
  size_t len = strlen(s) + 1;
  char *dup = malloc(len);
  if (dup) {
    memcpy(dup, s, len);
  }
  return dup;
}

void patterns_init(patterns_t *p) {
  p->patterns = malloc(PATTERNS_INIT_CAPACITY * sizeof(char *));
  p->count = 0;
  p->capacity = PATTERNS_INIT_CAPACITY;
}

void patterns_add(patterns_t *p, const char *pattern) {
  if (p->count >= p->capacity) {
    p->capacity *= 2;
    p->patterns = realloc(p->patterns, p->capacity * sizeof(char *));
  }
  p->patterns[p->count] = custom_strdup(pattern);
  p->count++;
}

void patterns_free(patterns_t *p) {
  for (int i = 0; i < p->count; i++) {
    free(p->patterns[i]);
  }
  free(p->patterns);
  p->patterns = NULL;
  p->count = p->capacity = 0;
}

char *read_line(FILE *file) {
  if (!file) return NULL;
  char buffer[BUFFER_SIZE];
  if (!fgets(buffer, sizeof(buffer), file)) {
    return NULL;
  }
  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
  }
  return custom_strdup(buffer);
}

int patterns_load_from_file(patterns_t *p, const char *filename,
                            int suppress_errors) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    if (!suppress_errors) {
      fprintf(stderr, "s21_grep: %s: Файл или директория не существуют\n",
              filename);
    }
    return 0;
  }
  char *line;
  while ((line = read_line(file)) != NULL) {
    if (strlen(line) > 0) {
      patterns_add(p, line);
    }
    free(line);
  }
  fclose(file);
  return 1;
}

int parse_arguments(int argc, char *argv[], flags_t *flags,
                    patterns_t *patterns) {
  int opt;
  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (opt) {
      case 'e':
        flags->e = 1;
        patterns_add(patterns, optarg);
        break;
      case 'i':
        flags->i = 1;
        break;
      case 'v':
        flags->v = 1;
        break;
      case 'c':
        flags->c = 1;
        break;
      case 'l':
        flags->l = 1;
        break;
      case 'n':
        flags->n = 1;
        break;
      case 'h':
        flags->h = 1;
        break;
      case 's':
        flags->s = 1;
        break;
      case 'f':
        flags->f = 1;
        if (!patterns_load_from_file(patterns, optarg, flags->s)) {
          return 0;
        }
        break;
      case 'o':
        flags->o = 1;
        break;
      default:
        fprintf(stderr, "s21_grep: неверная опция -- '%c'\n", optopt);
        return 0;
    }
  }
  if (flags->v && flags->o) {
    fprintf(stderr, "s21_grep: опции -v и -o несовместимы\n");
    return 0;
  }
  return 1;
}

regex_t *compile_patterns(patterns_t *patterns, flags_t flags, int *success) {
  *success = 1;
  if (patterns->count == 0) {
    *success = 0;
    return NULL;
  }
  regex_t *regexes = malloc(patterns->count * sizeof(regex_t));
  int regex_flags = REG_EXTENDED | REG_NEWLINE;
  if (flags.i) regex_flags |= REG_ICASE;
  for (int i = 0; i < patterns->count; i++) {
    int ret = regcomp(&regexes[i], patterns->patterns[i], regex_flags);
    if (ret != 0) {
      if (!flags.s) {
        char error_msg[100];
        regerror(ret, &regexes[i], error_msg, sizeof(error_msg));
        fprintf(stderr, "s21_grep: неверное регулярное выражение: %s\n",
                error_msg);
      }
      for (int j = 0; j < i; j++) {
        regfree(&regexes[j]);
      }
      free(regexes);
      *success = 0;
      return NULL;
    }
  }
  return regexes;
}

void free_regexes(regex_t *regexes, int count) {
  for (int i = 0; i < count; i++) {
    regfree(&regexes[i]);
  }
  free(regexes);
}

int has_match(const char *line, regex_t *regexes, int regex_count,
              flags_t flags) {
  for (int i = 0; i < regex_count; i++) {
    int match = (regexec(&regexes[i], line, 0, NULL, 0) == 0);
    if (flags.v) match = !match;
    if (match) return 1;
  }
  return 0;
}

void print_only_matches(const char *line, regex_t *regexes, int regex_count,
                        const char *filename, int line_num, flags_t flags) {
  for (int i = 0; i < regex_count; i++) {
    const char *cursor = line;
    regmatch_t match;
    while (regexec(&regexes[i], cursor, 1, &match, 0) == 0) {
      if (match.rm_so == -1) break;
      if (flags.multiple_files && !flags.h) {
        printf("%s:", filename);
      }
      if (flags.n) {
        printf("%d:", line_num);
      }
      for (int j = match.rm_so; j < match.rm_eo; j++) {
        putchar(cursor[j]);
      }
      putchar('\n');
      cursor += match.rm_eo;
      if (*cursor == '\0') break;
    }
  }
}

void print_full_line(const char *line, const char *filename, int line_num,
                     flags_t flags) {
  if (flags.multiple_files && !flags.h) {
    printf("%s:", filename);
  }
  if (flags.n) {
    printf("%d:", line_num);
  }
  printf("%s\n", line);
}

void print_count(const char *filename, flags_t flags, int count) {
  if (flags.multiple_files && !flags.h) {
    printf("%s:", filename);
  }
  printf("%d\n", count);
}

void print_final_results(const char *filename, flags_t flags, int match_count) {
  if (flags.c) {
    print_count(filename, flags, match_count);
  }
  if (flags.l && match_count > 0) {
    printf("%s\n", filename);
  }
}

void print_line_match(const char *line, const char *filename, int line_num,
                      flags_t flags, regex_t *regexes, int regex_count) {
  if (flags.o && !flags.v) {
    print_only_matches(line, regexes, regex_count, filename, line_num, flags);
  } else {
    print_full_line(line, filename, line_num, flags);
  }
}

int process_line(const char *line, const char *filename, int line_num,
                 flags_t flags, regex_t *regexes, int regex_count,
                 int *match_count) {
  int matches = has_match(line, regexes, regex_count, flags);
  if (!matches) return 0;
  (*match_count)++;
  if (flags.l) {
    *match_count = 1;
    return 1;
  }
  if (!flags.c) {
    print_line_match(line, filename, line_num, flags, regexes, regex_count);
  }
  return 0;
}

void remove_newline(char *line) {
  size_t len = strlen(line);
  if (len > 0 && line[len - 1] == '\n') {
    line[len - 1] = '\0';
  }
}

FILE *open_file(const char *filename, flags_t flags) {
  FILE *file = (strcmp(filename, "-") == 0) ? stdin : fopen(filename, "r");
  if (!file && !flags.s) {
    fprintf(stderr, "s21_grep: %s: Файл или директория не существуют\n",
            filename);
  }
  return file;
}

void cleanup_file(FILE *file) {
  if (file != stdin) {
    fclose(file);
  }
}

void process_file_content(FILE *file, const char *filename, flags_t flags,
                          regex_t *regexes, int regex_count) {
  char line[BUFFER_SIZE];
  int line_num = 0;
  int match_count = 0;
  while (fgets(line, sizeof(line), file)) {
    line_num++;
    remove_newline(line);

    if (process_line(line, filename, line_num, flags, regexes, regex_count,
                     &match_count)) {
      break;
    }
  }
  print_final_results(filename, flags, match_count);
}

void process_file(const char *filename, flags_t flags, regex_t *regexes,
                  int regex_count) {
  FILE *file = open_file(filename, flags);
  if (!file) return;
  process_file_content(file, filename, flags, regexes, regex_count);
  cleanup_file(file);
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "ru_RU.UTF-8");
  if (argc < 2) {
    fprintf(stderr, "использование: s21_grep [опции] шаблон [файл ...]\n");
    return 1;
  }
  flags_t flags = {0};
  patterns_t patterns;
  patterns_init(&patterns);
  if (!parse_arguments(argc, argv, &flags, &patterns)) {
    patterns_free(&patterns);
    return 1;
  }
  if (patterns.count == 0 && optind < argc) {
    patterns_add(&patterns, argv[optind++]);
  }
  if (patterns.count == 0) {
    fprintf(stderr, "s21_grep: шаблон не указан\n");
    patterns_free(&patterns);
    return 1;
  }
  flags.multiple_files = (argc - optind > 1);
  int compile_success;
  regex_t *regexes = compile_patterns(&patterns, flags, &compile_success);
  if (!compile_success) {
    patterns_free(&patterns);
    return 1;
  }
  if (optind >= argc) {
    process_file("-", flags, regexes, patterns.count);
  } else {
    for (int i = optind; i < argc; i++) {
      process_file(argv[i], flags, regexes, patterns.count);
    }
  }
  free_regexes(regexes, patterns.count);
  patterns_free(&patterns);

  return 0;
}
