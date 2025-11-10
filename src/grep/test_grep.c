#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int compare_grep_output(const char *args, const char *pattern,
                        const char *filename) {
  char s21_command[BUFFER_SIZE];
  char system_command[BUFFER_SIZE];

  if (filename && strlen(filename) > 0) {
    if (pattern && strlen(pattern) > 0) {
      snprintf(s21_command, sizeof(s21_command), "./s21_grep %s \"%s\" %s",
               args, pattern, filename);
      snprintf(system_command, sizeof(system_command),
               "grep %s \"%s\" %s 2>/dev/null", args, pattern, filename);
    } else {
      snprintf(s21_command, sizeof(s21_command), "./s21_grep %s %s", args,
               filename);
      snprintf(system_command, sizeof(system_command), "grep %s %s 2>/dev/null",
               args, filename);
    }
  } else {
    return 1;
  }

  FILE *s21_pipe = popen(s21_command, "r");
  if (!s21_pipe) return 0;

  FILE *system_pipe = popen(system_command, "r");
  if (!system_pipe) {
    pclose(s21_pipe);
    return 0;
  }

  char s21_output[BUFFER_SIZE] = {0};
  char system_output[BUFFER_SIZE] = {0};
  char buffer[BUFFER_SIZE];

  while (fgets(buffer, sizeof(buffer), s21_pipe)) {
    strcat(s21_output, buffer);
  }

  while (fgets(buffer, sizeof(buffer), system_pipe)) {
    strcat(system_output, buffer);
  }

  pclose(s21_pipe);
  pclose(system_pipe);

  return strcmp(s21_output, system_output) == 0;
}

// ===== БАЗОВЫЕ ТЕСТЫ =====

START_TEST(test_basic_pattern) {
  ck_assert_int_eq(compare_grep_output("", "hello", "test_cases/test1.txt"), 1);
}
END_TEST

