//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtBSpline2D_h
#define smtk_extension_qt_qtBSpline2D_h

#include <QPointF>

#include <vector>

namespace smtk
{
namespace extension
{

/**\brief Utilities for working with 2-D B-splines.
  *
  * These methods are used by smtk::extension::qtResourceDiagramArc to
  * convert a B-spline control polygon that interpolates only its endpoints
  * into a set of cubic Bézier points (since the latter is what Qt can render).
  */
class qtBSpline2D
{
public:
  /// Return a knot vector that, given a sequence of \a nn control points, will
  /// produce a spline of degree \ a dd (or lower) that interpolates its endpoints.
  ///
  /// An empty vector indicates that \a nn < 2.
  ///
  /// Example: if \a dd is 3 (a cubic spline is desired), then:
  /// + if \a nn == 2, the output degree will be 1;
  /// + if \a nn == 3, the output degree will be 2;
  /// + for \a nn >= 4, the output degree will be 3.
  static std::vector<double> uniformKnotVectorForInterpolatedEndpoints(
    std::size_t nn,
    std::size_t maxDegree = 3,
    std::size_t* actualDegree = nullptr);

  /// Insert a knot value, \a tt, at location \a ll in a spline of order \a kk.
  ///
  /// This algorithm modifies the passed \a knot and \a controlPoint vectors;
  /// on output, \a knot will include an additional value and \a controlPoint
  /// will contain an additional point.
  ///
  /// If you attempt an insertion that causes a non-monotonic knot vector,
  /// this method will return false and make no changes to \a knot and \a controlPoint.
  ///
  /// Note that \a kk = degree + 1, so if your curve is cubic, \a kk will be 4.
  static bool insertKnot(
    std::size_t kk,
    std::size_t ll,
    double tt,
    std::vector<double>& knot,
    std::vector<QPointF>& controlPoint);

  /// Insert knots into a cubic- (or lower-) degree spline to get cubic (or lower)
  /// Bezier points out. If the input degree is too high, false is returned.
  ///
  /// This repeatedly calls insertKnot on a *copy* of \a controlPoints and
  /// \a knotVector to determine the output path points. An alternative to
  /// repeated invocation of the Boehm insertKnot algorithm would be to
  /// implement the Oslo algorithm[1], but it is more complex.
  ///
  /// [1]: Cohen, Lyche, and Riesenfeld. Discrete B-Splines and Subdivision Techniques
  ///      in Computer-Aided Geometric Design and Computer Graphics. Computer Graphics
  ///      and Image Processing, v14n2. October, 1980. Academic Press.
  static bool splineToCubicPath(
    std::size_t degree,
    const std::vector<QPointF>& controlPoints,
    const std::vector<double>& knotVector,
    std::vector<QPointF>& outputPath);
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qt_qtBSpline2D_h
