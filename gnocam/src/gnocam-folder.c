#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-folder.h"

#include <gal/util/e-util.h>
#include <gal/e-table/e-table.h>
#include <gal/e-table/e-table-simple.h>
#include <gal/e-table/e-cell-toggle.h>

#include "utils.h"
#include "gnocam-configuration.h"

#define PARENT_TYPE E_TABLE_TYPE
static ETableClass* parent_class;

struct _GnoCamFolderPrivate
{
	Bonobo_UIContainer		container;

	BonoboUIComponent*		component;
	
	Bonobo_Storage			storage;
	Bonobo_Storage_DirectoryList*	list;

	GConfClient*			client;
	guint				cnxn;
	
	Camera*				camera;
	CameraWidget*			configuration;
	
	gchar*				path;

	GtkWidget*			widget;

	ETableModel*			model;

	GHashTable*			hash_table;
};

#define E_TABLE_SPEC																		\
"<ETableSpecification>"																		\
"  <ETableColumn model_col=\"0\" _title=\"Filename\"     expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\"  cell=\"string\"          compare=\"string\"/>"\
"  <ETableColumn model_col=\"1\" _title=\"Content Type\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\"  cell=\"string\"          compare=\"string\"/>"\
"  <ETableColumn model_col=\"2\" _title=\"Size\"         expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\"  cell=\"size\"            compare=\"string\"/>"\
"  <ETableColumn model_col=\"3\" _title=\"Permission\"   expansion=\"0.0\" minimum_width=\"20\" resizable=\"false\" cell=\"cell_permission\" compare=\"integer\""\
"                pixbuf=\"permission\" sortable=\"false\"/>"													\
"  <ETableState>"																		\
"    <column source=\"0\"/>"																	\
"    <column source=\"3\"/>"																	\
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
"        <placeholder name=\"Delete\"/>"									\
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

#define GNOCAM_FOLDER_UI_DELETE					\
"<placeholder name=\"Delete\">"					\
"  <menuitem name=\"Delete\" _label=\"Delete\" verb=\"\"/>"	\
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
static void	on_delete_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_config_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

static void	on_save_as_ok_button_clicked	(GtkButton* button, gpointer user_data);
static void	on_cancel_button_clicked	(GtkButton* button, gpointer user_data);

/**********************/
/* Internal functions */
/**********************/

static void
update_folder (GnoCamFolder* folder)
{
        CORBA_Environment       ev;
	CameraFileInfo*		info;
	gint			i, result;

	/* Free old list */
	if (folder->priv->list) {
		for (i = 0; i < folder->priv->list->_length; i++) {
			if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_REGULAR) {
				info = g_hash_table_lookup (folder->priv->hash_table, folder->priv->list->_buffer [i].name);
				if (info) {
					g_free (info);
					g_hash_table_remove (folder->priv->hash_table, folder->priv->list->_buffer [i].name);
				}
			}
		}
		CORBA_free (folder->priv->list);
	}

        /* Get the list of contents */
        CORBA_exception_init (&ev);
        folder->priv->list = Bonobo_Storage_listContents (folder->priv->storage, "", Bonobo_FIELD_TYPE | Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE, &ev);
        if (BONOBO_EX (&ev)) {
                g_warning (_("Could not get list of contents for '%s': %s!"), folder->priv->path, bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
        }
        CORBA_exception_free (&ev);

	/* Get file information */
	for (i = 0; i < folder->priv->list->_length; i++) {
		if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_REGULAR) {
			info = g_new0 (CameraFileInfo, 1);
			result = gp_camera_file_get_info (folder->priv->camera, folder->priv->path, folder->priv->list->_buffer [i].name, info);
			if (result != GP_OK) {
				g_warning (_("Could not get file information about file '%s' in folder '%s': %s!"), folder->priv->list->_buffer [i].name, 
					folder->priv->path, gp_camera_get_result_as_string (folder->priv->camera, result));
				g_free (info);
				continue;
			}
			g_hash_table_insert (folder->priv->hash_table, g_strdup (folder->priv->list->_buffer [i].name), info);
		}
	}

        /* Tell the table to update */
        e_table_model_changed (folder->priv->model);
}

