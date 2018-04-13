//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/qt/rggNucCoordinateConverter.h"
#include "smtk/bridge/rgg/qt/qtLattice.h"
#include "smtk/bridge/rgg/qt/rggNucPartDefinition.h"

#include "vtkMath.h"
#include <cassert>
#include <cmath>

const double rggNucMathConst::cos30 = 0.86602540378443864676372317075294;
const double rggNucMathConst::cos30Squared =
  0.86602540378443864676372317075294 * 0.86602540378443864676372317075294;

const double rggNucMathConst::radians60 = vtkMath::Pi() / 3.0;

const double rggNucMathConst::cosSinAngles[6][2] = { { -0.5, -rggNucMathConst::cos30 },
  { 0.5, -rggNucMathConst::cos30 }, { 1.0, 0.0 }, { 0.5, rggNucMathConst::cos30 },
  { -0.5, rggNucMathConst::cos30 }, { -1.0, 0.0 } };

class rggNucCoordinateConverterInternal
{
  // FIXME: Needed?
  friend class rggNucCoordinateConverter;

protected:
  rggNucCoordinateConverterInternal(qtLattice& g)
    : m_grid(g)
  {
  }
  virtual ~rggNucCoordinateConverterInternal() {}
  virtual void convertToPixelXY(int i, int j, double& x, double& y, double r1, double r2) = 0;
  virtual void computeRadius(int w, int h, double r[2]) = 0;
  qtLattice& m_grid;
};

namespace
{
class convertHex : public rggNucCoordinateConverterInternal
{
  friend class rggNucCoordinateConverter;

public:
  convertHex(qtLattice& g, bool ignoreRotation)
    : rggNucCoordinateConverterInternal(g)
  {
    assert(this->m_grid.GetGeometryType() == rggGeometryType::HEXAGONAL);
    static const int indx[] = { 0, 1, 2, 3, 4, 5, 0, 5, 4, 3, 2, 1, 0 };
    int const* off1 = indx;
    int const* off2 = indx;
    if (!ignoreRotation)
    {
      if (m_grid.GetGeometrySubType() & ANGLE_60 && m_grid.GetGeometrySubType() & VERTEX)
      {
        off1 += 7;
        off2 += 11;
      }
      else if (m_grid.GetGeometrySubType() & ANGLE_360 &&
        m_grid.getFullCellMode() == qtLattice::HEX_FULL_30)
      {
        off1 += 6;
        off2 += 11;
      }
    }
    for (int c = 0; c < 6; c++)
    {
      int ca = off1[c];
      m_corner[c][0] = rggNucMathConst::cosSinAngles[ca][off2[0]];
      m_corner[c][1] = rggNucMathConst::cosSinAngles[ca][off2[1]];
    }
    for (int c = 0; c < 6; c++)
    {
      int cp1 = (c + 1) % 6;
      m_dir[c][0] = (m_corner[cp1][0] - m_corner[c][0]);
      m_dir[c][1] = (m_corner[cp1][1] - m_corner[c][1]);
    }
  }
  virtual void convertToPixelXY(int i, int j, double& x, double& y, double r1, double /*r2*/)
  {
    if (i == 0)
    {
      x = 0;
      y = 0;
      return;
    }
    double layerRadius = r1 * i;
    int s = j / i;
    int sr = j % i;
    double deltax = m_dir[s][0] * layerRadius / i;
    double deltay = m_dir[s][1] * layerRadius / i;
    x = layerRadius * m_corner[s][0] + deltax * sr;
    y = layerRadius * m_corner[s][1] + deltay * sr;
  }

  virtual void computeRadius(int w, int h, double r[2])
  {
    int numLayers = static_cast<int>(m_grid.GetDimensions().first);
    double hexDiameter;
    if (this->m_grid.GetGeometrySubType() & ANGLE_60 ||
      this->m_grid.GetGeometrySubType() & ANGLE_30)
    {
      if (this->m_grid.GetGeometrySubType() & ANGLE_60 && this->m_grid.GetGeometrySubType() & FLAT)
      {
        double n = 1 / (1.75 * numLayers - 0.5);
        hexDiameter = std::min(w * n, h * n / rggNucMathConst::cos30);
      }
      else if (this->m_grid.GetGeometrySubType() & ANGLE_60 &&
        this->m_grid.GetGeometrySubType() & VERTEX)
      {
        double n = 2 * numLayers - 0.4;
        double nl = 0;
        if (numLayers % 2 == 0)
          nl = numLayers * 0.5 * 1.5 - 0.5; //even
        else
          nl = (numLayers - 1) * 0.5 * 1.5 + 0.5; //odd
        hexDiameter = std::min(
          w / (rggNucMathConst::cos30Squared * n), h / (2 * (nl + 0.1) * rggNucMathConst::cos30));
      }
      else
      {
        double n = 1 / (1.75 * numLayers - 0.5);
        hexDiameter = std::min(w * n, h * (n * 1.8) / rggNucMathConst::cos30);
      }
    }
    else
    {
      double nl = numLayers * 0.75 - 0.25;
      double den[] = { rggNucMathConst::cos30 * 4 * static_cast<double>(2 * numLayers - 1),
        static_cast<double>(8 * nl) };
      hexDiameter = std::min((w - 10) / den[this->m_grid.getFullCellMode()],
                      (h - 10) / den[(this->m_grid.getFullCellMode() + 1) % 2]) *
        2;
    }
    hexDiameter = std::max(hexDiameter, 20.0); // Enforce minimum size for hexes
    r[0] = hexDiameter / 2.0;
    r[1] = rggNucMathConst::cos30 * r[0] * 4;
  }

