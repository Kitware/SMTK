#include "qtLattice.h"
#include "cmbNucCordinateConverter.h"
#include "cmbNucFillLattice.h"
#include "cmbNucPartDefinition.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits.h>

#include <QDebug>

namespace
{
cmbNucPart SingleBlank("XX", "XX");
}

//radius helper functions
class cmbNucMaxRadiusFunction
{
public:
  cmbNucMaxRadiusFunction(double ri, double rj, double px, double py)
    : radiusX(ri)
    , radiusY(rj)
    , pitchX(px)
    , pitchY(py)
  {
    if (px == 0 || py == 0)
      setPitch(px, py);
    assert(ri > 0);
    assert(rj > 0);
  }
  virtual ~cmbNucMaxRadiusFunction() {}
  virtual double getMaxRadius(int i, int j) = 0;
  virtual cmbNucMaxRadiusFunction* clone() = 0;
  virtual double getCellRadius() const = 0;
  virtual cmbLaticeFillFunction* createFillFuntion(size_t i, size_t j, double r, Lattice* l) = 0;
  virtual double getRi() const { return pitchX * 0.5; }
  virtual double getRj() const { return pitchY * 0.5; }
  double radiusX, radiusY;
  double centerX, centerY;

  virtual void setPitch(double x, double y)
  {
    if (x == 0)
      x = 1;
    if (y == 0)
      y = 1;
    pitchX = x;
    pitchY = y;
  }

protected:
  double pitchX, pitchY;
};

class cmbNucMaxHexRadiusFunction : public cmbNucMaxRadiusFunction
{
public:
  cmbNucMaxHexRadiusFunction(double ri, double px)
    : cmbNucMaxRadiusFunction(ri, ri, px, px)
  {
  }
  virtual double getMaxRadius(int level, int /*j*/)
  {
    double tmp = level * pitchX * cmbNucMathConst::cos30;
    return std::max(0.0, this->radiusX - tmp); //less than zero has special meaning.
  }
  cmbNucMaxRadiusFunction* clone()
  {
    return new cmbNucMaxHexRadiusFunction(this->radiusX, this->pitchX);
  }
  virtual double getCellRadius() const { return pitchX * cmbNucMathConst::cos30; }
  virtual double getRi() const { return pitchX; }
  virtual double getRj() const { return getRi(); }
  virtual cmbLaticeFillFunction* createFillFuntion(size_t i, size_t j, double r, Lattice* l)
  {
    return cmbLaticeFillFunction::generateFunctionHex(
      i, j, r, true, cmbLaticeFillFunction::RELATIVE_CIRCLE, l);
  }
};

class cmbNucMaxRectRadiusFunction : public cmbNucMaxRadiusFunction
{
public:
  cmbNucMaxRectRadiusFunction(double cx, double cy, double ri, double rj, double px, double py)
    : cmbNucMaxRadiusFunction(ri, rj, px, py)
  {
    centerX = cx;
    centerY = cy;
  }
  virtual double getMaxRadius(int i, int j)
  {
    double di = std::abs(i - centerX);
    double dj = std::abs(j - centerY);
    di *= pitchX;
    dj *= pitchY;

    return std::max(
      0.0, std::min(this->radiusX - di, this->radiusY - dj)); //less than zero has special meaning.
  }
  cmbNucMaxRadiusFunction* clone()
  {
    assert(pitchX != 0 && pitchY != 0);
    return new cmbNucMaxRectRadiusFunction(
      this->centerX, this->centerY, this->radiusX, this->radiusY, this->pitchX, this->pitchY);
  }
  virtual double getCellRadius() const
  {
    assert(pitchX != 0 && pitchY != 0);
    return std::min(pitchX, pitchY) * 0.5;
  }
  virtual cmbLaticeFillFunction* createFillFuntion(size_t i, size_t j, double r, Lattice* l)
  {
    return cmbLaticeFillFunction::generateFunction(
      i, j, r, true, cmbLaticeFillFunction::RELATIVE_CIRCLE, l);
  }
};