static void
create_menu (GnoCamFolder* folder)
{
        bonobo_ui_component_freeze (folder->priv->component, NULL);
	
        bonobo_ui_component_set_translate (folder->priv->component, "/", GNOCAM_FOLDER_UI, NULL);
        bonobo_ui_component_add_verb (folder->priv->component, "Save", on_save_clicked, folder);
        bonobo_ui_component_add_verb (folder->priv->component, "SaveAs", on_save_as_clicked, folder);

	/* Do we need a folder-menu? */
	if (folder->priv->camera->abilities->folder_operations & (GP_FOLDER_OPERATION_CONFIG | GP_FOLDER_OPERATION_PUT_FILE | GP_FILE_OPERATION_DELETE))
		bonobo_ui_component_set_translate (folder->priv->component, "/menu", GNOCAM_FOLDER_UI_FOLDER, NULL);

        /* Upload? */
        if (folder->priv->camera->abilities->folder_operations & GP_FOLDER_OPERATION_PUT_FILE) {
                bonobo_ui_component_set_translate (folder->priv->component, "/menu/File/FileOperations", GNOCAM_FOLDER_UI_UPLOAD_MENUITEM, NULL);
                bonobo_ui_component_set_translate (folder->priv->component, "/Toolbar/Upload", GNOCAM_FOLDER_UI_UPLOAD_TOOLITEM, NULL);
                bonobo_ui_component_add_verb (folder->priv->component, "Upload", on_upload_clicked, folder);
        }

	/* Delete? */
	if (folder->priv->camera->abilities->file_operations & GP_FILE_OPERATION_DELETE) {
		bonobo_ui_component_set_translate (folder->priv->component, "/menu/File/FileOperations", GNOCAM_FOLDER_UI_DELETE, NULL);
		bonobo_ui_component_add_verb (folder->priv->component, "Delete", on_delete_clicked, folder);
	}

        /* Folder configuration? */
	if (folder->priv->camera->abilities->folder_operations & GP_FOLDER_OPERATION_CONFIG) {
		bonobo_ui_component_set_translate (folder->priv->component, "/menu/Folder/Folder", GNOCAM_FOLDER_UI_CONFIGURATION, NULL);
		bonobo_ui_component_add_verb (folder->priv->component, "Configuration", on_config_clicked, folder);
	}

        bonobo_ui_component_thaw (folder->priv->component, NULL);
}

static void
table_selected_row_foreach_save_as (int model_row, gpointer closure)
{
	GnoCamFolder* 		folder;
	GtkFileSelection*	filesel;
	GtkWindow*		window;
	gchar*			name;

	folder = GNOCAM_FOLDER (closure);
	window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (folder), GTK_TYPE_WINDOW));
         
        filesel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Save As")));
	gtk_window_set_transient_for (GTK_WINDOW (filesel), window);
        gtk_widget_show (GTK_WIDGET (filesel));
        gtk_signal_connect (GTK_OBJECT (filesel->ok_button), "clicked", GTK_SIGNAL_FUNC (on_save_as_ok_button_clicked), folder);
        gtk_signal_connect (GTK_OBJECT (filesel->cancel_button), "clicked", GTK_SIGNAL_FUNC (on_cancel_button_clicked), NULL);

	name = (gchar*) e_table_model_value_at (E_TABLE (folder)->model, 0, model_row);
	gtk_file_selection_set_filename (filesel, name);
	gtk_object_set_data (GTK_OBJECT (filesel), "folder", folder);
	gtk_object_set_data (GTK_OBJECT (filesel), "name", name);
}

