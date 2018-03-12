//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "qtLattice.h"
#include "smtk/bridge/rgg/qt/qtLatticeHelper.h"
#include "smtk/bridge/rgg/qt/rggNucCoordinateConverter.h"
#include "smtk/bridge/rgg/qt/rggNucPartDefinition.h"
#include "smtk/io/Logger.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/model/Manager.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits.h>

#include <QDebug>
#include <iostream>

// Radius helper functions
class rggNucMaxRadiusFunction
{
public:
  rggNucMaxRadiusFunction(double ri, double rj, double px, double py)
    : m_radiusX(ri)
    , m_radiusY(rj)
    , m_pitchX(px)
    , m_pitchY(py)
  {
    if (px == 0 || py == 0)
    { // Reassign 0 pitch to 1
      this->setPitch(px, py);
    }
    assert(ri > 0);
    assert(rj > 0);
  }
  virtual ~rggNucMaxRadiusFunction() {}
  virtual double getMaxRadius(int i, int j) = 0;
  virtual rggNucMaxRadiusFunction* clone() = 0;
  virtual double getCellRadius() const = 0;
  //virtual rggLatticeFillFunction * createFillFuntion(size_t i, size_t j, double r, qtLattice * l) = 0;
  virtual double getRi() const { return m_pitchX * 0.5; }
  virtual double getRj() const { return m_pitchY * 0.5; }
  double m_radiusX, m_radiusY;
  double m_centerX, m_centerY;

  virtual void setPitch(double x, double y)
  {
    this->m_pitchX = (x == 0) ? 1 : x;
    this->m_pitchY = (y == 0) ? 1 : y;
  }

protected:
  double m_pitchX, m_pitchY;
};

class rggNucMaxHexRadiusFunction : public rggNucMaxRadiusFunction
{
public:
  rggNucMaxHexRadiusFunction(double ri, double px)
    : rggNucMaxRadiusFunction(ri, ri, px, px)
  {
  }
  virtual double getMaxRadius(int level, int /*j*/)
  {
    double tmp = level * m_pitchX * rggNucMathConst::cos30;
    return std::max(0.0, this->m_radiusX - tmp); //less than zero has special meaning.
  }
  rggNucMaxRadiusFunction* clone()
  {
    return new rggNucMaxHexRadiusFunction(this->m_radiusX, this->m_pitchX);
  }
  virtual double getCellRadius() const { return m_pitchX * rggNucMathConst::cos30; }
  virtual double getRi() const { return m_pitchX; }
  virtual double getRj() const { return this->getRi(); }
  // FIXME when adding support for fill function
  // virtual cmbLaticeFillFunction* createFillFuntion(size_t i, size_t j, double r, Lattice * l)
  // {
  //   return cmbLaticeFillFunction::generateFunctionHex(i, j, r, true,
  //                                                     cmbLaticeFillFunction::RELATIVE_CIRCLE, l);
  // }
};

class rggNucMaxRectRadiusFunction : public rggNucMaxRadiusFunction
{
public:
  rggNucMaxRectRadiusFunction(double cx, double cy, double ri, double rj, double px, double py)
    : rggNucMaxRadiusFunction(ri, rj, px, py)
  {
    this->m_centerX = cx;
    this->m_centerY = cy;
  }
  virtual double getMaxRadius(int i, int j)
  {
    double di = std::abs(i - this->m_centerX);
    double dj = std::abs(j - this->m_centerY);
    di *= m_pitchX;
    dj *= m_pitchY;

    return std::max(0.0,
      std::min(this->m_radiusX - di, this->m_radiusY - dj)); //less than zero has special meaning.
  }
  rggNucMaxRadiusFunction* clone()
  {
    assert(m_pitchX != 0 && m_pitchY != 0);
    return new rggNucMaxRectRadiusFunction(this->m_centerX, this->m_centerY, this->m_radiusX,
      this->m_radiusY, this->m_pitchX, this->m_pitchY);
  }
  virtual double getCellRadius() const
  {
    assert(m_pitchX != 0 && m_pitchY != 0);
    return std::min(m_pitchX, m_pitchY) * 0.5;
  }
  //  virtual cmbLaticeFillFunction * createFillFuntion(size_t i, size_t j, double r, Lattice * l)
  //  {
  //    return cmbLaticeFillFunction::generateFunction(i, j, r, true,
  //                                                   cmbLaticeFillFunction::RELATIVE_CIRCLE, l);
  //  }
};

