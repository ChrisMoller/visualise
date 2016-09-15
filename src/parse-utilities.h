#ifndef PARSE_UTILITIES_H
#define PARSE_UTILITIES_H


node_u   create_value_node (complex double v);
node_u   create_string_node (char * s);
node_u   create_monadic_node (op_e op, node_u node);
node_u   create_dyadic_node (node_u l, op_e op, node_u r);
node_u   create_function_node (fcn_e fcn, node_u node);
void     create_vbl (char *name, node_u value, node_u min, node_u max);
void     create_label (param_s *options, node_u angle,
		       node_u x, node_u y, char *str);
curve_s *create_curve (char *name, param_s *options,
		       int intdiff, node_u expression);
void     create_range (node_u min, node_u max);
void	 create_subrn_node (char *str, node_u node);
void     set_bg (char *colour);
void     set_mode (char *mode);
void	 set_key_alpha (char *loc);
void	 set_key_numeric (node_u xloc, node_u yloc);
param_s *cat_param (param_s *prev, char *keyword, char*value);

#endif  /* PARSE_UTILITIES_H */
