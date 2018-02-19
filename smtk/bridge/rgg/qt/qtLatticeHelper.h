//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR //  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtLatticeHelper - Helper classes for qtLattice
// are cell and cellReference.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_qtLatticeHelper_h
#define __smtk_bridge_rgg_qt_qtLatticeHelper_h

#include <stdio.h>

#include "smtk/model/EntityRef.h"

#include "smtk/bridge/rgg/qt/Exports.h"

class qtOverflowPartReference;
class qtLattice;
class QColor;
class QString;

namespace smtk
{
namespace model
{
class EntityRef;
}
}

// Represents a cell in the lattice view widget. It's a wrapper class around
// EntityRef which has an additional class variable as count
class SMTKQTRGGSESSION_EXPORT qtCell
{
public:
  qtCell(smtk::model::EntityRef ent)
    : m_count(0)
    , m_part(ent)
  {
  }
  qtCell(qtCell const& other)
    : m_count(0)
    , m_part(other.m_part)
  {
  }

  bool isBlank() const;

  QString getLabel() const;
  QColor getColor() const;

  bool isValid() const;

  void setPart(smtk::model::EntityRef p) { this->m_part = p; }
  smtk::model::EntityRef getPart() const { return this->m_part; }

  void inc() { m_count++; }
  void dec() { m_count--; }

  size_t getCount() const { return m_count; }
protected:
  size_t m_count;
  smtk::model::EntityRef m_part;
};

// Each cellReference instance stores two cells - one for normal visualization
// and another for preview purpose.
class SMTKQTRGGSESSION_EXPORT qtCellReference
{
public:
  enum DrawMode
  {
    SELECTED = 1,
    UNSELECTED = 0,
    NORMAL = 2,
    FUNCTION_APPLY = 3
  };
  qtCellReference();
  qtCellReference(qtCellReference const& other);
  ~qtCellReference();

  bool setCell(qtCell* c);

  qtCell* getCell();
  qtCell const* getCell() const;

  qtCell* getPreviewCell();
  qtCell const* getPreviewCell() const;

  DrawMode getDrawMode() const { return this->m_mode; }
  void setDrawMode(DrawMode m) const { this->m_mode = m; }
  void setPreviewCell(qtCell* pv) { this->m_previewCell = pv; }
  void clearPreview() { this->m_previewCell = this->m_cell; }

  qtCell const* getModeCell() const;

  void setMaxRadius(double r) { this->m_maxRadius = r; }
  double getMaxRadius() const { return this->m_maxRadius; }

  bool isInConflict() const;

  bool radiusConflicts(double r) const;

  bool isOccupied() const;
  bool hasOccupiers() const { return !this->m_cellOccupier.empty(); }

  void addOverflow(qtCellReference* ref, size_t i, size_t j);

  std::vector<qtOverflowPartReference*> const& getOccupiers() const;
  void clearOverflow();

protected:
  qtCell* m_cell;
  qtCell* m_previewCell;

  std::vector<qtOverflowPartReference*> m_cellOccupier;

  mutable DrawMode m_mode;
  double m_maxRadius;
};

class SMTKQTRGGSESSION_EXPORT qtOverflowPartReference
{
public:
  qtOverflowPartReference(size_t ci, size_t cj, qtCellReference& ref);
  qtOverflowPartReference(qtOverflowPartReference const* o);
  size_t m_centerCellI, m_centerCellJ;
  qtCellReference m_reference;
};

#endif
