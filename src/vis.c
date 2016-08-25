#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-svg.h>
#include <fontconfig/fontconfig.h>
#include <alloca.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>


#include "vis.h"
#include "fcn-table.h"
#include "vis-parse.h"
#include "magick.h"

void set_string (const char *s);
void delete_buffer();
int yyparse (curve_s **curve);
static void force_redraw ();

GHashTable *vbls	= NULL;
GList *svbls		= NULL;
vbl_s *ivar		= NULL;
range_s range		= {-MAXDOUBLE, MAXDOUBLE};
GdkRGBA *bg_colour	= NULL;
double key_x		= KEY_LOC_LEFT;
double key_y		= KEY_LOC_TOP;
label_s *labels		= NULL;

#define DEFAULT_WIDTH  480
#define DEFAULT_HEIGHT 320

static gint width		= DEFAULT_WIDTH;
static gint height		= DEFAULT_HEIGHT;
static gdouble granularity	= 1.0;
static GList *curves		= NULL;
static GtkWidget *window	= NULL;
static GtkWidget *da		= NULL;
static cairo_surface_t *surface	= NULL;
static GtkAdjustment *vadj	= NULL;

static void
fc_fonts ()
{
  // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/fcpatternformat.html
  
#define FORMAT  "%{family}      %{style}\\n"
  // #define FORMAT "%{family|delete( )}\\n"
  if (FcInit ()) {
    FcPattern *pat = FcPatternCreate ();
    if (pat) {
      FcFontSet *fs  = FcFontList (0, pat, NULL);
      if (fs) {
	FILE *sort = popen ("sort -f | uniq", "w");
	if (sort) {
	  for (int j = 0; j < fs->nfont; j++) {
	    FcChar8 *s = FcPatternFormat (fs->fonts[j], (FcChar8 *)FORMAT);
	    if (s) {
	      fprintf (sort, "%s", s);
	      free (s);
	    }
	  }
	  pclose (sort);
	}
	FcFontSetDestroy (fs);
      }
      FcPatternDestroy (pat);
    }
    FcFini ();
  }
}

static double
draw_key (cairo_t *cr, double key_x, double key_offset,
	  double width, curve_s *curve)
{
  if (!curve) return key_offset;
  
  cairo_move_to (cr, key_x * width / 100.0, key_offset);
  cairo_line_to (cr, 30.0 + key_x * width / 100.0, key_offset);
  cairo_stroke (cr);

  if (curve_name (curve)) {
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, curve_name (curve), -1);

    PangoFontDescription *desc =
    pango_font_description_from_string ("Sans 8");

    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    PangoRectangle logical_rect;
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    double line_height = (double)logical_rect.height;
    key_offset += line_height;
    cairo_move_to (cr, 40.0 + key_x * width / 100.0,
		   key_offset - (line_height + 7.0));

    pango_cairo_show_layout (cr, layout);
  }
  else key_offset += 20.0;

  return key_offset;
}

static void
draw_label (cairo_t *cr, double width, double height, label_s *label)
{
  if (!label) return;

  // https://developer.gnome.org/pango/stable/pango-Layout-Objects.html
  // https://developer.gnome.org/pango/stable/pango-Text-Attributes.html

  PangoLayout *layout = pango_cairo_create_layout (cr);
  pango_layout_set_text (layout, label_string (label), -1);
  PangoFontDescription *desc = label_font (label);
  gboolean free_desc = FALSE;
  if (!desc) {
    desc = pango_font_description_from_string ("Sans 12");
    free_desc = TRUE;
  }
  pango_layout_set_font_description (layout, desc);
  if (free_desc) pango_font_description_free (desc);

  cairo_save (cr);
  
  cairo_set_source_rgba (cr,
			 label_rgba (label)->red,
			 label_rgba (label)->green,
			 label_rgba (label)->blue,
			 label_rgba (label)->alpha);

  /* rotate here */
  
  cairo_move_to (cr, label_x (label) * width, label_y (label) * height);

  cairo_rotate (cr, -label_angle (label));

  PangoAttribute *spacing =
    pango_attr_letter_spacing_new (label_stretch (label) * PANGO_SCALE);
  PangoAttrList *attrlist = pango_attr_list_new ();
  pango_attr_list_insert (attrlist, spacing);
  pango_layout_set_attributes (layout, attrlist);
  
  
  pango_cairo_update_layout (cr, layout);

  
  // pango_layout_get_size (layout, &width, &height);

  pango_cairo_show_layout (cr, layout);

  //  cairo_stroke (cr);

  pango_attribute_destroy (spacing);
  pango_attr_list_unref (attrlist);
  
  cairo_restore (cr);
}