  double m_corner[6][2];
  double m_dir[6][2];
};

class convertRect : public rggNucCoordinateConverterInternal
{
  friend class rggNucCoordinateConverter;

public:
  convertRect(qtLattice& g, bool fi)
    : rggNucCoordinateConverterInternal(g)
    , m_flip_i(fi)
  {
    assert(this->m_grid.GetGeometryType() != HEXAGONAL);
  }

  virtual void convertToPixelXY(int i, int j, double& x, double& y, double r1, double r2)
  {
    std::pair<size_t, size_t> wh = this->m_grid.GetDimensions();
    r1 *= 2;
    r2 *= 2;
    if (m_flip_i)
    {
      i = wh.first - 1 - i;
    }
    x = (i)*r1;
    y = -(j)*r2; // Convert from qt coordinate system to natural coordinate system
  }

  virtual void computeRadius(int w, int h, double r[2])
  {
    std::pair<int, int> wh = this->m_grid.GetDimensions();
    double radius = std::min(w, h) / std::max(wh.first, wh.second) * 0.5;
    r[0] = r[1] = std::max(radius, 20.0);
  }

private:
  bool m_flip_i;
};
}

rggNucCoordinateConverter::rggNucCoordinateConverter(qtLattice& lat, bool control)
{
  if (lat.GetGeometryType() == HEXAGONAL)
  {
    this->Internal = new convertHex(lat, control);
  }
  else
  {
    this->Internal = new convertRect(lat, control);
  }
}

rggNucCoordinateConverter::~rggNucCoordinateConverter()
{
  delete this->Internal;
}

void rggNucCoordinateConverter::convertToPixelXY(int i, int j, double& x, double& y, double radius)
{
  this->convertToPixelXY(i, j, x, y, radius, radius);
}

void rggNucCoordinateConverter::convertToPixelXY(
  int i, int j, double& x, double& y, double radius1, double radius2)
{
  this->Internal->convertToPixelXY(i, j, x, y, radius1, radius2);
}

void rggNucCoordinateConverter::computeRadius(int w, int h, double r[2])
{
  this->Internal->computeRadius(w, h, r);
}

void rggNucCoordinateConverter::convertToHexCoordinates(size_t level, size_t ringI, int& x, int& y)
{
  if (level == 0)
  {
    x = 0;
    y = 0;
    return;
  }
  static int origin[6][2] = { { -1, 0 }, { 0, -1 }, { 1, -1 }, { 1, 0 }, { 0, 1 }, { -1, 1 } };
  static int direction[6][2] = { { 1, -1 }, { 1, 0 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { 0, -1 } };
  size_t s = ringI / level;
  int r = static_cast<int>(ringI % level);
  x = static_cast<int>(level * origin[s][0] + direction[s][0] * r);
  y = static_cast<int>(level * origin[s][1] + direction[s][1] * r);
}

void rggNucCoordinateConverter::convertFromHexCoordinates(
  int x, int y, size_t& level, size_t& ringI)
{
  int zyx[] = { -(x + y), y, x };
  level = std::max(std::abs(zyx[0]), std::max(std::abs(zyx[1]), std::abs(zyx[2])));
  if (level == 0)
  {
    ringI = 0;
    return;
  }
  int ring = 0;
  int second = 0;
  static const int signs[] = { 1, -1 };
  for (int i = 0; i < 6; ++i)
  {
    int at = i % 3;
    int sign = signs[(i % 2)];
    if (zyx[at] == sign * static_cast<int>(level))
    {
      ring = i;
      second = (at + 1) % 3;
      break;
    }
  }
  ringI = static_cast<size_t>(level * ring + std::abs(zyx[second]));
}

void rggNucCoordinateConverter::convertFromHexCoordToEuclid(int i, int j, double& x, double& y)
{
  x = (i + j * 0.5);
  y = rggNucMathConst::cos30 * j;
}