static void
upload_file (GnoCamFolder* folder, const gchar* source)
{
	GnoCamCamera*		camera;
	Bonobo_Stream           dest;
        BonoboStream*           src;
        Bonobo_Stream_iobuf*    buffer;
        CORBA_Environment       ev;
	gchar*			destination;

	/* We could ask the user for a filename, but we don't :-) */
	destination = g_basename (source);

        CORBA_exception_init (&ev);

	/* Read */
	src = bonobo_stream_open_full ("fs", source, Bonobo_Storage_READ, 0644, &ev);
	if (BONOBO_EX (&ev)) {
                g_warning (_("Could not get stream for '%s': %s!"), source, bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
        }
        Bonobo_Stream_read (BONOBO_OBJREF (src), 4000000, &buffer, &ev);
	bonobo_object_release_unref (BONOBO_OBJREF (src), NULL);
	if (BONOBO_EX (&ev)) {
                g_warning (_("Could not read stream: %s"), bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
        }

	/* Write */
	dest = Bonobo_Storage_openStream (folder->priv->storage, destination, Bonobo_Storage_WRITE | Bonobo_Storage_FAILIFEXIST, &ev);
	if (BONOBO_EX (&ev)) {
                g_warning (_("Could not open stream for '%s': %s!"), destination, bonobo_exception_get_text (&ev));
                CORBA_free (buffer);
                CORBA_exception_free (&ev);
                return;
        }
        Bonobo_Stream_write (dest, buffer, &ev);
	bonobo_object_release_unref (dest, NULL);
	CORBA_free (buffer);
        if (BONOBO_EX (&ev)) {
                g_warning (_("Could not write stream: %s"), bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
       }

       CORBA_exception_free (&ev);

       camera = GNOCAM_CAMERA (gtk_widget_get_ancestor (GTK_WIDGET (folder), GNOCAM_TYPE_CAMERA));
       gtk_signal_emit_by_name (GTK_OBJECT (camera), "folder_updated", folder->priv->path);
}


static void
save_file (GnoCamFolder* folder, const gchar* name, const gchar* destination)
{
        Bonobo_Stream           stream;
        BonoboStream*           dest;
        Bonobo_Stream_iobuf*    buffer;
	Bonobo_Storage_OpenMode	mode;
	CORBA_Environment	ev;

        CORBA_exception_init (&ev);

	/* Read */
	mode = Bonobo_Storage_READ;
        if ((folder->priv->camera->abilities->file_operations & GP_FILE_OPERATION_PREVIEW) && 
	    gconf_client_get_bool (folder->priv->client, "/apps/" PACKAGE "/preview", NULL))
		mode |= Bonobo_Storage_COMPRESSED;
        stream = Bonobo_Storage_openStream (folder->priv->storage, name, mode, &ev);
        if (BONOBO_EX (&ev)) {
                g_warning (_("Could not get stream for '%s': %s!"), name, bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
        }
        Bonobo_Stream_read (stream, 4000000, &buffer, &ev);
        bonobo_object_release_unref (stream, NULL);
        if (BONOBO_EX (&ev)) {
                g_warning (_("Could not read stream: %s"), bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
        }

	/* Write */
        dest = bonobo_stream_open_full ("fs", destination, Bonobo_Storage_WRITE | Bonobo_Storage_CREATE, 0664, &ev);
        if (BONOBO_EX (&ev)) {
                g_warning (_("Could not open stream for '%s': %s!"), destination, bonobo_exception_get_text (&ev));
                CORBA_free (buffer);
                CORBA_exception_free (&ev);
                return;
        }
        Bonobo_Stream_write (BONOBO_OBJREF (dest), buffer, &ev);
        bonobo_object_release_unref (BONOBO_OBJREF (dest), NULL);
        CORBA_free (buffer);
        if (BONOBO_EX (&ev)) {
                g_warning (_("Could not write stream: %s"), bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
       }

       CORBA_exception_free (&ev);
}

static void
table_selected_row_foreach_save (int model_row, gpointer closure)
{
	GnoCamFolder*           folder;
        gchar*                  name;
	gchar*			path;
        gchar*                  full_path;

	folder = GNOCAM_FOLDER (closure);

	name = (gchar*) e_table_model_value_at (E_TABLE (folder)->model, 0, model_row);
	path = gconf_client_get_string (folder->priv->client, "/apps/" PACKAGE "/prefix", NULL);

        if (!strcmp (path, "/")) full_path = g_strconcat ("/", name, NULL);
        else full_path = g_strdup_printf ("%s/%s", path, name);

	save_file (folder, name, full_path);

        g_free (full_path);
}

static void
table_selected_row_foreach_delete (int model_row, gpointer user_data)
{
	GnoCamFolder*	folder;
	gchar*		name;
	gint		result;

	folder = GNOCAM_FOLDER (user_data);

	name = (gchar*) e_table_model_value_at (E_TABLE (folder)->model, 0, model_row);
	result = gp_camera_file_delete (folder->priv->camera, folder->priv->path, name);
	if (result != GP_OK) {
		g_warning (_("Could not delete file '%s' in folder '%s': %s!"), name, folder->priv->path, 
			   gp_camera_get_result_as_string (folder->priv->camera, result));
		return;
	}
}

/*************/
/* Callbacks */
/*************/

static void
on_cancel_button_clicked (GtkButton* button, gpointer user_data)
{
	gtk_widget_unref (gtk_widget_get_ancestor (GTK_WIDGET (button), GTK_TYPE_FILE_SELECTION));
}

static void
on_upload_ok_button_clicked (GtkButton* ok_button, gpointer user_data)
{
        GtkWidget*      filesel;
        GnoCamFolder*   folder;
        gchar*          source;

	folder = GNOCAM_FOLDER (user_data);

        filesel = gtk_widget_get_ancestor (GTK_WIDGET (ok_button), GTK_TYPE_FILE_SELECTION);
        source = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));

        upload_file (folder, source);

        gtk_widget_unref (filesel);
}

static void
on_upload_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*		folder;
	GtkFileSelection*	filesel;
	GtkWindow*		window;

	folder = GNOCAM_FOLDER (user_data);
	window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (folder), GTK_TYPE_WINDOW));
	
	filesel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Upload")));
	gtk_window_set_transient_for (GTK_WINDOW (filesel), window);
	gtk_widget_show (GTK_WIDGET (filesel));
	gtk_signal_connect (GTK_OBJECT (filesel->ok_button), "clicked", GTK_SIGNAL_FUNC (on_upload_ok_button_clicked), folder);
	gtk_signal_connect (GTK_OBJECT (filesel->cancel_button), "clicked", GTK_SIGNAL_FUNC (on_cancel_button_clicked), NULL);
}

static void
on_save_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	g_return_if_fail (GNOCAM_IS_FOLDER (user_data));
	
	e_table_selected_row_foreach (E_TABLE (user_data), table_selected_row_foreach_save, user_data);
}

