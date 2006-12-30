#include <config.h>
#include "knc-c-prefs.h"

#include <libknc/knc.h>

#include <gtk/gtktogglebutton.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkspinbutton.h>

#include <glade/glade.h>

#define _(s) (s)
#define N_(s) (s)

struct _KncCPrefsPriv {
	KncCntrl *c;
	GladeXML *xml;
};

static GObjectClass *parent_class;

static void
knc_c_prefs_finalize (GObject *o)
{
	KncCPrefs *p = KNC_C_PREFS (o);

	g_object_unref (G_OBJECT (p->priv->xml));
	knc_cntrl_unref (p->priv->c);
	g_free (p->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static void
knc_c_prefs_class_init (KncCPrefsClass *klass)
{
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_class->finalize = knc_c_prefs_finalize;
}

static void
knc_c_prefs_init (KncCPrefs *p)
{
	p->priv = g_new0 (KncCPrefsPriv, 1);
}

static gboolean
knc_c_prefs_check_results (KncCPrefs *p, KncCntrlRes cntrl_res,
			   KncCamRes cam_res)
{
	if (cntrl_res) {
		g_warning ("Operation failed: %s",
			   knc_cntrl_res_name (cntrl_res));
		return FALSE;
	}
	if (cam_res) {
		g_warning ("Operation failed: %s",
			   knc_cam_res_name (cam_res));
		return FALSE;
	}
	return TRUE;
}

static void
gtk_radio_button_set_all_inconsistent (GtkRadioButton *b, gboolean s)
{
	GSList *l;
	guint i;

	g_return_if_fail (GTK_IS_RADIO_BUTTON (b));

	l = gtk_radio_button_get_group (b);
	for (i = 0; i < g_slist_length (l); i++)
		gtk_toggle_button_set_inconsistent (
			GTK_TOGGLE_BUTTON (g_slist_nth_data (l, i)), s);
}

static void
on_date_format_toggled (GtkToggleButton *button, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;

	if (button->active) {
		cntrl_res = knc_loc_date_format_set (p->priv->c, &cam_res,
			GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button),
							    "format")));
		if (!knc_c_prefs_check_results (p, cntrl_res, cam_res))
			return;
		if (button->inconsistent)
			 gtk_radio_button_set_all_inconsistent (
					GTK_RADIO_BUTTON (button), FALSE);
	}
}

static void
on_tv_format_toggled (GtkToggleButton *button, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;

	if (button->active) {
		cntrl_res = knc_loc_tv_output_format_set (p->priv->c, &cam_res,
			GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button),
								"format")));
		if (!knc_c_prefs_check_results (p, cntrl_res, cam_res))
			return;
		if (button->inconsistent)
			gtk_radio_button_set_all_inconsistent (
				GTK_RADIO_BUTTON (button), FALSE);
	}
}

static void
knc_c_prefs_unset_flash (KncCPrefs *p)
{
	GtkWidget *w;

	w = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_off");
	gtk_radio_button_set_all_inconsistent (GTK_RADIO_BUTTON (w), TRUE);
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_red_eye");
	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (w), TRUE);
	gtk_widget_set_sensitive (w, FALSE);
}

static void
knc_c_prefs_unset_focus_self_timer (KncCPrefs *p)
{
	GtkWidget *w;

	w = glade_xml_get_widget (p->priv->xml, "checkbutton_focus");
	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (w), TRUE);
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_self_timer");
	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (w), TRUE);
	w = glade_xml_get_widget (p->priv->xml, "spinbutton_self_timer");
	gtk_widget_set_sensitive (w, FALSE);
}

static void
knc_c_prefs_set_auto_off_time (KncCPrefs *p, unsigned int t)
{
	GtkWidget *w;
	
	w = glade_xml_get_widget (p->priv->xml, "spinbutton_auto_off");
	gtk_adjustment_set_value (GTK_SPIN_BUTTON (w)->adjustment, t);
}

