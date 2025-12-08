#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cat_test(const char *filename, const char *flags) {
  char my_cat[256];
  char real_cat[256];

  snprintf(my_cat, sizeof(my_cat), "./s21_cat %s %s", flags, filename);
  snprintf(real_cat, sizeof(real_cat), "cat %s %s", flags, filename);

  FILE *your_output = popen(my_cat, "r");
  FILE *real_output = popen(real_cat, "r");

  char my_line[1024], real_line[1024];
  while (fgets(my_line, sizeof(my_line), your_output) &&
         fgets(real_line, sizeof(real_line), real_output)) {
    ck_assert_str_eq(my_line, real_line);
  }

  pclose(your_output);
  pclose(real_output);
}
// simple
START_TEST(one_file) { cat_test("test_cases/test1.txt", ""); }
END_TEST

START_TEST(two_files) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "");
}
END_TEST

START_TEST(three_files) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "");
}
END_TEST

// b
START_TEST(one_file_b) { cat_test("test_cases/test1.txt", "-b"); }
END_TEST
START_TEST(two_files_b) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "--number-nonblank");
}
END_TEST
START_TEST(three_files_b) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "--number-nonblank");
}
END_TEST

// e
START_TEST(one_file_e) { cat_test("test_cases/test1.txt", "-e"); }
START_TEST(two_files_e) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-e");
}
END_TEST
START_TEST(three_files_e) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-e");
}
END_TEST

// E
START_TEST(one_file_E) { cat_test("test_cases/test1.txt", "-E"); }
START_TEST(two_files_E) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-E");
}
END_TEST
START_TEST(three_files_E) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-E");
}
END_TEST

// n
START_TEST(one_file_n) { cat_test("test_cases/test1.txt", "-n"); }
START_TEST(two_files_n) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-n");
}
END_TEST
START_TEST(three_files_n) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-n");
}
END_TEST

// s
START_TEST(one_file_s) { cat_test("test_cases/test1.txt", "-s"); }
START_TEST(two_files_s) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-s");
}
END_TEST
START_TEST(three_files_s) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-s");
}
END_TEST

// t
START_TEST(one_file_t) { cat_test("test_cases/test1.txt", "-t"); }
START_TEST(two_files_t) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-t");
}
END_TEST
START_TEST(three_files_t) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-t");
}
END_TEST

// T
START_TEST(one_file_T) { cat_test("test_cases/test1.txt", "-T"); }
START_TEST(two_files_T) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-T");
}
END_TEST
START_TEST(three_files_T) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-T");
}
END_TEST

// v
START_TEST(one_file_v) { cat_test("test_cases/test1.txt", "-v"); }
START_TEST(two_files_v) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-v");
}
END_TEST
START_TEST(three_files_v) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-v");
}
END_TEST

// shake
START_TEST(one_file_shake) { cat_test("test_cases/test1.txt", "-b -E -s -T"); }
START_TEST(two_files_shake) {
  cat_test("test_cases/test1.txt test_cases/test2.txt", "-b -n -e -s");
}
END_TEST
START_TEST(three_files_shake) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-n -v -t");
}
END_TEST