struct Lattice::CellReference::OverflowPartReference
{
  OverflowPartReference(size_t ci, size_t cj, CellReference& ref)
    : centerCellI(ci)
    , centerCellJ(cj)
    , reference(ref)
  {
  }
  OverflowPartReference(OverflowPartReference const* o)
    : centerCellI(o->centerCellI)
    , centerCellJ(o->centerCellJ)
    , reference(o->reference)
  {
  }
  size_t centerCellI, centerCellJ;
  Lattice::CellReference reference;
};

//lattice stuff
Lattice::Lattice()
  : enGeometryType(RECTILINEAR)
  , subType(FLAT | ANGLE_360)
  , FullCellMode(RECT)
  , Blank(&SingleBlank)
{
  this->maxRadiusFun = new cmbNucMaxRectRadiusFunction(
    2.0, 2.0, std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 1, 1);
  this->SetDimensions(4, 4);
  this->radius = -1;
}

Lattice::Lattice(Lattice const& other)
  : enGeometryType(other.enGeometryType)
  , subType(other.subType)
  , FullCellMode(other.FullCellMode)
  , Blank(&SingleBlank)
{
  this->maxRadiusFun = other.maxRadiusFun->clone();
  setUpGrid(other);
  this->validRange = other.validRange;
  this->radius = -1;
}

Lattice::~Lattice()
{
  Grid.clear();
  for (std::map<cmbNucPart*, Cell*>::iterator i = PartToCell.begin(); i != PartToCell.end(); ++i)
  {
    delete i->second;
    i->second = NULL;
  }
  PartToCell.clear();
  delete this->maxRadiusFun;
}

Lattice& Lattice::operator=(Lattice const& other)
{
  delete this->maxRadiusFun;
  this->maxRadiusFun = NULL;
  this->maxRadiusFun = other.maxRadiusFun->clone();
  this->subType = other.subType;
  this->enGeometryType = other.enGeometryType;
  this->FullCellMode = other.FullCellMode;
  this->validRange = other.validRange;
  this->radius = other.radius;
  setUpGrid(other);
  return *this;
}

void Lattice::setUpGrid(Lattice const& other)
{
  this->Grid.clear();
  this->Grid.resize(other.Grid.size());
  for (unsigned int i = 0; i < other.Grid.size(); ++i)
  {
    this->Grid[i].resize(other.Grid[i].size());
  }

  //Clear out old data
  for (std::map<cmbNucPart*, Cell*>::iterator i = this->PartToCell.begin();
       i != this->PartToCell.end(); ++i)
  {
    delete i->second;
    i->second = NULL;
  }

  this->PartToCell.clear();

  //Add the parts to cells
  for (std::map<cmbNucPart*, Cell*>::const_iterator i = other.PartToCell.begin();
       i != other.PartToCell.end(); ++i)
  {
    this->PartToCell[i->first] = new Cell(*(i->second));
  }

  //update grid
  this->Grid.clear();
  this->Grid.resize(other.Grid.size());
  for (unsigned int i = 0; i < other.Grid.size(); ++i)
  {
    this->Grid[i].resize(other.Grid[i].size());
  }
  for (unsigned int i = 0; i < other.Grid.size(); ++i)
  {
    for (unsigned j = 0; j < other.Grid[i].size(); ++j)
    {
      Cell* c = this->getCell(other.Grid[i][j].getCell()->getPart());
      int oc = c->getCount();
      (void)(oc);
      this->Grid[i][j].setCell(c);
      assert(c->getCount() == static_cast<unsigned int>(oc) + 1);
    }
  }
  this->computeValidRange();
  this->sendMaxRadiusToReference();
}

void Lattice::setInvalidCells()
{
  //TODO Look at this
  Cell* invalid = this->getCell(NULL);
  //invalid->setInvalid();
  for (size_t i = 0; i < this->Grid.size(); i++)
  {
    size_t start = (subType & FLAT) ? (i) : (i - (i) / 2);
    size_t cols = ((subType & FLAT) ? (i + 1) : (((i + 1) - (i + 2) % 2))) + start;
    if (subType & ANGLE_30)
    {
      start = 2 * i - i / 2;
      cols = (i % 2 ? (i + 1) / 2 : (i + 2) / 2) + start;
    }
    for (unsigned int j = 0; j < start; ++j)
    {
      this->Grid[i][j].setCell(invalid);
    }
    for (size_t j = cols; j < this->Grid[i].size(); j++)
    {
      this->Grid[i][j].setCell(invalid);
    }
  }
}