static void
knc_c_prefs_set_slide_show_interval (KncCPrefs *p, unsigned int t)
{
	GtkWidget *w;

	w = glade_xml_get_widget (p->priv->xml, "spinbutton_slide_show");
	gtk_adjustment_set_value (GTK_SPIN_BUTTON (w)->adjustment, t);
}

static void
on_slide_show_value_changed (GtkAdjustment *a, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;

	cntrl_res = knc_set_pref (p->priv->c, &cam_res,
				  KNC_PREF_SLIDE_SHOW_INTERVAL, a->value);
	knc_c_prefs_check_results (p, cntrl_res, cam_res);
}

static void
on_self_timer_value_changed (GtkAdjustment *a, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;

	cntrl_res = knc_set_pref (p->priv->c, &cam_res,
				  KNC_PREF_SELF_TIMER_TIME, a->value);
	if (!knc_c_prefs_check_results (p, cntrl_res, cam_res))
		knc_c_prefs_unset_focus_self_timer (p);
}

static void
on_auto_off_value_changed (GtkAdjustment *a, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;

	cntrl_res = knc_set_pref (p->priv->c, &cam_res,
				  KNC_PREF_AUTO_OFF_TIME, a->value);
	knc_c_prefs_check_results (p, cntrl_res, cam_res);
}

static KncFocusSelfTimer
knc_c_prefs_get_focus_self_timer (KncCPrefs *p)
{
	GtkWidget *wf, *wt;

	wf = glade_xml_get_widget (p->priv->xml, "checkbutton_focus");
	wt = glade_xml_get_widget (p->priv->xml, "checkbutton_self_timer");
	if (GTK_TOGGLE_BUTTON (wf)->active)
		if (GTK_TOGGLE_BUTTON (wt)->active)
			return KNC_FOCUS_SELF_TIMER_FIXED_SELF_TIMER;
		else
			return KNC_FOCUS_SELF_TIMER_FIXED;
	else
		if (GTK_TOGGLE_BUTTON (wt)->active)
			return KNC_FOCUS_SELF_TIMER_AUTO_SELF_TIMER;
		else
			return KNC_FOCUS_SELF_TIMER_AUTO;
}

static void
knc_c_prefs_set_beep (KncCPrefs *p, unsigned int beep)
{
	GtkWidget *w;

	w = glade_xml_get_widget (p->priv->xml, "checkbutton_beep");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), beep > 0);
	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (w), FALSE);
}

static void
knc_c_prefs_set_exposure (KncCPrefs *p, unsigned int exposure)
{
	GtkWidget *w;

	w = glade_xml_get_widget (p->priv->xml, "spinbutton_exposure");
	gtk_adjustment_set_value (GTK_SPIN_BUTTON (w)->adjustment, exposure);
}

static void
knc_c_prefs_set_resolution (KncCPrefs *p, KncResolution resolution)
{
	GtkWidget *w = NULL;

	switch (resolution) {
	case KNC_RESOLUTION_HIGH:
		w = glade_xml_get_widget (p->priv->xml,
					  "radiobutton_resolution_high");
		break;
	case KNC_RESOLUTION_MEDIUM:
		w = glade_xml_get_widget (p->priv->xml,
					  "radiobutton_resolution_medium");
		break;
	case KNC_RESOLUTION_LOW:
		w = glade_xml_get_widget (p->priv->xml,
					  "radiobutton_resolution_low");
		break;
	default:
		g_warning ("Unknown resolution %i!", resolution);
		return;
	}
	gtk_radio_button_set_all_inconsistent (GTK_RADIO_BUTTON (w), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
}

static void
knc_c_prefs_set_focus_self_timer (KncCPrefs *p, KncFocusSelfTimer v)
{
	GtkWidget *w = NULL;

	w = glade_xml_get_widget (p->priv->xml, "checkbutton_focus");
	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (w), FALSE);
	switch (v) {
	case KNC_FOCUS_SELF_TIMER_FIXED:
	case KNC_FOCUS_SELF_TIMER_FIXED_SELF_TIMER:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
		break;
	default:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), FALSE);
	}
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_self_timer");
	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (w), FALSE);
	switch (v) {
	case KNC_FOCUS_SELF_TIMER_FIXED:
	case KNC_FOCUS_SELF_TIMER_AUTO:
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), FALSE);
	    w = glade_xml_get_widget (p->priv->xml, "spinbutton_self_timer");
	    gtk_widget_set_sensitive (w, FALSE);
	    break;
	default:
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
	    w = glade_xml_get_widget (p->priv->xml, "spinbutton_self_timer");
	    gtk_widget_set_sensitive (w, TRUE);
	}
}

