#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
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
	static GdkPixbuf*       pixbuf = NULL;

	if (pixbuf) return (pixbuf);
	
	if (!g_file_exists (IMAGEDIR "/gnome-folder.png")) return (NULL);

	pixbuf = scale_pixbuf (gdk_pixbuf_new_from_file (IMAGEDIR "/gnome-folder.png"));
	return (pixbuf);
}

GdkPixbuf*
util_pixbuf_file (void)
{
	static GdkPixbuf*	pixbuf = NULL;

	if (pixbuf) return (pixbuf);

	if (!g_file_exists (IMAGEDIR "/gnome-file-h.png")) return (NULL);

	pixbuf = scale_pixbuf (gdk_pixbuf_new_from_file (IMAGEDIR "/gnome-file-h.png"));
	return (pixbuf);
}

GdkPixbuf*
util_pixbuf_lock (void)
{
	static GdkPixbuf*	pixbuf = NULL;

	if (pixbuf) return (pixbuf);

	if (!g_file_exists (IMAGEDIR "/gnome-lockscreen.png")) return (NULL);

	pixbuf = scale_pixbuf (gdk_pixbuf_new_from_file (IMAGEDIR "/gnome-lockscreen.png"));
	return (pixbuf);
}

GdkPixbuf*
util_pixbuf_unlock (void)
{
	static GdkPixbuf*	pixbuf = NULL;

	if (pixbuf) return (pixbuf);

	if (!g_file_exists (IMAGEDIR "/gnocam-unlock.png")) return (NULL);

	pixbuf = scale_pixbuf (gdk_pixbuf_new_from_file (IMAGEDIR "/gnocam-unlock.png"));
	return (pixbuf);
}

