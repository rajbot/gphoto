#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-folder.h"

#include <gal/util/e-util.h>
#include <gal/e-table/e-table.h>
#include <gal/e-table/e-table-simple.h>

#include "utils.h"
#include "gnocam-configuration.h"

#define PARENT_TYPE E_TABLE_TYPE
static ETableClass* gnocam_folder_parent_class;

struct _GnoCamFolderPrivate
{
	GtkWidget*			window;
	
	Bonobo_UIContainer		container;
	BonoboUIComponent*		component;
	
	Bonobo_Storage			storage;
	Bonobo_Storage_DirectoryList*	list;

	GConfClient*			client;
	
	Camera*				camera;
	CameraWidget*			configuration;
	
	gchar*				path;

	GtkWidget*			widget;
};

#define E_TABLE_SPEC																		\
"<ETableSpecification>"																		\
"  <ETableColumn model_col=\"0\" _title=\"Filename\"     expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" compare=\"string\"/>"	\
"  <ETableColumn model_col=\"1\" _title=\"Content Type\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" compare=\"string\"/>"	\
"  <ETableColumn model_col=\"2\" _title=\"Size\"         expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"size\" compare=\"string\"/>"		\
"  <ETableState>"																		\
"    <column source=\"0\"/>"																	\
"    <column source=\"1\"/>"																	\
"    <column source=\"2\"/>"																	\
"    <grouping/>"																		\
"  </ETableState>"																		\
"</ETableSpecification>"

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
"  </menu>"													\
"  <dockitem name=\"Toolbar\">"											\
"    <toolitem name=\"Save\" _label=\"Save\" verb=\"\" pixtype=\"stock\" pixname=\"Save\"/>"			\
"    <toolitem name=\"SaveAs\" _label=\"Save As\" verb=\"\" pixtype=\"stock\" pixname=\"Save As\"/>"		\
"    <placeholder name=\"Upload\"/>"										\
"  </dockitem>"													\
"</Root>"

#define GNOCAM_FOLDER_UI_FOLDER					\
"<placeholder name=\"Folder\">"					\
"  <submenu name=\"Folder\" _label=\"Folder\">"			\
"    <placeholder name=\"Configuration\" delimit=\"top\"/>"	\
"  </submenu>"							\
"</placeholder>"

#define GNOCAM_FOLDER_UI_CONFIGURATION											\
"<placeholder name=\"Configuration\">"											\
"  <menuitem name=\"Configuration\" _label=\"Configuration\" verb=\"\" pixtype=\"stock\" pixname=\"Properties\"/>"	\
"</placeholder>"

#define GNOCAM_FOLDER_UI_UPLOAD_MENUITEM										\
"<placeholder name=\"Upload\">"												\
"  <menuitem name=\"Upload\" _label=\"Upload\" verb=\"\" pixtype=\"stock\" pixname=\"Open\"/>"				\
"</placeholder>"

#define GNOCAM_FOLDER_UI_UPLOAD_TOOLITEM                        							\
"<placeholder name=\"Upload\">"                                 							\
"  <toolitem name=\"Upload\" _label=\"Upload\" verb=\"\" pixtype=\"stock\" pixname=\"Open\"/>"				\
"</placeholder>"

/**************/
/* Prototypes */
/**************/

static void 	on_save_clicked 	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_save_as_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_upload_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_config_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

/**********************/
/* Internal functions */
/**********************/

static gint
set_container (gpointer user_data)
{
	GnoCamFolder*	folder;

	folder = GNOCAM_FOLDER (user_data);

	if (!folder->priv->component) return (TRUE);

	if (bonobo_ui_component_get_container (folder->priv->component) != folder->priv->container)
		bonobo_ui_component_set_container (folder->priv->component, folder->priv->container);

	return (FALSE);
}

