//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_exodus_Operator_h
#define __smtk_session_exodus_Operator_h

#include "smtk/bridge/exodus/Exports.h"
#include "smtk/model/Operator.h"

class vtkDataObject;

namespace smtk
{
namespace bridge
{
namespace exodus
{

class Session;
struct EntityHandle;

/**\brief An operator using the Exodus "kernel."
  *
  * This is a base class for actual operators.
  * It provides convenience methods for accessing Exodus-specific data
  * for its subclasses to use internally.
  */
class SMTKEXODUSSESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  Session* exodusSession();
  vtkDataObject* exodusData(const smtk::model::EntityRef& smtkEntity);
  EntityHandle exodusHandle(const smtk::model::EntityRef& smtkEntity);
};

} // namespace exodus
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_exodus_Operator_h
