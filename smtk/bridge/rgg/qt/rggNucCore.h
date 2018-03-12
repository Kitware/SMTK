//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR //  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME rggNucCore - Represents a core for schema planner.
// Cores are composed of a bunch of assemblies.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_rggNucCore_h
#define __smtk_bridge_rgg_qt_rggNucCore_h

#include "smtk/bridge/rgg/Exports.h"
#include "smtk/bridge/rgg/qt/rggLatticeContainer.h"
#include "smtk/model/EntityRef.h"

// Represents a nuclear core in schema planner.
class SMTKQTRGGSESSION_EXPORT rggNucCore : public rggLatticeContainer
{
public:
  rggNucCore(smtk::model::EntityRef entity);
  ~rggNucCore() override;
  virtual QString extractLabel(QString const&);
  // Create a list with pair<${name}(${label}, entity)>
  virtual void fillList(std::vector<std::pair<QString, smtk::model::EntityRef> >& l);
  virtual smtk::model::EntityRef getFromLabel(const QString& label);
  virtual bool IsHexType();
  virtual void calculateExtraTranslation(double& transX, double& transY);
  virtual void calculateTranslation(double& transX, double& transY);
  virtual void setUpdateUsed();
  virtual void getRadius(double& ri, double& rj) const;

  // This function would use the assy to populate the rggNucCore
  void resetBySMTKCore(const smtk::model::EntityRef& core);

protected:
};

#endif