void Lattice::SetDimensions(size_t iin, size_t jin, bool reset)
{
  Cell* invalid = this->getCell(NULL);
  Cell* XX = this->getCell(this->Blank);
  if (this->enGeometryType == RECTILINEAR)
  {
    this->Grid.resize(iin);
    for (size_t k = 0; k < iin; k++)
    {
      int old = static_cast<int>(reset ? 0 : this->Grid[k].size());
      this->Grid[k].resize(jin);
      for (size_t r = old; r < jin; ++r)
      {
        this->Grid[k][r].setCell(XX);
      }
    }
  }
  else if (this->enGeometryType == HEXAGONAL)
  {
    size_t current = reset ? static_cast<size_t>(0) : this->Grid.size();
    if (current == iin)
    {
      return;
    }

    this->Grid.resize(iin);

    this->computeValidRange();

    if (iin > current)
    {
      for (size_t k = current; k < iin; k++)
      {
        if (k == 0)
        {
          this->Grid[k].resize(1);
          this->Grid[k][0].setCell(XX);
        }
        else
        {
          // for each layer, we need 6*Layer cells
          size_t cols = 6 * k;
          this->Grid[k].resize(cols);
          size_t start = 0, end = 0;
          if (!getValidRange(k, start, end))
            continue;
          for (size_t c = 0; c < cols; c++)
          {
            if (start <= c && c <= end)
            {
              this->Grid[k][c].setCell(XX);
            }
            else
            {
              this->Grid[k][c].setCell(invalid);
            }
          }
        }
      }
    }
  }
  this->maxRadiusFun->centerX = (iin - 1) * 0.5;
  this->maxRadiusFun->centerY = (jin - 1) * 0.5;
  this->sendMaxRadiusToReference();
}

bool Lattice::getValidRange(size_t layer, size_t& start, size_t& end) const
{
  if (this->enGeometryType == HEXAGONAL)
  {
    start = this->validRange[layer].first;
    end = this->validRange[layer].second;
    return true;
  }
  return false;
}

void Lattice::computeValidRange()
{
  if (this->enGeometryType == HEXAGONAL)
  {
    this->validRange.resize(Grid.size());
    for (std::size_t layer = 0; layer < Grid.size(); ++layer)
    {
      this->validRange[layer].first = 0;
      if (layer == 0)
      {
        this->validRange[layer].second = 0;
      }
      else
      {
        const size_t tl = static_cast<int>(layer);
        this->validRange[layer].second = 6 * tl - 1;
        if (subType != 0 && !(subType & ANGLE_360))
        {
          this->validRange[layer].first = (subType & FLAT) ? (tl) : (tl - (tl / 2));
          this->validRange[layer].second =
            ((subType & FLAT) ? (tl + 1) : ((tl + 1) - (tl + 2) % 2)) +
            this->validRange[layer].first - 1;
          if (subType & ANGLE_30)
          {
            this->validRange[layer].first = 2 * tl - tl / 2;
            this->validRange[layer].second =
              (layer % 2 ? (tl + 1) / 2 : (tl + 2) / 2) + this->validRange[layer].first - 1;
          }
        }
      }
    }
  }
}

std::pair<size_t, size_t> Lattice::GetDimensions() const
{
  if (this->Grid.size() == 0)
    return std::make_pair(0, 0);
  if (this->enGeometryType == RECTILINEAR)
  {
    return std::make_pair(this->Grid.size(), this->Grid[0].size());
  }
  else
  {
    return std::make_pair(this->Grid.size(), 6);
  }
}

bool Lattice::SetCell(size_t i, size_t j, cmbNucPart* part, bool valid, bool overInvalid)
{
  Cell* cr = NULL;
  if (part == NULL && valid)
  {
    cr = this->getCell(this->Blank);
  }
  else
  {
    cr = this->getCell(part);
  }
  if (i >= this->Grid.size() || j >= this->Grid[i].size())
    return false;
  if (overInvalid || this->Grid[i][j].getCell()->isValid())
  {
    return this->Grid[i][j].setCell(cr);
  }
  return false;
}