// all
START_TEST(one_file_all) {
  cat_test("test_cases/test1.txt",
           "-b -E -e -n --number -s --squeeze-blank -T -t -v");
}
START_TEST(two_files_all) {
  cat_test("test_cases/test1.txt test_cases/test2.txt",
           "-b -E -e -n --number -s --squeeze-blank -T -t -v");
}
END_TEST
START_TEST(three_files_all) {
  cat_test("test_cases/test1.txt test_cases/test2.txt test_cases/test3.txt",
           "-b -E -e -n --number -s --squeeze-blank -T -t -v");
}
END_TEST
Suite *suite_simple() {
  Suite *suite = suite_create("simple");
  TCase *simple = tcase_create("simple");
  tcase_add_test(simple, one_file);
  tcase_add_test(simple, two_files);
  tcase_add_test(simple, three_files);
  suite_add_tcase(suite, simple);
  return suite;
}
Suite *suite_b() {
  Suite *suite = suite_create("b_flag");
  TCase *b_flag = tcase_create("b_flag");
  tcase_add_test(b_flag, one_file_b);
  tcase_add_test(b_flag, two_files_b);
  tcase_add_test(b_flag, three_files_b);
  suite_add_tcase(suite, b_flag);
  return suite;
}
Suite *suite_e() {
  Suite *suite = suite_create("e_flag");
  TCase *e_flag = tcase_create("e_flag");
  tcase_add_test(e_flag, one_file_e);
  tcase_add_test(e_flag, two_files_e);
  tcase_add_test(e_flag, three_files_e);
  suite_add_tcase(suite, e_flag);
  return suite;
}
Suite *suite_E() {
  Suite *suite = suite_create("E_flag");
  TCase *E_flag = tcase_create("E_flag");
  tcase_add_test(E_flag, one_file_E);
  tcase_add_test(E_flag, two_files_E);
  tcase_add_test(E_flag, three_files_E);
  suite_add_tcase(suite, E_flag);
  return suite;
}
Suite *suite_n() {
  Suite *suite = suite_create("n_flag");
  TCase *n_flag = tcase_create("n_flag");
  tcase_add_test(n_flag, one_file_n);
  tcase_add_test(n_flag, two_files_n);
  tcase_add_test(n_flag, three_files_n);
  suite_add_tcase(suite, n_flag);
  return suite;
}
Suite *suite_s() {
  Suite *suite = suite_create("s_flag");
  TCase *s_flag = tcase_create("s_flag");
  tcase_add_test(s_flag, one_file_s);
  tcase_add_test(s_flag, two_files_s);
  tcase_add_test(s_flag, three_files_s);
  suite_add_tcase(suite, s_flag);
  return suite;
}
Suite *suite_t() {
  Suite *suite = suite_create("t_flag");
  TCase *t_flag = tcase_create("t_flag");
  tcase_add_test(t_flag, one_file_t);
  tcase_add_test(t_flag, two_files_t);
  tcase_add_test(t_flag, three_files_t);
  suite_add_tcase(suite, t_flag);
  return suite;
}
Suite *suite_T() {
  Suite *suite = suite_create("T_flag");
  TCase *T_flag = tcase_create("T_flag");
  tcase_add_test(T_flag, one_file_T);
  tcase_add_test(T_flag, two_files_T);
  tcase_add_test(T_flag, three_files_T);
  suite_add_tcase(suite, T_flag);
  return suite;
}
Suite *suite_v() {
  Suite *suite = suite_create("v_flag");
  TCase *v_flag = tcase_create("v_flag");
  tcase_add_test(v_flag, one_file_v);
  tcase_add_test(v_flag, two_files_v);
  tcase_add_test(v_flag, three_files_v);
  suite_add_tcase(suite, v_flag);
  return suite;
}
Suite *suite_shake() {
  Suite *suite = suite_create("shake_flag");
  TCase *shake_flag = tcase_create("shake_flag");
  tcase_add_test(shake_flag, one_file_shake);
  tcase_add_test(shake_flag, two_files_shake);
  tcase_add_test(shake_flag, three_files_shake);
  suite_add_tcase(suite, shake_flag);
  return suite;
}
Suite *suite_all() {
  Suite *suite = suite_create("all_flag");
  TCase *all_flag = tcase_create("all_flag");
  tcase_add_test(all_flag, one_file_all);
  tcase_add_test(all_flag, two_files_all);
  tcase_add_test(all_flag, three_files_all);
  suite_add_tcase(suite, all_flag);
  return suite;
}
int main() {
  SRunner *sr = srunner_create(NULL);
  srunner_add_suite(sr, suite_simple());
  srunner_add_suite(sr, suite_b());
  srunner_add_suite(sr, suite_E());
  srunner_add_suite(sr, suite_e());
  srunner_add_suite(sr, suite_n());
  srunner_add_suite(sr, suite_s());
  srunner_add_suite(sr, suite_t());
  srunner_add_suite(sr, suite_T());
  srunner_add_suite(sr, suite_v());
  srunner_add_suite(sr, suite_shake());
  srunner_add_suite(sr, suite_all());
  srunner_run_all(sr, CK_NORMAL);
  int failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
