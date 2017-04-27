//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/discrete/Operator.h"
#include "smtk/bridge/discrete/Session.h"

#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

/// Return a shared pointer to the session backing a discrete operator.
Session* Operator::discreteSession()
{
  return dynamic_cast<smtk::bridge::discrete::Session*>(this->session());
}

/// A helper to return the CGM entity associated with \a smtkEntity.
vtkModelItem* Operator::discreteEntity(const smtk::model::EntityRef& smtkEntity)
{
  return this->discreteSession()->entityForUUID(smtkEntity.entity());
}

} // namespace discrete
} //namespace bridge
} // namespace smtk
