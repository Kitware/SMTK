//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_Operator_h
#define __smtk_session_cgm_Operator_h

#include "smtk/bridge/cgm/Exports.h"
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "DLIList.hpp"

class RefEntity;
class ToolDataUser;

namespace smtk
{
namespace bridge
{
namespace cgm
{

class Session;

/**\brief An operator using the CGM kernel.
  *
  * This is a base class for actual CGM operators.
  * It provides convenience methods for accessing CGM-specific data
  * for its subclasses to use internally.
  */
class SMTKCGMSESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  Session* cgmSession();
  ToolDataUser* cgmData(const smtk::model::EntityRef& smtkEntity);
  RefEntity* cgmEntity(const smtk::model::EntityRef& smtkEntity);

  template <typename T>
  T cgmEntityAs(const smtk::model::EntityRef& smtkEntity);

  template <typename T, typename U>
  bool cgmEntities(const T& smtkContainer, DLIList<U>& cgmContainer, int keepInputs,
    smtk::model::EntityRefArray& expunged);

  template <typename T>
  void addEntitiesToResult(DLIList<T>& cgmContainer, smtk::model::OperatorResult result,
    ResultEntityOrigin origin = UNKNOWN);
};

/// A convenience method for returning the CGM counterpart of an SMTK entity already cast to a subtype.
template <typename T>
T Operator::cgmEntityAs(const smtk::model::EntityRef& smtkEntity)
{
  return dynamic_cast<T>(this->cgmEntity(smtkEntity));
}

/**\brief A helper to return CGM counterparts for all the SMTK entities in a set.
  *
  * This method returns true when it is able to convert each and every entry in
  * \a smtkContainer into a non-NULL entry in \a cgmContainer.
  * If \a keepInputs is 0, then each SMTK entry in \a smtkContainer is erased
  * from the model manager and added to \a expunged.
  * If \a keepInputs is positive, then no entries of \a smtkContainer have their
  * storage in the model manager affected nor are they added to \a expunged.
  * If \a keepInputs is negative, then all but the first entry are removed and
  * added to \a expunged.
  */
template <typename T, typename U>
bool Operator::cgmEntities(const T& smtkContainer, DLIList<U>& cgmContainer, int keepInputs,
  smtk::model::EntityRefArray& expunged)
{
  bool ok = true;
  typename T::const_iterator it;
  U cgmEntity;
  for (it = smtkContainer.begin(); it != smtkContainer.end(); ++it)
  {
    cgmEntity = this->cgmEntityAs<U>(*it);
    if (cgmEntity)
    {
      cgmContainer.append(cgmEntity);
    }
    else
    {
      ok = false;
      smtkInfoMacro(log(), "Could not find CGM counterpart for SMTK entity \""
          << it->name() << "\" (" << it->flagSummary() << ").");
    }

    if (!keepInputs || (keepInputs < 0 && it != smtkContainer.begin()))
    {
      this->manager()->eraseModel(*it);
      expunged.push_back(*it);
    }
  }
  return ok;
}

template <typename T>
void Operator::addEntitiesToResult(
  DLIList<T>& cgmContainer, smtk::model::OperatorResult result, ResultEntityOrigin origin)
{
  Session* session = this->cgmSession();
  int numBodiesOut = cgmContainer.size();

  smtk::model::EntityRefArray creArr;
  smtk::model::EntityRefArray modArr;

  for (int i = 0; i < numBodiesOut; ++i)
  {
    T cgmEnt = cgmContainer.get_and_step();
    if (!cgmEnt)
      continue;

    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmEnt, true);
    smtk::common::UUID entId = refId->entityId();
    bool isNew = (origin == CREATED
        ? true
        : (origin == MODIFIED ? false
                              : (this->manager()->findEntity(entId, false) ? false : true)));
    smtk::model::EntityRef smtkEntry(this->manager(), entId);
    if (session->transcribe(smtkEntry, smtk::model::SESSION_EVERYTHING, false))
    {
      if (isNew)
        creArr.push_back(smtkEntry);
      else
        modArr.push_back(smtkEntry);
    }
  }
  smtk::attribute::ModelEntityItem::Ptr entCreOut = result->findModelEntity("created");
  smtk::attribute::ModelEntityItem::Ptr entModOut = result->findModelEntity("modified");

  entModOut->setValues(modArr.begin(), modArr.end());
  entCreOut->setValues(creArr.begin(), creArr.end());
}

} // namespace cgm
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_Operator_h
