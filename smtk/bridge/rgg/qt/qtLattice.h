//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR //  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtLattice - UI component for rgg lattice. It has 2 nested classes that
// are cell and cellReference.
// .SECTION Description
// .SECTION See Also

#ifndef qtLattice_h
#define qtLattice_h

#include <QColor>
#include <QStringList>

#include <cassert>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

#include "smtk/model/EntityRef.h"

class qtDrawLatticeItem;
class qtDraw2DLattice;
class cmbLaticeFillFunction;
class cmbNucMaxRadiusFunction;

namespace smtk
{
namespace model
{
class EntityRef;
}
}

class qtLattice
{
public:
  enum enumGeometryType
  {
    RECTILINEAR = 0x0140,
    HEXAGONAL = 0x0241,
  };
  enum changeMode
  {
    Same = 0,
    SizeDiff = 1,
    ContentDiff = 2
  };
  // FIXME
  friend class qtDrawLatticeItem;
  friend class qtDraw2DLattice;

  enum CellDrawMode
  {
    RECT = -1,
    HEX_FULL = 0,
    HEX_FULL_30 = 1,
    HEX_SIXTH_FLAT_BOTTOM,
    HEX_SIXTH_FLAT_CENTER,
    HEX_SIXTH_FLAT_TOP,
    HEX_SIXTH_VERT_BOTTOM,
    HEX_SIXTH_VERT_CENTER,
    HEX_SIXTH_VERT_TOP,
    HEX_TWELFTH_BOTTOM,
    HEX_TWELFTH_CENTER,
    HEX_TWELFTH_TOP
  };

  static QString generate_string(QString, CellDrawMode);

  // Represents a cell in the lattice view widget. It's a wrapper class around
  // EntityRef which has an additional class variable as count
  struct Cell
  {
    Cell(smtk::model::EntityRef ent)
      : Count(0)
      , Part(ent)
    {
    }
    Cell(Cell const& other)
      : Count(0)
      , Part(other.Part)
    {
    }

    bool isBlank() const { return this->getLabel() == "XX"; }

    QString getLabel() const
    {
      return (this->isValid() && Part.hasStringProperty("label")) ? Part.stringProperty("label")
                                                                  : "";
    }
    QColor getColor() const
    {
      if (this->isValid() && Part.hasColor())
      {
        smtk::model::FloatList color = Part.color();
        return QColor::fromRgbF(color[0], color[1], color[2], color[3]);
      }
      else
      {
        return Qt::black;
      }
    }

    bool isValid() const { return this->Part.isValid(); }

    void setPart(smtk::model::EntityRef p) { this->Part = p; }
    smtk::model::EntityRef getPart() const { return this->Part; }

    void inc() { Count++; }
    void dec() { Count--; }

    size_t getCount() const { return Count; }
  protected:
    size_t Count;
    smtk::model::EntityRef Part;
  };

  qtLattice();

  qtLattice(qtLattice const& other);

  ~qtLattice();

  qtLattice& operator=(qtLattice const& arg);

  void setInvalidCells();

  // Sets the dimensions of the cell assembly.
  // For Hex type, i is number of layers, j will be ignored.
  // To force a resize of all layers, pass reset=true
  void SetDimensions(size_t i, size_t j, bool reset = false);

  // Returns the dimensions of the cell assembly.
  // For Hex type, first is number of layers, second is set to 6 (for hex)
  std::pair<size_t, size_t> GetDimensions() const;

  // Sets the contents of the cell (i, j) to name.
  // For Hex type, i is layer/ring index, j is index on that layer
  bool SetCell(
    size_t i, size_t j, smtk::model::EntityRef part, bool valid = true, bool overInvalid = true);

  // Returns the contents of the cell (i, j).
  // For Hex type, i is layer/ring index, j is index on that layer
  Cell GetCell(size_t i, size_t j) const;

  // Clears the contents of the cell (i, j).
  // For Hex type, i is layer/ring index, j is index on that layer
  void ClearCell(size_t i, size_t j);
  bool ClearCell(const QString& label);

  bool getValidRange(size_t layer, size_t& start, size_t& end) const;

  bool replacePart(smtk::model::EntityRef oldObj, smtk::model::EntityRef newObj);

  bool deletePart(smtk::model::EntityRef obj);

  std::vector<smtk::model::EntityRef> getUsedParts() const;

  // get/set Geometry type (hex or rect)
  void SetGeometryType(enumGeometryType type);

  enumGeometryType GetGeometryType();

  void SetGeometrySubType(int type);
  int GetGeometrySubType() const;

  size_t getSize() const { return Grid.size(); }
  size_t getSize(size_t i) const { return Grid[i].size(); }

