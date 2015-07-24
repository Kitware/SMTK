#include "EdgeSizeSimpleImplicit.hpp"

namespace moab { 

EdgeSizeSimpleImplicit::EdgeSizeSimpleImplicit()
{
  int i;
  // Default to the plane: x = 0.
  this->coeffC = 0.;
  for ( i = 0; i < 3; ++i )
    {
    this->coeffB[i] = this->coeffA[i] = this->coeffA[i+3] = 0.;
    }
  this->coeffB[0] = 1.;
  // Default to a scaling ratio of 1.
  this->ratio = 1.;
}

EdgeSizeSimpleImplicit::~EdgeSizeSimpleImplicit()
{
}

bool EdgeSizeSimpleImplicit::evaluate_edge(
  const double* p0, const void* t0,
  double* p1, void* t1,
  const double* p2, const void* t2 )
{
  (void)t0;
  (void)t1;
  (void)t2;
  double L2 = 0.;
  double delta;
  int i;
  for ( i = 0; i < 3; ++i )
    {
    delta = p2[i+3] - p0[i+3];
    L2 += delta * delta;
    }
  // parametric coords in p1[{0,1,2}]
  double x = p1[3];
  double y = p1[4];
  double z = p1[5];
  double F2 =
    this->coeffA[0] * x * x + 2. * this->coeffA[1] * x * y + 2. * this->coeffA[2] * x * z +
    this->coeffA[3] * y * y + 2. * this->coeffA[4] * y * z +
    this->coeffA[5] * z * z +
    this->coeffB[0] * x + this->coeffB[1] * y + this->coeffB[2] * z +
    this->coeffC;
  F2 = F2 * F2; // square it
  double r2 = this->ratio * this->ratio;
  if ( 4. * F2 / L2 < r2 )
    return true; // Midpoint is close to surface => split edge

  return false; // Don't split edge
}

void EdgeSizeSimpleImplicit::set_implicit_function( double* coeffs )
{
  int i;
  // Default to the plane: x = 0.
  for ( i = 0; i < 3; ++i )
    {
    this->coeffA[i  ] = coeffs[i];
    this->coeffA[i+3] = coeffs[i + 3];
    this->coeffB[i  ] = coeffs[i + 6];
    }
  this->coeffC = coeffs[9];
}

void EdgeSizeSimpleImplicit::get_implicit_function( double*& coeffs )
{
  int i;
  // Default to the plane: x = 0.
  for ( i = 0; i < 3; ++i )
    {
    coeffs[i] = this->coeffA[i  ];
    coeffs[i + 3] = this->coeffA[i+3];
    coeffs[i + 6] = this->coeffB[i  ];
    }
  coeffs[9] = this->coeffC;
}

} // namespace moab
