/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#ifndef WIN32
#include <signal.h>
#endif

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "main.h"

#include "actions.h"
#include "foreach.h"
#include "interface.h"
#include "options.h"
#include "range.h"
#include "shell.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#define CHECK_RESULT(result) {int r = (result); if (r < 0) return (r);}

/* Takes the current globals, and sets up the gPhoto lib with them */
static int set_globals (void);

/* Command-line option
   -----------------------------------------------------------------------
   this is funky and may look a little scary, but sounded cool to do.
   it makes sense since this is just a wrapper for a functional/flow-based
   library.
   AFTER NOTE: whoah. gnome does something like this. cool deal :)

   When the program starts it calls (in order):
        1) verify_options() to make sure all options are valid,
           and that any options needing an argument have one (or if they
           need an argument, they don't have one).
        2) (optional) option_is_present() to see if a particular option was
           typed in the command-line. This can be used to set up something
           before the next step.
        3) execute_options() to actually parse the command-line and execute
           the command-line options in order.

   This might sound a little complex, but makes adding options REALLY easy,
   and it is very flexible.

   How to add a command-line option:
*/

/* 1) Add a forward-declaration here                                    */
/*    ----------------------------------------------------------------- */
/*    Use the OPTION_CALLBACK(function) macro.                          */

OPTION_CALLBACK(abilities);
OPTION_CALLBACK(help);
OPTION_CALLBACK(version);
OPTION_CALLBACK(shell);
OPTION_CALLBACK(list_cameras);
OPTION_CALLBACK(auto_detect);
OPTION_CALLBACK(list_ports);
OPTION_CALLBACK(filename);
OPTION_CALLBACK(port);
OPTION_CALLBACK(speed);
OPTION_CALLBACK(model);
OPTION_CALLBACK(quiet);
OPTION_CALLBACK(debug);
OPTION_CALLBACK(use_folder);
OPTION_CALLBACK(recurse);
OPTION_CALLBACK(use_stdout);
OPTION_CALLBACK(use_stdout_size);
OPTION_CALLBACK(list_folders);
OPTION_CALLBACK(list_files);
OPTION_CALLBACK(num_pictures);
OPTION_CALLBACK(get_picture);
OPTION_CALLBACK(get_all_pictures);
OPTION_CALLBACK(get_thumbnail);
OPTION_CALLBACK(get_all_thumbnails);
OPTION_CALLBACK(get_raw_data);
OPTION_CALLBACK(get_all_raw_data);
OPTION_CALLBACK(delete_picture);
OPTION_CALLBACK(delete_all_pictures);
OPTION_CALLBACK(upload_picture);
OPTION_CALLBACK(capture_image);
OPTION_CALLBACK(capture_preview);
OPTION_CALLBACK(capture_movie);
OPTION_CALLBACK(capture_sound);
OPTION_CALLBACK(summary);
OPTION_CALLBACK(manual);
OPTION_CALLBACK(about);

/* 2) Add an entry in the option table                          */
/*    ----------------------------------------------------------------- */
/*    Format for option is:                                             */
/*     {"short", "long", "argument", "description", callback_function, required}, */
/*    if it is just a flag, set the argument to "".                     */
/*    Order is important! Options are exec'd in the order in the table! */