//lattice stuff
qtLattice::qtLattice()
  : m_enGeometryType(RECTILINEAR)
  , m_subType(FLAT | ANGLE_360)
  , m_fullCellMode(RECT)
{
  this->m_maxRadiusFun = new rggNucMaxRectRadiusFunction(
    2.0, 2.0, std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 1, 1);
  smtk::model::ManagerPtr ptr = qtActiveObjects::instance().activeModel().manager();
  if (!ptr)
  {
    return;
  }
  if (ptr->findEntitiesByProperty("label", "XX").size() == 0)
  { // Create a blank entity fits lattice need
    smtk::model::AuxiliaryGeometry blankAux = ptr->addAuxiliaryGeometry();
    blankAux.setName("Empty Cell");
    blankAux.setStringProperty("label", "XX");
    blankAux.setColor(1, 1, 1);
    this->m_blank = smtk::model::EntityRef(ptr, blankAux.entity());
  }
  else
  {
    this->m_blank = ptr->findEntitiesByProperty("label", "XX")[0];
  }

  this->SetDimensions(4, 4);
  this->m_radius = -1.0;
}

qtLattice::qtLattice(qtLattice const& other)
  : m_enGeometryType(other.m_enGeometryType)
  , m_subType(other.m_subType)
  , m_fullCellMode(other.m_fullCellMode)
{
  this->m_maxRadiusFun = other.m_maxRadiusFun->clone();
  this->setUpGrid(other);
  this->m_validRange = other.m_validRange;
  this->m_radius = -1.0;
  this->m_blank = other.m_blank;
}

qtLattice::~qtLattice()
{
  m_grid.clear(); // FIXME: store pointers instead of reference
  for (auto i = m_partToCell.begin(); i != m_partToCell.end(); ++i)
  {
    delete i->second;
    i->second = nullptr;
  }
  m_partToCell.clear();
  if (this->m_maxRadiusFun != nullptr)
  {
    delete this->m_maxRadiusFun;
  }
}

qtLattice& qtLattice::operator=(qtLattice const& other)
{
  delete this->m_maxRadiusFun;
  this->m_maxRadiusFun = nullptr;
  this->m_maxRadiusFun = other.m_maxRadiusFun->clone();
  this->m_subType = other.m_subType;
  this->m_enGeometryType = other.m_enGeometryType;
  this->m_fullCellMode = other.m_fullCellMode;
  this->m_validRange = other.m_validRange;
  this->m_radius = other.m_radius;
  setUpGrid(other);
  return *this;
}

void qtLattice::setUpGrid(qtLattice const& other)
{
  this->m_grid.clear();
  this->m_grid.resize(other.m_grid.size());
  for (size_t i = 0; i < other.m_grid.size(); ++i)
  {
    this->m_grid[i].resize(other.m_grid[i].size());
  }

  //Clear out old data
  for (auto i = this->m_partToCell.begin(); i != this->m_partToCell.end(); ++i)
  {
    delete i->second;
    i->second = nullptr;
  }

  this->m_partToCell.clear();

  //Add the parts to cells
  for (auto i = other.m_partToCell.begin(); i != other.m_partToCell.end(); ++i)
  {
    this->m_partToCell[i->first] = new qtCell(*(i->second));
  }

  //update grid
  this->m_grid.clear();
  this->m_grid.resize(other.m_grid.size());
  for (size_t i = 0; i < other.m_grid.size(); ++i)
  {
    this->m_grid[i].resize(other.m_grid[i].size());
  }
  for (size_t i = 0; i < other.m_grid.size(); ++i)
  {
    for (size_t j = 0; j < other.m_grid[i].size(); ++j)
    {
      qtCell* c = this->getCell(other.m_grid[i][j].getCell()->getPart());
      size_t oc = c->getCount();
      this->m_grid[i][j].setCell(c);
      assert(c->getCount() == (oc + 1));
    }
  }
  this->computeValidRange();
  this->sendMaxRadiusToReference();
}