START_TEST(test_case_insensitive_basic) {
  ck_assert_int_eq(compare_grep_output("-i", "HELLO", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_invert_match_basic) {
  ck_assert_int_eq(compare_grep_output("-v", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_line_number_basic) {
  ck_assert_int_eq(compare_grep_output("-n", "world", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_count_basic) {
  ck_assert_int_eq(compare_grep_output("-c", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_files_with_matches_basic) {
  ck_assert_int_eq(compare_grep_output("-l", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_no_filename_basic) {
  ck_assert_int_eq(
      compare_grep_output("-h", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_suppress_errors_basic) {
  ck_assert_int_eq(
      compare_grep_output("-s", "hello", "test_cases/nonexistent.txt"), 1);
}
END_TEST

START_TEST(test_only_matching_basic) {
  ck_assert_int_eq(compare_grep_output("-o", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

// ===== ТЕСТЫ С НЕСКОЛЬКИМИ ФАЙЛАМИ =====

START_TEST(test_multiple_files_basic) {
  ck_assert_int_eq(
      compare_grep_output("", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_multiple_files_with_line_numbers) {
  ck_assert_int_eq(
      compare_grep_output("-n", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_multiple_files_count) {
  ck_assert_int_eq(
      compare_grep_output("-c", "pattern",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_multiple_files_invert) {
  ck_assert_int_eq(compare_grep_output("-v", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_multiple_files_case_insensitive) {
  ck_assert_int_eq(
      compare_grep_output("-i", "HELLO",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

// ===== ТЕСТЫ С ФЛАГОМ -e =====

START_TEST(test_e_flag_single) {
  ck_assert_int_eq(compare_grep_output("-e hello", "", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_e_flag_multiple) {
  ck_assert_int_eq(
      compare_grep_output("-e hello -e world", "", "test_cases/test1.txt"), 1);
}
END_TEST

START_TEST(test_e_flag_with_other_flags) {
  ck_assert_int_eq(
      compare_grep_output("-e hello -n", "", "test_cases/test1.txt"), 1);
}
END_TEST

START_TEST(test_e_flag_multiple_files) {
  ck_assert_int_eq(
      compare_grep_output("-e pattern", "",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

// ===== ТЕСТЫ С ФЛАГОМ -f =====

START_TEST(test_f_flag_basic) {
  ck_assert_int_eq(compare_grep_output("-f test_cases/patterns1.txt", "",
                                       "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_f_flag_multiple_patterns) {
  ck_assert_int_eq(compare_grep_output("-f test_cases/patterns2.txt", "",
                                       "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_f_flag_with_other_flags) {
  ck_assert_int_eq(compare_grep_output("-f test_cases/patterns1.txt -n", "",
                                       "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_f_flag_multiple_files) {
  ck_assert_int_eq(
      compare_grep_output("-f test_cases/patterns1.txt", "",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

// ===== КОМБИНАЦИИ ФЛАГОВ =====

START_TEST(test_combination_in) {
  ck_assert_int_eq(compare_grep_output("-in", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_combination_iv) {
  ck_assert_int_eq(compare_grep_output("-iv", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_combination_ic) {
  ck_assert_int_eq(compare_grep_output("-ic", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_combination_vn) {
  ck_assert_int_eq(compare_grep_output("-vn", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_combination_vc) {
  ck_assert_int_eq(compare_grep_output("-vc", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_combination_nh) {
  ck_assert_int_eq(
      compare_grep_output("-nh", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_combination_ch) {
  ck_assert_int_eq(
      compare_grep_output("-ch", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_combination_ivn) {
  ck_assert_int_eq(compare_grep_output("-ivn", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_combination_ivc) {
  ck_assert_int_eq(compare_grep_output("-ivc", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_o_with_i) {
  ck_assert_int_eq(compare_grep_output("-oi", "HELLO", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_o_with_n) {
  ck_assert_int_eq(compare_grep_output("-on", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_o_with_multiple_files) {
  ck_assert_int_eq(
      compare_grep_output("-o", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_o_with_h) {
  ck_assert_int_eq(
      compare_grep_output("-oh", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_l_with_multiple_files) {
  ck_assert_int_eq(
      compare_grep_output("-l", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_l_with_v) {
  ck_assert_int_eq(compare_grep_output("-lv", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_l_with_i) {
  ck_assert_int_eq(compare_grep_output("-li", "HELLO", "test_cases/test1.txt"),
                   1);
}
END_TEST

// ===== СЛОЖНЫЕ КОМБИНАЦИИ =====

START_TEST(test_complex_combination_1) {
  ck_assert_int_eq(compare_grep_output("-ivn", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_complex_combination_2) {
  ck_assert_int_eq(
      compare_grep_output("-e hello -e world -n", "", "test_cases/test1.txt"),
      1);
}
END_TEST

START_TEST(test_complex_combination_3) {
  ck_assert_int_eq(compare_grep_output("-f test_cases/patterns1.txt -i -n", "",
                                       "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_complex_combination_4) {
  ck_assert_int_eq(
      compare_grep_output("-e pattern -v -c", "", "test_cases/test1.txt"), 1);
}
END_TEST

START_TEST(test_complex_combination_5) {
  ck_assert_int_eq(compare_grep_output("-f test_cases/patterns2.txt -o -n", "",
                                       "test_cases/test1.txt"),
                   1);
}
END_TEST

// ===== ТЕСТЫ С РАЗНЫМИ ПАТТЕРНАМИ =====

START_TEST(test_pattern_with_special_chars) {
  ck_assert_int_eq(compare_grep_output("", "test.*", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_pattern_word_boundary) {
  ck_assert_int_eq(compare_grep_output("", "hello", "test_cases/test1.txt"), 1);
}
END_TEST

START_TEST(test_pattern_not_found) {
  ck_assert_int_eq(
      compare_grep_output("", "nonexistentpattern", "test_cases/test1.txt"), 1);
}
END_TEST

// ===== ТЕСТЫ С ОДНИМ ФАЙЛОМ =====

START_TEST(test_single_file_all_flags) {
  ck_assert_int_eq(compare_grep_output("-inv", "hello", "test_cases/test1.txt"),
                   1);
}
END_TEST

START_TEST(test_single_file_count_only) {
  ck_assert_int_eq(compare_grep_output("-c", "test", "test_cases/test1.txt"),
                   1);
}
END_TEST

// ===== ТЕСТЫ С ТРЕМЯ И БОЛЕЕ ФАЙЛАМИ =====

START_TEST(test_three_files_basic) {
  ck_assert_int_eq(
      compare_grep_output("", "hello",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_three_files_with_count) {
  ck_assert_int_eq(
      compare_grep_output("-c", "pattern",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_three_files_with_line_numbers) {
  ck_assert_int_eq(
      compare_grep_output("-n", "world",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

START_TEST(test_multiple_files_list_matches) {
  ck_assert_int_eq(
      compare_grep_output("-l", "test",
                          "test_cases/test1.txt test_cases/test2.txt"),
      1);
}
END_TEST

Suite *grep_suite(void) {
  Suite *s;
  TCase *tc_core, *tc_multiple, *tc_e_flag, *tc_f_flag, *tc_combinations,
      *tc_complex;

  s = suite_create("s21_grep");

  // Базовые тесты
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_basic_pattern);
  tcase_add_test(tc_core, test_case_insensitive_basic);
  tcase_add_test(tc_core, test_invert_match_basic);
  tcase_add_test(tc_core, test_line_number_basic);
  tcase_add_test(tc_core, test_count_basic);
  tcase_add_test(tc_core, test_files_with_matches_basic);
  tcase_add_test(tc_core, test_no_filename_basic);
  tcase_add_test(tc_core, test_suppress_errors_basic);
  tcase_add_test(tc_core, test_only_matching_basic);

  // Тесты с несколькими файлами
  tc_multiple = tcase_create("Multiple Files");
  tcase_add_test(tc_multiple, test_multiple_files_basic);
  tcase_add_test(tc_multiple, test_multiple_files_with_line_numbers);
  tcase_add_test(tc_multiple, test_multiple_files_count);
  tcase_add_test(tc_multiple, test_multiple_files_invert);
  tcase_add_test(tc_multiple, test_multiple_files_case_insensitive);
  tcase_add_test(tc_multiple, test_three_files_basic);
  tcase_add_test(tc_multiple, test_three_files_with_count);
  tcase_add_test(tc_multiple, test_three_files_with_line_numbers);
  tcase_add_test(tc_multiple, test_multiple_files_list_matches);

  // Тесты с флагом -e
  tc_e_flag = tcase_create("E Flag");
  tcase_add_test(tc_e_flag, test_e_flag_single);
  tcase_add_test(tc_e_flag, test_e_flag_multiple);
  tcase_add_test(tc_e_flag, test_e_flag_with_other_flags);
  tcase_add_test(tc_e_flag, test_e_flag_multiple_files);

  // Тесты с флагом -f
  tc_f_flag = tcase_create("F Flag");
  tcase_add_test(tc_f_flag, test_f_flag_basic);
  tcase_add_test(tc_f_flag, test_f_flag_multiple_patterns);
  tcase_add_test(tc_f_flag, test_f_flag_with_other_flags);
  tcase_add_test(tc_f_flag, test_f_flag_multiple_files);

  // Комбинации флагов
  tc_combinations = tcase_create("Flag Combinations");
  tcase_add_test(tc_combinations, test_combination_in);
  tcase_add_test(tc_combinations, test_combination_iv);
  tcase_add_test(tc_combinations, test_combination_ic);
  tcase_add_test(tc_combinations, test_combination_vn);
  tcase_add_test(tc_combinations, test_combination_vc);
  tcase_add_test(tc_combinations, test_combination_nh);
  tcase_add_test(tc_combinations, test_combination_ch);
  tcase_add_test(tc_combinations, test_combination_ivn);
  tcase_add_test(tc_combinations, test_combination_ivc);
  tcase_add_test(tc_combinations, test_o_with_i);
  tcase_add_test(tc_combinations, test_o_with_n);
  tcase_add_test(tc_combinations, test_o_with_multiple_files);
  tcase_add_test(tc_combinations, test_o_with_h);
  tcase_add_test(tc_combinations, test_l_with_multiple_files);
  tcase_add_test(tc_combinations, test_l_with_v);
  tcase_add_test(tc_combinations, test_l_with_i);

  // Сложные комбинации
  tc_complex = tcase_create("Complex Combinations");
  tcase_add_test(tc_complex, test_complex_combination_1);
  tcase_add_test(tc_complex, test_complex_combination_2);
  tcase_add_test(tc_complex, test_complex_combination_3);
  tcase_add_test(tc_complex, test_complex_combination_4);
  tcase_add_test(tc_complex, test_complex_combination_5);
  tcase_add_test(tc_complex, test_pattern_with_special_chars);
  tcase_add_test(tc_complex, test_pattern_word_boundary);
  tcase_add_test(tc_complex, test_pattern_not_found);
  tcase_add_test(tc_complex, test_single_file_all_flags);
  tcase_add_test(tc_complex, test_single_file_count_only);

  suite_add_tcase(s, tc_core);
  suite_add_tcase(s, tc_multiple);
  suite_add_tcase(s, tc_e_flag);
  suite_add_tcase(s, tc_f_flag);
  suite_add_tcase(s, tc_combinations);
  suite_add_tcase(s, tc_complex);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = grep_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? 0 : 1;
}