Lattice::Cell Lattice::GetCell(size_t i, size_t j) const
{
  return *(this->Grid[i][j].getCell());
}

void Lattice::ClearCell(size_t i, size_t j)
{
  Cell* cr = this->getCell(Blank);
  this->Grid[i][j].setCell(cr);
}

bool Lattice::ClearCell(const QString& label)
{
  bool r = false;
  for (size_t i = 0; i < this->Grid.size(); i++)
  {
    for (size_t j = 0; j < this->Grid[i].size(); j++)
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

bool Lattice::replacePart(cmbNucPart* oldObj, cmbNucPart* newObj)
{
  bool change = false;
  if (oldObj == NULL)
    oldObj = this->Blank;
  if (newObj == NULL)
    newObj = this->Blank;
  Cell* lcOld = this->getCell(oldObj);
  Cell* lcNew = this->getCell(newObj);
  for (size_t i = 0; i < this->Grid.size(); i++)
  {
    for (size_t j = 0; j < this->Grid[i].size(); j++)
    {
      if (this->Grid[i][j].getCell() == lcOld)
      {
        change = true;
        this->Grid[i][j].setCell(lcNew);
      }
    }
  }
  return change;
}

bool Lattice::deletePart(cmbNucPart* obj)
{
  bool r = this->replacePart(obj, this->Blank);
  std::map<cmbNucPart*, Cell*>::iterator iter = PartToCell.find(obj);
  if (iter != PartToCell.end())
  {
    delete iter->second;
    PartToCell.erase(iter);
  }
  return r;
}

void Lattice::SetGeometryType(enumGeometryType type)
{
  if (this->enGeometryType == type)
    return;
  this->enGeometryType = type;
  delete this->maxRadiusFun;
  if (type == RECTILINEAR)
  {
    this->FullCellMode = RECT;
    this->maxRadiusFun = new cmbNucMaxRectRadiusFunction(
      2.0, 2.0, std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 1, 1);
    this->SetDimensions(4, 4);
  }
  else
  {
    this->maxRadiusFun = new cmbNucMaxHexRadiusFunction(std::numeric_limits<double>::max(), 1);
    this->FullCellMode = HEX_FULL;
    this->SetDimensions(1, 1, true);
  }
}

enumGeometryType Lattice::GetGeometryType()
{
  return this->enGeometryType;
}

void Lattice::SetGeometrySubType(int type)
{
  subType = type;
}

int Lattice::GetGeometrySubType() const
{
  return subType;
}

Lattice::Cell* Lattice::getCell(cmbNucPart* part)
{
  Cell* result = NULL;
  std::map<cmbNucPart*, Cell*>::iterator iter = PartToCell.find(part);
  if (iter == PartToCell.end())
  {
    result = PartToCell[part] = new Cell(part);
  }
  else
  {
    result = iter->second;
  }
  return result;
}

Lattice::CellDrawMode Lattice::getDrawMode(size_t index, size_t layer) const
{
  if (this->enGeometryType == RECTILINEAR)
    return Lattice::RECT;
  size_t start = 0, end = 0;
  if (!this->getValidRange(layer, start, end))
  {
    return Lattice::RECT;
  }
  if (layer == 0)
  {
    if (this->subType & ANGLE_360)
    {
      return this->getFullCellMode();
    }
    else if (this->subType & ANGLE_60 && this->subType & FLAT)
    {
      return Lattice::HEX_SIXTH_FLAT_CENTER;
    }
    else if (this->subType & ANGLE_60 && this->subType & VERTEX)
    {
      return Lattice::HEX_SIXTH_VERT_CENTER;
    }
    else if (this->subType & ANGLE_30)
    {
      return Lattice::HEX_TWELFTH_CENTER;
    }
  }
  else if (this->subType & ANGLE_360)
  {
    return FullCellMode;
  }
  else if (this->subType & ANGLE_60 && this->subType & FLAT)
  {
    if (index == start)
    {
      return Lattice::HEX_SIXTH_FLAT_TOP;
    }
    else if (index == end)
    {
      return Lattice::HEX_SIXTH_FLAT_BOTTOM;
    }
    else
    {
      return Lattice::HEX_FULL;
    }
  }
  else if (this->subType & ANGLE_60 && this->subType & VERTEX)
  {
    if (index == start && layer % 2 == 0)
      return Lattice::HEX_SIXTH_VERT_TOP;
    else if (index == end && layer % 2 == 0)
      return Lattice::HEX_SIXTH_VERT_BOTTOM;
    return Lattice::HEX_FULL_30;
  }
  else if (this->subType & ANGLE_30)
  {
    if (index == end)
      return Lattice::HEX_TWELFTH_BOTTOM;
    else if (index == start && layer % 2 == 0)
      return Lattice::HEX_TWELFTH_TOP;
  }
  return Lattice::HEX_FULL;
}

QString Lattice::generate_string(QString in, CellDrawMode mode)
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

bool Lattice::fill(cmbLaticeFillFunction* fun, cmbNucPart* obj)
{
  if (fun == NULL)
    return false;
  bool change = false;
  fun->begin();
  size_t i, j;
  while (fun->getNext(i, j))
  {
    change |= this->SetCell(i, j, obj, true, false);
  }
  if (change)
    this->sendMaxRadiusToReference();
  return change;
}

std::vector<cmbNucPart*> Lattice::getUsedParts() const
{
  std::vector<cmbNucPart*> result;
  for (std::map<cmbNucPart*, Cell*>::const_iterator i = PartToCell.begin(); i != PartToCell.end();
       ++i)
  {
    Cell* c = i->second;
    if (!c->isBlank() && c->isValid() && c->getCount() != 0)
    {
      result.push_back(c->getPart());
    }
  }
  return result;
}

Lattice::changeMode Lattice::compair(Lattice const& other) const
{
  std::pair<int, int> dim = this->GetDimensions();
  std::pair<int, int> oDim = other.GetDimensions();
  if (dim.first != oDim.first || dim.second != oDim.second)
    return SizeDiff;
  for (unsigned int i = 0; i < this->Grid.size(); ++i)
  {
    for (unsigned int j = 0; j < this->Grid[i].size(); ++j)
    {
      if (this->Grid[i][j].getCell()->getPart() != other.Grid[i][j].getCell()->getPart())
      {
        return ContentDiff;
      }
    }
  }
  return Same;
}

void Lattice::unselect()
{
  for (size_t i = 0; i < this->Grid.size(); ++i)
  {
    for (size_t j = 0; j < this->Grid[i].size(); ++j)
    {
      this->Grid[i][j].setDrawMode(CellReference::UNSELECTED);
      this->Grid[i][j].clearPreview();
    }
  }
}

void Lattice::setPotentialConflictCells(double r)
{
  for (size_t i = 0; i < this->Grid.size(); ++i)
  {
    for (size_t j = 0; j < this->Grid[i].size(); ++j)
    {
      if (this->Grid[i][j].radiusConflicts(r))
      {
        this->Grid[i][j].setDrawMode(CellReference::UNSELECTED);
      }
    }
  }
}

void Lattice::selectCell(size_t i, size_t j)
{
  this->Grid[i][j].setDrawMode(CellReference::SELECTED);
}

void Lattice::clearSelection()
{
  for (size_t i = 0; i < this->Grid.size(); ++i)
  {
    for (size_t j = 0; j < this->Grid[i].size(); ++j)
    {
      this->Grid[i][j].setDrawMode(CellReference::NORMAL);
      this->Grid[i][j].clearPreview();
    }
  }
}

void Lattice::functionPreview(cmbLaticeFillFunction* fun, cmbNucPart* part)
{
  if (fun == NULL)
    return;
  fun->begin();
  size_t i, j;
  Cell* pv = NULL;
  if (part == NULL)
  {
    pv = this->getCell(this->Blank);
  }
  else
  {
    pv = this->getCell(part);
  }
  while (fun->getNext(i, j))
  {
    if (i < this->Grid.size() && j < this->Grid[i].size())
    {
      this->Grid[i][j].setDrawMode(CellReference::FUNCTION_APPLY);
      this->Grid[i][j].setPreviewCell(pv);
    }
  }
}

void Lattice::checkCellRadiiForValidity()
{
}

void Lattice::sendMaxRadiusToReference()
{
  cmbNucCordinateConverter converter(*this);
  std::vector<std::pair<size_t, size_t> > pinOutsideCell;
  double cellR = this->maxRadiusFun->getCellRadius();
  bool radiusGt0 = false;
  for (size_t i = 0; i < this->Grid.size(); ++i)
  {
    for (size_t j = 0; j < this->Grid[i].size(); ++j)
    {
      Lattice::CellReference& cr = this->Grid[i][j];
      cr.setMaxRadius(this->maxRadiusFun->getMaxRadius(i, j));
      cr.clearOverflow();
      cmbNucPart* part = cr.getCell()->getPart();
      double r = (part == NULL) ? -1 : cr.getCell()->getPart()->getRadius();
      radiusGt0 |= r > 0;
      if (cr.isOccupied() && r > cellR)
      {
        pinOutsideCell.push_back(std::pair<size_t, size_t>(i, j));
      }
    }
  }
  if (!radiusGt0)
    return; //this is for core, to short curcuit extra calculation
  double ri = this->maxRadiusFun->getRi();
  double rj = this->maxRadiusFun->getRj();
  for (size_t pit = 0; pit < pinOutsideCell.size(); ++pit)
  {
    std::pair<size_t, size_t>& at = pinOutsideCell[pit];
    double atX, atY;
    converter.convertToPixelXY(at.first, at.second, atX, atY, ri, rj);
    double partRadius = this->Grid[at.first][at.second].getCell()->getPart()->getRadius();
    double partR2 = partRadius * partRadius;
    cmbLaticeFillFunction* fun =
      this->maxRadiusFun->createFillFuntion(at.first, at.second, partRadius / cellR, this);
    fun->begin();
    size_t i, j;
    bool hasConflict = false;

    qDebug() << at.first << at.second;

    while (fun->getNext(i, j))
    {
      if (i >= this->Grid.size() || j >= this->Grid[i].size())
        continue;
      if (i == at.first && j == at.second)
        continue;
      double iX, jY;
      converter.convertToPixelXY(i, j, iX, jY, ri, rj);

      double di = atX - iX;
      double dj = atY - jY;
      double tmp = (di * di + dj * dj) / partR2;
      if (tmp <= 1)
      {
        this->Grid[i][j].setMaxRadius(-0.5);
        hasConflict |= this->Grid[i][j].isOccupied();
        qDebug() << "\t" << i << j << this->Grid[i][j].isOccupied();
        Lattice::CellReference& lcr = this->Grid[at.first][at.second];
        this->Grid[i][j].addOverflow(&lcr, at.first, at.second);
      }
    }

    if (hasConflict)
    {
      this->Grid[at.first][at.second].setMaxRadius(-0.5);
    }
  }
  for (size_t ai = 0; ai < this->Grid.size(); ++ai)
  {
    for (size_t aj = 0; aj < this->Grid[ai].size(); ++aj)
    {
      Lattice::CellReference& cr = this->Grid[ai][aj];
      double maxRadius = cr.getMaxRadius();
      if (maxRadius < 0)
        continue; //Already in conflict
      double atX, atY;
      converter.convertToPixelXY(ai, aj, atX, atY, ri, rj);
      cmbLaticeFillFunction* fun =
        this->maxRadiusFun->createFillFuntion(ai, aj, maxRadius / cellR, this);
      fun->begin();
      size_t i, j;

      while (fun->getNext(i, j))
      {
        if (i >= this->Grid.size() || j >= this->Grid[i].size())
          continue;
        if (i == ai && j == aj)
          continue;
        if (this->Grid[i][j].isOccupied())
        {
          double r = this->Grid[i][j].getCell()->getPart()->getRadius();
          double iX, jY;
          converter.convertToPixelXY(i, j, iX, jY, ri, rj);
          double di = atX - iX;
          double dj = atY - jY;
          double tmp = std::sqrt(di * di + dj * dj) - r;
          if (tmp < maxRadius)
          {
            cr.setMaxRadius(tmp);
            maxRadius = tmp;
          }
        }
        if (this->Grid[i][j].hasOccupiers())
        {
          std::vector<CellReference::OverflowPartReference*> const& oc =
            this->Grid[i][j].getOccupiers();
          for (size_t k = 0; k < oc.size(); ++k)
          {
            CellReference::OverflowPartReference const* opr = oc[k];
            if (opr->centerCellI == ai && opr->centerCellJ == aj)
              continue;

            double iX, jY;
            converter.convertToPixelXY(opr->centerCellI, opr->centerCellJ, iX, jY, ri, rj);
            double r =
              this->Grid[opr->centerCellI][opr->centerCellJ].getCell()->getPart()->getRadius();
            double di = atX - iX;
            double dj = atY - jY;
            double tmp = std::sqrt(di * di + dj * dj) - r;
            if (tmp < maxRadius)
            {
              cr.setMaxRadius(tmp);
              maxRadius = tmp;
            }
          }
        }
      }
      delete fun;
    }
  }
}

void Lattice::updatePitchForMaxRadius(double x, double y)
{
  this->maxRadiusFun->setPitch(x, y);
}

void Lattice::updateMaxRadius(double ri, double rj)
{
  this->maxRadiusFun->radiusX = ri;
  this->maxRadiusFun->radiusY = rj;
  this->maxRadiusFun->centerX = (this->Grid.size() - 1) * 0.5;
  this->maxRadiusFun->centerY = (this->Grid[0].size() - 1) * 0.5;
}

//latice cell reference stuff
Lattice::CellReference::CellReference()
  : cell(NULL)
  , previewCell(NULL)
  , mode(NORMAL)
{
}

Lattice::CellReference::CellReference(CellReference const& other)
  : cell(other.cell)
  , previewCell(other.previewCell)
  , mode(NORMAL)
  , maxRadius(other.maxRadius)
{
  if (cell)
    cell->inc();
  for (size_t i = 0; i < other.cellOccupier.size(); ++i)
  {
    this->cellOccupier.push_back(new OverflowPartReference(other.cellOccupier[i]));
  }
}

Lattice::CellReference::~CellReference()
{
  if (cell)
    cell->dec();
  this->clearOverflow();
}

std::vector<Lattice::CellReference::OverflowPartReference*> const&
Lattice::CellReference::getOccupiers() const
{
  return cellOccupier;
}

bool Lattice::CellReference::setCell(Lattice::Cell* c)
{
  if (c == cell)
    return false;
  if (cell && cell->getCount())
    cell->dec();
  if (c)
    c->inc();
  cell = c;
  previewCell = c;
  return true;
}

Lattice::Cell* Lattice::CellReference::getCell()
{
  return cell;
}

Lattice::Cell const* Lattice::CellReference::getCell() const
{
  return cell;
}

Lattice::Cell* Lattice::CellReference::getPreviewCell()
{
  return previewCell;
}

Lattice::Cell const* Lattice::CellReference::getPreviewCell() const
{
  return previewCell;
}

Lattice::Cell const* Lattice::CellReference::getModeCell() const
{
  switch (this->getDrawMode())
  {
    case Lattice::CellReference::SELECTED:
    case Lattice::CellReference::FUNCTION_APPLY:
      return this->getPreviewCell();
    default:
      return this->getCell();
  }
}

bool Lattice::CellReference::isInConflict() const
{
  return this->isOccupied() && this->radiusConflicts(cell->getPart()->getRadius());
}

bool Lattice::CellReference::radiusConflicts(double r) const
{
  return r >= this->getMaxRadius();
}

bool Lattice::CellReference::isOccupied() const
{
  return cell != NULL && !cell->isBlank() && cell->getPart() != NULL;
}

void Lattice::CellReference::addOverflow(CellReference* ref, size_t i, size_t j)
{
  this->cellOccupier.push_back(new OverflowPartReference(i, j, *ref));
}

void Lattice::CellReference::clearOverflow()
{
  for (size_t i = 0; i < this->cellOccupier.size(); ++i)
  {
    delete this->cellOccupier[i];
  }
  this->cellOccupier.clear();
}

//latice container functions
void LatticeContainer::updateLaticeFunction()
{
  double ri, rj;
  this->getRadius(ri, rj);
  lattice.updatePitchForMaxRadius(this->getPitchX(), this->getPitchY());
  lattice.updateMaxRadius(ri, rj);
  lattice.sendMaxRadiusToReference();
}

void LatticeContainer::getRadius(double& ri, double& rj) const
{
  ri = std::numeric_limits<double>::max();
  rj = std::numeric_limits<double>::max();
}