void qtLattice::setInvalidCells()
{
  //TODO Look at this
  qtCell* invalid = this->getCell(smtk::model::EntityRef());
  //invalid->setInvalid();
  for (size_t i = 0; i < this->m_grid.size(); i++)
  {
    size_t start = (m_subType & FLAT) ? (i) : (i - (i) / 2);
    size_t cols = ((m_subType & FLAT) ? (i + 1) : (((i + 1) - (i + 2) % 2))) + start;
    if (m_subType & ANGLE_30)
    {
      start = 2 * i - i / 2;
      cols = (i % 2 ? (i + 1) / 2 : (i + 2) / 2) + start;
    }
    for (unsigned int j = 0; j < start; ++j)
    {
      this->m_grid[i][j].setCell(invalid);
    }
    for (size_t j = cols; j < this->m_grid[i].size(); j++)
    {
      this->m_grid[i][j].setCell(invalid);
    }
  }
}

void qtLattice::SetDimensions(size_t iin, size_t jin, bool reset)
{
  qtCell* invalid = this->getCell(smtk::model::EntityRef());
  qtCell* XX = this->getCell(this->m_blank);
  if (this->m_enGeometryType == RECTILINEAR)
  {
    this->m_grid.resize(iin);
    for (size_t k = 0; k < iin; k++)
    {
      size_t old = reset ? static_cast<size_t>(0) : this->m_grid[k].size();
      this->m_grid[k].resize(jin);
      for (size_t r = old; r < jin; ++r)
      {
        this->m_grid[k][r].setCell(XX);
      }
    }
  }
  else if (this->m_enGeometryType == HEXAGONAL)
  {
    size_t current = reset ? static_cast<size_t>(0) : this->m_grid.size();
    if (current == iin)
    {
      return;
    }

    this->m_grid.resize(iin);

    this->computeValidRange();

    if (iin > current)
    {
      for (size_t k = current; k < iin; k++)
      {
        if (k == 0)
        {
          this->m_grid[k].resize(1);
          this->m_grid[k][0].setCell(XX);
        }
        else
        {
          // for each layer, we need 6*Layer cells
          size_t cols = 6 * k;
          this->m_grid[k].resize(cols);
          size_t start = 0, end = 0;
          if (!this->getValidRange(k, start, end))
          {
            continue;
          }
          for (size_t c = 0; c < cols; c++)
          {
            if (start <= c && c <= end)
            {
              this->m_grid[k][c].setCell(XX);
            }
            else
            {
              this->m_grid[k][c].setCell(invalid);
            }
          }
        }
      }
    }
  }
  this->m_maxRadiusFun->m_centerX = (iin - 1) * 0.5;
  this->m_maxRadiusFun->m_centerY = (jin - 1) * 0.5;
  this->sendMaxRadiusToReference();
}

bool qtLattice::getValidRange(size_t layer, size_t& start, size_t& end) const
{
  if (this->m_enGeometryType == HEXAGONAL)
  {
    start = this->m_validRange[layer].first;
    end = this->m_validRange[layer].second;
    return true;
  }
  return false;
}

void qtLattice::computeValidRange()
{
  if (this->m_enGeometryType == HEXAGONAL)
  {
    this->m_validRange.resize(m_grid.size());
    for (std::size_t layer = 0; layer < m_grid.size(); ++layer)
    {
      this->m_validRange[layer].first = 0;
      if (layer == 0)
      {
        this->m_validRange[layer].second = 0;
      }
      else
      {
        const size_t tl = static_cast<int>(layer);
        this->m_validRange[layer].second = 6 * tl - 1;
        if (m_subType != 0 && !(m_subType & ANGLE_360))
        {
          this->m_validRange[layer].first = (m_subType & FLAT) ? (tl) : (tl - (tl / 2));
          this->m_validRange[layer].second =
            ((m_subType & FLAT) ? (tl + 1) : ((tl + 1) - (tl + 2) % 2)) +
            this->m_validRange[layer].first - 1;
          if (m_subType & ANGLE_30)
          {
            this->m_validRange[layer].first = 2 * tl - tl / 2;
            this->m_validRange[layer].second =
              (layer % 2 ? (tl + 1) / 2 : (tl + 2) / 2) + this->m_validRange[layer].first - 1;
          }
        }
      }
    }
  }
}

