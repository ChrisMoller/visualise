#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <malloc.h>
#include <stdlib.h>

#include "vis.h"

extern GHashTable *vbls;
extern GList *svbls;
extern vbl_s *ivar;
extern limits_s limits;
extern GdkRGBA *bg_colour;

typedef void (*parse_fcn)(curve_s *curve, char *str);

static void
parse_colour (curve_s *curve, char *str)
{
  GdkRGBA rgba = {1.0, 0.0, 0.0, 1.0};
  if (str) gdk_rgba_parse (&rgba, str);
  curve_rgba (curve) = gdk_rgba_copy (&rgba);
}

static void
parse_weight (curve_s *curve, char *str)
{
  curve_weight (curve) = strtod (str, NULL);
}

typedef struct {
  char *key;
  parse_fcn fcn;
} parse_ops_s;
#define parse_ops_key(p) ((p)->key)
#define parse_ops_fcn(p) ((p)->fcn)

parse_ops_s parse_ops[] = {
  {"color",	parse_colour},
  {"colour",	parse_colour},
  {"weight",	parse_weight},
};

static GHashTable *parse_hash = NULL;


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
    if (!parse_hash) {
      parse_hash = g_hash_table_new (g_str_hash, g_str_equal);
      for (int i = 0; i < sizeof(parse_ops) / sizeof(parse_ops_s); i++) 
	g_hash_table_insert (parse_hash, parse_ops_key (&parse_ops[i]),
			     &parse_ops[i]);	
    }
    param_s *opt;
    for (opt = options; opt; opt = param_next (opt)) {
      parse_ops_s *pp = g_hash_table_lookup (parse_hash, param_kwd (opt));
      if (pp) {
	parse_fcn fcn = parse_ops_fcn (pp);
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
create_limits (node_u min, node_u max)
{
  if (min) limits_min (limits) = evaluate_phrase (min);
  if (max) limits_max (limits) = evaluate_phrase (max);
  if (limits_max (limits) < limits_min (limits)) {
    double t = limits_min (limits);
    limits_min (limits) = limits_max (limits);
    limits_max (limits) = t;
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
