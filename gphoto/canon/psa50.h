/*
 * psa50.h - Canon PowerShot A50 "native" operations.
 *
 * Written 1999 by Wolfgang G. Reissnegger and Werner Almesberger
 */

#ifndef PSA50_H
#define PSA50_H

/* @@@ change this to canon_dir when merging with other models */
struct psa50_dir {
    const char *name; /* NULL if at end */
    size_t size;
    time_t date;
    int is_file;
    void *user;	/* user-specific data */
};


extern char psa50_id[]; /* ditto @@@ */

/*
 * All functions returning a pointer have malloc'ed the data. The caller must
 * free() it when done.
 */

int psa50_ready(void);
char *psa50_get_disk(void);
int psa50_disk_info(const char *name,int *capacity,int *available);
struct psa50_dir *psa50_list_directory(const char *name);
void psa50_free_dir(struct psa50_dir *list);
unsigned char *psa50_get_file(const char *name,int *length);

#endif