Option option[] = {

/* Settings needed for formatting output */
{"",  "debug",          "",             "Turn on debugging",            debug,          0},
{"q", "quiet",          "",             "Quiet output (default=verbose)",quiet,         0},

/* Display and die actions */

{"v", "version",        "",             "Display version and exit",     version,        0},
{"h", "help",           "",             "Displays this help screen",    help,           0},
{"",  "list-cameras",   "",             "List supported camera models", list_cameras,   0},
{"",  "list-ports",     "",             "List supported port devices",  list_ports,     0},
{"",  "stdout",         "",             "Send file to stdout",          use_stdout,     0},
{"",  "stdout-size",    "",             "Print filesize before data",   use_stdout_size,0},
{"",  "auto-detect",    "",             "List auto-detected cameras",   auto_detect,    0},

/* Settings needed for camera functions */
{"" , "port",           "path",         "Specify port device",          port,           0},
{"" , "speed",          "speed",        "Specify serial transfer speed",speed,          0},
{"" , "camera",         "model",        "Specify camera model",         model,          0},
{"" , "filename",       "filename",     "Specify a filename",           filename,       0},

/* Actions that depend on settings */
{"a", "abilities",      "",             "Display camera abilities",     abilities,      0},
{"f", "folder",         "folder",       "Specify camera folder (default=\"/\")",use_folder,0},
{"R", "recurse",        "",             "Recursively descend through folders",recurse,  0},
{"l", "list-folders",   "",             "List folders in folder",       list_folders,   0},
{"L", "list-files",     "",             "List files in folder",         list_files,     0},
{"n", "num-images",   "",             "Display number of pictures",   num_pictures,   0},
{"p", "get-image",    "range",        "Get pictures given in range",  get_picture,    0},
{"P", "get-all-images","",            "Get all pictures from folder", get_all_pictures,0},
{"t", "get-thumbnail",  "range",        "Get thumbnails given in range",get_thumbnail,  0},
{"T", "get-all-thumbnails","",          "Get all thumbnails from folder",get_all_thumbnails,0},
{"r", "get-raw-data", "range",      "Get raw data given in range", get_raw_data, 0},
{"", "get-all-raw-data", "",        "Get all raw data from folder", get_all_raw_data, 0},
{"d", "delete-picture", "range",        "Delete pictures given in range", delete_picture, 0},
{"D", "delete-all-images","",         "Delete all pictures in folder",delete_all_pictures,0},
{"u", "upload-image", "filename",     "Upload a picture to camera",   upload_picture, 0},
{"" , "capture-preview","",             "Capture a quick preview",      capture_preview, 0},
{"" , "capture-image",  "",             "Capture an image",             capture_image,  0},
{"" , "capture-movie",  "",             "Capture a movie ",             capture_movie,  0},
{"" , "capture-sound",  "",             "Capture an audio clip",        capture_sound,  0},
{"",  "summary",        "",             "Summary of camera status",     summary,        0},
{"",  "manual",         "",             "Camera driver manual",         manual,         0},
{"",  "about",          "",             "About the camera driver",      about,          0},
{"",  "shell",          "",             "gPhoto shell",                 shell,          0},

/* End of list                  */
{"" , "",               "",             "",                             NULL,           0}
};

/* 3) Add any necessary global variables                                */
/*    ----------------------------------------------------------------- */
/*    Most flags will set options, like choosing a port, camera model,  */
/*    etc...                                                            */

int  glob_option_count;
char glob_port[128];
char glob_model[64];
char glob_folder[1024];
char glob_owd[1024];
char glob_cwd[1024];
int  glob_speed;
int  glob_num=1;

Camera         *glob_camera=NULL;

int  glob_debug;
int  glob_shell=0;
int  glob_quiet=0;
int  glob_filename_override=0;
int  glob_recurse=0;
char glob_filename[128];
int  glob_stdout=0;
int  glob_stdout_size=0;

/* 4) Finally, add your callback function.                              */
/*    ----------------------------------------------------------------- */
/*    The callback function is passed "char *arg" to the argument of    */
/*    command-line option. It must return GP_OK or GP_ERROR.            */
/*    Again, use the OPTION_CALLBACK(function) macro.                   */
/*    glob_debug is set if the user types in the "--debug" flag. Use    */
/*    cli_debug_print(char *format, ...) to display debug output. Use   */
/*    cli_error_print(char *format, ...) to display error output.       */


OPTION_CALLBACK(help) {

        cli_debug_print("Displaying usage");

        usage();
        exit(EXIT_SUCCESS);
        return GP_OK;
}

OPTION_CALLBACK(version) {

        cli_debug_print("Displaying version");

        print_version();
        exit(EXIT_SUCCESS);
        return GP_OK;
}

OPTION_CALLBACK(use_stdout) {

        glob_quiet  = 1; /* implied */
        glob_stdout = 1;

        return GP_OK;
}

OPTION_CALLBACK(use_stdout_size) {

        glob_stdout_size = 1;
        use_stdout(arg);

        return GP_OK;
}

OPTION_CALLBACK (auto_detect)
{
        int x, count;
        CameraList* list;
        const char *name, *value;

        CHECK_RESULT (gp_list_new (&list));
        CHECK_RESULT (gp_autodetect (list));
        CHECK_RESULT (count = gp_list_count (list));

        printf("%-30s %-16s\n", "Model", "Port");
        printf("----------------------------------------------------------\n");
        for (x = 0; x < count; x++) {
                CHECK_RESULT (gp_list_get_name  (list, x, &name));
                CHECK_RESULT (gp_list_get_value (list, x, &value));
                printf("%-30s %-16s\n", name, value);
        }

        /* We don't need to check for error here. */
        gp_list_free (list);

        return GP_OK;
}