static void
knc_c_prefs_set_flash (KncCPrefs *p, KncFlash f)
{
	GtkWidget *w = NULL;

	switch (f) {
	case KNC_FLASH_OFF:
	    w = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_off");
	    break;
	case KNC_FLASH_ON:
	case KNC_FLASH_ON_RED_EYE:
	    w = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_on");
	    break;
	case KNC_FLASH_AUTO:
	case KNC_FLASH_AUTO_RED_EYE:
	    w = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_auto");
	    break;
	default:
	    g_warning ("Unknown value %i for flash!", f);
	    knc_c_prefs_unset_flash (p);
	    return;
	}
	gtk_radio_button_set_all_inconsistent (GTK_RADIO_BUTTON (w), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_red_eye");
	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (w), FALSE);
	gtk_widget_set_sensitive (w, f != KNC_FLASH_OFF);
	switch (f) {
	case KNC_FLASH_ON_RED_EYE:
	case KNC_FLASH_AUTO_RED_EYE:
		if (!GTK_TOGGLE_BUTTON (w)->active)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w),
						      TRUE);
		break;
	default:
		if (GTK_TOGGLE_BUTTON (w)->active)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w),
						      FALSE);
	}
}

static KncFlash
knc_c_prefs_get_flash (KncCPrefs *p)
{
	GtkWidget *radio, *check;
	
	radio = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_off");
	if (GTK_TOGGLE_BUTTON (radio)->active)
		return KNC_FLASH_OFF;
	check = glade_xml_get_widget (p->priv->xml, "checkbutton_red_eye");
	radio = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_on");
	if (GTK_TOGGLE_BUTTON (radio)->active)
		return GTK_TOGGLE_BUTTON (check)->active ?
			KNC_FLASH_ON_RED_EYE : KNC_FLASH_ON;
	return GTK_TOGGLE_BUTTON (check)->active ? 
			KNC_FLASH_AUTO_RED_EYE : KNC_FLASH_AUTO;
}

static void
on_flash_toggled (GtkToggleButton *button, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;
	KncFlash f;

	f = knc_c_prefs_get_flash (p);
	cntrl_res = knc_set_pref (p->priv->c, &cam_res, KNC_PREF_FLASH, f);
	if (!knc_c_prefs_check_results (p, cntrl_res, cam_res)) {
		knc_c_prefs_unset_flash (p);
		return;
	}
	knc_c_prefs_set_flash (p, f);
}

static void
on_resolution_toggled (GtkToggleButton *button, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;
	KncResolution r;
	GtkWidget *w;

	w = glade_xml_get_widget (p->priv->xml, "radiobutton_resolution_high");
	if (GTK_TOGGLE_BUTTON (w)->active) r = KNC_RESOLUTION_HIGH;
	else {
		w = glade_xml_get_widget (p->priv->xml, 
					  "radiobutton_resolution_medium");
		if (GTK_TOGGLE_BUTTON (w)->active) r = KNC_RESOLUTION_MEDIUM;
		else r = KNC_RESOLUTION_LOW;
	}
	cntrl_res = knc_set_pref (p->priv->c, &cam_res, KNC_PREF_RESOLUTION, r);
	if (!knc_c_prefs_check_results (p, cntrl_res, cam_res)) {
		gtk_radio_button_set_all_inconsistent (
					GTK_RADIO_BUTTON (button), TRUE);
		return;
	}
	knc_c_prefs_set_resolution (p, r);
}