std::pair<size_t, size_t> qtLattice::GetDimensions() const
{
  if (this->m_grid.size() == 0)
    return std::make_pair(0, 0);
  if (this->m_enGeometryType == RECTILINEAR)
  {
    return std::make_pair(this->m_grid.size(), this->m_grid[0].size());
  }
  else
  {
    return std::make_pair(this->m_grid.size(), 6);
  }
}

bool qtLattice::SetCell(
  size_t i, size_t j, smtk::model::EntityRef part, bool valid, bool overInvalid)
{
  qtCell* cr = nullptr;
  if (!part.isValid() && valid)
  {
    cr = this->getCell(this->m_blank);
  }
  else
  {
    cr = this->getCell(part);
  }
  if (i >= this->m_grid.size() || j >= this->m_grid[i].size())
  {
    return false;
  }

  if (overInvalid || this->m_grid[i][j].getCell()->isValid())
  {
    return this->m_grid[i][j].setCell(cr);
  }
  return false;
}

qtCell qtLattice::GetCell(size_t i, size_t j) const
{
  return *(this->m_grid[i][j].getCell());
}

void qtLattice::ClearCell(size_t i, size_t j)
{
  qtCell* cr = this->getCell(m_blank);
  this->m_grid[i][j].setCell(cr);
}

bool qtLattice::ClearCell(const QString& label)
{
  bool r = false;
  for (size_t i = 0; i < this->m_grid.size(); i++)
  {
    for (size_t j = 0; j < this->m_grid[i].size(); j++)
    {
      if (this->GetCell(static_cast<int>(i), static_cast<int>(j)).getLabel() == label)
      {
        r = true;
        this->ClearCell(static_cast<int>(i), static_cast<int>(j));
      }
    }
  }
  return r;
}

bool qtLattice::replacePart(smtk::model::EntityRef oldObj, smtk::model::EntityRef newObj)
{
  bool change = false;
  if (!oldObj.isValid())
  {
    oldObj = this->m_blank;
  }
  if (!newObj.isValid())
  {
    newObj = this->m_blank;
  }
  qtCell* lcOld = this->getCell(oldObj);
  qtCell* lcNew = this->getCell(newObj);
  for (size_t i = 0; i < this->m_grid.size(); i++)
  {
    for (size_t j = 0; j < this->m_grid[i].size(); j++)
    {
      if (this->m_grid[i][j].getCell() == lcOld)
      {
        change = true;
        this->m_grid[i][j].setCell(lcNew);
      }
    }
  }
  return change;
}

bool qtLattice::deletePart(smtk::model::EntityRef obj)
{
  bool r = this->replacePart(obj, this->m_blank);
  auto iter = m_partToCell.find(obj);
  if (iter != m_partToCell.end())
  {
    delete iter->second;
    m_partToCell.erase(iter);
  }
  return r;
}

void qtLattice::SetGeometryType(rggGeometryType type)
{
  if (this->m_enGeometryType == type)
  {
    return;
  }
  this->m_enGeometryType = type;
  delete this->m_maxRadiusFun;
  if (type == RECTILINEAR)
  {
    this->m_fullCellMode = RECT;
    this->m_maxRadiusFun = new rggNucMaxRectRadiusFunction(
      2.0, 2.0, std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 1, 1);
    this->SetDimensions(4, 4); // Default size
  }
  else
  {
    this->m_maxRadiusFun = new rggNucMaxHexRadiusFunction(std::numeric_limits<double>::max(), 1);
    this->m_fullCellMode = HEX_FULL;
    this->SetDimensions(1, 1, true); // Default size
  }
}

rggGeometryType qtLattice::GetGeometryType()
{
  return this->m_enGeometryType;
}

void qtLattice::SetGeometrySubType(int type)
{
  this->m_subType = type;
}

int qtLattice::GetGeometrySubType() const
{
  return m_subType;
}

qtCell* qtLattice::getCell(smtk::model::EntityRef part)
{
  // An invalid entityRef is allowed because it means an invalid input
  qtCell* result = nullptr;
  auto iter = m_partToCell.find(part);
  if (iter == m_partToCell.end())
  {
    result = m_partToCell[part] = new qtCell(part);
  }
  else
  {
    result = iter->second;
  }
  return result;
}

