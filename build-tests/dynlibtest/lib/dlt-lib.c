#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dlt-lib.h>

#include <ltdl.h>
#include <config.h>


typedef char *(*dynlibtest_func)(const char *);


/**********************************************************************
 * Utility functions
 **********************************************************************/

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
    if (sep != '\0')
      *d++ = sep;
    s = (char *)b;
    while (*s != '\0')
      *d++ = *s++;
    *d++ = '\0';
    return res;
  }
}


/*
#ifdef LT_DIRSEP_CHAR
#define DIRSEP_CHAR LT_DIRSEP_CHAR
#else
*/
#define DIRSEP_CHAR '/'
/* #endif */

inline static char *
alloc_pathjoin(const char *a, const char *b)
{
  return alloc_stringjoin(DIRSEP_CHAR, a, b);
}



/**********************************************************************
 * Common test infrastructure
 **********************************************************************/

volatile int mod_callback_count = 0;

int something_worked = 0;
int succ_count = 0;
int total_count = 0;
int primary_total = 0;
int primary_error = 0;


inline static void
print_big_separator (void)
{
  printf("========================================================================\n");
}


inline static void
print_separator (void)
{
  printf("-------------------------------------------------------\n");
}


typedef void (*test_iter_func) (const char *name, const char *directory,
				const int try_non_modules);

static void
test_iter(const char *name, test_iter_func test_func,
	  const int argc, const char *argv[],
	  const int try_non_modules)
{
  assert(test_func != NULL);

  print_big_separator();
  printf("Iterations for \"%s\": Started.\n", name);

  something_worked += succ_count;
  total_count = 0;
  succ_count = 0;
  primary_total = 0;
  primary_error = 0;

  if (1) {
    char *tmp = alloc_stringjoin(' ', name, "(build default)");
    print_separator();
    test_func(tmp, MODULE_DIR, try_non_modules);
    free(tmp);
  }

  if (1) {
    const char *env = getenv(ENV_MOD_DIR);
    if (env) {
      char *tmp = alloc_stringjoin(' ', name, 
				   "(envvar " ENV_MOD_DIR ")");
      print_separator();
      test_func(tmp, env, try_non_modules);
      free(tmp);
    }
  }

  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      char *tmp = alloc_stringjoin(' ', name, "(cmdline)");
      char *tmp2 = alloc_stringjoin(' ', tmp, argv[i]);
      print_separator();
      test_func(tmp2, argv[i], try_non_modules);
      free(tmp2);
      free(tmp);
    }
  }

  print_separator();
  printf("Symbols loaded: %d total, %d successful, %d errors.\n"
	 "Primary symbol: %d total, %d successful, %d errors.\n",
	 total_count, succ_count, total_count - succ_count,
	 primary_total, primary_total - primary_error, primary_error);
  printf("Iterations for \"%s\": Finished.\n", name);
}


/**********************************************************************
 * Test explicitly loading modules
 **********************************************************************/

