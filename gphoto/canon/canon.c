/****************************************************************************
 * 
 * canon.c
 *
 *   Canon Camera library for the gphoto project,
 *   (c) 1999 Wolfgang G. Reissnegger
 *   Developed for the Canon PowerShot A50
 *
 ****************************************************************************/

/****************************************************************************
 *
 * include files
 *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../src/gphoto.h"

#include "util.h"
#include "serial.h"
#include "psa50.h"
#include "canon.h"

#define D(c)  c


/*
 * Directory access may be rather expensive, so we cache some information.
 * The first variable in each block indicates whether the block is valid.
 */

static int cached_ready = 0;

static int cached_disk = 0;
static char cached_drive[10]; /* usually something like C: */
static int cached_capacity,cached_available;

static int cached_dir = 0;
static char **cached_paths;
static int cached_images;


static void clear_readiness(void)
{
    cached_ready = 0;
}


static int check_readiness(void)
{
    if (cached_ready) return 1;
    if (psa50_ready()) {
	cached_ready = 1;
	return 1;
    }
    update_status("Camera unavailable");
    return 0;
}


static int update_disk_cache(void)
{
    char root[10]; /* D:\ or such */
    char *disk;

    if (cached_disk) return 1;
    if (!check_readiness()) return 0;
    disk = psa50_get_disk();
    if (!disk) {
	update_status("No response");
	return 0;
    }
    strcpy(cached_drive,disk);
    sprintf(root,"%s\\",disk);
    if (!psa50_disk_info(root,&cached_capacity,&cached_available)) {
	update_status("No response");
	return 0;
    }
    cached_disk = 1;
    return 1;
}


static int is_image(const char *name)
{
    const char *pos;

    pos = strchr(name,'.');
    if (!pos) return 0;
    return !strcmp(pos,".JPG");
}


static int recurse(const char *name)
{
    struct psa50_dir *dir,*walk;
    char buffer[300]; /* longest path, etc. */
    int count,curr;

    dir = psa50_list_directory(name);
    if (!dir) return 1; /* assume it's empty @@@ */
    count = 0;
    for (walk = dir; walk->name; walk++)
	if (walk->size && is_image(walk->name)) count++;
    cached_paths = realloc(cached_paths,sizeof(char *)*(cached_images+count+1));
    memset(cached_paths+cached_images+1,0,sizeof(char *)*count);
    if (!cached_paths) {
	perror("realloc");
	return 0;
    }
    curr = cached_images;
    cached_images += count;
    for (walk = dir; walk->name; walk++) {
	sprintf(buffer,"%s\\%s",name,walk->name);
	if (!walk->size) {
	    if (!recurse(buffer)) return 0;
	}
	else {
	    if (!is_image(walk->name)) continue;
	    curr++;
	    cached_paths[curr] = strdup(buffer);
	    if (!cached_paths[curr]) {
		perror("strdup");
		return 0;
	    }
	}
    }
    free(dir);
    return 1;
}


static void clear_dir_cache(void)
{
    int i;

    for (i = 0; i < cached_images; i++)
	if (cached_paths[i]) free(cached_paths[i]);
    free(cached_paths);
}


static int update_dir_cache(void)
{
    if (cached_dir) return 1;
    if (!update_disk_cache()) return 0;
    if (!check_readiness()) return 0;
    cached_images = 0;
    if (recurse(cached_drive)) {
	cached_dir = 1;
	return 1;
    }
    clear_dir_cache();
    return 0;    
}


/****************************************************************************
 *
 * gphoto library interface calls
 *
 ****************************************************************************/


static int canon_configure(void)
{
    if (!confirm_dialog("Clear driver caches ?")) return 1;
    cached_ready = 0;
    cached_disk = 0;
    if (cached_dir) clear_dir_cache();
    cached_dir = 0;
    return 1;
}

/****************************************************************************/

static struct Image *canon_get_picture(int picture_number, int thumbnail)
{
    struct Image *image;

    if (thumbnail) return NULL;
    clear_readiness();
    image = malloc(sizeof(*image));
    if (!image) {
	perror("malloc");
	return NULL;
    }
    memset(image,0,sizeof(*image));
    strcpy(image->image_type,"jpg");
    if (!picture_number || picture_number > cached_images) {
	update_status("Invalid index");
	free(image);
	return NULL;
    }
    update_status(cached_paths[picture_number]);
    if (!check_readiness()) {
	free(image);
	return NULL;
    }
    image->image = psa50_get_file(cached_paths[picture_number],
      &image->image_size);
    if (image->image) return image;
    free(image);
    return NULL;
}


/****************************************************************************/


static int canon_number_of_pictures(void)
{
    clear_readiness();
    if (!update_dir_cache()) {
	update_status("Could not obtain directory listing");
	return 0;
    }
    return cached_images;
};

/****************************************************************************/

static int canon_initialize(void)
{
    D(printf("canon_initialize()\n"));

    return !canon_serial_init(serial_port);
}


/****************************************************************************/


static void pretty_number(int number,char *buffer)
{
    int len,tmp,digits;
    char *pos;

    len = 0;
    tmp = number;
    do {
	len++;
	tmp /= 10;
    }
    while (tmp);
    len += (len-1)/3;
    pos = buffer+len;
    *pos = 0;
    digits = 0;
    do {
	*--pos = (number % 10)+'0';
	number /= 10;
	if (++digits == 3) {
	    *--pos = '\'';
	    digits = 0;
	}
    }
    while (number);
}


static char *canon_summary(void)
{
    static char buffer[200],a[20],b[20];

    clear_readiness();
    if (!update_disk_cache()) return "Could not obtain disk information";
    pretty_number(cached_capacity,a);
    pretty_number(cached_available,b);
    sprintf(buffer,"%s\nDrive %s\n%11s bytes total\n%11s bytes available",
      psa50_id,cached_drive,a,b);
    return buffer;
}

/****************************************************************************/

static char *canon_description(void)
{
    return ("Canon PowerShot A50 by\n"
	    "Wolfgang G. Reissnegger,\n"
	    "Werner Almesberger.\n");
}

/****************************************************************************/


static struct Image *canon_get_preview(void) { return NULL; }
static int canon_delete_image(int picture_number) { return 0; }
static int canon_take_picture(void) { return 0; };


struct _Camera canon =
{
    canon_initialize,
    canon_get_picture,
    canon_get_preview,
    canon_delete_image,
    canon_take_picture,
    canon_number_of_pictures,
    canon_configure,
    canon_summary,
    canon_description
};