static void
axis_range (double min, double max, double *floor_p, double *ceil_p)
{
  double intfl;
  double intce;
  double fl = log10 (fabs (min));
  double ce = log10 (fabs (max));
  fl = modf (fl, &intfl);
  ce = modf (ce, &intce);
  fl = pow (10.0, 1.0 + fl);
  ce = pow (10.0, 1.0 + ce);
  fl = floor (copysign (fl, min)) / 10.0;
  ce = ceil  (copysign (ce, max)) / 10.0;
  if (floor_p) *floor_p = fl;
  if (ceil_p)  *ceil_p  = ce;
}

static void
da_draw (cairo_t *cr, gdouble width, gdouble height)
{
  if (bg_colour) cairo_set_source_rgba (cr,
					bg_colour->red,
					bg_colour->green,
					bg_colour->blue,
					bg_colour->alpha);
  else cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
  cairo_rectangle (cr, 0.0, 0.0, width, height);
  cairo_fill (cr);
  
  double min_y =  MAXDOUBLE;
  double max_y = -MAXDOUBLE;
  double scale_x;
  double scale_y;
  double key_offset = 0;
  
#define xformx(xx) (((xx) - vbl_min (ivar)) * scale_x)
#define xformy(yy) (height - ((yy) - min_y) * scale_y)
  
#define KEY_OFFSET_INCR 15.0

  void scale_curve (gpointer data, gpointer user_data)
  {
    curve_s *curve = data;
    if (curve) {
      double xi = (vbl_max (ivar) - vbl_min (ivar)) / width;
      if (xi > 0.0) {
	for (double x = vbl_min (ivar); x <= vbl_max (ivar); x += xi) {
	  vbl_value (ivar) = x;
	  double r = evaluate_phrase (curve_expression (curve));
	  if (min_y > r && r >= range_min (range)) min_y = r;
	  if (max_y < r && r <= range_max (range)) max_y = r;
	}
      }
    }	
  }

  void draw_curve (gpointer data, gpointer user_data)
  {
    curve_s *curve = data;
    if (curve) {
      double red   = curve_rgba (curve)->red;
      double green = curve_rgba (curve)->green;
      double blue  = curve_rgba (curve)->blue;
      double alpha = curve_rgba (curve)->alpha;
      cairo_set_source_rgba (cr, red, green, blue, alpha);
      cairo_set_line_width (cr, curve_weight (curve));
      double xi = (vbl_max (ivar) - vbl_min (ivar)) / width;
      if (xi > 0.0) {
	for (double x = vbl_min (ivar); x <= vbl_max (ivar);
	     x += granularity * xi) {
	  vbl_value (ivar) = x;
	  double r = evaluate_phrase (curve_expression (curve));
	  double xp = xformx (x);
	  double yp = xformy (r);
	  if (x == vbl_min (ivar)) cairo_move_to (cr, xp, yp);
	  else                     cairo_line_to (cr, xp, yp);
	}
	cairo_stroke (cr);
      }

      if (key_x >= 0.0 && key_y >= 0.0) 
	key_offset = draw_key (cr, key_x, key_offset, width, curve);
    }
  }

  if (ivar && curves) {

    /********** compute scale **********/
    
    g_list_foreach (curves, scale_curve, NULL);

    scale_x = width  / (vbl_max (ivar) - vbl_min (ivar));
    scale_y = height / (max_y - min_y);


    /****** axes ******/
    
    double floorx, ceilx;
    axis_range (vbl_min (ivar), vbl_max (ivar), &floorx, &ceilx);
    double floory, ceily;
    axis_range (min_y, max_y, &floory, &ceily);
    
    double ddx = ceilx - floorx;
    double ddy = ceily - floory;

    double splits[] = {1.0, 2.0, 5.0, 10.0};
#define TARGET_INC	10.0
    double least_deltax = MAXDOUBLE;
    int least_idxx = -1;
    double least_deltay = MAXDOUBLE;
    int least_idxy = -1;
    for (int i = 0; i < sizeof(splits) / sizeof(double); i++) {
      double delta = fabs (TARGET_INC - (width / (splits[i] * ddx)));
      if (least_deltax > delta) {
	least_deltax = delta;
	least_idxx = i;
      }
    }
    for (int i = 0; i < sizeof(splits) / sizeof(double); i++) {
      double delta = fabs (TARGET_INC - (height / (splits[i] * ddy)));
      if (least_deltay > delta) {
	least_deltay = delta;
	least_idxy = i;
      }
    }

    double incrx = 1.0 / splits[least_idxx];
    double incry = 1.0 / splits[least_idxy];

    cairo_set_line_width (cr, 0.5);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 10.0);

    cairo_move_to (cr, 0.0,   xformy (0.0));	/* x axis */
    cairo_line_to (cr, width, xformy (0.0));
    
    cairo_move_to (cr, xformx (0.0), 0.0);	/* y axis */
    cairo_line_to (cr, xformx (0.0), height);

    double tt;
    for (tt = 0.0, ddx = floorx; ddx <= ceilx; ddx += incrx, tt += 1.0) {
      double rtt = nearbyint (10.0 * tt) / 10.0;
      if (0.0 == fmod (rtt, splits[least_idxx])) {
	cairo_move_to (cr, xformx (ddx), xformy (0.0) - 10.0);
	cairo_line_to (cr, xformx (ddx), xformy (0.0) + 10.0);
	cairo_move_to (cr, xformx (ddx), xformy (0.0) - 10.0);
	double rdx = nearbyint (10.0 * ddx) / 10.0;
	gchar *str = g_strdup_printf ("%0.2g", rdx);
	cairo_show_text (cr, str);
	g_free (str);
      }
      else {
	cairo_move_to (cr, xformx (ddx), xformy (0.0) - 5.0);
	cairo_line_to (cr, xformx (ddx), xformy (0.0) + 5.0);
      }
    }
    
    for (tt = 0.0, ddy = floory; ddy <= ceily; ddy += incry, tt += 1.0) {
      double rtt = nearbyint (10.0 * tt) / 10.0;
      if (0.0 == fmod (rtt, splits[least_idxy])) {
	cairo_move_to (cr, xformx (0.0) + 10.0, xformy (ddy));
	cairo_line_to (cr, xformx (0.0) - 10.0, xformy (ddy));
	cairo_move_to (cr, xformx (0.0) + 10.0, xformy (ddy));
	double rdy = nearbyint (10.0 * ddy) / 10.0;
	gchar *str = g_strdup_printf ("%0.2g", rdy);
	cairo_show_text (cr, str);
	g_free (str);
      }
      else {
	cairo_move_to (cr, xformx (0.0) + 5.0, xformy (ddy));
	cairo_line_to (cr, xformx (0.0) - 5.0, xformy (ddy));
      }
    }

    cairo_stroke (cr);


    /********* labels *********/
    
    if (labels) {
      for (label_s *lbl = labels; lbl; lbl = label_next (lbl))
	draw_label (cr, width, height, lbl);
    }

    /*********** key setup *********/

    double blk_ht = (double)g_list_length (curves) * KEY_OFFSET_INCR;
    key_offset = key_y * (height - blk_ht) / 100.0;

    /********* draw curves *********/

    g_list_foreach (curves, draw_curve, NULL);
  }
}