OPTION_CALLBACK (abilities)
{
        int x = 0;
        CameraAbilities abilities;

        if (!strlen (glob_model)) {
                cli_error_print ("Must specify a camera model using \"%scamera model\"",LONG_OPTION);
                return (GP_ERROR_BAD_PARAMETERS);
        }

        CHECK_RESULT (gp_camera_abilities_by_name (glob_model, &abilities));

        /* Output a parsing friendly abilities table. Split on ":" */

        printf("Abilities for camera:            : %s\n",
                abilities.model);
        printf("Serial port support              : %s\n",
                SERIAL_SUPPORTED(abilities.port)? "yes":"no");
        printf("Parallel port support            : %s\n",
                PARALLEL_SUPPORTED(abilities.port)? "yes":"no");
        printf("USB support                      : %s\n",
                USB_SUPPORTED(abilities.port)? "yes":"no");
        printf("IEEE1394 support                 : %s\n",
                IEEE1394_SUPPORTED(abilities.port)? "yes":"no");
        printf("Network support                  : %s\n",
                NETWORK_SUPPORTED(abilities.port)? "yes":"no");

        if (abilities.speed[0] != 0) {
        printf("Transfer speeds supported        :\n");
                do {
        printf("                                 : %i\n", abilities.speed[x]);
                        x++;
                } while (abilities.speed[x]!=0);
        }
        printf("Capture choices                  :\n");
        if (abilities.operations & GP_OPERATION_CAPTURE_IMAGE)
            printf("                                 : Image\n");
        if (abilities.operations & GP_OPERATION_CAPTURE_VIDEO)
            printf("                                 : Video\n");
        if (abilities.operations & GP_OPERATION_CAPTURE_AUDIO)
            printf("                                 : Audio\n");
        if (abilities.operations & GP_OPERATION_CAPTURE_PREVIEW)
            printf("                                 : Preview\n");
        printf("Configuration support            : %s\n",
                abilities.operations & GP_OPERATION_CONFIG? "yes":"no");
        printf("Delete files on camera support   : %s\n",
                abilities.file_operations & GP_FILE_OPERATION_DELETE? "yes":"no");
        printf("File preview (thumbnail) support : %s\n",
                abilities.file_operations & GP_FILE_OPERATION_PREVIEW? "yes":"no");
        printf("File upload support              : %s\n",
                abilities.folder_operations & GP_FOLDER_OPERATION_PUT_FILE? "yes":"no");

        return (GP_OK);
}


OPTION_CALLBACK(list_cameras) {

        int x, n;
        const char *buf;
        CameraAbilities abilities;

        cli_debug_print("Listing Cameras");

        CHECK_RESULT (n = gp_camera_count ());

        if (glob_quiet)
                printf("%i\n", n);
           else
                printf("Number of supported cameras: %i\nSupported cameras:\n", n);

        for (x = 0; x < n; x++) {
                CHECK_RESULT (gp_camera_name (x, &buf));
                CHECK_RESULT (gp_camera_abilities (x, &abilities));

                if (glob_quiet)
                        printf("%s\n", buf);
                else
                        switch (abilities.status) {
                        case GP_DRIVER_STATUS_TESTING:
                                printf ("\t\"%s\" (TESTING)\n", buf);
                                break;
                        case GP_DRIVER_STATUS_EXPERIMENTAL:
                                printf ("\t\"%s\" (EXPERIMENTAL)\n", buf);
                                break;
                        default:
                        case GP_DRIVER_STATUS_PRODUCTION:
                                printf ("\t\"%s\"\n", buf);
                                break;
                        }
        }

        return (GP_OK);
}

OPTION_CALLBACK(list_ports)
{
        gp_port_info info;
        int x, count;

        CHECK_RESULT (count = gp_port_count_get ());

        if (!glob_quiet) {
          printf("Devices found: %i\n", count);
          printf("Path                             Description\n"
                 "--------------------------------------------------------------\n");
        } else
          printf("%i\n", count);
        for(x = 0; x < count; x++) {
                CHECK_RESULT (gp_port_info_get (x, &info));
                printf ("%-32s %-32s\n", info.path, info.name);
        }

        return (GP_OK);
}