static gint
create_menu (gpointer user_data)
{
	GnoCamFolder*	folder;

	g_return_val_if_fail (user_data, FALSE);
	folder = GNOCAM_FOLDER (user_data);

	folder->priv->component = bonobo_ui_component_new (PACKAGE "Folder");
	bonobo_ui_component_set_container (folder->priv->component, folder->priv->container);
	
        bonobo_ui_component_freeze (folder->priv->component, NULL);
	
        bonobo_ui_component_set_translate (folder->priv->component, "/", GNOCAM_FOLDER_UI, NULL);
        bonobo_ui_component_add_verb (folder->priv->component, "Save", on_save_clicked, folder);
        bonobo_ui_component_add_verb (folder->priv->component, "SaveAs", on_save_as_clicked, folder);

	/* Do we need a folder-menu? */
	if (folder->priv->camera->abilities->config & GP_CONFIG_FOLDER)
		bonobo_ui_component_set_translate (folder->priv->component, "/menu", GNOCAM_FOLDER_UI_FOLDER, NULL);

        /* Upload? */
        if (folder->priv->camera->abilities->file_put) {
                bonobo_ui_component_set_translate (folder->priv->component, "/menu/File/FileOperations", GNOCAM_FOLDER_UI_UPLOAD_MENUITEM, NULL);
                bonobo_ui_component_set_translate (folder->priv->component, "/Toolbar/Upload", GNOCAM_FOLDER_UI_UPLOAD_TOOLITEM, NULL);
                bonobo_ui_component_add_verb (folder->priv->component, "Upload", on_upload_clicked, folder);
        }

        /* Folder configuration? */
	if (folder->priv->camera->abilities->config & GP_CONFIG_FOLDER) {
		bonobo_ui_component_set_translate (folder->priv->component, "/menu/Folder/Folder", GNOCAM_FOLDER_UI_CONFIGURATION, NULL);
		bonobo_ui_component_add_verb (folder->priv->component, "Configuration", on_config_clicked, folder);
	}

        bonobo_ui_component_thaw (folder->priv->component, NULL);

	return (FALSE);
}

/*************/
/* Callbacks */
/*************/

static void
on_cancel_button_clicked (GtkButton* button, gpointer user_data)
{
	gtk_widget_destroy (gtk_widget_get_ancestor (GTK_WIDGET (button), GTK_TYPE_FILE_SELECTION));
}

static void
on_upload_ok_button_clicked (GtkButton* button, gpointer user_data)
{
	GnoCamFolder*	folder;

	folder = GNOCAM_FOLDER (user_data);

	g_message (_("Not implemented!"));

	gtk_widget_destroy (gtk_widget_get_ancestor (GTK_WIDGET (button), GTK_TYPE_FILE_SELECTION));
}

static void
on_upload_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*		folder;
	GtkFileSelection*	filesel;

	folder = GNOCAM_FOLDER (user_data);
	
	filesel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Upload")));
	gtk_widget_show (GTK_WIDGET (filesel));
	gtk_signal_connect (GTK_OBJECT (filesel->ok_button), "clicked", GTK_SIGNAL_FUNC (on_upload_ok_button_clicked), folder);
	gtk_signal_connect (GTK_OBJECT (filesel->cancel_button), "clicked", GTK_SIGNAL_FUNC (on_cancel_button_clicked), folder);
}

static void
on_save_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
#if 0
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
#endif
}

static void
on_config_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*	folder;
	GtkWidget*	widget;

	folder = GNOCAM_FOLDER (user_data);

	widget = gnocam_configuration_new (folder->priv->camera, folder->priv->path, NULL, folder->priv->window);
	gtk_widget_show (widget);
}

#if 0
static void
on_save_as_ok_button_clicked (GtkButton* ok_button, gpointer user_data)
{
	g_message (_("Not implemented!"));
}
#endif

static void
on_save_as_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
#if 0
	GnoCamFolder*   folder;
	GList*		selection;
	gint 		i;

	folder = GNOCAM_FOLDER (user_data);

	selection = g_list_first (GTK_CLIST (folder->priv->clist)->selection);

	for (i = 0; i < g_list_length (selection); i++) {
		gint			row;
		gchar*			name;
		GtkFileSelection*	filesel;

		row = GPOINTER_TO_INT (g_list_nth_data (selection, i));
		gtk_clist_get_text (GTK_CLIST (folder->priv->clist), row, 1, &name);

		filesel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Save As")));
		gtk_widget_show (GTK_WIDGET (filesel));
		gtk_file_selection_set_filename (filesel, name);
		gtk_signal_connect (GTK_OBJECT (filesel->ok_button), "clicked", GTK_SIGNAL_FUNC (on_save_as_ok_button_clicked), NULL);
		gtk_signal_connect (GTK_OBJECT (filesel->cancel_button), "clicked", GTK_SIGNAL_FUNC (on_cancel_button_clicked), NULL);
	}
#endif
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_folder_show_menu (GnoCamFolder* folder)
{
	g_return_if_fail (folder);

gtk_idle_add (set_container, folder);
//	bonobo_ui_component_set_container (folder->priv->component, folder->priv->container);
}

