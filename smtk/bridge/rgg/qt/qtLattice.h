//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtLattice - UI component for rgg lattice.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_qtLattice_h
#define __smtk_bridge_rgg_qt_qtLattice_h

#include "smtk/bridge/rgg/qt/qtLatticeHelper.h"
#include "smtk/bridge/rgg/qt/rggNucPartDefinition.h"
#include "smtk/model/EntityRef.h"

#include "smtk/bridge/rgg/qt/Exports.h"

#include <QColor>
#include <QStringList>

#include <cassert>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

class qtCell;
class qtDrawLatticeItem;
class qtDraw2DLattice;
// FIXME when adding support for fill function
//class cmbLaticeFillFunction;
class rggNucMaxRadiusFunction;

namespace smtk
{
namespace model
{
class EntityRef;
}
}

class SMTKQTRGGSESSION_EXPORT qtLattice
{
public:
  enum ChangeMode
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
  qtCell GetCell(size_t i, size_t j) const;

  // Clears the contents of the cell (i, j).
  // For Hex type, i is layer/ring index, j is index on that layer
  void ClearCell(size_t i, size_t j);
  bool ClearCell(const QString& label);

  bool getValidRange(size_t layer, size_t& start, size_t& end) const;

  bool replacePart(smtk::model::EntityRef oldObj, smtk::model::EntityRef newObj);

  bool deletePart(smtk::model::EntityRef obj);

  std::vector<smtk::model::EntityRef> getUsedParts() const;

  // get/set Geometry type (hex or rect)
  void SetGeometryType(rggGeometryType type);

  rggGeometryType GetGeometryType();

  void SetGeometrySubType(int type);
  int GetGeometrySubType() const;

  size_t getSize() const { return m_grid.size(); }
  size_t getSize(size_t i) const { return m_grid[i].size(); }

  qtLattice::CellDrawMode getDrawMode(size_t index, size_t layer) const;

  void setFullCellMode(qtLattice::CellDrawMode m)
  {
    if (this->m_enGeometryType == HEXAGONAL)
    {
      m_fullCellMode = m;
    }
    else
    {
      m_fullCellMode = RECT;
    }
  }

  qtLattice::CellDrawMode getFullCellMode() const
  {
    assert(m_fullCellMode == RECT || m_fullCellMode == HEX_FULL || m_fullCellMode == HEX_FULL_30);
    return m_fullCellMode;
  }

  // FIXME when adding support for fill function
  //bool fill(cmbLaticeFillFunction * fun, smtk::model::EntityRef* obj);

  ChangeMode compair(qtLattice const& other) const;

  void selectCell(size_t i, size_t j);
  void unselect();
  void clearSelection();
  // FIXME when adding support for fill function
  //void functionPreview(cmbLaticeFillFunction * fun, smtk::model::EntityRef* p);
  void setPotentialConflictCells(double r);

  void setRadius(double r) { this->m_radius = r; }
  void checkCellRadiiForValidity();

  void updatePitchForMaxRadius(double x, double y);
  void updateMaxRadius(double ri, double rj);
  void sendMaxRadiusToReference();

protected:
  qtCell* getCell(smtk::model::EntityRef part);

  void setUpGrid(qtLattice const& other);

  qtCellReference const& getRerference(int i, int j) { return this->m_grid[i][j]; }

  void computeValidRange();

  std::vector<std::vector<qtCellReference> > m_grid;
  std::map<smtk::model::EntityRef, qtCell*> m_partToCell;
  rggGeometryType m_enGeometryType;
  int m_subType;
  std::vector<std::pair<size_t, size_t> > m_validRange;
  qtLattice::CellDrawMode m_fullCellMode;
  smtk::model::EntityRef m_blank;
  double m_radius;
  rggNucMaxRadiusFunction* m_maxRadiusFun;
};

#endif
