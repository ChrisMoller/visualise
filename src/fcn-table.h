#ifndef FCN_TABLE_H
#define FCN_TABLE_H

#include <math.h>

#include <vis.h>


//typedef double (*fcn_t)(double x);

#undef FCN_DECL
#define FCN_DECL(idx,sym) sym,
fcn_t fcns[] = {
#include "fcn-decls.h"
};  


#endif  /*  FCN_TABLE_H*/