static void 
test(const char *symname,
     const char *dirname, const char *filename)
{
  lt_dlhandle handle = NULL;
  char *fpath = alloc_pathjoin(dirname, filename);
  total_count++;

  handle = lt_dlopen(fpath);
  if (handle) {
    dynlibtest_func func = NULL;
    printf("  module file \"%s\" loaded\n", fpath);
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
}

typedef struct {
  char *symbol;
  char *fileext;
} sym_ext_tuple;

static void
explicit_load(const char *testname, const char *dir,
	      const int try_non_modules)
{
  static const char *module_list[] = {
    "mod_a",
    "mod_b",
    "mod_c",
    "mod_d",
    "mod_e",
    NULL
  };
  static const sym_ext_tuple symexts[] = {
    { "dynlibtest1", SOEXT },
    { "dynlibtest2", ".la" },
    { NULL, NULL }
  };
  printf("Test \"%s\": Starting.\n", testname);
  assert(lt_dlinit() == 0);
  if (1) {
    int i;
    for (i=0; module_list[i] != NULL; i++) {
      int j;
      for (j=0; symexts[j].symbol != NULL; j++) {
	char *tmp = alloc_stringjoin('\0', 
				     module_list[i], symexts[j].fileext);
	test(symexts[j].symbol, dir, tmp);
	free(tmp);
      }
    }
  }
  assert(lt_dlexit() == 0);
  printf("Test \"%s\": Finished.\n", testname);
}


/**********************************************************************
 * Test loading modules from path
 **********************************************************************/

static int
load_and_test (lt_dlhandle handle, const char *symname)
{
    dynlibtest_func func = NULL;

    func = lt_dlsym (handle, symname);
    mod_callback_count = 0;
    if (func != NULL) {
      char *tmp = func(NULL);
      succ_count++;
      printf("    Result of %s() (%d callbacks):\n      \"%s\"\n",
	     symname, mod_callback_count, tmp);
      return ((tmp == NULL) || (mod_callback_count != 1));
    } else {
      printf("    lt_dlsym of %s() failed:\n      %s\n",
	     symname, lt_dlerror());
      return (0 == 0);
    }
}


/* This is a cheap version of a set.
 * If we get the elements in order, we only need to compare to
 * the last element we have handled.
 *
 * We should use a (hash) map from module names to functions.
 */
static char *last_real_file = NULL;


typedef struct {
  char *symbol_name;
  int try_non_modules;
} foreach_params;


static int
foreach_func (const char *filename, lt_ptr data)
{
  const foreach_params *params = (const foreach_params *) data;
  lt_dlhandle handle = NULL;

  assert(filename != NULL);
  assert(data != NULL);
  printf("  Filename: %s\n", filename);
  total_count++;

  handle = lt_dlopenext(filename);
  if (handle != NULL) {
    char *module_name = NULL;

    if (1) {
      const lt_dlinfo *dlinfo = lt_dlgetinfo(handle);
      if (dlinfo != NULL) {
	if (dlinfo->name) {
	  module_name = dlinfo->name;
	  printf("    Module name: \"%s\"\n", module_name);
	}
	printf("    Module filename: \"%s\" (refcount %d)\n",
	       dlinfo->filename, dlinfo->ref_count);
	if (last_real_file != NULL) {
	  if (strcmp(last_real_file, dlinfo->filename) == 0) {
	    printf("      (duplicate module)\n");
	  }
	  free(last_real_file);
	  last_real_file = NULL;
	}
	last_real_file = strdup(dlinfo->filename);
	assert(last_real_file != NULL);
      } else {
	printf("    No module information:\n    %s\n",
	       lt_dlerror());
      }
    }
  
    if ((module_name == NULL) && (!params->try_non_modules)) {
      printf("    Ignoring non-module\n");
    } else {
      const int invert_primary_logic = 
	((module_name != NULL) && 
	 (strcmp(module_name, "mod_e") == 0));
      const int test_result = load_and_test(handle, params->symbol_name);
      primary_total++;
      if (test_result) {
	if (invert_primary_logic) {
	  printf("    The symbol was supposed to be local. Good.\n");
	} else {
	  printf("    PRIMARY error\n");
	  primary_error++;
	}
      } else {
	if (invert_primary_logic) {
	  printf("    Strange. The symbol was supposed to be local.\n");
	  succ_count--;
	} else {
	  /* Everything OK. */
	}
      }
      
      if (module_name) {
	const char *my_tmpname = 
	  alloc_stringjoin('_', "LTX", params->symbol_name);
	const char *my_symname = 
	  alloc_stringjoin('_', module_name, my_tmpname);
	total_count++;
	load_and_test(handle, my_symname);
      }
    }

    lt_dlclose(handle);
  }

  return 0;
}


static void
test_path(const char *testname, const char *dirname,
	  const int try_non_modules)
{
  foreach_params params = {
    "dynlibtest0",
    try_non_modules
  };
  printf("Test \"%s\" in \"%s\": Starting\n", testname, dirname);
  assert(lt_dlinit() == 0);
  lt_dlforeachfile(dirname, foreach_func, (lt_ptr) &params);
  assert(lt_dlexit() == 0);
  printf("Test \"%s\" in \"%s\": Finished\n", testname, dirname);
  if (last_real_file != NULL) {
    free(last_real_file);
    last_real_file = NULL;
  }
}


/**********************************************************************
 * Public library functions
 **********************************************************************/

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
dlt_test (const int argc, const char *argv[],
	  const int load_explicit_files,
	  const int try_non_modules)
{
  if (load_explicit_files) {
    test_iter("explicit loads", explicit_load,
	      argc, argv, try_non_modules);
  }
  test_iter("path search", test_path, 
	    argc, argv, try_non_modules);
  print_big_separator();
  return (primary_error == 0)?0:1;
}

void
dlt_mod_callback (void)
{
  mod_callback_count++;
}
