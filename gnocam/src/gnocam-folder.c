#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-folder.h"

#include <gal/util/e-util.h>
#include <gal/widgets/e-scroll-frame.h>

#include "utils.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass* gnocam_folder_parent_class;

struct _GnoCamFolderPrivate
{
	Bonobo_Storage		storage;
	BonoboUIComponent*	component;

	GConfClient*		client;
	
	Camera*			camera;
	CameraWidget*		configuration;
	
	gchar*			path;

	GtkWidget*		widget;
	GtkWidget*		clist;
};

#define GNOCAM_FOLDER_UI	 										\
"<Root>"													\
"  <menu>"													\
"    <submenu name=\"File\">"											\
"      <placeholder name=\"FileOperations\">"									\
"        <menuitem name=\"Save\" _label=\"Save\" verb=\"\" pixtype=\"stock\" pixname=\"Save\"/>"		\
"        <menuitem name=\"SaveAs\" _label=\"Save As\" verb=\"\" pixtype=\"stock\" pixname=\"Save As\"/>"	\
"        <placeholder name=\"Upload\"/>"									\
"      </placeholder>"												\
"    </submenu>"												\
"    <placeholder name=\"Folder\">"										\
"      <submenu name=\"Folder\" _label=\"Folder\">"								\
"        <placeholder name=\"Upload\"/>"									\
"        <placeholder name=\"Configuration\"/>"									\
"      </submenu>"												\
"    </placeholder>"												\
"  </menu>"													\
"  <dockitem name=\"Toolbar\">"											\
"    <toolitem name=\"Save\" _label=\"Save\" verb=\"\" pixtype=\"stock\" pixname=\"Save\"/>"			\
"    <toolitem name=\"SaveAs\" _label=\"Save As\" verb=\"\" pixtype=\"stock\" pixname=\"Save As\"/>"		\
"    <placeholder name=\"Upload\"/>"										\
"  </dockitem>"													\
"</Root>"

#define GNOCAM_FOLDER_UI_UPLOAD_MENUITEM			\
"<placeholder name=\"Upload\">"					\
"  <menuitem name=\"Upload\" _label=\"Upload\" verb=\"\"/>"	\
"</placeholder/>"

#define GNOCAM_FOLDER_UI_UPLOAD_TOOLITEM                        \
"<placeholder name=\"Upload\">"                                 \
"  <toolitem name=\"Upload\" _label=\"Upload\" verb=\"\"/>"     \
"</placeholder/>"

/*************/
/* Callbacks */
/*************/

static void
on_upload_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*	folder;

	folder = GNOCAM_FOLDER (user_data);
	
	g_warning ("Implement upload!");
}

static void
on_save_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*	folder;
	GList*		selection;
	gint		i;
	gchar*		path;

	folder = GNOCAM_FOLDER (user_data);
	selection = g_list_first (GTK_CLIST (folder->priv->clist)->selection);
	path = gconf_client_get_string (folder->priv->client, "/apps/" PACKAGE "/prefix", NULL);
	g_return_if_fail (path);

	for (i = 0; i < g_list_length (selection); i++) {
		Bonobo_Stream		stream;
		BonoboStream*		dest;
		Bonobo_Stream_iobuf*    buffer;
		gint 			row;
		gchar*			name;
		gchar*			full_path;
		CORBA_Environment	ev;

		row = GPOINTER_TO_INT (g_list_nth_data (selection, i));
		gtk_clist_get_text (GTK_CLIST (folder->priv->clist), row, 1, &name);

		CORBA_exception_init (&ev);
		stream = Bonobo_Storage_openStream (folder->priv->storage, name, Bonobo_Storage_READ, &ev);
		if (BONOBO_EX (&ev)) {
			g_warning (_("Could not get stream for '%s': %s"), name, bonobo_exception_get_text (&ev));
			CORBA_exception_free (&ev);
			continue;
		}
                Bonobo_Stream_read (stream, 4000000, &buffer, &ev);
		Bonobo_Stream_unref (stream, NULL);
		if (BONOBO_EX (&ev)) {
			g_warning (_("Could not read stream: %s"), bonobo_exception_get_text (&ev));
			CORBA_exception_free (&ev);
			continue;
		}
		if (!strcmp (path, "/")) full_path = g_strconcat ("/", name, NULL);
		else full_path = g_strdup_printf ("%s/%s", path, name);
		dest = bonobo_stream_open_full ("fs", full_path, Bonobo_Storage_WRITE | Bonobo_Storage_CREATE, 0664, &ev);
                if (BONOBO_EX (&ev)) {
			g_warning (_("Could not get stream for '%s': %s"), full_path, bonobo_exception_get_text (&ev));
			g_free (full_path);
			CORBA_free (buffer);
			CORBA_exception_free (&ev);
			continue;
		}
		g_free (full_path);
                Bonobo_Stream_write (BONOBO_OBJREF (dest), buffer, &ev);
		Bonobo_Stream_unref (BONOBO_OBJREF (dest), NULL);
		CORBA_free (buffer);
		if (BONOBO_EX (&ev)) {
			g_warning (_("Could not write stream: %s"), bonobo_exception_get_text (&ev));
			CORBA_exception_free (&ev);
			continue;
                }
		CORBA_exception_free (&ev);
	}
}

static void
on_cancel_button_clicked (GtkButton* cancel_button, gpointer user_data)
{
	gtk_widget_destroy (gtk_widget_get_ancestor (GTK_WIDGET (cancel_button), GTK_TYPE_WINDOW));
}

static void
on_ok_button_clicked (GtkButton* ok_button, gpointer user_data)
{
	g_warning ("Implement");
}

