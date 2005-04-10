#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dynlibtest.h>
#include <ltdl.h>
#include <config.h>


#ifndef assert
#define assert(expr) do { if (!(expr)) { fprintf(stderr, __FILE__ ":%d: Assertion failed: %s\n", __LINE__, #expr); exit(13); } } while (0)
#endif


#if defined(WIN32) || defined(OS2)
#  define DIRSEP '\\'
#else
#  define DIRSEP '/'
#endif


static char *
alloc_pathjoin(const char *a, const char *b)
{
  size_t al = strlen(a);
  size_t bl = strlen(b);
  size_t rl = al+bl+2;
  char *s, *d;
  char *res = malloc(rl);
  assert(res != NULL);
  s = (char *)a;
  d = res;
  while (*s != '\0')
    *d++ = *s++;
  *d++ = DIRSEP;
  s = (char *)b;
  while (*s != '\0')
    *d++ = *s++;
  *d++ = '\0';
  return res;
}


int succ_count = 0;
int total_count = 0;


static void 
test(const char *testname, const char *symname,
     const char *dirname, const char *filename)
{
  char *fpath = alloc_pathjoin(dirname, filename);
  assert(fpath != NULL);
  printf("Test \"%s\":\n", testname);
  total_count++;
  if (lt_dlinit()) {
    printf("    libltdl NOT initialized\n");
  } else {
    lt_dlhandle handle = NULL;
    printf("    libltdl initialized\n");
    handle = lt_dlopen(fpath);
    if (handle) {
      dynlibtest_func func = NULL;
      printf("    module \"%s\" loaded\n", fpath);
      func = lt_dlsym (handle, symname);
      if (func) {
	char *val;
	printf("    symbol \"%s\" loaded\n", symname);
	val = func(NULL);
	printf("    return value: \"%s\"\n", val);
	succ_count++;
      } else {
	printf("    symbol \"%s\" could NOT loaded:\n        %s\n",
	       symname, lt_dlerror());
      }
      if (lt_dlclose(handle)) {
	printf("    module NOT unloaded\n");
      } else {
	printf("    module unloaded\n");
      }
    } else {
      printf("    library file \"%s\" NOT loaded:\n        %s\n",
	     fpath, lt_dlerror());
    }
    free(fpath);
    if (lt_dlexit()) {
      printf("    libltdl NOT shut down\n");
    } else {
      printf("    libltdl shut down\n");
    }
  }
}


static int
foreach_func (const char *filename, lt_ptr data)
{
  printf("Filename: %s\n", filename);
  total_count++;
  lt_dlhandle handle = NULL;
  dynlibtest_func func = NULL;  
  assert(lt_dlinit() == 0);
  handle = lt_dlopenext(filename);
  assert(handle != NULL);
  func = lt_dlsym (handle, "dynlibtest0");
  assert(func != NULL);
  succ_count++;
  printf("  Result: %s\n", func(NULL));
  assert(lt_dlexit() == 0);
  return 0;
}


#if defined(WIN32)
#  define SOEXT ".dll"
#elif defined(OS2)
#  define SOEXT ".dll"
#elif defined(DARWIN)
#  define SOEXT ".dynlib"
#else
#  define SOEXT ".so"
#endif


static void
explicit_load(const char *testname, const char *dir,
	      const char *sym1, const char *sym2)
{
  test("static", sym1, dir, "foo" SOEXT);
  test("static", sym2, dir, "foo.la");
  test("static", sym1, dir, "bar" SOEXT);
  test("static", sym2, dir, "bar.la");
}


static void
explicit_loads(const int argc, const char *argv[])
{
  int i;
  printf("Starting explicit loads\n");
  total_count = 0;
  succ_count = 0;
  explicit_load("static", TESTLIBDIR, "dynlibtest1", "dynlibtest2");
  for (i=1; i<argc; i++) {
    explicit_load(argv[i], argv[i], "dynlibtest3", "dynlibtest4");
  }
  printf("%d total, %d successful, %d errors.\n",
	 total_count, succ_count, total_count - succ_count);
}


static void
test_path(const char *path)
{
  printf("Starting path search for: \"%s\"\n", path);
  lt_dlforeachfile (TESTLIBDIR, foreach_func, NULL);
}


static void
test_pathes(const int argc, const char *argv[])
{
  int i;
  printf("Starting path searches\n");
  total_count = 0;
  succ_count = 0;
  test_path(TESTLIBDIR);
  for (i=1; i<argc; i++) {
    test_path(argv[i]);
  }
  printf("%d total, %d successful, %d errors.\n",
	 total_count, succ_count, total_count - succ_count);
}


int main(const int argc, const char *argv[])
{
  printf("dynlibtest-bin (" PACKAGE_NAME ") " PACKAGE_VERSION "\n");
  explicit_loads(argc, argv);
  test_pathes(argc, argv);
  printf("dynlibtest-bin: Finished.\n");
  return 0;
}