static void
on_focus_self_timer_toggled (GtkToggleButton *button, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;
	KncFocusSelfTimer v;

	v = knc_c_prefs_get_focus_self_timer (p);
	cntrl_res = knc_set_pref (p->priv->c, &cam_res,
				  KNC_PREF_FOCUS_SELF_TIMER, v);
	if (!knc_c_prefs_check_results (p, cntrl_res, cam_res)) {
		knc_c_prefs_unset_focus_self_timer (p);
		return;
	}
	knc_c_prefs_set_focus_self_timer (p, v);
}

static void
on_beep_toggled (GtkToggleButton *button, KncCPrefs *p)
{
	KncCntrlRes cntrl_res;
	KncCamRes cam_res;

	cntrl_res = knc_set_pref (p->priv->c, &cam_res, KNC_PREF_BEEP,
				  button->active ? 1 : 0);
	if (!knc_c_prefs_check_results (p, cntrl_res, cam_res)) {
		gtk_toggle_button_set_inconsistent (button, TRUE);
		return;
	}
	knc_c_prefs_set_beep (p, button->active ? 1 : 0);
}

KncCPrefs *
knc_c_prefs_new (KncCntrl *c)
{
	KncCPrefs *p = g_object_new (KNC_C_TYPE_PREFS, NULL);
	GtkWidget *w;
	KncCamRes cam_res;
	KncStatus s;
	KncPrefs prefs;
	KncCntrlRes cntrl_res;

	p->priv->c = c;
	knc_cntrl_ref (c);

	/* Read the interface description */
	p->priv->xml = glade_xml_new (KNC_GLADE_DIR "/knc.glade",
				      "vbox_prefs", NULL);
	if (!p->priv->xml) p->priv->xml = glade_xml_new (
			KNC_SRC_DIR "/corba/knc.glade", "vbox_prefs", NULL);
	if (!p->priv->xml) {
		g_warning ("Could not find interface description!");
		g_object_unref (p);
		return NULL;
	}
	w = glade_xml_get_widget (p->priv->xml, "vbox_prefs");
	if (!w) {
		g_warning ("Could not find widget!");
		g_object_unref (p);
		return NULL;
	}
	bonobo_control_construct (BONOBO_CONTROL (p), w);

	/* Load the current settings */
	w = glade_xml_get_widget (p->priv->xml, "entry_language");
	gtk_entry_set_text (GTK_ENTRY (w), "");
	cntrl_res = knc_get_status (p->priv->c, &cam_res, &s);
	if (cntrl_res || cam_res) {
		g_warning ("knc_get_status failed: '%s'/'%s'.",
			   knc_cntrl_res_name (cntrl_res),
			   knc_cam_res_name (cam_res));
	} else {
		knc_c_prefs_set_flash (p, s.flash);
		knc_c_prefs_set_exposure (p, s.exposure);
		knc_c_prefs_set_resolution (p, s.resolution);
	}
	cntrl_res = knc_get_prefs (p->priv->c, &cam_res, &prefs);
	if (cntrl_res || cam_res) {
		g_warning ("knc_get_prefs failed: '%s'/'%s'.",
			   knc_cntrl_res_name (cntrl_res),
			   knc_cam_res_name (cam_res));
	} else {
		knc_c_prefs_set_focus_self_timer (p, s.focus_self_timer);
		knc_c_prefs_set_auto_off_time (p, prefs.auto_off_time);
		knc_c_prefs_set_slide_show_interval (p,
						prefs.slide_show_interval);
		knc_c_prefs_set_beep (p, prefs.beep);
	}

	/* Date format */
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_dmy");
	g_object_set_data (G_OBJECT (w), "format",
			   GINT_TO_POINTER (KNC_DATE_FORMAT_DMY));
	g_signal_connect (w, "toggled", G_CALLBACK (on_date_format_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_mdy");
	g_object_set_data (G_OBJECT (w), "format",
			   GINT_TO_POINTER (KNC_DATE_FORMAT_MDY));
	g_signal_connect (w, "toggled", G_CALLBACK (on_date_format_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_ymd");
	g_object_set_data (G_OBJECT (w), "format",
			   GINT_TO_POINTER (KNC_DATE_FORMAT_YMD));
	g_signal_connect (w, "toggled", G_CALLBACK (on_date_format_toggled), p);

	/* TV Output Format */
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_NTSC");
	g_object_set_data (G_OBJECT (w), "format",
			   GINT_TO_POINTER (KNC_TV_OUTPUT_FORMAT_NTSC));
	g_signal_connect (w, "toggled", G_CALLBACK (on_tv_format_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_PAL");
	g_object_set_data (G_OBJECT (w), "format",
			   GINT_TO_POINTER (KNC_TV_OUTPUT_FORMAT_PAL));
	g_signal_connect (w, "toggled", G_CALLBACK (on_tv_format_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_Hide");
	g_object_set_data (G_OBJECT (w), "format",
			   GINT_TO_POINTER (KNC_TV_OUTPUT_FORMAT_HIDE));
	g_signal_connect (w, "toggled", G_CALLBACK (on_tv_format_toggled), p);

	/* Flash */
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_off");
	g_signal_connect (w, "toggled", G_CALLBACK (on_flash_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_auto");
	g_signal_connect (w, "toggled", G_CALLBACK (on_flash_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_flash_on");
	g_signal_connect (w, "toggled", G_CALLBACK (on_flash_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_red_eye");
	g_signal_connect (w, "toggled", G_CALLBACK (on_flash_toggled), p);

	/* Resolution */
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_resolution_high");
	g_signal_connect (w, "toggled", G_CALLBACK (on_resolution_toggled), p);
	w = glade_xml_get_widget (p->priv->xml,
				  "radiobutton_resolution_medium");
	g_signal_connect (w, "toggled", G_CALLBACK (on_resolution_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "radiobutton_resolution_low");
	g_signal_connect (w, "toggled", G_CALLBACK (on_resolution_toggled), p);

	/* Other */
	w = glade_xml_get_widget (p->priv->xml, "spinbutton_auto_off");
	g_signal_connect (GTK_SPIN_BUTTON (w)->adjustment, "value_changed",
			  G_CALLBACK (on_auto_off_value_changed), p);
	w = glade_xml_get_widget (p->priv->xml, "spinbutton_slide_show");
	g_signal_connect (GTK_SPIN_BUTTON (w)->adjustment, "value_changed",
			  G_CALLBACK (on_slide_show_value_changed), p);

	w = glade_xml_get_widget (p->priv->xml, "spinbutton_self_timer");
	g_signal_connect (GTK_SPIN_BUTTON (w)->adjustment, "value_changed",
			  G_CALLBACK (on_self_timer_value_changed), p);
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_self_timer");
	g_signal_connect (w, "toggled",
			  G_CALLBACK (on_focus_self_timer_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_focus");
	g_signal_connect (w, "toggled",
			  G_CALLBACK (on_focus_self_timer_toggled), p);
	w = glade_xml_get_widget (p->priv->xml, "checkbutton_beep");
	g_signal_connect (w, "toggled", G_CALLBACK (on_beep_toggled), p);

	g_warning ("FIXME: Connect signals!");

	return p;
}

BONOBO_TYPE_FUNC (KncCPrefs, BONOBO_TYPE_CONTROL, knc_c_prefs);
