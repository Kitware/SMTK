//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Operator_h
#define __smtk_session_mesh_Operator_h

#include "smtk/bridge/mesh/Exports.h"
#include "smtk/model/Operator.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;
struct EntityHandle;

/**\brief An operator using the Mesh "kernel."
  *
  * This is a base class for actual operators.
  * It provides convenience methods for accessing Mesh-specific data
  * for its subclasses to use internally.
  */
class SMTKMESHSESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  SessionPtr activeSession();
};

} // namespace mesh
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_mesh_Operator_h
