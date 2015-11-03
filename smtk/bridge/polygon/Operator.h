//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_polygon_Operator_h
#define __smtk_session_polygon_Operator_h

#include "smtk/bridge/polygon/Exports.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Manager.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

class Session;

/**\brief An operator using the polygon kernel.
  *
  * This is a base class for actual polygon operators.
  * It provides convenience methods for accessing polygon-specific data
  * for its subclasses to use internally.
  */
class SMTKPOLYGONSESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  Session* polygonSession();
  /*
  internal::Entity* polygonEntity(const smtk::model::EntityRef& smtkEntity);

  template<typename T>
  T polygonEntityAs(const smtk::model::EntityRef& smtkEntity);
  */
};

/*
/// A convenience method for returning the polygon counterpart of an SMTK entity already cast to a subtype.
template<typename T>
T Operator::polygonEntityAs(const smtk::model::EntityRef& smtkEntity)
{
  return dynamic_cast<T>(this->polygonEntity(smtkEntity));
}
*/

    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Operator_h