  qtLattice::CellDrawMode getDrawMode(size_t index, size_t layer) const;

  void setFullCellMode(qtLattice::CellDrawMode m)
  {
    if (this->enGeometryType == HEXAGONAL)
    {
      FullCellMode = m;
    }
    else
    {
      FullCellMode = RECT;
    }
  }

  qtLattice::CellDrawMode getFullCellMode() const
  {
    assert(FullCellMode == RECT || FullCellMode == HEX_FULL || FullCellMode == HEX_FULL_30);
    return FullCellMode;
  }

  // FIXME when adding support for fill function
  //bool fill(cmbLaticeFillFunction * fun, smtk::model::EntityRef* obj);

  changeMode compair(qtLattice const& other) const;

  void selectCell(size_t i, size_t j);
  void unselect();
  void clearSelection();
  // FIXME when adding support for fill function
  //void functionPreview(cmbLaticeFillFunction * fun, smtk::model::EntityRef* p);
  void setPotentialConflictCells(double r);

  void setRadius(double r) { this->radius = r; }
  void checkCellRadiiForValidity();

  void updatePitchForMaxRadius(double x, double y);
  void updateMaxRadius(double ri, double rj);
  void sendMaxRadiusToReference();

protected:
  class CellReference
  {
  public:
    enum DrawMode
    {
      SELECTED = 1,
      UNSELECTED = 0,
      NORMAL = 2,
      FUNCTION_APPLY = 3
    };
    CellReference();
    CellReference(CellReference const& other);
    ~CellReference();

    bool setCell(Cell* c);

    Cell* getCell();
    Cell const* getCell() const;

    Cell* getPreviewCell();
    Cell const* getPreviewCell() const;

    DrawMode getDrawMode() const { return mode; }
    void setDrawMode(DrawMode m) const { mode = m; }
    void setPreviewCell(Cell* pv) { previewCell = pv; }
    void clearPreview() { previewCell = cell; }

    Cell const* getModeCell() const;

    void setMaxRadius(double r) { this->maxRadius = r; }
    double getMaxRadius() const { return maxRadius; }

    bool isInConflict() const;

    bool radiusConflicts(double r) const;

    bool isOccupied() const;
    bool hasOccupiers() const { return !cellOccupier.empty(); }

    void addOverflow(CellReference* ref, size_t i, size_t j);

    struct OverflowPartReference;

    std::vector<CellReference::OverflowPartReference*> const& getOccupiers() const;
    void clearOverflow();

  protected:
    Cell* cell;
    Cell* previewCell;

    std::vector<OverflowPartReference*> cellOccupier;

    mutable DrawMode mode;
    double maxRadius;
  };

  Cell* getCell(smtk::model::EntityRef* part);

  void setUpGrid(qtLattice const& other);

  CellReference const& getRerference(int i, int j) { return this->Grid[i][j]; }

  std::vector<std::vector<CellReference> > Grid;
  std::map<smtk::model::EntityRef*, Cell*> PartToCell;
  enumGeometryType enGeometryType;
  int subType;
  std::vector<std::pair<size_t, size_t> > validRange;
  void computeValidRange();
  qtLattice::CellDrawMode FullCellMode;
  smtk::model::EntityRef* Blank;
  double radius;
  cmbNucMaxRadiusFunction* maxRadiusFun;
};

class LatticeContainer : public smtk::model::EntityRef
{
public:
  LatticeContainer(QString label, QString name, QColor color, double px, double py)
    : smtk::model::EntityRef(label, name, color)
    , pitchX(px)
    , pitchY(py)
  {
    std::cout << "Create LatticeContainer" << std::endl;
  }
  qtLattice& getLattice() { return this->lattice; }
  virtual QString extractLabel(QString const&) = 0;
  virtual void fillList(std::vector<std::pair<QString, smtk::model::EntityRef*> >& l) = 0;
  virtual smtk::model::EntityRef* getFromLabel(const QString&) = 0;
  virtual bool IsHexType() = 0;
  virtual void calculateExtraTranslation(double& transX, double& transY) = 0;
  virtual void calculateTranslation(double& transX, double& transY) = 0;
  virtual void setUpdateUsed() = 0;
  virtual void updateLaticeFunction();
  virtual void getRadius(double& ri, double& rj) const;
  virtual double getRadius() const { return -1; }
  double getPitchX() const { return this->pitchX; }
  double getPitchY() const { return this->pitchY; }
  void setPitch(double x, double y)
  {
    this->pitchX = x;
    this->pitchY = y;
  }
  std::pair<int, int> GetDimensions() { return this->lattice.GetDimensions(); }
protected:
  qtLattice lattice;
  double pitchX;
  double pitchY;
};

#endif
