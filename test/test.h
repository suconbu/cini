#pragma once

#include <stdio.h>

#define TEST(cond)            add_test_result((int)(cond), #cond, __LINE__)
#define TEST_PRINT(file, ...) fprintf(file, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void test_c(const char* path);
void test_cpp(const char* path);

void add_test_result(int cond, const char* cond_text, int line);
void print_test_summary();
int get_test_result();

#ifdef __cplusplus
}
#endif //__cplusplus