static void
save_dialogue (GtkWidget *widget, gpointer data)
{
  gint response;
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (GTK_FILE_FILTER (filter), _ ("Images"));
  gtk_file_filter_add_pattern (filter, "*.ps");
  gtk_file_filter_add_pattern (filter, "*.pdf");
  gtk_file_filter_add_pattern (filter, "*.svg");
  gtk_file_filter_add_pattern (filter, "*.png");
  gtk_file_filter_add_pattern (filter, "*.jpg");
  gtk_file_filter_add_pattern (filter, "*.tif");

  GtkWidget *dialog =
    gtk_file_chooser_dialog_new (_ ("Save image"),
				 GTK_WINDOW (window),
				 GTK_FILE_CHOOSER_ACTION_SAVE,
				 "_OK", GTK_RESPONSE_ACCEPT,
				 "_Cancel", GTK_RESPONSE_CANCEL,
				 NULL);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                   GTK_RESPONSE_ACCEPT);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  GtkWidget *label = gtk_label_new ("Width");
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 2);
  
  GtkAdjustment *wadj = gtk_adjustment_new ((gdouble)width,	// value
					    1.0,      		// lower
					    10000.0,       	// upper
					    0.1,       		// step
					    0.5,       		// page
					    0.5);      		// page size
  GtkWidget *wbutton = gtk_spin_button_new (wadj, 1.0, 3);
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (wbutton), FALSE, FALSE, 2);
  
  label = gtk_label_new ("Height");
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 2);
  
  GtkAdjustment *hadj = gtk_adjustment_new ((gdouble)height,	// value
					    1.0,     		// lower
					    10000.0,       	// upper
					    0.1,       		// step
					    0.5,       		// page
					    0.5);      		// page size
  GtkWidget *hbutton = gtk_spin_button_new (hadj, 1.0, 3);
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (hbutton), FALSE, FALSE, 2);
  
  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), hbox);
  gtk_widget_show_all (dialog);
  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_ACCEPT) {
    gdouble ww = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wbutton));
    gdouble hh = gtk_spin_button_get_value (GTK_SPIN_BUTTON (hbutton));
    gchar *file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    cairo_surface_t *tsurface = NULL;

    int wwi = (int)ww;
    int hhi = (int)hh;
    gboolean do_magick = FALSE;
    
    if (g_str_has_suffix (file, ".ps"))
      tsurface = cairo_ps_surface_create (file, wwi, hhi);
    else if (g_str_has_suffix (file, ".pdf"))
      tsurface = cairo_pdf_surface_create (file, wwi, hhi);
    else if (g_str_has_suffix (file, ".svg"))
      tsurface = cairo_svg_surface_create (file, wwi, hhi);
    else if (g_str_has_suffix (file, ".png") ||
	     g_str_has_suffix (file, ".jpg") ||
	     g_str_has_suffix (file, ".tif")
	     ) {
      tsurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, wwi, hhi);
      do_magick = TRUE;
    }

    if (tsurface) {
      cairo_t *cr = cairo_create (tsurface);
      da_draw (cr, ww, hh);
      if (do_magick) {
	int wwr = cairo_image_surface_get_width (tsurface);
	int hhr = cairo_image_surface_get_height (tsurface);
	unsigned char *pixels = cairo_image_surface_get_data (tsurface);
	magick_dump (hhr, wwr, file, pixels);
      }
      cairo_surface_finish (tsurface);
      cairo_destroy (cr);
    }
    else {
      GtkWidget *mdlg =
	gtk_message_dialog_new (GTK_WINDOW (window),
				GTK_DIALOG_MODAL |
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				_ ("Unknown file type: %s"), file);
      gtk_window_set_keep_above (GTK_WINDOW (mdlg), TRUE);
      gtk_window_set_position (GTK_WINDOW (mdlg), GTK_WIN_POS_MOUSE);
      gtk_dialog_run (GTK_DIALOG (mdlg));
      gtk_widget_destroy (mdlg);
    }
    g_free (file);
  }
  gtk_widget_destroy (dialog);
}

