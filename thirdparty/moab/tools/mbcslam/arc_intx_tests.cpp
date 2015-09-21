/*
 * arc_intx_tests.cpp
 */
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CslamUtils.hpp"
#include "moab/Types.hpp"

#include "../test/TestUtil.hpp"

using namespace moab;
void test_great_arc_intx()
{
  double A[3]={0., 0., 1.};
  double B[3]={1., 0., 0.};
  double C[3]={0., 0.6, 0.8};
  double D[3]={0.6, -0.8, 0.};

  double E[3];
  double R=1.0;
  ErrorCode rval = intersect_great_circle_arcs(A, B, C, D, R, E);
  CHECK_ERR(rval);
  std::cout << "E: " << E[0] << " " << E[1] << " " << E[2] << "\n";
  rval = intersect_great_circle_arcs(A, C, B, D, R, E);
  CHECK(rval == MB_FAILURE);

}
void test_great_arc_clat_intx()
{
  double d3=1/sqrt(3.);
  double A[3]={0., 0., 1.};
  double B[3]={1., 0., 0.};
  double C[3]={d3, d3, d3};
  double D[3]={d3, -d3, d3};

  double E[9];
  double R=1.0;
  int np=0;
  ErrorCode rval = intersect_great_circle_arc_with_clat_arc(A, B, C, D, R, E, np);
  CHECK_ERR(rval);
  std::cout << "E: " << E[0] << " " << E[1] << " " << E[2] << "\n";
  double F[3]={-d3, d3, d3};
  rval = intersect_great_circle_arc_with_clat_arc(A, B, C, F, R, E, np);
  CHECK(rval == MB_FAILURE);
}
int main()
{
  int failures=0;
  failures += RUN_TEST(test_great_arc_intx);
  failures += RUN_TEST(test_great_arc_clat_intx);
  return failures;
}
