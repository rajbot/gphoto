
/* Type Definitions */

typedef struct {
	guint		id;
	gchar* 		name;
	guint		ref_count;
	GladeXML*	xml;
	GladeXML*	xml_properties;
	GladeXML*	xml_preview;
} frontend_data_t;


