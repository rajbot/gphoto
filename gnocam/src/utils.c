#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "utils.h"
#include "e-shell-constants.h"

static GdkPixbuf*
scale_pixbuf (GdkPixbuf* pixbuf)
{
        GdkPixbuf*      pixbuf_scaled;

        pixbuf_scaled = gdk_pixbuf_new (
                gdk_pixbuf_get_colorspace (pixbuf), gdk_pixbuf_get_has_alpha (pixbuf), gdk_pixbuf_get_bits_per_sample (pixbuf),
                E_SHELL_MINI_ICON_SIZE, E_SHELL_MINI_ICON_SIZE);
        gdk_pixbuf_scale (pixbuf, pixbuf_scaled, 0, 0, E_SHELL_MINI_ICON_SIZE, E_SHELL_MINI_ICON_SIZE, 0.0, 0.0,
                (double) E_SHELL_MINI_ICON_SIZE / gdk_pixbuf_get_width (pixbuf),
                (double) E_SHELL_MINI_ICON_SIZE / gdk_pixbuf_get_height (pixbuf), GDK_INTERP_HYPER);
        return (pixbuf_scaled);
}

GdkPixbuf*
util_pixbuf_folder (void)
{
	if (!g_file_exists ("/usr/share/pixmaps/gnome-folder.png")) return (NULL);
	return (scale_pixbuf (gdk_pixbuf_new_from_file ("/usr/share/pixmaps/gnome-folder.png")));
}

GdkPixbuf*
util_pixbuf_file (void)
{
	if (!g_file_exists ("/usr/share/pixmaps/gnome-file-h.png")) return (NULL);
	return (scale_pixbuf (gdk_pixbuf_new_from_file ("/usr/share/pixmaps/gnome-file-h.png")));
}