static void
on_delete_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;
	GnoCamFolder*	folder;

	g_return_if_fail (GNOCAM_IS_FOLDER (user_data));
	folder = GNOCAM_FOLDER (user_data);

	e_table_selected_row_foreach (E_TABLE (user_data), table_selected_row_foreach_delete, user_data);

	camera = GNOCAM_CAMERA (gtk_widget_get_ancestor (GTK_WIDGET (folder), GNOCAM_TYPE_CAMERA));
	gtk_signal_emit_by_name (GTK_OBJECT (camera), "folder_updated", folder->priv->path);
}

static void
on_config_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*	folder;
	GtkWidget*	widget;
	GtkWindow*	window;

	folder = GNOCAM_FOLDER (user_data);
	window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (folder), GTK_TYPE_WINDOW));

	widget = gnocam_configuration_new (folder->priv->camera, folder->priv->path, NULL, window);
	if (!widget) return;
	gtk_widget_show (widget);
}

static void
on_save_as_ok_button_clicked (GtkButton* ok_button, gpointer user_data)
{
	GtkWidget*	filesel;
	GnoCamFolder*	folder;
	gchar*		name;
	gchar*		dest;

	folder = GNOCAM_FOLDER (user_data);

	filesel = gtk_widget_get_ancestor (GTK_WIDGET (ok_button), GTK_TYPE_FILE_SELECTION);
	name = gtk_object_get_data (GTK_OBJECT (filesel), "name");
	dest = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));
	
	save_file (folder, name, dest);
	
	gtk_widget_unref (filesel);
}