void
gnocam_folder_hide_menu (GnoCamFolder* folder)
{
	bonobo_ui_component_unset_container (folder->priv->component);
}

/***********/
/* E-Table */
/***********/

static gint
col_count (ETableModel* model, gpointer user_data)
{
	return (3);
}

static gint
row_count (ETableModel* model, gpointer user_data)
{
	GnoCamFolder*	folder;
	gint		i;
	gint		j = 0;

	folder = GNOCAM_FOLDER (user_data);

	for (i = 0; i < folder->priv->list->_length; i++) {
		if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_REGULAR) j++;
	}
	return (j);
}

static void*
value_at (ETableModel* model, gint col, gint row, gpointer user_data)
{
	GnoCamFolder*	folder;
	gint		i;

	folder = GNOCAM_FOLDER (user_data);

	for (i = 0; i < row + 1; i++)
		if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY) row++;

	if (col == 0) return (folder->priv->list->_buffer [row].name);
	if (col == 1) return (folder->priv->list->_buffer [row].content_type);
	if (col == 2) return (GINT_TO_POINTER (folder->priv->list->_buffer [row].size));
	
	return (NULL);
}

static void
set_value_at (ETableModel* model, gint col, gint row, const void* value, gpointer user_data)
{
}

static gboolean
is_cell_editable (ETableModel* model, gint col, gint row, gpointer user_data)
{
	return (FALSE);
}

static void*
duplicate_value (ETableModel* model, gint col, const void* value, gpointer user_data)
{
	return (NULL);
}

static void
free_value (ETableModel* model, gint col, void* value, gpointer user_data)
{
}

static void*
initialize_value (ETableModel* model, gint col, gpointer user_data)
{
	return (NULL);
}

static gboolean
value_is_empty (ETableModel* model, gint col, const void* value, gpointer user_data)
{
	return (FALSE);
}

static gchar*
value_to_string (ETableModel* model, gint col, const void* value, gpointer user_data)
{
	return (NULL);
}

/****************************/
/* Bonobo-X-Object specific */
/****************************/

static void
gnocam_folder_destroy (GtkObject* object)
{
	GnoCamFolder* folder;

	folder = GNOCAM_FOLDER (object);

	gtk_widget_unref (folder->priv->window);

	bonobo_object_release_unref (folder->priv->container, NULL);
	bonobo_object_unref (BONOBO_OBJECT (folder->priv->component));

	bonobo_object_release_unref (folder->priv->storage, NULL);

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
	folder->priv = g_new0 (GnoCamFolderPrivate, 1);
}

GtkWidget*
gnocam_folder_new (Camera* camera, Bonobo_Storage storage, const gchar* path, Bonobo_UIContainer container, GConfClient* client, GtkWidget* window)
{
	GnoCamFolder*			new;
	Bonobo_Storage_DirectoryList*   list;
	const gchar*			directory;
	CORBA_Environment		ev;
	ETableModel*			model;
	ETableExtras*			extras;

	if (!strcmp (path, "/")) directory = path;
	else directory = g_basename (path);

	CORBA_exception_init (&ev);
        list = Bonobo_Storage_listContents (storage, directory, Bonobo_FIELD_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
		g_warning (_("Could not get list of files for '%s': %s!"), path, bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return (NULL);
	}
	CORBA_exception_free (&ev);

	new = gtk_type_new (GNOCAM_TYPE_FOLDER);
	gp_camera_ref (new->priv->camera = camera);
	gtk_widget_ref (new->priv->window = window);
	new->priv->path = g_strdup (path);
	new->priv->storage = bonobo_object_dup_ref (storage, NULL);
	new->priv->list = list;
	new->priv->container = bonobo_object_dup_ref (container, NULL);
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));

	/* Create the model */
	model = e_table_simple_new (col_count, row_count, value_at, set_value_at, is_cell_editable, 
		duplicate_value, free_value, initialize_value, value_is_empty, value_to_string, new);

	/* Create the extras */
	extras = e_table_extras_new ();

	/* Create the table */
	e_table_construct (E_TABLE (new), model, extras, E_TABLE_SPEC, NULL);

	/* Create menu */
	gtk_idle_add (create_menu, new);
//	create_menu (new);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_folder, "GnoCamFolder", GnoCamFolder, gnocam_folder_class_init, gnocam_folder_init, PARENT_TYPE);

