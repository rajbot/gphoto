#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gphoto2.h>

/* Create/free a widget								*/
/* --------------------------------------------------------------------------	*/

CameraWidget* gp_widget_new(CameraWidgetType type, char *label) {

	CameraWidget *w;
	int x;

	w = (CameraWidget*)malloc(sizeof(CameraWidget));
	memset(w, 0, sizeof(CameraWidget));

	w->type = type;
	strcpy(w->label, label);
	
	/* set the value to nothing */
	strcpy(w->value, "");

	/* for now, for ease of mem management, pre-alloc 64 children pointers */
	memset(w->children, 0, sizeof(CameraWidget*)*64);	

	w->children_count = 0;

	/* clear out the choices */
	for (x=0; x<32; x++)
		strcpy(w->choice[x], "");
	w->choice_count = 0;

	return (w);
}

int gp_widget_free_rec (CameraWidget *widget) {
	/* Recursively delete the widget and all children */

	int x;

	/* Return if they can't have any children */
	if ((widget->type != GP_WIDGET_WINDOW) && (widget->type != GP_WIDGET_SECTION))
		return (GP_OK);

	for (x=0; x<widget->children_count; x++) {
		gp_widget_free_rec(widget->children[x]);
		free(widget->children[x]);
	}

	return (GP_OK);
}

int gp_widget_free (CameraWidget *widget) {

	gp_widget_free_rec (widget);

	free(widget);

	return (GP_OK);
}

/* Retrieve some common widget properties					*/
/* --------------------------------------------------------------------------	*/

int gp_widget_type (CameraWidget *widget) {

	return (widget->type);
}


char *gp_widget_label(CameraWidget *widget) {

	return (widget->label);
}

/* Set and unset the value of a widget 						*/
/* --------------------------------------------------------------------------	*/

int gp_widget_value_set (CameraWidget *widget, char *value) {

	strcpy(widget->value, value);

	return (GP_OK);
}

char *gp_widget_value_get (CameraWidget *widget) {

	return (widget->value);
}

/* Attach/Retrieve widgets to/from a parent widget				*/
/* --------------------------------------------------------------------------	*/

int gp_widget_append(CameraWidget *parent, CameraWidget *child) {

	/* Return if they can't have any children */
	if ((parent->type != GP_WIDGET_WINDOW) && (parent->type != GP_WIDGET_SECTION))
		return (GP_ERROR);

	parent->children[parent->children_count] = child;
	parent->children_count += 1;
	
	return (GP_OK);
}

int gp_widget_prepend(CameraWidget *parent, CameraWidget *child) {
	/* Do we really need this one? */

	int x;
	
	/* Return if they can't have any children */
	if ((parent->type != GP_WIDGET_WINDOW) && (parent->type != GP_WIDGET_SECTION))
		return (GP_ERROR);

	/* Shift down 1 */
	for (x=parent->children_count; x>0; x--)
		parent->children[x] = parent->children[x-1];

	/* Prepend the child */
	parent->children[0] = child;
	parent->children_count += 1;

	return (GP_OK);
}

int gp_widget_child_count(CameraWidget *parent) {

	/* Return if they can't have any children */
	if ((parent->type != GP_WIDGET_WINDOW) && (parent->type != GP_WIDGET_SECTION))
		return (0);

	return (parent->children_count);
}

CameraWidget* gp_widget_child(CameraWidget *parent, int child_number) {

	/* Return if they can't have any children */
	if ((parent->type != GP_WIDGET_WINDOW) && (parent->type != GP_WIDGET_SECTION))
		return (NULL);

	return (parent->children[child_number]);
}

CameraWidget* gp_widget_child_by_label (CameraWidget *widget, char *label) {

	int x;
	CameraWidget *child;

	if (strcmp(widget->label, label)==0)
		return (widget);

	for (x=0; x<widget->children_count; x++) {
		child = gp_widget_child_by_label(widget->children[x],label);
		if (child)
			return (child);
	}

	return (NULL);
}

/* Set/get the value of the range widget 					*/
/* --------------------------------------------------------------------------	*/

int gp_widget_range_set (CameraWidget *range, float min, float max, float increment) {

	if (range->type != GP_WIDGET_RANGE)
		return (GP_ERROR);

	range->min = min;
	range->max = max;
	range->increment = increment;

	return (GP_OK);
}

int gp_widget_range_get (CameraWidget *range, float *min, float *max, float *increment) {

	if (range->type != GP_WIDGET_RANGE)
		return (GP_ERROR);

	*min = range->min;
	*max = range->max;
	*increment = range->increment;

	return (GP_OK);
}

/* Retrieve and set choices for menus/radio buttons 				*/
/* --------------------------------------------------------------------------	*/

int gp_widget_choice_add (CameraWidget *widget, char *choice) {

	if ((widget->type != GP_WIDGET_RADIO) &&
	    (widget->type != GP_WIDGET_MENU))
		return (GP_ERROR);

	strncpy(widget->choice[widget->choice_count], choice, 64);
	widget->choice_count += 1;


	return (GP_OK);
}

int gp_widget_choice_count (CameraWidget *widget) {

	if ((widget->type != GP_WIDGET_RADIO) &&
	    (widget->type != GP_WIDGET_MENU))
		return (GP_ERROR);

	return (widget->choice_count);
}

char *gp_widget_choice (CameraWidget *widget, int choice_number) {

	if ((widget->type != GP_WIDGET_RADIO) &&
	    (widget->type != GP_WIDGET_MENU))
		return (NULL);

	if (choice_number > widget->choice_count)
		return (NULL);

	return (widget->choice[choice_number]);
}

/* Debugging output								*/
/* --------------------------------------------------------------------------	*/

void gp_widget_dump_rec (CameraWidget *widget, int depth) {

	int x;

	printf("core: ");
	for (x=0; x<depth*2; x++)
		printf(" ");
	printf("%s\n", widget->label);

	for (x=0; x<widget->children_count; x++)
		gp_widget_dump_rec(widget->children[x], depth+1);
}

int gp_widget_dump(CameraWidget *widget) {

	printf("core: Dumping widget \"%s\" and children:\n", widget->label);
	gp_widget_dump_rec(widget, 0);

	return (GP_OK);
}
