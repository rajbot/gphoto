#define CHECK_RESULT(result,ev)         G_STMT_START{			      \
	gint	r;							      \
									      \
	if (!BONOBO_EX (ev) && ((r = result) < 0)) {			      \
                switch (r) {						      \
                case GP_ERROR_IO:                                             \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
					     ex_Bonobo_IOError, NULL);	      \
                        break;                                                \
                case GP_ERROR_DIRECTORY_NOT_FOUND:                            \
                case GP_ERROR_FILE_NOT_FOUND:                                 \
                case GP_ERROR_MODEL_NOT_FOUND:                                \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
					 ex_Bonobo_Storage_NotFound, NULL);   \
                        break;                                                \
		case GP_ERROR_DIRECTORY_EXISTS:				      \
                case GP_ERROR_FILE_EXISTS:                                    \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 	      \
					 ex_Bonobo_Storage_NameExists, NULL); \
                        break;                                                \
                case GP_ERROR_NOT_SUPPORTED:                                  \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
				         ex_Bonobo_NotSupported, NULL); \
                        break;                                                \
                default:                                                      \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
					     ex_Bonobo_IOError, NULL);	      \
                        break;                                                \
                }                                                             \
        }                               }G_STMT_END
