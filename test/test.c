#include "test.h"
#define CINI_IMPLEMENTATION
#include "cini.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    const char* path = "test.ini";
    test_c(path);
    // test_cpp(path);
    print_test_summary();

    return get_test_result();
}