OPTION_CALLBACK(filename)
{
        cli_debug_print("Setting filename to %s", arg);

        strcpy (glob_filename, arg);
        glob_filename_override = 1;

        return (GP_OK);
}

OPTION_CALLBACK(port)
{
        cli_debug_print("Setting port to %s", arg);

        strcpy (glob_port, arg);

        return (GP_OK);
}

OPTION_CALLBACK(speed)
{
        cli_debug_print("Setting speed to %s", arg);

        glob_speed = atoi (arg);

        return (GP_OK);
}

OPTION_CALLBACK(model)
{
        cli_debug_print("Setting camera model to %s", arg);

        strcpy (glob_model, arg);

        return (GP_OK);
}

OPTION_CALLBACK (debug)
{
        glob_debug = GP_DEBUG_HIGH;
        cli_debug_print("Turning on debug mode");

	gp_debug_set_level (GP_DEBUG_HIGH);

        return (GP_OK);
}

OPTION_CALLBACK(quiet)
{
        cli_debug_print("Setting to quiet mode");

        glob_quiet=1;

        return (GP_OK);
}

OPTION_CALLBACK(shell)
{
        cli_debug_print("Entering shell mode");

        CHECK_RESULT (set_globals ());

        glob_shell = 1;

        return (shell_prompt ());
}

OPTION_CALLBACK (use_folder)
{
        cli_debug_print("Setting folder to %s", arg);

        strcpy (glob_folder, arg);

        return (GP_OK);
}

OPTION_CALLBACK (recurse)
{
        cli_debug_print("Setting recursive mode");

        glob_recurse = 1;

        return (GP_OK);
}

OPTION_CALLBACK (list_folders)
{
        CHECK_RESULT (set_globals ());

        return for_each_subfolder(glob_folder, print_folder, NULL, glob_recurse);
}

OPTION_CALLBACK (list_files)
{
        CHECK_RESULT (set_globals());
        CHECK_RESULT (for_each_image (glob_folder, print_picture_action, 0));

        if (!glob_recurse)
                return (GP_OK);

        return for_each_subfolder(glob_folder, for_each_image, print_picture_action, glob_recurse);
}

OPTION_CALLBACK (num_pictures)
{
        int count;
        CameraList list;

        cli_debug_print ("Counting pictures");

        CHECK_RESULT (set_globals ());
        CHECK_RESULT (gp_camera_folder_list_files (glob_camera, glob_folder,
                                                   &list));
        CHECK_RESULT (count = gp_list_count (&list));

        if (glob_quiet)
                printf ("%i\n", count);
           else
                printf ("Number of pictures in folder %s: %i\n",
                        glob_folder, count);

        return (GP_OK);
}

static int
save_camera_file_to_file (CameraFile *file, CameraFileType type)
{
        char out_filename[1024], out_folder[1024], buf[1024], msg[1024];
        int result;
        char *f;
        int resp1, resp2;
        const char *name;

        /* Determine the folder and filename */
        strcpy (out_filename, "");
        strcpy (out_folder, "");

        if (glob_filename_override) {
                if (strchr(glob_filename, '/')) {               /* Check if they specified an output dir */
                        f = strrchr(glob_filename, '/');
                        strcpy(buf, f+1);                       /* Get the filename */
                        sprintf(out_filename, buf, glob_num++);
                        *f = 0;
                        strcpy(out_folder, glob_filename);      /* Get the folder */
                        strcat(out_folder, "/");
                        *f = '/';
                } else {                                        /* If not, subst and set */
                        sprintf(out_filename, glob_filename, glob_num++);
                }
        } else {
                gp_file_get_name (file, &name);
                strcat(out_filename, name);
        }

        switch (type) {
        case GP_FILE_TYPE_PREVIEW:
                sprintf(buf, "%sthumb_%s", out_folder, out_filename);
                break;
        case GP_FILE_TYPE_NORMAL:
                sprintf(buf, "%s%s", out_folder, out_filename);
                break;
        case GP_FILE_TYPE_RAW:
                sprintf(buf, "%sraw_%s", out_folder, out_filename);
                break;
        default:
                return (GP_ERROR_NOT_SUPPORTED);
        }
        if (!glob_quiet) {
                while (GP_SYSTEM_IS_FILE(buf)) {
                        sprintf(msg, "File %s exists. Overwrite?", buf);
                        resp1 = gp_frontend_confirm(glob_camera, msg);
                        if ((resp1==GP_CONFIRM_NO)||(resp1==GP_CONFIRM_NOTOALL)) {
                                resp2 = gp_frontend_confirm(glob_camera, "Specify new filename?");
                                if ((resp2==GP_CONFIRM_NO)||(resp2==GP_CONFIRM_NOTOALL)) {
                                        gp_file_free(file);
                                        return (GP_OK);
                                }
                                printf("Enter new filename: ");
                                fflush(stdout);
                                fgets(buf, 1023, stdin);
                                buf[strlen(buf)-1]=0;
                        } else {
                                break;
                        }
                }
                printf("Saving file as %s\n", buf);
        }

        if ((result = gp_file_save(file, buf)) != GP_OK)
                cli_error_print("Can not save file as %s", buf);

        return (result);
}


