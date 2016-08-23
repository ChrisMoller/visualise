#ifndef PARSE_UTILITIES_H
#define PARSE_UTILITIES_H


node_u   create_value_node (double v);
node_u   create_string_node (char * s);
node_u   create_monadic_node (op_e op, node_u node);
node_u   create_dyadic_node (node_u l, op_e op, node_u r);
node_u   create_function_node (fcn_e fcn, node_u node);
void     create_vbl (char *name, node_u value, node_u min, node_u max);
curve_s *create_curve (char *name, param_s *options, node_u expression);
void     create_limits (node_u min, node_u max);
void     set_bg (char *colour);
param_s *cat_param (param_s *prev, char *keyword, char*value);

#endif  /* PARSE_UTILITIES_H */
