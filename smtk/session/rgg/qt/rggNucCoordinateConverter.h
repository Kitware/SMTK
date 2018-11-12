//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME rggNucCoordinateConverter - Helper class to convert rgg nuc coordinates
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_session_rgg_qt_rggNucCoordinateConverter_h
#define __smtk_session_rgg_qt_rggNucCoordinateConverter_h

#include "smtk/session/rgg/qt/Exports.h"

#include <stdlib.h>
class qtLattice;
class rggNucCoordinateConverterInternal;

class SMTKQTRGGSESSION_EXPORT rggNucMathConst
{
public:
  static const double cos30;
  static const double cos30Squared;
  static const double cosSinAngles[6][2];
  static const double radians60;
};

class SMTKQTRGGSESSION_EXPORT rggNucCoordinateConverter
{
public:
  // for hex: control = true will disable rotation based on pointyness
  // for rec: control = true will disable fliping i axis
  rggNucCoordinateConverter(qtLattice& lat, bool control = false);
  ~rggNucCoordinateConverter();
  void convertToPixelXY(int i, int j, double& x, double& y, double radius = 0.5);
  void convertToPixelXY(int i, int j, double& x, double& y, double radius1, double radius2);
  void computeRadius(int w, int h, double r[2]);

  static void convertToHexCoordinates(size_t level, size_t ringI, int& x, int& y);
  static void convertFromHexCoordinates(int x, int y, size_t& level, size_t& ringI);
  static void convertFromHexCoordToEuclid(int i, int j, double& x, double& y);

private:
  rggNucCoordinateConverterInternal* Internal;
};

#endif
