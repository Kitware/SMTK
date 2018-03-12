//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR //  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME rggLatticeContainer - An abstract class for ui representation of
// rgg assemblies and cores
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_rggLatticeContainer_h
#define __smtk_bridge_rgg_qt_rggLatticeContainer_h

#include "smtk/bridge/rgg/qt/qtLattice.h"
#include "smtk/model/EntityRef.h"

// Represents an assembly/core in schema planner.
class rggLatticeContainer
{
public:
  rggLatticeContainer(smtk::model::EntityRef entity);
  virtual ~rggLatticeContainer();
  qtLattice& getLattice() { return this->m_lattice; }
  virtual QString extractLabel(QString const&) = 0;
  virtual void fillList(std::vector<std::pair<QString, smtk::model::EntityRef> >& l) = 0;
  virtual smtk::model::EntityRef getFromLabel(const QString&) = 0;
  virtual bool IsHexType() = 0;
  virtual void calculateExtraTranslation(double& transX, double& transY) = 0;
  virtual void calculateTranslation(double& transX, double& transY) = 0;
  virtual void setUpdateUsed() = 0;
  virtual void updateLaticeFunction();
  virtual void getRadius(double& ri, double& rj) const;
  virtual double getRadius() const { return -1; }
  double getPitchX() const { return this->m_pitchX; }
  double getPitchY() const { return this->m_pitchY; }
  void setPitch(double x, double y)
  {
    this->m_pitchX = x;
    this->m_pitchY = y;
  }
  std::pair<int, int> GetDimensions();

protected:
  qtLattice m_lattice;
  smtk::model::EntityRef m_entity;
  double m_pitchX;
  double m_pitchY;
};

#endif
