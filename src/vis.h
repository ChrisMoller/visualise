#ifndef VIS_H
#define VIS_H

#include <math.h>

/* forward refs */
typedef void *node_u;
typedef struct _param_s param_s;

typedef double (*fcn_t)(double x);

#undef FCN_DECL
#define FCN_DECL(idx,sym) idx,
typedef enum {
#include "fcn-decls.h"
} fcn_e;

typedef struct {
  double min;
  double max;
} limits_s;
#define limits_min(l) ((l).min)
#define limits_max(l) ((l).max)

typedef struct _param_s {
  char *keyword;
  char *value;
  param_s *next;
} param_s;
#define param_kwd(p)	((p)->keyword)
#define param_val(p)	((p)->value)
#define param_next(p)	((p)->next)

typedef struct {
  node_u   expression;
  char    *name;
  double   min;
  double   max;
  GdkRGBA *rgba;
  double   weight;
} curve_s;
#define curve_expression(c)	((c)->expression)
#define curve_name(c)		((c)->name)
#define curve_min(c)		((c)->min)
#define curve_max(c)		((c)->max)
#define curve_rgba(c)		((c)->rgba)
#define curve_weight(c)		((c)->weight)

typedef enum {
  SYM_PLUS,
  SYM_MINUS, 
  SYM_STAR,  
  SYM_SLASH, 
  SYM_CARET, 
  SYM_BANG,
} op_e;

typedef enum {
  TYPE_VALUE,
  TYPE_STRING,
  TYPE_MONADIC,
  TYPE_DYADIC,
  TYPE_FUNCTION,
} type_e;

typedef struct {
  type_e  type;		// must be first entry
} type_s;
#define node_type(n) (((type_s *)(n))->type)

typedef struct {
  type_e  type;		// must be first entry
  double  val;
} value_s;
#define value_type(v) (v)->type
#define value_val(v)  (v)->val
#define node_value(n) ((value_s *)(n))

typedef struct {
  type_e  type;		// must be first entry
  char   *str;
} string_s;
#define string_type(s) (s)->type
#define string_str(s)  (s)->str
#define node_string(n) ((string_s *)(n))

typedef struct {
  type_e  type;		// must be first entry
  op_e    op;
  node_u *arg;
} monadic_s;
#define monadic_type(m) (m)->type
#define monadic_op(m)   (m)->op
#define monadic_arg(m)  (m)->arg
#define node_monadic(n) ((monadic_s *)(n))

typedef struct {
  type_e  type;		// must be first entry
  op_e    op;
  node_u *left;
  node_u *right;
} dyadic_s;
#define dyadic_type(m)  (m)->type
#define dyadic_op(m)    (m)->op
#define dyadic_left(m)  (m)->left
#define dyadic_right(m) (m)->right
#define node_dyadic(n) ((dyadic_s *)(n))

typedef struct {
  type_e  type;		// must be first entry
  fcn_e   op;
  node_u *arg;
} function_s;
#define function_type(m) (m)->type
#define function_op(m)   (m)->op
#define function_arg(m)  (m)->arg
#define node_function(n) ((function_s *)(n))

typedef enum {
  VBL_INDEPENDENT,
  VBL_CONTROL
} vbl_type_e;

typedef struct {
  char      *name;
  double     value;
  double     min;
  double     max;
  vbl_type_e type;
  GtkAdjustment *adj;
} vbl_s;
#define vbl_name(v)  (v)->name
#define vbl_value(v) (v)->value
#define vbl_min(v)   (v)->min
#define vbl_max(v)   (v)->max
#define vbl_type(v)  (v)->type
#define vbl_adj(v)   (v)->adj

double   evaluate_phrase (node_u node);
void     show_node (int indent, node_u node);

#endif  /* VIS_H */
