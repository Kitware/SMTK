//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR //  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME rggNucAssembly - Represents an assembly for schema planner.
// Assemblies are composed of pin cells (cmbNucPinCell) and the surrounding ducting.
// Assemblies are grouped together into cores (rggNucCore).
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_rggNucAssembly_h
#define __smtk_bridge_rgg_qt_rggNucAssembly_h

#include "smtk/bridge/rgg/Exports.h"
#include "smtk/bridge/rgg/qt/rggLatticeContainer.h"
#include "smtk/model/EntityRef.h"

// Represents an assembly/core in schema planner.
class SMTKQTRGGSESSION_EXPORT rggNucAssembly : public rggLatticeContainer
{
public:
  rggNucAssembly(smtk::model::EntityRef entity);
  ~rggNucAssembly();
  virtual QString extractLabel(QString const&);
  // Create a list with pair<${name}(${label}, entity>
  virtual void fillList(std::vector<std::pair<QString, smtk::model::EntityRef> >& l);
  virtual smtk::model::EntityRef getFromLabel(const QString& label);
  virtual bool IsHexType();
  virtual void calculateExtraTranslation(double& transX, double& transY);
  virtual void calculateTranslation(double& transX, double& transY);
  virtual void setUpdateUsed();
  virtual void getRadius(double& ri, double& rj) const;

  // This function would use the assy to populate the rggNucAssembly
  void resetBySMTKAssembly(const smtk::model::EntityRef& assy);

  void setAssyDuct(smtk::model::EntityRef duct);
  smtk::model::EntityRef const& getAssyDuct() const;
  smtk::model::EntityRef& getAssyDuct();

  void setZAxisRotation(int value);
  int getZAxisRotation() const;

  void setCenterPins(bool isCentered);
  bool isPinsCentered() const;

protected:
  smtk::model::EntityRef m_duct;
  int m_zAxisRotation;
  bool m_centerPins;
};

#endif
