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


/**
 * Various Powershot camera types
 */
typedef enum {
  CANON_PS_A5,
  CANON_PS_A5_ZOOM,
  CANON_PS_A50,
  CANON_PS_S10,
  CANON_PS_S20,
  CANON_PS_A70
} canonCamModel;


struct canon_info
{
  canonCamModel model;
  int speed;        /* The speed we're using for this camera */
  char ident[32];   /* Model ID string given by the camera */
  char owner[32];   /* Owner name */
};


extern struct canon_info camera;
extern char psa50_id[]; /* ditto @@@ */
int A5;

typedef unsigned long u32;
#ifndef byteswap32
#ifdef __sparc
#define byteswap32(val) ({ u32 x = val; x = (x << 24) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | (x >> 24); x; })
#else
#define byteswap32(val) val
#endif
#endif


/*
 * All functions returning a pointer have malloc'ed the data. The caller must
 * free() it when done.
 */

/**
 * Switches the camera on, detects the model and sets its speed
 */
int psa50_ready(void);

/**
 *
 */
char *psa50_get_disk(void);

/**
 *
 */
int psa50_disk_info(const char *name,int *capacity,int *available);

/**
 *
 */
struct psa50_dir *psa50_list_directory(const char *name);
void psa50_free_dir(struct psa50_dir *list);
unsigned char *psa50_get_file(const char *name,int *length);
unsigned char *psa50_get_thumbnail(const char *name,int *length);
int psa50_delete_file(const char *name, const char *dir);
int psa50_end(void);
int psa50_off(void);
int psa50_sync_time(void);
time_t psa50_get_time(void);
int psa50_get_owner_name(void);
int psa50_set_owner_name(const char *name);

#endif
