/****************************************************************************
 * 
 * canon.c
 *
 *   Canon Camera library for the gphoto project,
 *   (c) 1999 Wolfgang G. Reissnegger
 *   Developed for the Canon PowerShot A50
 *   Additions for PowerShot A5 by Ole W. Saastad
 *
 ****************************************************************************/

 /****************************************************************************
 *
 * OWS 990925 Changed canon_get_picture and canon_number_of_pictures to
 *            work with A5. 
 *
 *
 ****************************************************************************/

/****************************************************************************
 *
 * include files
 *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gtk/gtk.h>

#include "../src/gphoto.h"

#include "util.h"
#include "serial.h"
#include "psa50.h"
#include "canon.h"

#define D(c)  c

/* #define A5 TRUE */

/*
 * Directory access may be rather expensive, so we cache some information.
 * The first variable in each block indicates whether the block is valid.
 */

static int cached_ready = 0;

static int cached_disk = 0;
static char cached_drive[10]; /* usually something like C: */
static int cached_capacity,cached_available;

static int cached_dir = 0;
static struct psa50_dir *cached_tree;
static int cached_images;


static void clear_readiness(void)
{
    cached_ready = 0;
}

static int check_readiness(void)
{
    if (cached_ready) return 1;
    if (psa50_ready()) {
       D(printf("A5 i canon.c %d\n",A5));
       if (A5) update_status("Powershot A5");
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


static int comp_dir(const void *a,const void *b)
{
    return strcmp(((const struct psa50_dir *) a)->name,
      ((const struct psa50_dir *) b)->name);
}


static struct psa50_dir *dir_tree(const char *path)
{
    struct psa50_dir *dir,*walk;
    char buffer[300]; /* longest path, etc. */

    dir = psa50_list_directory(path);
    if (!dir) return NULL; /* assume it's empty @@@ */
    for (walk = dir; walk->name; walk++) {
	if (walk->is_file) {
	    if (is_image(walk->name)) cached_images++;
	}
	else {
	    sprintf(buffer,"%s\\%s",path,walk->name);
	    walk->user = dir_tree(buffer);
	}
    }
    qsort(dir,walk-dir,sizeof(*dir),comp_dir);
    return dir;
}


static void clear_dir_cache(void)
{
    psa50_free_dir(cached_tree);
}


static int update_dir_cache(void)
{
    if (cached_dir) return 1;
    if (!update_disk_cache()) return 0;
    if (!check_readiness()) return 0;
    cached_images = 0;
    cached_tree = dir_tree(cached_drive);
    if (!cached_tree) return 0;
    cached_dir = 1;
    return 1;
}


/****************************************************************************
 *
 * gphoto library interface calls
 *
 ****************************************************************************/


static int _entry_path(const struct psa50_dir *tree,
  const struct psa50_dir *entry,char *path)
{
    path = strchr(path,0);
    while (tree->name) {
	*path = '\\';
	strcpy(path+1,tree->name);
	if (tree == entry) return 1;
	if (!tree->is_file && tree->user)
	    if (_entry_path(tree->user,entry,path)) return 1;
	tree++;
    }
    return 0;
}


static char *entry_path(const struct psa50_dir *tree,
  const struct psa50_dir *entry)
{
    static char path[300];

    strcpy(path,cached_drive);
    (void) _entry_path(cached_tree,entry,path);
    return path;
}


static void cb_select(GtkItem *item,struct psa50_dir *entry)
{
    char *path;
    unsigned char *file;
    int length,size;
    int fd;

    if (!entry || !entry->is_file) {
	gtk_item_deselect(item);
	return;
    }
    path = entry_path(cached_tree,entry);
    update_status(path);
    file = psa50_get_file(path,&length);
    if (!file) return;
    fd = creat(entry->name,0644);
    if (fd < 0) {
	perror("creat");
	free(file);
	return;
    }
    size = write(fd,file,length);
    if (size < 0) perror("write");
    else if (size < length) fprintf(stderr,"short write: %d/%d\n",size,length);
    if (close(fd) < 0) perror("close");
    free(file);
    update_status("File saved");
}


static int populate(struct psa50_dir *entry,GtkWidget *branch)
{
    GtkWidget *item,*subtree;

    item = gtk_tree_item_new_with_label(entry ? (char *) entry->name :
      cached_drive);
    if (!item) return 0;
    gtk_tree_append(GTK_TREE(branch),item);
    gtk_widget_show(item);
    gtk_signal_connect(GTK_OBJECT(item),"select",GTK_SIGNAL_FUNC(cb_select),
      entry);
    if (entry && entry->is_file) {
	entry->user = item;
	return 1;
    }
    entry = entry ? entry->user : cached_tree;
    if (!entry) return 1;
    subtree = gtk_tree_new();
    if (!subtree) return 0;
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(item),subtree);
    gtk_tree_item_expand(GTK_TREE_ITEM(item));
    for (; entry->name; entry++)
	if (!populate(entry,subtree)) return 0;
    return 1;
}


static void cb_clear(GtkWidget *widget,GtkWidget *window)
{
    gtk_widget_destroy(window);
    cached_ready = 0;
    cached_disk = 0;
    if (cached_dir) clear_dir_cache();
    cached_dir = 0;
}

static void cb_done(GtkWidget *widget,GtkWidget *window)
{
    gtk_widget_destroy(window);
}


static int canon_configure(void)
{
    GtkWidget *window,*box,*scrolled_win,*tree,*clear,*done;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!window) return 0;
    gtk_signal_connect(GTK_OBJECT(window),"delete_event",
      GTK_SIGNAL_FUNC(gtk_widget_destroy),window);
    box = gtk_vbox_new(FALSE,0);
    if (!box) {
	gtk_widget_destroy(window);
	return 0;
    }
    gtk_container_add(GTK_CONTAINER(window),box);
    gtk_widget_show(box);
    scrolled_win = gtk_scrolled_window_new(NULL,NULL);
    if (!scrolled_win) {
	gtk_widget_destroy(window);
	return 0;
    }
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
      GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize(scrolled_win,200,400);
    gtk_widget_show(scrolled_win);
    gtk_box_pack_start(GTK_BOX(box),scrolled_win,TRUE,TRUE,FALSE);
    clear = gtk_button_new_with_label("Clear camera cache");
    if (!clear) {
	gtk_widget_destroy(window);
	return 0;
    }
    gtk_signal_connect(GTK_OBJECT(clear),"clicked",GTK_SIGNAL_FUNC(cb_clear),
      window);
    gtk_widget_show(clear);
    gtk_box_pack_start(GTK_BOX(box),clear,FALSE,FALSE,FALSE);
    done = gtk_button_new_with_label("Done");
    if (!done) {
	gtk_widget_destroy(window);
	return 0;
    }
    gtk_signal_connect(GTK_OBJECT(done),"clicked",GTK_SIGNAL_FUNC(cb_done),
      window);
    gtk_widget_show(done);
    gtk_box_pack_start(GTK_BOX(box),done,FALSE,FALSE,FALSE);
    tree = gtk_tree_new();
    if (!tree) {
	gtk_widget_destroy(window);
	return 0;
    }
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_win),
      tree);
    gtk_widget_show(tree);
    populate(NULL,tree);
    gtk_widget_show(window);