static void
vis_quit (GtkWidget *object, gpointer data)
{
  gtk_main_quit ();
}

static void
granularity_cb (GtkSpinButton *spin_button,
		gpointer       user_data)
{
  granularity = gtk_spin_button_get_value (spin_button);
  force_redraw ();
}

static void
granularity_cb_response (GtkDialog *dialog,
			 gint       response_id,
			 gpointer   user_data)
{
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
colour_set_cb (GtkColorButton *widget,
               gpointer        user_data)
{
  GdkRGBA colour;
  if (bg_colour) gdk_rgba_free (bg_colour);
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (widget), &colour);
  bg_colour = gdk_rgba_copy (&colour);
  force_redraw ();
}


static void
preferences (GtkWidget *object, gpointer data)
{
  GtkWidget     *dialog;
  GtkWidget     *content;
  GtkWidget     *vbox;
  GtkWidget     *hbox;
  GtkWidget     *label;
  GtkWidget     *button;
  GtkWidget     *thing;

  dialog =  gtk_dialog_new_with_buttons (_ ("Preferences"),
                                         GTK_WINDOW (window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
                                         _ ("_Cancel"), GTK_RESPONSE_CANCEL,
                                         NULL);
  g_signal_connect (dialog, "response",
		    G_CALLBACK (granularity_cb_response), NULL);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);

  
  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_container_add (GTK_CONTAINER (content), vbox);

  /******** granularity **********/
  
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (hbox), FALSE, FALSE, 2);
  
  label = gtk_label_new ("Granularity");
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 2);

  GtkAdjustment *gadj = gtk_adjustment_new (granularity,	// value
					    1.0,      		// lower
					    10000.0,       	// upper
					    0.1,       		// step
					    0.5,       		// page
					    0.5);      		// page size
  button = gtk_spin_button_new (gadj, 1.0, 3);
  g_signal_connect (button, "value-changed",
		    G_CALLBACK (granularity_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), TRUE, TRUE, 2);

  thing = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (thing), FALSE, FALSE, 2);


  /**************** bg colour **************/
  
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (hbox), FALSE, FALSE, 2);

  label = gtk_label_new ("Background colour");
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 2);

  button = gtk_color_button_new_with_rgba (bg_colour);
  g_signal_connect (button, "color-set",
		    G_CALLBACK (colour_set_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), TRUE, FALSE, 2);

  gtk_widget_show_all (dialog);
}