qtLattice::CellDrawMode qtLattice::getDrawMode(size_t index, size_t layer) const
{
  if (this->m_enGeometryType == RECTILINEAR)
    return qtLattice::RECT;
  size_t start = 0, end = 0;
  if (!this->getValidRange(layer, start, end))
  {
    return qtLattice::RECT;
  }
  if (layer == 0)
  {
    if (this->m_subType & ANGLE_360)
    {
      return this->getFullCellMode();
    }
    else if (this->m_subType & ANGLE_60 && this->m_subType & FLAT)
    {
      return qtLattice::HEX_SIXTH_FLAT_CENTER;
    }
    else if (this->m_subType & ANGLE_60 && this->m_subType & VERTEX)
    {
      return qtLattice::HEX_SIXTH_VERT_CENTER;
    }
    else if (this->m_subType & ANGLE_30)
    {
      return qtLattice::HEX_TWELFTH_CENTER;
    }
  }
  else if (this->m_subType & ANGLE_360)
  {
    return m_fullCellMode;
  }
  else if (this->m_subType & ANGLE_60 && this->m_subType & FLAT)
  {
    if (index == start)
    {
      return qtLattice::HEX_SIXTH_FLAT_TOP;
    }
    else if (index == end)
    {
      return qtLattice::HEX_SIXTH_FLAT_BOTTOM;
    }
    else
    {
      return qtLattice::HEX_FULL;
    }
  }
  else if (this->m_subType & ANGLE_60 && this->m_subType & VERTEX)
  {
    if (index == start && layer % 2 == 0)
      return qtLattice::HEX_SIXTH_VERT_TOP;
    else if (index == end && layer % 2 == 0)
      return qtLattice::HEX_SIXTH_VERT_BOTTOM;
    return qtLattice::HEX_FULL_30;
  }
  else if (this->m_subType & ANGLE_30)
  {
    if (index == end)
      return qtLattice::HEX_TWELFTH_BOTTOM;
    else if (index == start && layer % 2 == 0)
      return qtLattice::HEX_TWELFTH_TOP;
  }
  return qtLattice::HEX_FULL;
}

QString qtLattice::generate_string(QString in, CellDrawMode mode)
{
  switch (mode)
  {
    case RECT:
    case HEX_FULL:
    case HEX_FULL_30:
      return in;
    case HEX_SIXTH_FLAT_BOTTOM:
    case HEX_SIXTH_VERT_BOTTOM:
    case HEX_TWELFTH_BOTTOM:
      return in + "_bottom";
    case HEX_SIXTH_FLAT_CENTER:
    case HEX_SIXTH_VERT_CENTER:
    case HEX_TWELFTH_CENTER:
      return in + "_center";
    case HEX_SIXTH_FLAT_TOP:
    case HEX_SIXTH_VERT_TOP:
    case HEX_TWELFTH_TOP:
      return in + "_top";
  }
  assert(false);
  return in;
}

//bool qtLattice::fill(cmbLaticeFillFunction * fun, cmbNucPart * obj)
//{
//  if(fun == NULL) return false;
//  bool change = false;
//  fun->begin();
//  size_t i, j;
//  while( fun->getNext(i,j) )
//  {
//    change |= this->SetCell(i, j, obj, true, false);
//  }
//  if(change) this->sendMaxRadiusToReference();
//  return change;
//}

std::vector<smtk::model::EntityRef> qtLattice::getUsedParts() const
{
  std::vector<smtk::model::EntityRef> result;
  for (auto i = m_partToCell.begin(); i != m_partToCell.end(); ++i)
  {
    qtCell* c = i->second;
    if (!c->isBlank() && c->isValid() && c->getCount() != 0)
    {
      result.push_back(c->getPart());
    }
  }
  return result;
}

qtLattice::ChangeMode qtLattice::compair(qtLattice const& other) const
{
  std::pair<int, int> dim = this->GetDimensions();
  std::pair<int, int> oDim = other.GetDimensions();
  if (dim.first != oDim.first || dim.second != oDim.second)
    return ChangeMode::SizeDiff;
  for (unsigned int i = 0; i < this->m_grid.size(); ++i)
  {
    for (unsigned int j = 0; j < this->m_grid[i].size(); ++j)
    {
      if (this->m_grid[i][j].getCell()->getPart() != other.m_grid[i][j].getCell()->getPart())
      {
        return ChangeMode::ContentDiff;
      }
    }
  }
  return ChangeMode::Same;
}