int
save_picture_to_file (char *folder, char *filename, CameraFileType type)
{
        int res;

        CameraFile *file;

        CHECK_RESULT (gp_file_new (&file));
        CHECK_RESULT (gp_camera_file_get (glob_camera, folder, filename, type,
                                          file));

        if (glob_stdout) {
                const char *data;
                long int size;

                CHECK_RESULT (gp_file_get_data_and_size (file, &data, &size));

                if (glob_stdout_size)
                        printf ("%li\n", size);
                fwrite (data, sizeof(char), size, stdout);
                gp_file_free (file);
                return (GP_OK);
        }

        res = save_camera_file_to_file (file, type);

        gp_file_free (file);

        return (res);
}


/*
  get_picture_common() - parse range, download specified images, or their
        thumbnails according to thumbnail argument, and save to files.
*/

int
get_picture_common(char *arg, CameraFileType type )
{
        cli_debug_print("Getting %s", arg);

        CHECK_RESULT (set_globals ());

        if (strchr(arg, '.'))
                return (save_picture_to_file(glob_folder, arg, type));

        switch (type) {
        case GP_FILE_TYPE_PREVIEW:
                return for_each_image_in_range(glob_folder, arg, save_thumbnail_action, 0);
        case GP_FILE_TYPE_NORMAL:
                return for_each_image_in_range(glob_folder, arg, save_picture_action, 0);
        case GP_FILE_TYPE_RAW:
                return for_each_image_in_range(glob_folder, arg, save_raw_action, 0);
        default:
                return (GP_ERROR_NOT_SUPPORTED);
        }
}

OPTION_CALLBACK (get_picture)
{
        return get_picture_common(arg, GP_FILE_TYPE_NORMAL);
}

OPTION_CALLBACK (get_all_pictures)
{
        cli_debug_print("Getting all pictures");

        CHECK_RESULT (set_globals ());
        CHECK_RESULT (for_each_image (glob_folder, save_picture_action, 0));

        if (!glob_recurse)
                return (GP_OK);

        return for_each_subfolder(glob_folder, for_each_image, save_picture_action, glob_recurse);
}

OPTION_CALLBACK (get_thumbnail)
{
        return (get_picture_common(arg, GP_FILE_TYPE_PREVIEW));
}

OPTION_CALLBACK(get_all_thumbnails)
{
        cli_debug_print("Getting all thumbnails");

        CHECK_RESULT (set_globals ());
        CHECK_RESULT (for_each_image (glob_folder, save_thumbnail_action, 0));

        if (!glob_recurse)
                return (GP_OK);

        return for_each_subfolder(glob_folder, for_each_image, save_thumbnail_action, glob_recurse);
}

OPTION_CALLBACK (get_raw_data)
{
        return (get_picture_common(arg, GP_FILE_TYPE_RAW));
}

OPTION_CALLBACK (get_all_raw_data)
{
        CHECK_RESULT (set_globals ());
        CHECK_RESULT (for_each_image (glob_folder, save_raw_action, 0));

        if (!glob_recurse)
                return (GP_OK);

        return for_each_subfolder(glob_folder, for_each_image, save_raw_action, glob_recurse);
}

OPTION_CALLBACK (delete_picture)
{
        cli_debug_print("Deleting picture(s) %s", arg);

        CHECK_RESULT (set_globals ());

        if (!(glob_camera->abilities->file_operations & GP_FILE_OPERATION_DELETE)) {
                cli_error_print("Camera can not delete pictures");
                return (GP_ERROR_NOT_SUPPORTED);
        }

        return for_each_image_in_range(glob_folder, arg, delete_picture_action, 1);
}