static void
on_save_as_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	g_return_if_fail (GNOCAM_IS_FOLDER (user_data));

	e_table_selected_row_foreach (E_TABLE (user_data), table_selected_row_foreach_save_as, user_data);
}

static void
on_preview_changed (GConfClient *client, guint cnxn_id, GConfEntry* entry, gpointer user_data)
{
	GnoCamFolder*		folder;
	Bonobo_Storage		storage_new;
	Bonobo_Storage_OpenMode	mode;
	CORBA_Environment	ev;

	folder = GNOCAM_FOLDER (user_data);

	/* What open mode? */
	mode = Bonobo_Storage_READ;
	if (gconf_client_get_bool (client, "/apps/" PACKAGE "/preview", NULL) && (folder->priv->camera->abilities->file_operations & GP_FILE_OPERATION_PREVIEW))
		mode |= Bonobo_Storage_COMPRESSED;
	if (folder->priv->camera->abilities->file_operations & GP_FOLDER_OPERATION_PUT_FILE) mode |= Bonobo_Storage_WRITE;

	CORBA_exception_init (&ev);
	storage_new = Bonobo_Storage_openStorage (folder->priv->storage, "", mode, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning (_("Could not open storage for folder %s: %s!"), folder->priv->path, bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);
	bonobo_object_release_unref (folder->priv->storage, NULL);
	folder->priv->storage = storage_new;
	update_folder (folder);
}

static void
on_folder_updated (GnoCamCamera* camera, const gchar* path, gpointer user_data)
{
	GnoCamFolder*	folder;

	folder = GNOCAM_FOLDER (user_data);

	if (!strcmp (path, folder->priv->path))
		update_folder (folder);
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_folder_show_menu (GnoCamFolder* folder)
{
	/* Do we already show the menu? */
	if (bonobo_ui_component_get_container (folder->priv->component) == folder->priv->container)
		return;

	bonobo_ui_component_set_container (folder->priv->component, folder->priv->container);
	create_menu (folder);
}

void
gnocam_folder_hide_menu (GnoCamFolder* folder)
{
	bonobo_ui_component_unset_container (folder->priv->component);
}

gchar*
gnocam_folder_get_path (GnoCamFolder* folder)
{
	return (folder->priv->path);
}

/***********/
/* E-Table */
/***********/

static gint
col_count (ETableModel* model, gpointer user_data)
{
	return (4);
}

static gint
row_count (ETableModel* model, gpointer user_data)
{
	GnoCamFolder*	folder;
	gint		i;
	gint		j = 0;

	folder = GNOCAM_FOLDER (user_data);

	if (!folder->priv->list) return (0);

	for (i = 0; i < folder->priv->list->_length; i++) if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_REGULAR) j++;
	return (j);
}

static void*
value_at (ETableModel* model, gint col, gint row, gpointer user_data)
{
	GnoCamFolder*	folder;
	gint		i;
	CameraFileInfo*	info;

	folder = GNOCAM_FOLDER (user_data);
	if (!folder->priv->list) return (NULL);

	/* Search our record in the list */
	for (i = 0; i < row + 1; i++) if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY) row++;

	switch (col) {
	case 0: 
		return (folder->priv->list->_buffer [row].name);
	case 1:
		return (folder->priv->list->_buffer [row].content_type);
	case 2:
		return (GINT_TO_POINTER (folder->priv->list->_buffer [row].size));
	case 3:

		info = g_hash_table_lookup (folder->priv->hash_table, folder->priv->list->_buffer [row].name);
		if (!info) return (NULL);
		if (!(info->file.fields & GP_FILE_INFO_PERMISSIONS)) return (NULL);
		if (info->file.permissions & GP_FILE_PERM_DELETE) return (GINT_TO_POINTER (1));
		return (GINT_TO_POINTER (0));

	default:
		return (NULL);
	}
}