void qtLattice::unselect()
{
  for (size_t i = 0; i < this->m_grid.size(); ++i)
  {
    for (size_t j = 0; j < this->m_grid[i].size(); ++j)
    {
      this->m_grid[i][j].setDrawMode(qtCellReference::UNSELECTED);
      this->m_grid[i][j].clearPreview();
    }
  }
}

void qtLattice::setPotentialConflictCells(double r)
{
  for (size_t i = 0; i < this->m_grid.size(); ++i)
  {
    for (size_t j = 0; j < this->m_grid[i].size(); ++j)
    {
      if (this->m_grid[i][j].radiusConflicts(r))
      {
        this->m_grid[i][j].setDrawMode(qtCellReference::UNSELECTED);
      }
    }
  }
}

void qtLattice::selectCell(size_t i, size_t j)
{
  this->m_grid[i][j].setDrawMode(qtCellReference::SELECTED);
}

void qtLattice::clearSelection()
{
  for (size_t i = 0; i < this->m_grid.size(); ++i)
  {
    for (size_t j = 0; j < this->m_grid[i].size(); ++j)
    {
      this->m_grid[i][j].setDrawMode(qtCellReference::NORMAL);
      this->m_grid[i][j].clearPreview();
    }
  }
}

//void qtLattice::functionPreview(cmbLaticeFillFunction * fun, cmbNucPart * part)
//{
//  if(fun == NULL) return;
//  fun->begin();
//  size_t i, j;
//  Cell * pv = NULL;
//  if(part == NULL)
//  {
//    pv = this->getCell(this->m_blank);
//  }
//  else
//  {
//    pv = this->getCell(part);
//  }
//  while( fun->getNext(i,j) )
//  {
//    if(i < this->m_grid.size() &&  j < this->m_grid[i].size())
//    {
//      this->m_grid[i][j].setDrawMode( CellReference::FUNCTION_APPLY );
//      this->m_grid[i][j].setPreviewCell(pv);
//    }
//  }
//}

void qtLattice::checkCellRadiiForValidity()
{
}

