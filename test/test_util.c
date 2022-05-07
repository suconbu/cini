#include "test.h"

int G_TEST_COUNT = 0;
int G_OK_COUNT = 0;

void add_test_result(int cond, const char* cond_text, int line)
{
    G_TEST_COUNT++;
    if (cond) {
        TEST_PRINT(stdout, "%4d\t(^-^)\t[%s]\n", G_TEST_COUNT, cond_text);
        G_OK_COUNT++;
    } else {
        TEST_PRINT(stdout, "%4d\t(x_x)\t[%s] at %d\n", G_TEST_COUNT, cond_text, line);
    }
    fflush(stdout);
}

void print_test_summary()
{
    fprintf(stdout, "----------------------------------------\n");
    fprintf(stdout, "count\t(^-^)\t%%\n");
    fprintf(stdout, "%d\t%d\t%.01f\n",
        G_TEST_COUNT,
        G_OK_COUNT,
        G_OK_COUNT * 100.F / G_TEST_COUNT);
    fprintf(stdout, "----------------------------------------\n");
}

int get_test_result()
{
    return (G_OK_COUNT < G_TEST_COUNT) ? 1 : 0;
}
