#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <malloc.h>
#include <stdlib.h>
#include <strings.h>

#include "vis.h"

extern GHashTable *vbls;
extern GList      *svbls;
extern vbl_s      *ivar;
extern range_s     range;
extern GdkRGBA    *bg_colour;
extern double      key_x;
extern double      key_y;
extern label_s    *labels;
extern mode_e      plot_mode;

typedef void (*parse_curve_fcn)(curve_s *curve, char *str);

static void
parse_curve_colour (curve_s *curve, char *str)
{
  GdkRGBA rgba = {1.0, 0.0, 0.0, 1.0};
  if (str) gdk_rgba_parse (&rgba, str);
  curve_rgba (curve) = gdk_rgba_copy (&rgba);
}

static void
parse_curve_weight (curve_s *curve, char *str)
{
  curve_weight (curve) = strtod (str, NULL);
}

typedef struct {
  char *key;
  parse_curve_fcn fcn;
} parse_curve_ops_s;
#define parse_curve_ops_key(p) ((p)->key)
#define parse_curve_ops_fcn(p) ((p)->fcn)

parse_curve_ops_s parse_curve_ops[] = {
  {"color",	parse_curve_colour},
  {"colour",	parse_curve_colour},
  {"weight",	parse_curve_weight},
};

static GHashTable *parse_curve_hash = NULL;

typedef void (*parse_label_fcn)(label_s *label, char *str);

static void
parse_label_stretch (label_s *label, char *str)
{
  label_stretch (label) = atoi (str);
}

static void
parse_label_angle (label_s *label, char *str)
{
  label_angle (label) = strtod (str, NULL) / 180.0 * M_PI;
}

static void
parse_label_colour (label_s *label, char *str)
{
  GdkRGBA rgba = {1.0, 0.0, 0.0, 1.0};
  if (str) gdk_rgba_parse (&rgba, str);
  label_rgba (label) = gdk_rgba_copy (&rgba);
}

static void
parse_label_font (label_s *label, char *str)
{
  /***
      examples:

      [FAMILY-LIST] [STYLE-OPTIONS] [SIZE]

      where [FAMILY-LIST] is Sans, Serif, Normal, Monoface, etc.
      can be comma-separated list:  Sans,Serif

      [STYLE-OPTIONS]
   style:   none or one of Normal, Oblique, Italic
   weight:  none or one of Ultra-Light, Light, Normal, Bold, Ultra-Bold, Heavy
   variant: none or one of Normal, Small-Caps
   stretch: none or one of Ultra-Condensed, Extra-Condensed, Condensed,
                           Semi-Condensed, Normal, Semi-Expanded,
			   Expanded, Extra-Expanded, Ultra-Expanded

      [SIZE] in points
      
      Sans Bold 12
      Serif 16
  ***/
  label_font (label) = pango_font_description_from_string (str);
}

typedef struct {
  char *key;
  parse_label_fcn fcn;
} parse_label_ops_s;
#define parse_label_ops_key(p) ((p)->key)
#define parse_label_ops_fcn(p) ((p)->fcn)

parse_label_ops_s parse_label_ops[] = {
  {"color",	parse_label_colour},
  {"colour",	parse_label_colour},
  {"angle",	parse_label_angle},
  {"stretch",	parse_label_stretch},
  {"font",	parse_label_font},
};

static GHashTable *parse_label_hash = NULL;

node_u
create_value_node (double v)
{
  value_s *vv = malloc (sizeof(value_s));
  value_type (vv) = TYPE_VALUE;
  value_val  (vv) = v;
  return (node_u)vv;
}

node_u
create_string_node (char *s)
{
  string_s *ss = malloc (sizeof(string_s));
  string_type (ss) = TYPE_STRING;
  string_str  (ss) = s;
  return (node_u)ss;
}

node_u
create_monadic_node (op_e op, node_u node)
{
  monadic_s *mm = malloc (sizeof(monadic_s));
  monadic_type (mm) = TYPE_MONADIC;
  monadic_op (mm)   = op;
  monadic_arg (mm)  = node;
  return (node_u)mm;
}

node_u
create_dyadic_node (node_u l, op_e op, node_u r)
{
  dyadic_s *dd = malloc (sizeof(dyadic_s));
  dyadic_type (dd)  = TYPE_DYADIC;
  dyadic_op (dd)    = op;
  dyadic_left (dd)  = l;
  dyadic_right (dd) = r;
  return (node_u)dd;
}

node_u
create_function_node (fcn_e fcn, node_u node)
{
  function_s *ff = malloc (sizeof(function_s));
  function_type (ff) = TYPE_FUNCTION;
  function_op (ff)   = fcn;
  function_arg (ff)  = node;
  return (node_u)ff;
}

void
create_vbl (char *name, node_u value, node_u min, node_u max)
{
  vbl_s *vv = NULL;

  // if vbl exists, overwrite it, otherwise initialise it
  
  if (vbls) vv = g_hash_table_lookup (vbls, name);
  else vbls = g_hash_table_new (g_str_hash, g_str_equal);
  
  if (!vv) {
    vv = malloc (sizeof(vbl_s));
    g_hash_table_insert (vbls, name, vv);
    svbls = g_list_append (svbls, vv);
  }
  vbl_name (vv)  = name;
  vbl_value (vv) = evaluate_phrase (value);
  vbl_min (vv)   = evaluate_phrase (min);
  vbl_max (vv)   = evaluate_phrase (max);
  if (vbl_min (vv) > vbl_max (vv)) {
    double t = vbl_min (vv);
    vbl_min (vv) = vbl_max (vv);
    vbl_max (vv) = t;
  }
  vbl_adj (vv)   = NULL;

  if (isnan (vbl_value (vv))) {
    ivar = vv;
    vbl_type (vv) = VBL_INDEPENDENT;
  }
  else vbl_type (vv) = VBL_CONTROL;
}

