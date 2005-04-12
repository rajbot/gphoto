#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dlt-lib.h>

#include <ltdl.h>
#include <config.h>


typedef char *(*dynlibtest_func)(const char *);


static char *
alloc_stringjoin(const char sep, const char *a, const char *b)
{
  assert(a != NULL);
  assert(b != NULL);
  if (1) {
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
    *d++ = sep;
    s = (char *)b;
    while (*s != '\0')
      *d++ = *s++;
    *d++ = '\0';
    return res;
  }
}


#ifdef LT_DIRSEP_CHAR
#define DIRSEP_CHAR LT_DIRSEP_CHAR
#else
#define DIRSEP_CHAR '/'
#endif

inline static char *
alloc_pathjoin(const char *a, const char *b)
{
  return alloc_stringjoin(DIRSEP_CHAR, a, b);
}


int something_worked = 0;
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


static void
load_and_test (lt_dlhandle handle, const char *symname)
{
    dynlibtest_func func = NULL;

    func = lt_dlsym (handle, symname);
    if (func != NULL) {
      succ_count++;
      printf("    Result of %s():\n      \"%s\"\n", symname, func(NULL));
    } else {
      printf("    lt_dlsym of %s() failed:\n      %s\n",
	     symname, lt_dlerror());
    }

}


static int
foreach_func (const char *filename, lt_ptr data)
{
  const char *symname = (const char *) data;
  lt_dlhandle handle = NULL;

  assert(filename != NULL);
  assert(symname != NULL);
  printf("  Filename: %s\n", filename);
  total_count++;

  assert(lt_dlinit() == 0);

  handle = lt_dlopenext(filename);
  if (handle != NULL) {
    char *module_name = NULL;

    if (1) {
      const lt_dlinfo *dlinfo = lt_dlgetinfo(handle);
      if (dlinfo != NULL) {
	if (dlinfo->name) {
	  printf("    Module name: \"%s\"\n", dlinfo->name);
	  module_name = dlinfo->name;
	}
	printf("    Module filename: \"%s\"\n", dlinfo->filename);
      } else {
	printf("    No module information:\n    %s\n",
	       lt_dlerror());
      }
    }
    
    load_and_test(handle, symname);

    if (module_name) {
      const char *my_tmpname = 
	alloc_stringjoin('_', "LTX", symname);
      const char *my_symname = 
	alloc_stringjoin('_', module_name, my_tmpname);
      load_and_test(handle, my_symname);
    }

  }

  assert(lt_dlexit() == 0);
  return 0;
}


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
  something_worked += succ_count;
  total_count = 0;
  succ_count = 0;
  explicit_load("static", MODULE_DIR, "dynlibtest1", "dynlibtest2");
  for (i=1; i<argc; i++) {
    explicit_load(argv[i], argv[i], "dynlibtest3", "dynlibtest4");
  }
  printf("%d total, %d successful, %d errors.\n",
	 total_count, succ_count, total_count - succ_count);
}


static void
test_path(const char *path)
{
  printf("Path search in directory \"%s\": Starting.\n", path);
  lt_dlforeachfile (path, foreach_func, "dynlibtest0");
  printf("Path search in directory \"%s\": Done.\n", path);
}


static void
test_pathes(const int argc, const char *argv[])
{
  int i;
  printf("Starting path searches\n");
  something_worked += succ_count;
  total_count = 0;
  succ_count = 0;
  test_path(MODULE_DIR);
  for (i=1; i<argc; i++) {
    test_path(argv[i]);
  }
  printf("%d total, %d successful, %d errors.\n",
	 total_count, succ_count, total_count - succ_count);
}


int
dlt_init (void)
{
  printf("dlt-lib (" PACKAGE_NAME ") " PACKAGE_VERSION "\n");
  return 0;
}


int
dlt_exit (void)
{
  printf("dlt-lib: Finished.\n");
  return 0;  
}


int
dlt_test (const int argc, const char *argv[])
{
  if (0) {
    explicit_loads(argc, argv);
    printf("---------------------------------------------------------\n");
  }
  test_pathes(argc, argv);
  something_worked += succ_count;
  return (something_worked > 0)?0:1;
}