//    find(cached_tree,cached_drive,entry);
    return 1;
}

/****************************************************************************/

static int _pick_nth(struct psa50_dir *tree, int n, char *path) {          
                                                                           
  int i=0;                                                                 
                                                                           
  if (tree == NULL)                                                        
    return 0;                                                              
                                                                           
  path = strchr(path, 0);                                                  
  *path = '\\';                                                            
                                                                           
  while (i<n && tree->name) {                                              
    strcpy(path+1, tree->name);                                            
    if (is_image(tree->name))                                              
      i++;                                                                 
    else if (!tree->is_file)                                               
      i += _pick_nth(tree->user, n-i, path);                               
    tree++;                                                                
  }                                                                        
  return i;                                                                
}                                                                          

static void pick_nth(int n,char *path)
{
    (void) _pick_nth(cached_tree,n,path);
}


static struct Image *canon_get_picture(int picture_number, int thumbnail)
{
    struct Image *image;
    char path[300];

    if (A5) {
      picture_number=picture_number*2-1;
      if (thumbnail) picture_number+=1;
      D(printf("Picture number %d\n",picture_number));

    } else /* For A50 or others */
      if (thumbnail) return NULL;

    clear_readiness();
    if (!update_dir_cache()) {
	update_status("Could not obtain directory listing");
	return 0;
    }
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
    strcpy(path,cached_drive);
    pick_nth(picture_number,path);
    update_status(path);
    if (!check_readiness()) {
	free(image);
	return NULL;
    }
    image->image = psa50_get_file(path,&image->image_size);
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
  if (A5) return cached_images/2; /* Odd is pictures even is thumbs */
  else  return cached_images;
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
            "Werner Almesberger.\n"
            "A5 additions by Ole W. Saastad\n");

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