static void
force_redraw ()
{
  if (da && gtk_widget_get_mapped (da))
    gtk_widget_queue_draw_area (da, 0, 0,
                                gtk_widget_get_allocated_width (da),
                                gtk_widget_get_allocated_height (da));
}

static gboolean
da_configure_cb (GtkWidget *widget,
                 GdkEvent  *event,
                 gpointer   user_data)
{
  surface =
    gdk_window_create_similar_surface (gtk_widget_get_window (widget),
				     CAIRO_CONTENT_COLOR,
				     gtk_widget_get_allocated_width (widget),
				     gtk_widget_get_allocated_height (widget));
  return GDK_EVENT_PROPAGATE;
}

static gboolean
da_draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  gdouble  width = (gdouble)gtk_widget_get_allocated_width (widget);
  gdouble height = (gdouble)gtk_widget_get_allocated_height (widget);
  //  gdouble wscale = gtk_adjustment_get_value (hadj);
  //  gdouble wscale = 1.0;
  //gdouble mscale = gtk_adjustment_get_value (vadj);

  cairo_set_source_surface (cr, surface, 0.0, 0.0);

  da_draw (cr, width, height);

  return GDK_EVENT_STOP;
}

#if 0
static void
scroll_cb_amp (GtkAdjustment *adjustment,
	       gpointer       user_data)
{
  force_redraw ();
}
#endif

static void
scroll_cb_const (GtkAdjustment *adjustment,
		 gpointer       user_data)
{
  vbl_s *vv = user_data;
  if (vv) {
    vbl_value (vv) = gtk_adjustment_get_value (vbl_adj (vv));
    force_redraw ();
  }
}

static vbl_s *
lookup_vbl (char *name)
{
  vbl_s *vv = NULL;
  if (vbls) vv = g_hash_table_lookup (vbls, name);
  return vv;
}

void
show_node (int indent, node_u node)
{
  switch(node_type (node)) {
  case TYPE_VALUE:
    {
      value_s *vv = node_value (node);
      printf ("%*svalue node, value = %g\n",
	      indent, " ",
	      value_val (vv));
    }
    break;
  case TYPE_STRING:
    {
      vbl_s *vv = NULL;
      string_s *ss = node_string (node);
      printf ("%*sstring node, value = \"%s\"\n",
	      indent, " ",
	      string_str (ss));
      if (ss) vv = lookup_vbl (string_str (ss));
      if (vv) {
	printf ("%*srefs val %g, min %g, max %g\n",
		indent+2, " ",
		vbl_value (vv), vbl_min (vv), vbl_max (vv));
      }
    }
    break;
  case TYPE_MONADIC:
    {
      monadic_s *mm = node_monadic (node);
      printf ("%*smonadic node, op = %d\n",
	      indent, " ", monadic_op (mm));
      node_u   *arg = monadic_arg (mm);
      printf ("%*sarg = ", indent, " ");
      show_node (indent+2, arg);
    }
    break;
  case TYPE_DYADIC:
    {
      dyadic_s *dd = node_dyadic (node);
      printf ("%*sdyadic node, op = %d\n",
	      indent, " ", dyadic_op (dd));
      node_u   *left  = dyadic_left (dd);
      node_u   *right = dyadic_right (dd);
      printf ("%*sleft = ", indent, " ");
      show_node (indent + 2, left);
      printf ("%*sright = ", indent, " ");
      show_node (indent+2, right);
    }
    break;
  case TYPE_FUNCTION:
    break;
  }
}