void qtLattice::sendMaxRadiusToReference()
{
  rggNucCoordinateConverter converter(*this);
  std::vector<std::pair<size_t, size_t> > pinOutsideCell;
  double cellR = this->m_maxRadiusFun->getCellRadius();
  bool radiusGt0 = false;

  auto getPinMaxRadius = [](const smtk::model::EntityRef& part, bool warning = true) -> double {
    double partRadius = (!part.isValid() || !part.hasFloatProperty("max radius"))
      ? -1.0
      : part.floatProperty("max radius")[0];
    if (partRadius <= 0 && warning)
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "when sending max radius to instance,"
          << part.name() << " has an invalid radius");
    }
  };

  for (size_t i = 0; i < this->m_grid.size(); ++i)
  {
    for (size_t j = 0; j < this->m_grid[i].size(); ++j)
    {
      qtCellReference& cr = this->m_grid[i][j];
      cr.setMaxRadius(this->m_maxRadiusFun->getMaxRadius(i, j));
      cr.clearOverflow();
      smtk::model::EntityRef part = cr.getCell()->getPart();
      double r = getPinMaxRadius(part, false);
      radiusGt0 |= r > 0;
      if (cr.isOccupied() && r > cellR)
      {
        pinOutsideCell.push_back(std::pair<size_t, size_t>(i, j));
      }
    }
  }
  if (!radiusGt0)
    return; //this is for core, to short curcuit extra calculation
  double ri = this->m_maxRadiusFun->getRi();
  double rj = this->m_maxRadiusFun->getRj();
  for (size_t pit = 0; pit < pinOutsideCell.size(); ++pit)
  {
    std::pair<size_t, size_t>& at = pinOutsideCell[pit];
    double atX, atY;
    converter.convertToPixelXY(at.first, at.second, atX, atY, ri, rj);
    // Get radius
    smtk::model::EntityRef part = this->m_grid[at.first][at.second].getCell()->getPart();
    double partRadius = getPinMaxRadius(part);

    double partR2 = partRadius * partRadius;
    // FIXME
    //    cmbLaticeFillFunction * fun = this->m_maxRadiusFun->createFillFuntion(at.first, at.second,
    //                                                                        partRadius/cellR, this);
    //    fun->begin();
    //    size_t i, j;
    //    bool hasConflict = false;

    //    qDebug() << at.first << at.second;

    //    while( fun->getNext(i,j) )
    //    {
    //      if(i >= this->m_grid.size() || j >= this->m_grid[i].size()) continue;
    //      if(i == at.first && j == at.second) continue;
    //      double iX, jY;
    //      converter.convertToPixelXY(i, j, iX, jY, ri, rj);

    //      double di = atX - iX;
    //      double dj = atY - jY;
    //      double tmp = (di*di + dj*dj)/partR2;
    //      if(tmp <= 1)
    //      {
    //        this->m_grid[i][j].setMaxRadius(-0.5);
    //        hasConflict |= this->m_grid[i][j].isOccupied();
    //        qDebug() << "\t" << i << j << this->m_grid[i][j].isOccupied();
    //        qtLattice::CellReference & lcr = this->m_grid[at.first][at.second];
    //        this->m_grid[i][j].addOverflow(&lcr, at.first, at.second);
    //      }
    //    }

    //    if(hasConflict)
    //    {
    //      this->m_grid[at.first][at.second].setMaxRadius(-0.5);
    //    }
  }
  for (size_t ai = 0; ai < this->m_grid.size(); ++ai)
  {
    for (size_t aj = 0; aj < this->m_grid[ai].size(); ++aj)
    {
      qtCellReference& cr = this->m_grid[ai][aj];
      double maxRadius = cr.getMaxRadius();
      if (maxRadius < 0)
        continue; //Already in conflict
      double atX, atY;
      converter.convertToPixelXY(ai, aj, atX, atY, ri, rj);
      //      cmbLaticeFillFunction * fun = this->m_maxRadiusFun->createFillFuntion(ai, aj,
      //                                                                          maxRadius/cellR, this);
      //      fun->begin();
      //      size_t i, j;

      //      while( fun->getNext(i,j) )
      //      {
      //        if(i >= this->m_grid.size() || j >= this->m_grid[i].size()) continue;
      //        if(i == ai && j == aj) continue;
      //        if(this->m_grid[i][j].isOccupied())
      //        {
      //          double r = getPinMaxRadius(this->m_grid[i][j].getCell()->getPart());
      //          double iX, jY;
      //          converter.convertToPixelXY(i, j, iX, jY, ri, rj);
      //          double di = atX - iX;
      //          double dj = atY - jY;
      //          double tmp = std::sqrt(di*di + dj*dj) - r;
      //          if(tmp < maxRadius)
      //          {
      //            cr.setMaxRadius(tmp);
      //            maxRadius = tmp;
      //          }
      //        }
      //        if(this->m_grid[i][j].hasOccupiers())
      //        {
      //          std::vector<CellReference::OverflowPartReference *> const& oc =
      //                                                                    this->m_grid[i][j].getOccupiers();
      //          for(size_t k = 0; k < oc.size(); ++k)
      //          {
      //            CellReference::OverflowPartReference const* opr = oc[k];
      //            if(opr->centerCellI == ai && opr->centerCellJ == aj) continue;

      //            double iX, jY;
      //            converter.convertToPixelXY(opr->centerCellI, opr->centerCellJ, iX, jY, ri, rj);
      //            double r =
      //                  getPinMaxRadius(this->m_grid[opr->centerCellI][opr->centerCellJ].getCell()->getPart());
      //            double di = atX - iX;
      //            double dj = atY - jY;
      //            double tmp = std::sqrt(di*di + dj*dj) - r;
      //            if(tmp < maxRadius)
      //            {
      //              cr.setMaxRadius(tmp);
      //              maxRadius = tmp;
      //            }
      //          }
      //        }
      //      }
      //      delete fun;
    }
  }
}

void qtLattice::updatePitchForMaxRadius(double x, double y)
{
  this->m_maxRadiusFun->setPitch(x, y);
}

void qtLattice::updateMaxRadius(double ri, double rj)
{
  this->m_maxRadiusFun->m_radiusX = ri;
  this->m_maxRadiusFun->m_radiusY = rj;
  this->m_maxRadiusFun->m_centerX = (this->m_grid.size() - 1) * 0.5;
  this->m_maxRadiusFun->m_centerY = (this->m_grid[0].size() - 1) * 0.5;
}