static void
on_save_as_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*   folder;
	GList*		selection;
	gint 		i;

	folder = GNOCAM_FOLDER (user_data);

	selection = g_list_first (GTK_CLIST (folder->priv->clist)->selection);

	for (i = 0; i < g_list_length (selection); i++) {
		gint		row;
		gchar*		name;
		GtkWidget*	widget;

		row = GPOINTER_TO_INT (g_list_nth_data (selection, i));
		gtk_clist_get_text (GTK_CLIST (folder->priv->clist), row, 1, &name);

		gtk_widget_show (widget = gtk_file_selection_new (_("Save As")));
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (widget), name);
		gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (widget)->ok_button), "clicked", GTK_SIGNAL_FUNC (on_ok_button_clicked), NULL);
		gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (widget)->cancel_button), "clicked", GTK_SIGNAL_FUNC (on_cancel_button_clicked), NULL);
	}
}

/*****************/
/* Our functions */
/*****************/

BonoboUIComponent*
gnocam_folder_get_ui_component (GnoCamFolder* folder)
{
	return (folder->priv->component);
}

void
gnocam_folder_set_ui_container (GnoCamFolder* folder, BonoboUIContainer* container)
{
        bonobo_ui_component_set_container (folder->priv->component, BONOBO_OBJREF (container));
//	bonobo_object_unref (BONOBO_OBJECT (container));
}

GtkWidget*
gnocam_folder_get_widget (GnoCamFolder* folder)
{
        return (folder->priv->widget);
}

/****************************/
/* Bonobo-X-Object specific */
/****************************/

static void
gnocam_folder_destroy (GtkObject* object)
{
	GnoCamFolder* folder;

	folder = GNOCAM_FOLDER (object);

	bonobo_object_unref (BONOBO_OBJECT (folder->priv->component));
	if (folder->priv->configuration) gp_widget_unref (folder->priv->configuration);
	gp_camera_unref (folder->priv->camera);
	g_free (folder->priv->path);
	gtk_object_unref (GTK_OBJECT (folder->priv->client));
	
	g_free (folder->priv);
}

static void
gnocam_folder_class_init (GnoCamFolderClass* klass)
{
	GtkObjectClass* 	object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_folder_destroy;

	gnocam_folder_parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_folder_init (GnoCamFolder* folder)
{
	GnoCamFolderPrivate*	priv;

	priv = g_new (GnoCamFolderPrivate, 1);
	folder->priv = priv;
	folder->priv->configuration = NULL;
}

GnoCamFolder*
gnocam_folder_new (Camera* camera, Bonobo_Storage storage, const gchar* path, BonoboUIContainer* container, GConfClient* client)
{
	GnoCamFolder*			new;
	Bonobo_Storage_DirectoryList*   list;
	gchar*				row [] = {NULL, NULL};
	const gchar*			directory;
	gint				i;
	gint				result;
	CORBA_Environment		ev;

	if (!strcmp (path, "/")) directory = path;
	else directory = g_basename (path);

	CORBA_exception_init (&ev);
        list = Bonobo_Storage_listContents (storage, directory, Bonobo_FIELD_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		return (NULL);
	}
	CORBA_exception_free (&ev);

	new = gtk_type_new (GNOCAM_TYPE_FOLDER);
	gp_camera_ref (new->priv->camera = camera);
	new->priv->path = g_strdup (path);
	new->priv->storage = storage;
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));

	/* Create the scroll-frame */
	gtk_widget_show (new->priv->widget = e_scroll_frame_new (NULL, NULL));
	e_scroll_frame_set_policy (E_SCROLL_FRAME (new->priv->widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (new->priv->widget), GTK_SHADOW_IN);

	/* Create the clist */
        gtk_widget_show (new->priv->clist = gtk_clist_new (1));
	gtk_clist_set_selection_mode (GTK_CLIST (new->priv->clist), GTK_SELECTION_MULTIPLE);
	gtk_container_add (GTK_CONTAINER (new->priv->widget), new->priv->clist);
        for (i = 0; i < list->_length; i++) {
		if (list->_buffer [i].type == Bonobo_STORAGE_TYPE_REGULAR) {
                	row [0] = g_strdup (list->_buffer [i].name);
			gtk_clist_append (GTK_CLIST (new->priv->clist), row);
		}
        }

	/* Create menu */
	new->priv->component = bonobo_ui_component_new (PACKAGE "Folder");
	bonobo_ui_component_set_container (new->priv->component, BONOBO_OBJREF (container));
	bonobo_ui_component_set_translate (new->priv->component, "/", GNOCAM_FOLDER_UI, NULL);
	bonobo_ui_component_add_verb (new->priv->component, "Save", on_save_clicked, new);
	bonobo_ui_component_add_verb (new->priv->component, "SaveAs", on_save_as_clicked, new);

        /* Upload? */
        if (new->priv->camera->abilities->file_put) {
		bonobo_ui_component_set_translate (new->priv->component, "/menu/File/FileOperations", GNOCAM_FOLDER_UI_UPLOAD_MENUITEM, NULL);
		bonobo_ui_component_set_translate (new->priv->component, "/Toolbar/Upload", GNOCAM_FOLDER_UI_UPLOAD_TOOLITEM, NULL);
                bonobo_ui_component_add_verb (new->priv->component, "Upload", on_upload_clicked, new);
        }

        /* Folder configuration? */
        result = gp_camera_folder_config_get (new->priv->camera, &(new->priv->configuration), new->priv->path);
	if (result == GP_OK) menu_setup (new->priv->component, new->priv->camera, new->priv->configuration, "/menu/Folder/Folder", new->priv->path, NULL);

	return (new);
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamFolder, GNOME_GnoCam_folder, PARENT_TYPE, gnocam_folder);