curve_s *
create_curve (char *name, param_s *options, node_u expression)
{
  curve_s *curve = malloc (sizeof(curve_s));
  curve_expression (curve) = expression;
  curve_name (curve)	 = name;
  curve_min (curve)	 = NAN;
  curve_max (curve)	 = NAN;
  curve_rgba (curve)	 = NULL;
  curve_weight (curve)	 = 1.0;
  if (options) {
    if (!parse_curve_hash) {
      parse_curve_hash = g_hash_table_new (g_str_hash, g_str_equal);
      for (int i = 0;
	   i < sizeof(parse_curve_ops) / sizeof(parse_curve_ops_s); i++) 
	g_hash_table_insert (parse_curve_hash,
			     parse_curve_ops_key (&parse_curve_ops[i]),
			     &parse_curve_ops[i]);	
    }
    param_s *opt;
    for (opt = options; opt; opt = param_next (opt)) {
      parse_curve_ops_s *pp =
	g_hash_table_lookup (parse_curve_hash, param_kwd (opt));
      if (pp) {
	parse_curve_fcn fcn = parse_curve_ops_fcn (pp);
	if (fcn) (*fcn)(curve, param_val (opt));
	else g_print ("error\n");	// fixme
      }
      else g_print ("error\n");	// fixme
    }
  }
  if (!curve_rgba (curve)) {
    GdkRGBA rgba = {1.0, 0.0, 0.0, 1.0};
    curve_rgba (curve) = gdk_rgba_copy (&rgba);
  }
  return curve;
}

void
create_range (node_u min, node_u max)
{
  if (min) range_min (range) = evaluate_phrase (min);
  if (max) range_max (range) = evaluate_phrase (max);
  if (range_max (range) < range_min (range)) {
    double t = range_min (range);
    range_min (range) = range_max (range);
    range_max (range) = t;
  }
}

void
set_bg (char *colour)
{
  GdkRGBA rgba = {0.0, 0.0, 0.0, 1.0};
  if (colour) gdk_rgba_parse (&rgba, colour);
  if (bg_colour) gdk_rgba_free (bg_colour);
  bg_colour = gdk_rgba_copy (&rgba);
}

void
set_key_alpha (char *loc)
{
  if (!loc) return;
  if (!strcasecmp (loc, "off")) key_x = key_y = KEY_LOC_OFF;
  else {
    char *p = loc;
    for (; *p; p++) {
      switch (*p) {
      case 't':
      case 'T':
	key_y = KEY_LOC_TOP;
	break;
      case 'm':
      case 'M':
	key_y = KEY_LOC_MIDDLE;
	break;
      case 'b':
      case 'B':
	key_y = KEY_LOC_BOTTOM;
	break;
      case 'l':
      case 'L':
	key_x = KEY_LOC_LEFT;
	break;
      case 'c':
      case 'C':
	key_x = KEY_LOC_CENTRE;
	break;
      case 'r':
      case 'R':
	key_x = KEY_LOC_RIGHT;
	break;
      default:
	// fixme
	break;
      }
    }
  }
}

void
set_key_numeric (node_u xloc, node_u yloc)
{
  key_x = evaluate_phrase (xloc);
  key_y = evaluate_phrase (yloc);
}

void
set_mode (char *mode)
{
  if (!mode) return;
  switch (*mode) {
  case 'p':
  case 'P':
    plot_mode = MODE_POLAR;
    break;
  case 'r':
  case 'R':
  case 'c':
  case 'C':
    plot_mode = MODE_CARTESIAN;
    break;
  default:
    // fixme
    break;
  }
}

void
create_label (param_s *options, node_u x, node_u y, char *str)
{
  label_s *label = malloc (sizeof(label_s));
  label_x (label) = evaluate_phrase (x) / 100.0;
  label_y (label) = evaluate_phrase (y) / 100.0;
  label_angle (label) = 0.0;
  label_stretch (label) = 0;
  label_font (label) = NULL;
  label_rgba (label) = NULL;
  label_string (label) = str;
  if (options) {
    if (!parse_label_hash) {
      parse_label_hash = g_hash_table_new (g_str_hash, g_str_equal);
      for (int i = 0;
	   i < sizeof(parse_label_ops) / sizeof(parse_label_ops_s); i++) 
	g_hash_table_insert (parse_label_hash,
			     parse_label_ops_key (&parse_label_ops[i]),
			     &parse_label_ops[i]);	
    }
    param_s *opt;
    for (opt = options; opt; opt = param_next (opt)) {
      parse_label_ops_s *pp =
	g_hash_table_lookup (parse_label_hash, param_kwd (opt));
      if (pp) {
	parse_label_fcn fcn = parse_label_ops_fcn (pp);
	if (fcn) (*fcn)(label, param_val (opt));
	else g_print ("error\n");	// fixme
      }
      else g_print ("error\n");	// fixme
    }
  }

  if (!label_rgba (label)) {
    GdkRGBA rgba = {1.0, 0.0, 0.0, 1.0};
    label_rgba (label) = gdk_rgba_copy (&rgba);
  }

  label_next (label) = NULL;
  if (labels) label_next (labels) = label;
  else labels = label;
}

param_s *
cat_param (param_s *prev, char *keyword, char*value)
{
  param_s *rc;
  
  param_s *param = malloc (sizeof(param_s));
  param_kwd (param)  = keyword;
  param_val (param)  = value;
  param_next (param) = NULL;

  if (prev) {
    param_next (prev) = param;
    rc = prev;
  }
  else rc = param;
  
  return rc;
}
