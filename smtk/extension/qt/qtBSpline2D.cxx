//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtBSpline2D.h"

#define SMTK_DBG_SPLINE 0

#if SMTK_DBG_SPLINE
#include <iostream>
#endif // SMTK_DBG_SPLINE

namespace smtk
{
namespace extension
{
namespace
{

/// Compute the weight for Boehm knot insertion.
///
/// + \a ii is the index of the control point (in the range [0, points.size()]);
/// + \a ll is the location in the knot vector where the new knot value, \a tt, should be inserted
///   (between knot[ll] and knot[ll + 1]);
/// + \a kk is the order of the spline (i.e., the degree + 1; e.g., 4 for a cubic spline).
/// + \a tt is the knot value being inserted.
/// + \a knot is the original (unmodified) knot vector.
///
/// See Patrikalakis, Maekawa, and Cho. Shape Interrogation for Computer Aided Design and Manufacturing,
/// Hyperbook edition. http://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/mathe.html
/// Retrieved 2023-12-14.
double
alpha(std::size_t ii, std::size_t ll, std::size_t kk, double tt, const std::vector<double>& knot)
{
  if (ii <= ll - kk + 1)
  {
    return 1.0;
  }
  else if (ii >= ll + 1)
  {
    return 0.0;
  }
  double ff = (tt - knot[ii]) / (knot[ll + kk - 1] - knot[ii]);
  return ff;
}

} // anonymous namespace

std::vector<double> qtBSpline2D::uniformKnotVectorForInterpolatedEndpoints(
  std::size_t nn,
  std::size_t maxDegree,
  std::size_t* actualDegree)
{
  std::vector<double> knot;
  if (nn < 2)
  {
    if (actualDegree)
    {
      *actualDegree = -1;
    }
    return knot; // Empty knot vector for a single point as input.
  }
  std::size_t degree = maxDegree;
  if (nn < degree + 1)
  {
    degree = nn - 1;
  }
  // Repeat endpoint knot values degree + 1 times and insert unique knot values.
  std::size_t knotSize = (degree + 1) * 2 + (nn - degree);
  knot.reserve(knotSize);
  double knotValue = 0.;
  for (std::size_t ii = 0; ii <= degree; ++ii)
  {
    knot.push_back(knotValue);
  }
  knotValue += 1.0;
  for (std::size_t ii = 1; ii < nn - degree; ++ii)
  {
    knot.push_back(knotValue);
    knotValue += 1.0;
  }
  for (std::size_t ii = 0; ii <= degree; ++ii)
  {
    knot.push_back(knotValue);
  }
  if (actualDegree)
  {
    *actualDegree = degree;
  }
  return knot;
}

bool qtBSpline2D::insertKnot(
  std::size_t kk,
  std::size_t ll,
  double tt,
  std::vector<double>& knot,
  std::vector<QPointF>& controlPoint)
{
  if (ll >= knot.size() || knot[ll] > tt || (ll + 1 < knot.size() && knot[ll + 1] < tt))
  {
    return false;
  }
  std::vector<double> knotOut;
  std::vector<QPointF> ptsOut;
  knotOut.reserve(knot.size() + 1);
  ptsOut.reserve(controlPoint.size() + 1);
  knotOut.insert(knotOut.end(), knot.begin(), knot.begin() + ll + 1);
  knotOut.push_back(tt);
  if (ll + 1 < knot.size())
  {
    knotOut.insert(knotOut.end(), knot.begin() + ll + 1, knot.end());
  }

  ptsOut.push_back(controlPoint.front());
  for (std::size_t ii = 1; ii < controlPoint.size(); ++ii)
  {
    double aa = alpha(ii, ll, kk, tt, knot);
    ptsOut.push_back((1.0 - aa) * controlPoint[ii - 1] + aa * controlPoint[ii]);
  }
  ptsOut.push_back(controlPoint.back());

  knot = knotOut;
  controlPoint = ptsOut;

  return true;
}

bool qtBSpline2D::splineToCubicPath(
  std::size_t degree,
  const std::vector<QPointF>& controlPoints,
  const std::vector<double>& knot,
  std::vector<QPointF>& outputPath)
{
  outputPath.clear();
#if SMTK_DBG_SPLINE
  std::cout << "knot = [";
  for (const auto& vv : knot)
  {
    std::cout << " " << vv;
  }
  std::cout << " ], degree " << degree << " #cp " << controlPoints.size() << "\n";
#endif // SMTK_DBG_SPLINE
  std::size_t order = degree + 1;

  std::vector<double> knotOut;
  knotOut = knot;
  outputPath = controlPoints;
  std::size_t mm = 0; // position in knotOut (not knot) where we insert values.
  for (std::size_t ii = 0; ii < knot.size();)
  {
    std::size_t jj = ii;
    std::size_t multiplicityIn = 0;
    while (jj < knot.size() && knot[ii] == knot[jj])
    {
      ++jj;
      ++multiplicityIn;
    }
#if SMTK_DBG_SPLINE
    std::cout << "  " << knot[ii] << " multiplicity " << multiplicityIn << "\n";
#endif

    // Need to insert knot values to make multiplicity == degree.
    std::size_t numToInsert = multiplicityIn > order ? 0 : order - multiplicityIn;
    for (std::size_t nn = 0; nn < numToInsert; ++nn)
    {
      qtBSpline2D::insertKnot(order, mm, knot[ii], knotOut, outputPath);
      ++mm;
    }

    // Advance loop:
    mm += (jj - ii);
    ii = jj;
  }

#if SMTK_DBG_SPLINE
  std::cout << "interpolatedKnot = [";
  for (const auto& vv : knotOut)
  {
    std::cout << " " << vv;
  }
  std::cout << " ], degree " << degree << "\n";
  std::cout << "path = [";
  for (const auto& xx : outputPath)
  {
    std::cout << " (" << xx.x() << " " << xx.y() << ")";
  }
  std::cout << " ] (" << outputPath.size() << " pts)\n";
#endif // SMTK_DBG_SPLINE
  return true;
}

} // namespace extension
} // namespace smtk
