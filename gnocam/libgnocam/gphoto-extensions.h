
#ifndef _GPHOTO_EXTENSIONS_H_
#define _GPHOTO_EXTENSIONS_H_

#include <gphoto2.h>
#include <bonobo.h>
#include <libgnomevfs/gnome-vfs.h>

BEGIN_GNOME_DECLS

#define CHECK_RESULT(result,ev)         G_STMT_START{                                                                           \
        if (result <= 0) {                                                                                                      \
                switch (result) {                                                                                               \
                case GP_OK:                                                                                                     \
                        break;                                                                                                  \
                case GP_ERROR_IO:                                                                                               \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_IOError, NULL);				\
                        break;                                                                                                  \
                case GP_ERROR_DIRECTORY_NOT_FOUND:                                                                              \
                case GP_ERROR_FILE_NOT_FOUND:                                                                                   \
                case GP_ERROR_MODEL_NOT_FOUND:                                                                                  \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotFound, NULL);                       \
                        break;                                                                                                  \
                case GP_ERROR_FILE_EXISTS:                                                                                      \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NameExists, NULL);                     \
                        break;                                                                                                  \
                case GP_ERROR_NOT_SUPPORTED:                                                                                    \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_NotSupported, NULL);				\
                        break;                                                                                                  \
                default:                                                                                                        \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_IOError, NULL);				\
                        break;                                                                                                  \
                }                                                                                                               \
        }                               }G_STMT_END

#define GNOME_VFS_RESULT(result)	(gp_result_as_gnome_vfs_result(result))




GnomeVFSResult 	gp_result_as_gnome_vfs_result 	(gint result);

gint 		gp_camera_new_from_gconf 	(Camera** camera, const gchar* name_or_url);




END_GNOME_DECLS

#endif /* _GPHOTO_EXTENSIONS_H_ */


	
