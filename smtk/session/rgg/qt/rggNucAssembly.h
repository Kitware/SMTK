//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
// .NAME rggNucAssembly - Represents an assembly for schema planner.
// Assemblies are composed of pin cells (cmbNucPinCell) and the surrounding ducting.
// Assemblies are grouped together into cores (rggNucCore).
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_session_rgg_qt_rggNucAssembly_h
#define __smtk_session_rgg_qt_rggNucAssembly_h

#include "smtk/model/EntityRef.h"
#include "smtk/session/rgg/Exports.h"
#include "smtk/session/rgg/qt/rggLatticeContainer.h"

// Represents a nuclear assembly in schema planner.
class SMTKQTRGGSESSION_EXPORT rggNucAssembly : public rggLatticeContainer
{
public:
  rggNucAssembly(smtk::model::EntityRef entity);
  ~rggNucAssembly() override;
  virtual QString extractLabel(QString const&) override;
  // Create a list with pair<${name}(${label}, entity)>
  virtual void fillList(std::vector<std::pair<QString, smtk::model::EntityRef> >& l) override;
  virtual smtk::model::EntityRef getFromLabel(const QString& label) override;
  virtual bool IsHexType() override;
  virtual void calculateExtraTranslation(double& transX, double& transY) override;
  virtual void calculateTranslation(double& transX, double& transY) override;
  virtual void setUpdateUsed() override;
  virtual void getRadius(double& ri, double& rj) const override;

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