OPTION_CALLBACK (delete_all_pictures)
{
        cli_debug_print("Deleting all pictures");

        CHECK_RESULT (set_globals ());
        CHECK_RESULT (gp_camera_folder_delete_all (glob_camera, glob_folder));

        return (GP_OK);
}

OPTION_CALLBACK (upload_picture)
{
        CameraFile *file;
        int res;

        cli_debug_print("Uploading picture");

        CHECK_RESULT (set_globals ());

        if (!(glob_camera->abilities->folder_operations &
              GP_FOLDER_OPERATION_PUT_FILE))
                return (GP_ERROR_NOT_SUPPORTED);

        CHECK_RESULT (gp_file_new (&file));
        CHECK_RESULT (gp_file_open (file, arg));

        res = gp_camera_folder_put_file (glob_camera, glob_folder, file);

        gp_file_free (file);

        return (res);
}

int
capture_generic (int type, char *name)
{
        CameraFilePath path;
        CameraFile* file;
        char *pathsep;
        int result;

        CHECK_RESULT (set_globals ());

        if (type == GP_OPERATION_CAPTURE_PREVIEW) {
                CHECK_RESULT (gp_file_new (&file));
                result = gp_camera_capture_preview (glob_camera, file);
                if (result != GP_OK) {
                        cli_error_print("Could not capture the preview.");
                        return (result);
                }
                CHECK_RESULT (save_camera_file_to_file (file,
                                                        GP_FILE_TYPE_NORMAL));
                gp_file_free (file);
        } else {
                result =  gp_camera_capture (glob_camera, type, &path);
                if (result != GP_OK) {
                        cli_error_print("Could not capture.");
                        return (result);
                }

                if (strcmp(path.folder, "/") == 0)
                                pathsep = "";
                        else
                                pathsep = "/";

                if (glob_quiet)
                        printf ("%s%s%s\n", path.folder, pathsep, path.name);
                else
                        printf ("New file is in location %s%s%s on the camera\n",
                                path.folder, pathsep, path.name);
        }

        return (GP_OK);
}


OPTION_CALLBACK (capture_image)
{
        return (capture_generic (GP_OPERATION_CAPTURE_IMAGE, arg));
}

OPTION_CALLBACK (capture_movie)
{
        return (capture_generic (GP_OPERATION_CAPTURE_VIDEO, arg));
}

OPTION_CALLBACK (capture_sound)
{
        return (capture_generic (GP_OPERATION_CAPTURE_AUDIO, arg));
}

OPTION_CALLBACK (capture_preview)
{
        return (capture_generic (GP_OPERATION_CAPTURE_PREVIEW, arg));
}

OPTION_CALLBACK(summary)
{
        CameraText buf;

        CHECK_RESULT (set_globals ());
        CHECK_RESULT (gp_camera_get_summary (glob_camera, &buf));

        printf("Camera Summary:\n%s\n", buf.text);

        return (GP_OK);
}

OPTION_CALLBACK (manual)
{
        CameraText buf;

        CHECK_RESULT (set_globals ());
        CHECK_RESULT (gp_camera_get_manual (glob_camera, &buf));

        printf ("Camera Manual:\n%s\n", buf.text);

        return (GP_OK);
}

OPTION_CALLBACK (about)
{
        CameraText buf;

        CHECK_RESULT (set_globals ());
        CHECK_RESULT (gp_camera_get_about (glob_camera, &buf));

        printf ("About the library:\n%s\n", buf.text);

        return (GP_OK);
}

/* Set/init global variables                                    */
/* ------------------------------------------------------------ */

static int
set_globals (void)
{
        /* takes all the settings and sets up the gphoto lib */

        cli_debug_print ("Setting globals...");
        CHECK_RESULT (gp_camera_new (&glob_camera));
        CHECK_RESULT (gp_camera_set_model (glob_camera, glob_model));
        if (strcmp (glob_model, "Directory Browse"))
                CHECK_RESULT (gp_camera_set_port_path (glob_camera, glob_port));
        CHECK_RESULT (gp_camera_set_port_speed (glob_camera, glob_speed));

        cli_debug_print ("Initializing camera...");
        CHECK_RESULT (gp_camera_init (glob_camera));

        return (GP_OK);
}

