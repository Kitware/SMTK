//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_discrete_Operator_h
#define __smtk_session_discrete_Operator_h

#include "smtk/bridge/discrete/Exports.h"
#include "smtk/bridge/discrete/Session.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

class vtkModelItem;
class vtkModelItemIterator;

namespace smtk
{
namespace bridge
{
namespace discrete
{

class Session;
typedef shared_ptr<Session> SessionPtr;

/**\brief An operator using the discrete kernel.
  *
  * This is a base class for actual discrete operators.
  * It provides convenience methods for accessing discrete-specific data
  * for its subclasses to use internally.
  */
class SMTKDISCRETESESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  SessionPtr discreteSession() const;
  vtkModelItem* discreteEntity(const smtk::model::EntityRef& smtkEntity);

  template <typename T>
  T discreteEntityAs(const smtk::model::EntityRef& smtkEntity);
};

/// A convenience method for returning the discrete counterpart of an SMTK entity already cast to a subtype.
template <typename T>
T Operator::discreteEntityAs(const smtk::model::EntityRef& smtkEntity)
{
  return dynamic_cast<T>(this->discreteEntity(smtkEntity));
}

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_Operator_h