static void
set_value_at (ETableModel* model, gint col, gint row, const void* value, gpointer user_data)
{
	GnoCamFolder*		folder;
	gchar*			name_old;
	gchar*			name_new;
	gint			i, result;
	CameraFileInfo*		info;
	CORBA_Environment	ev;

	folder = GNOCAM_FOLDER (user_data);
	g_return_if_fail (folder->priv->list);

	/* Search our record in the list */
	for (i = 0; i < row + 1; i++) if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY) row++;

	switch (col) {
	case 0:

		CORBA_exception_init (&ev);
		name_new = (gchar*) value;
		name_old = folder->priv->list->_buffer [row].name;
		Bonobo_Storage_rename (folder->priv->storage, name_old, name_new, &ev);
		if (BONOBO_EX (&ev)) {
			g_warning (_("Could not rename '%s' to '%s': %s!"), name_old, name_new, bonobo_exception_get_text (&ev));
			CORBA_exception_free (&ev);
			return;
		}
		CORBA_exception_free (&ev);
		CORBA_free (folder->priv->list->_buffer [row].name);
		folder->priv->list->_buffer [row].name = CORBA_string_dup (value);
		break;

	case 3:

		info = g_hash_table_lookup (folder->priv->hash_table, folder->priv->list->_buffer [row].name);
		if (!info) return;
		if (!(info->file.fields & GP_FILE_INFO_PERMISSIONS)) {
			g_warning (_("File permissions are not supported by your camera!"));
			return;
		}
		info->preview.fields = GP_FILE_INFO_NONE;
		info->file.fields = GP_FILE_INFO_PERMISSIONS;
		info->file.permissions = GP_FILE_PERM_READ;
		if (GPOINTER_TO_INT (value) == 1) info->file.permissions |= GP_FILE_PERM_DELETE;
		result = gp_camera_file_set_info (folder->priv->camera, folder->priv->path, folder->priv->list->_buffer [row].name, info);
		if (result != GP_OK) {
			g_warning (_("Could not set file information of file '%s' in folder '%s': %s!"), folder->priv->list->_buffer [row].name, folder->priv->path, 
				gp_camera_get_result_as_string (folder->priv->camera, result));
			return;
		}
		break;

	default:
		break;
	}
}

