
/* Type Definitions */

typedef struct {
	guint		id;
	gchar* 		name;
	GladeXML*	xml;
	GladeXML*	xml_properties;
	GladeXML*	xml_preview;
	GMutex*		lock;
} frontend_data_t;


