//
// main.cpp - Execute the test code for cini
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
extern "C" {
#define CINI_IMPLEMENTATION
#include "cini.h"
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    const char* path = "test.ini";
    test_c(path);
    test_cpp(path);
    print_test_summary();
    append_test_result();

    return 0;
}
