#include <ctype.h>
#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int number_nonblank;   // -b, --number-nonblank
  int show_ends;         // -e, -E
  int number;            // -n, --number
  int squeeze_blank;     // -s, --squeeze-blank
  int show_tabs;         // -t, -T
  int show_nonprinting;  // -v
} cat_flags;

typedef struct {
  int line_number;
  int empty_lines;
  int new_line_flag;
  int prev_char;
} file_context;

void init_flags(cat_flags *flags) {
  flags->number_nonblank = 0;
  flags->show_ends = 0;
  flags->number = 0;
  flags->squeeze_blank = 0;
  flags->show_tabs = 0;
  flags->show_nonprinting = 0;
}

void init_context(file_context *ctx) {
  ctx->line_number = 1;
  ctx->empty_lines = 0;
  ctx->new_line_flag = 1;
  ctx->prev_char = '\n';
}

void resolve_flag_conflicts(cat_flags *flags) {
  if (flags->number_nonblank) {
    flags->number = 0;
  }
}

int parse_arguments(int argc, char *argv[], cat_flags *flags) {
  const struct option long_options[] = {
      {"number-nonblank", no_argument, NULL, 'b'},
      {"number", no_argument, NULL, 'n'},
      {"squeeze-blank", no_argument, NULL, 's'},
      {NULL, 0, NULL, 0}};

  int opt;
  while ((opt = getopt_long(argc, argv, "beEnstTv", long_options, NULL)) !=
         -1) {
    switch (opt) {
      case 'b':
        flags->number_nonblank = 1;
        break;
      case 'e':
        flags->show_ends = 1;
        flags->show_nonprinting = 1;
        break;
      case 'E':
        flags->show_ends = 1;
        break;
      case 'n':
        flags->number = 1;
        break;
      case 's':
        flags->squeeze_blank = 1;
        break;
      case 't':
        flags->show_tabs = 1;
        flags->show_nonprinting = 1;
        break;
      case 'T':
        flags->show_tabs = 1;
        break;
      case 'v':
        flags->show_nonprinting = 1;
        break;
      default:
        return 0;
    }
  }

  resolve_flag_conflicts(flags);
  return 1;
}

void print_line_number(file_context *ctx) {
  printf("%6d\t", ctx->line_number++);
  ctx->new_line_flag = 0;
}

void print_nonprinting(unsigned char c) {
  if (c == 127) {
    printf("^?");
  } else if (c < 32) {
    printf("^%c", c + 64);
  } else if (c > 127 && c < 160) {
    printf("M-^%c", c - 64);
  } else if (c >= 160) {
    printf("M-%c", c - 128);
  } else {
    putchar(c);
  }
}

int should_number_line(cat_flags flags, unsigned char c, file_context *ctx) {
  if (flags.number) {
    return ctx->new_line_flag;
  }
  if (flags.number_nonblank) {
    return ctx->new_line_flag && c != '\n';
  }
  return 0;
}

void process_char(unsigned char c, cat_flags flags, file_context *ctx) {
  if (flags.squeeze_blank) {
    if (ctx->prev_char == '\n' && c == '\n') {
      ctx->empty_lines++;
      if (ctx->empty_lines > 1) {
        ctx->prev_char = c;
        return;
      }
    } else {
      ctx->empty_lines = 0;
    }
  }

  if (should_number_line(flags, c, ctx)) {
    print_line_number(ctx);
  }

  if (flags.show_tabs && c == '\t') {
    printf("^I");
  } else if (flags.show_ends && c == '\n') {
    printf("$");
    putchar('\n');
  } else if (flags.show_nonprinting) {
    if (c == '\n' || c == '\t') {
      putchar(c);
    } else if ((c < 32 && c != 9) || c == 127 || c > 127) {
      print_nonprinting(c);
    } else {
      putchar(c);
    }
  } else {
    putchar(c);
  }

  if (c == '\n') {
    ctx->new_line_flag = 1;
  } else {
    ctx->new_line_flag = 0;
  }

  ctx->prev_char = c;
}

void process_file(FILE *file, cat_flags flags, file_context *ctx) {
  int c;
  while ((c = fgetc(file)) != EOF) {
    process_char((unsigned char)c, flags, ctx);
  }
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "en_US.UTF-8");

  cat_flags flags;
  init_flags(&flags);

  if (!parse_arguments(argc, argv, &flags)) {
    fprintf(stderr, "Error: Unknown option\n");
    fprintf(stderr, "Usage: %s [-beEnstTv] [file...]\n", argv[0]);
    return 1;
  }

  file_context ctx;
  init_context(&ctx);

  if (optind >= argc) {
    process_file(stdin, flags, &ctx);
  } else {
    for (int i = optind; i < argc; i++) {
      FILE *file = fopen(argv[i], "r");
      if (file == NULL) {
        fprintf(stderr, "s21_cat: %s: No such file or directory\n", argv[i]);
        continue;
      }

      process_file(file, flags, &ctx);
      fclose(file);
    }
  }

  return 0;
}