static gboolean
is_cell_editable (ETableModel* model, gint col, gint row, gpointer user_data)
{
	GnoCamFolder*	folder;
	CameraFileInfo*	info;
	gint		i;

	folder = GNOCAM_FOLDER (user_data);
	g_return_val_if_fail (folder->priv->list, FALSE);
	
	/* Search our record in the list */
	for (i = 0; i < row + 1; i++) if (folder->priv->list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY) row++;

	switch (col) {
	case 0:
		return (TRUE);
	case 3:

		info = g_hash_table_lookup (folder->priv->hash_table, folder->priv->list->_buffer [row].name);
                if (!info) return (FALSE);
		if (info->file.fields & GP_FILE_INFO_PERMISSIONS) return (TRUE);
		return (FALSE);
				
	default:
		return (FALSE);
	}
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

/*************/
/* Gtk stuff */
/*************/

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

	g_hash_table_destroy (folder->priv->hash_table);
	
	g_free (folder->priv);
	folder->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_folder_class_init (GnoCamFolderClass* klass)
{
	GtkObjectClass* 	object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_folder_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_folder_init (GnoCamFolder* folder)
{
	folder->priv = g_new0 (GnoCamFolderPrivate, 1);
}

GtkWidget*
gnocam_folder_new (GnoCamCamera* camera, const gchar* path)
{
	GnoCamFolder*			new;
	Bonobo_Storage			storage_new;
	Bonobo_Storage			storage;
	Bonobo_Storage_DirectoryList*   list;
	Bonobo_Storage_OpenMode		mode;
	const gchar*			directory;
	CORBA_Environment		ev;
	ETableExtras*			extras;
	Camera*				cam;
	GConfClient*			client;
	GdkPixbuf*			images [2];

	if (*path == '/') directory = path;
	else directory = ++path;

	CORBA_exception_init (&ev);

	/* Create a new storage for this folder */
	storage = gnocam_camera_get_storage (camera);
	mode = Bonobo_Storage_READ;
	cam = gnocam_camera_get_camera (camera);
	client = gnocam_camera_get_client (camera);
	if ((cam->abilities->file_operations & GP_FILE_OPERATION_PREVIEW) && gconf_client_get_bool (client, "/apps/" PACKAGE "/preview", NULL))
		mode |= Bonobo_Storage_COMPRESSED;
	storage_new = Bonobo_Storage_openStorage (storage, path + 1, mode, &ev);
	bonobo_object_release_unref (storage, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning (_("Could not open storage for '%s': %s!"), path, bonobo_exception_get_text (&ev));
		gtk_object_unref (GTK_OBJECT (client));
		gp_camera_unref (cam);
		CORBA_exception_free (&ev);
		return (NULL);
	}

	/* Get the list of contents */
        list = Bonobo_Storage_listContents (storage_new, "", Bonobo_FIELD_TYPE | Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE, &ev);
        if (BONOBO_EX (&ev)) {
		g_warning (_("Could not get list of contents for '%s': %s!"), path, bonobo_exception_get_text (&ev));
		bonobo_object_release_unref (storage_new, NULL);
		CORBA_exception_free (&ev);
		return (NULL);
	}
	CORBA_exception_free (&ev);

	new = gtk_type_new (GNOCAM_TYPE_FOLDER);
	new->priv->camera = cam;
	new->priv->path = g_strdup (path);
	new->priv->storage = storage_new;
	new->priv->container = gnocam_camera_get_container (camera);
	new->priv->client = client;
	new->priv->cnxn = gconf_client_notify_add (new->priv->client, "/apps/" PACKAGE "/preview", on_preview_changed, new, NULL, NULL);
	new->priv->hash_table = g_hash_table_new (g_str_hash, g_str_equal);

	/* Create the model */
	new->priv->model = e_table_simple_new (col_count, row_count, value_at, set_value_at, is_cell_editable, 
		duplicate_value, free_value, initialize_value, value_is_empty, value_to_string, new);

	/* Create the extras */
	extras = e_table_extras_new ();
	images [0] = util_pixbuf_lock ();
	images [1] = util_pixbuf_unlock ();
	e_table_extras_add_pixbuf (extras, "permission", images [0]);
	e_table_extras_add_cell (extras, "cell_permission", e_cell_toggle_new (0, 2, images));

	/* Create the table */
	e_table_construct (E_TABLE (new), new->priv->model, extras, E_TABLE_SPEC, NULL);

	/* Create menu */
	new->priv->component = bonobo_ui_component_new (PACKAGE "Folder");
	bonobo_ui_component_set_container (new->priv->component, new->priv->container);
	create_menu (new);

	/* Fill the table */
	update_folder (new);

	gtk_signal_connect (GTK_OBJECT (camera), "folder_updated", GTK_SIGNAL_FUNC (on_folder_updated), new);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_folder, "GnoCamFolder", GnoCamFolder, gnocam_folder_class_init, gnocam_folder_init, PARENT_TYPE);