double
evaluate_phrase (node_u node)
{
  double rr = NAN;

  if (!node) return NAN;
  
  switch(node_type (node)) {
  case TYPE_VALUE:
    {
      value_s *vv = node_value (node);
      rr = value_val (vv);
    }
    break;
  case TYPE_STRING:
    {
      vbl_s *vv = NULL;
      string_s *ss = node_string (node);
      if (ss) {
	vv = lookup_vbl (string_str (ss));
	if (vv) rr = vbl_value (vv);
      }
    }
    break;
  case TYPE_MONADIC:
    {
      monadic_s *mm = node_monadic (node);
      node_u   *arg = monadic_arg (mm);
      rr = evaluate_phrase (arg);
      switch(monadic_op (mm)) {
      case SYM_MINUS:
	rr = -rr;
	break;
      case SYM_BANG:
	rr = tgamma (rr + 1.0);
	break;
      default:
	break;
      }
    }
    break;
  case TYPE_DYADIC:
    {
      dyadic_s *dd = node_dyadic (node);
      node_u   *left  = dyadic_left (dd);
      node_u   *right = dyadic_right (dd);
      double l = evaluate_phrase (left);
      double r = evaluate_phrase (right);
      switch(dyadic_op (dd)) {
      case SYM_PLUS:
	rr = l + r;
	break;
      case SYM_MINUS:
	rr = l - r;
	break;
      case SYM_STAR:
	rr = l * r;
	break;
      case SYM_SLASH:
	rr = l / r;
	break;
      case SYM_CARET:
	rr = pow (l, r);
	break;
      default:
	break;
      }
    }
    break;
  case TYPE_FUNCTION:
    {
      function_s *ff = node_function (node);
      node_u    *arg = function_arg (ff);
      fcn_e op = function_op (ff);
      if (op >= 0 && op < FCN_LAST) {
	fcn_t ptr = fcns[op];
	if (ptr) {
	  rr = evaluate_phrase (arg);
	  rr = (*ptr)(rr);
	}
      }
    }
    break;
  }
  return rr;
}

static gboolean
da_key_cb (GtkWidget *widget,
           GdkEvent  *event,
           gpointer   user_data)
{
  GdkEventAny *any = (GdkEventAny *)event;
  if (any->type == GDK_KEY_PRESS) {
    GdkEventKey *key = (GdkEventKey *)event;
    if (key->keyval == GDK_KEY_q) vis_quit (NULL, NULL);
  }
  return GDK_EVENT_PROPAGATE;
}

static void
build_menu (GtkWidget *vbox)
{
  GtkWidget *menubar;
  GtkWidget *menu;
  GtkWidget *item;

  menubar = gtk_menu_bar_new();

  /********* file menu ********/

  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_label (_ ("File"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item);

  item = gtk_menu_item_new_with_label (_ ("Export..."));
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (save_dialogue), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_separator_menu_item_new();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_menu_item_new_with_label (_ ("Quit"));
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (vis_quit), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  /********* edit ********/

  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_label (_ ("Preferences"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item);

  item = gtk_menu_item_new_with_label (_ ("Environment"));
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (preferences), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);


  /********* end of menus ********/

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (menubar), FALSE, FALSE, 2);
}

int
main (int ac, char *av[])
{
  GError *error = NULL;
  gchar **vars  = NULL;
  gchar **files = NULL;
  gboolean list_fonts = FALSE;
  GOptionEntry entries[] = {
    { "setvar", 'v', 0, G_OPTION_ARG_STRING_ARRAY,
      &vars, "Set variable.", NULL },
    { "source", 's', 0, G_OPTION_ARG_STRING_ARRAY,
      &files, "Source file.", NULL },
    { "fonts", 'f', 0, G_OPTION_ARG_NONE,
      &list_fonts, "List available fonts.", NULL },
    { NULL }
  };

  GOptionContext *context = g_option_context_new ("file1 file2...");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));

  gtk_init (&ac, &av);

  if (!g_option_context_parse (context, &ac, &av, &error)) {
    g_warning ("option parsing failed: %s\n", error->message);
    g_clear_error (&error);
  }

  if (list_fonts) {
    fc_fonts ();
    return 0;
  }

#if 0
  if (vars) {
    for (int i = 0; vars[i]; i++) {
      g_print ("var \"%s\"\n", vars[i]);
    }
  }
