//
// test_util.cpp - Utilities for test
//
// Copyright (C) 2016 suconbu.
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

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
