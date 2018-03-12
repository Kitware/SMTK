//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/qt/qtLatticeHelper.h"
#include "smtk/bridge/rgg/qt/qtLattice.h"

#include "smtk/model/EntityRef.h"

#include <limits>

#include <QColor>
#include <QString>

// qtCell
bool qtCell::isBlank() const
{
  // FIXME: check when to show XX
  if (!this->isValid())
  {
    return true;
  }
  return this->getLabel() == "XX";
}

QString qtCell::getLabel() const
{
  std::string label =
    (this->isValid() && m_part.hasStringProperty("label")) ? m_part.stringProperty("label")[0] : "";
  return QString::fromStdString(label);
}

QColor qtCell::getColor() const
{
  if (this->isValid() && m_part.hasColor())
  {
    smtk::model::FloatList color = m_part.color();
    if (color[3] == -1)
    { //TODO: Color not defined. Humm, make it black for now
      return Qt::black;
    }
    return QColor::fromRgbF(color[0], color[1], color[2], color[3]);
  }
  else
  {
    return Qt::black;
  }
}

bool qtCell::isValid() const
{
  return m_part.isValid();
}

qtCellReference::qtCellReference()
  : m_cell(nullptr)
  , m_previewCell(nullptr)
  , m_mode(NORMAL)
{
}

qtCellReference::qtCellReference(qtCellReference const& other)
  : m_cell(other.m_cell)
  , m_previewCell(other.m_previewCell)
  , m_mode(NORMAL)
  , m_maxRadius(other.m_maxRadius)
{
  if (m_cell)
  {
    m_cell->inc();
  }
  for (size_t i = 0; i < other.m_cellOccupier.size(); ++i)
  {
    this->m_cellOccupier.push_back(new qtOverflowPartReference(other.m_cellOccupier[i]));
  }
}

qtCellReference::~qtCellReference()
{
  if (m_cell != nullptr)
  {
    m_cell->dec();
  }
  this->clearOverflow();
}

std::vector<qtOverflowPartReference*> const& qtCellReference::getOccupiers() const
{
  return this->m_cellOccupier;
}

bool qtCellReference::setCell(qtCell* c)
{
  if (c == m_cell)
    return false;
  if (m_cell && m_cell->getCount())
  {
    m_cell->dec();
  }
  if (c)
    c->inc();
  this->m_cell = c;
  this->m_previewCell = c;
  return true;
}

qtCell* qtCellReference::getCell()
{
  return this->m_cell;
}

qtCell const* qtCellReference::getCell() const
{
  return this->m_cell;
}

qtCell* qtCellReference::getPreviewCell()
{
  return this->m_previewCell;
}

qtCell const* qtCellReference::getPreviewCell() const
{
  return this->m_previewCell;
}

qtCell const* qtCellReference::getModeCell() const
{
  switch (this->getDrawMode())
  {
    case qtCellReference::SELECTED:
    case qtCellReference::FUNCTION_APPLY:
      return this->getPreviewCell();
    default:
      return this->getCell();
  }
}

// qtCellReference
bool qtCellReference::isInConflict() const
{
  // FIXME: get radius for EntityRef

  //return this->isOccupied() && this->radiusConflicts(m_cell->getPart()->getRadius());
  return false;
}

bool qtCellReference::radiusConflicts(double r) const
{
  return false;
  // TODO: add radius check support
  std::cout << "  radiusConflicts: r=" << r << " maxradius=" << this->getMaxRadius() << std::endl;
  return r >= this->getMaxRadius();
}

bool qtCellReference::isOccupied() const
{
  return m_cell != nullptr && !m_cell->isBlank() && m_cell->getPart().isValid();
}

void qtCellReference::addOverflow(qtCellReference* ref, size_t i, size_t j)
{
  this->m_cellOccupier.push_back(new qtOverflowPartReference(i, j, *ref));
}

void qtCellReference::clearOverflow()
{
  for (size_t i = 0; i < this->m_cellOccupier.size(); ++i)
  {
    delete this->m_cellOccupier[i];
  }
  this->m_cellOccupier.clear();
}

qtOverflowPartReference::qtOverflowPartReference(size_t ci, size_t cj, qtCellReference& ref)
  : m_centerCellI(ci)
  , m_centerCellJ(cj)
  , m_reference(ref)
{
}
qtOverflowPartReference::qtOverflowPartReference(qtOverflowPartReference const* o)
  : m_centerCellI(o->m_centerCellI)
  , m_centerCellJ(o->m_centerCellJ)
  , m_reference(o->m_reference)
{
}