#endif

  yydebug = 0;

  for (int i = 1; i < ac; i++) {
    set_string(av[i]);
    curve_s *curve = NULL;
    int rc = yyparse (&curve);
    if (curve) {
      if (rc) {
	/* error -- delete function */
      }
      else curves = g_list_append (curves, curve);
    }
    delete_buffer();
  }

  if (files) {
    char *line = NULL;
    size_t line_length = 0;
    for (int i = 0; files[i]; i++) {
      FILE *source = fopen (files[i], "r");
      int run = 1;
      if (source) {
	while (run) {
	  size_t grc = getline (&line, &line_length, source);
	  if (grc != -1) {
	    set_string(line);
	    curve_s *curve = NULL;
	    int rc = yyparse (&curve);
	    if (curve) {
	      if (rc) {
		/* error -- delete function */
	      }
	      else curves = g_list_append (curves, curve);
	    }
	    delete_buffer();
	  }
	  else run = 0;
	}
      }
    }
    if (line) free (line);
  }

  /***************** start gtk stuff ****************/

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "key-press-event",
                    G_CALLBACK (da_key_cb), NULL);
  //  gtk_window_set_default_size (GTK_WINDOW (window), width, height);
  g_signal_connect (window, "destroy", G_CALLBACK (vis_quit), NULL);

  gtk_window_set_title (GTK_WINDOW (window), _ ("Visualiser"));

  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  build_menu (vbox);
  
  GtkWidget *grid = gtk_grid_new ();
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (grid), TRUE, TRUE, 2);
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), FALSE);

  da = gtk_drawing_area_new ();
  gtk_widget_set_size_request (da, width, height);
  g_signal_connect (da, "configure-event",
                    G_CALLBACK (da_configure_cb), NULL);
  g_signal_connect (da, "draw",
                    G_CALLBACK (da_draw_cb), NULL);
  gtk_widget_set_hexpand (da, TRUE);
  gtk_widget_set_vexpand (da, TRUE);
  gtk_grid_attach (GTK_GRID (grid), da, 0, 0, 2, 1);

  GtkWidget *spin;
  GtkWidget *scroll;
  GtkWidget *label;

  vadj = gtk_adjustment_new (1.0,       // value
                             -1.0,      // lower
                             1.0,       // upper
                             0.1,       // step
                             0.2,       // page
                             0.2);      // page size


#if 0
  label = gtk_label_new ("Amplitude");
  gtk_label_set_angle (GTK_LABEL (label), 90.0);
  gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 1, 1);
  
  scroll = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, vadj);
  g_signal_connect (scroll, "value-changed",
                    G_CALLBACK (scroll_cb_amp), NULL);
  gtk_grid_attach (GTK_GRID (grid), scroll, 1, 0, 1, 1);
#endif

  int next_line = 1;
  void vbl_found (gpointer value, gpointer user_data)
  {
    vbl_s *vv = value;

    if (vv && vbl_type (vv) == VBL_CONTROL) {
      label = gtk_label_new (vbl_name (vv));
      gtk_grid_attach (GTK_GRID (grid), label, 0, next_line, 1, 1);

      double delta = vbl_max (vv) - vbl_min (vv);
      vbl_adj (vv) = gtk_adjustment_new (vbl_value (vv),     // value
					 vbl_min (vv),       // lower
					 vbl_max (vv),       // upper
					 0.01 * delta,       // step
					 0.05 * delta,       // page
					 0.05 * delta);      // page size
  
      scroll = gtk_scrollbar_new (GTK_ORIENTATION_HORIZONTAL, vbl_adj (vv));
      gtk_widget_set_hexpand (scroll, TRUE);
      gtk_widget_set_vexpand (scroll, FALSE);
      g_signal_connect (scroll, "value-changed",
			G_CALLBACK (scroll_cb_const), vv);
      gtk_grid_attach (GTK_GRID (grid), scroll, 0, next_line, 1, 1);

      spin = gtk_spin_button_new (vbl_adj (vv), 0.01 * delta, 3);
      gtk_widget_set_hexpand (spin, FALSE);
      gtk_widget_set_vexpand (spin, FALSE);
      gtk_grid_attach (GTK_GRID (grid), spin, 1, next_line, 1, 1);
      next_line++;
    }
  }

  if (svbls) g_list_foreach (svbls, vbl_found, NULL);

  gtk_widget_show_all (window);
  gtk_main ();

}

