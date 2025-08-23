#include <stdio.h>
#define HEADERS
#include "test_all.c"
#undef HEADERS

#define TEST(name) test = name;
#define ASSERT(ast)\
  do {\
    assertion = #ast;\
    file = __FILE__;\
    line = __LINE__;\
    if(ast) putchar('.'); else goto fail;\
  } while(0)

int main() {
  const char *test = "";
  const char *assertion = "";
  const char *file = "";
  int line = 0;

# define TESTS
# include "test_all.c"
# undef TESTS

  putchar('\n');
  return 0;

fail:
  printf("!\nTest failed at %s:%d\n    %s: %s\n",
      file, line,
      test, assertion);
  return -1;
}