static int
init_globals (void)
{
        glob_option_count = 0;

        strcpy (glob_model, "");
        strcpy (glob_port, "");
        strcpy (glob_filename, "gphoto");
        strcpy (glob_folder, "/");
        if (!getcwd (glob_owd, 1023))
                strcpy (glob_owd, "./");
        strcpy (glob_cwd, glob_owd);

        glob_camera = NULL;
        glob_speed = 0;
        glob_debug = 0;
        glob_quiet = 0;
        glob_filename_override = 0;
        glob_recurse = 0;

        return GP_OK;
}

/* Misc functions                                                       */
/* ------------------------------------------------------------------   */

void
cli_debug_print (char *format, ...)
{
        va_list         pvar;

        if (glob_debug) {
                fprintf(stderr, "cli: ");
                va_start(pvar, format);
                vfprintf(stderr, format, pvar);
                va_end(pvar);
                fprintf(stderr, "\n");
        }
}

void
cli_error_print (char *format, ...)
{
        va_list         pvar;

        fprintf(stderr, "ERROR: ");
        va_start(pvar, format);
        vfprintf(stderr, format, pvar);
        va_end(pvar);
        fprintf(stderr, "\n");
}

static void
signal_exit (int signo)
{
        if (!glob_quiet)
                printf("\nExiting gPhoto...\n");

        /* Exit gPhoto core */
        if (glob_camera)
               gp_camera_free(glob_camera);
        gp_exit();

        if (strlen(glob_owd)>0)
                chdir(glob_owd);

        exit (EXIT_SUCCESS);
}

/* Main :)                                                              */
/* -------------------------------------------------------------------- */

int
main (int argc, char **argv)
{
        int result;

#ifdef ENABLE_NLS
        bindtextdomain (PACKAGE, GPHOTO2_LOCALEDIR);
        textdomain (PACKAGE);
#endif

        /* Initialize the globals */
        init_globals();

        signal(SIGINT, signal_exit);

        /* Count the number of command-line options we have */
        while ((strlen(option[glob_option_count].short_id)>0) ||
               (strlen(option[glob_option_count].long_id)>0)) {
                glob_option_count++;
        }

        /* Peek ahead: Turn on quiet if they gave the flag */
        if (option_is_present("q", argc, argv)==GP_OK)
                glob_quiet=1;

        /* Peek ahead: Check to see if we need to turn on debugging output */
        if (option_is_present("debug", argc, argv)==GP_OK)
                glob_debug=GP_DEBUG_HIGH;

#ifdef OS2 /*check if environment is set otherwise quit*/
        if(CAMLIBS==NULL)
        {
printf("gPhoto2 for OS/2 requires you to set the enviroment value CAMLIBS \
to the location of the camera libraries. \
e.g. SET CAMLIBS=C:\\GPHOTO2\\CAM\n");
                exit(EXIT_FAILURE);
        }
#endif

#ifdef OS2 /*check if environment is set otherwise quit*/
        if(IOLIBS==NULL)
        {
printf("gPhoto2 for OS/2 requires you to set the enviroment value IOLIBS \
to the location of the io libraries. \
e.g. SET IOLIBS=C:\\GPHOTO2\\IOLIB\n");
                exit(EXIT_FAILURE);
        }
#endif

        /* Peek ahead: Make sure we were called correctly */
        if ((argc == 1)||(verify_options(argc, argv) != GP_OK)) {
                if (!glob_quiet)
                        usage();
                exit(EXIT_FAILURE);
        }

        gp_frontend_register (NULL, NULL, gp_interface_message,
			      gp_interface_confirm, NULL);
	gp_camera_set_status_func (glob_camera, status_func, NULL);
	gp_camera_set_progress_func (glob_camera, progress_func, NULL);
        if ((result = execute_options(argc, argv)) != GP_OK) {
                printf ("gPhoto2 reported the error '%s'\n",
                        gp_camera_get_result_as_string (glob_camera, result));

		if (gp_debug_get_level () == GP_DEBUG_NONE) {
			printf ("Last debugging messages (use --debug for "
				"more):\n");
			printf ("%s\n", gp_debug_history_get ());
		}
                exit (EXIT_FAILURE);
        }

#ifdef OS2
//       printf("\nErrors occuring beyond this point are 'expected' on OS/2\ninvestigation pending\n");
#endif

        /* Exit gPhoto core */
        if (glob_camera)
               gp_camera_free(glob_camera);
        gp_exit();
        return (EXIT_SUCCESS);
}

