#include <complex.h>
#include <math.h>
#include <stdio.h>


complex double
cgamma (complex double z)
{
  if (cimag (z) != 0.0) return NAN;
  else return lgamma (z) + 0.0 * I;
}

complex double
ccabs (complex double z)
{
  return cabs (z) + 0.0 * I;
}

complex double
ccarg (complex double z)
{
  return carg (z) + 0.0 * I;
}

complex double
ccreal (complex double z)
{
  return creal (z) + 0.0 * I;
}

complex double
ccimag (complex double z)
{
  return cimag (z) + 0.0 * I;
}

