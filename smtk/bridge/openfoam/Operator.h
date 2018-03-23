//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_openfoam_Operator_h
#define __smtk_session_openfoam_Operator_h

#include "smtk/bridge/openfoam/Exports.h"
#include "smtk/model/Operator.h"

class vtkDataObject;

namespace smtk
{
namespace bridge
{
namespace openfoam
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;

/**\brief An operator using the OpenFOAM "kernel."
  *
  * This is a base class for actual operators.
  * It provides convenience methods for accessing OpenFOAM-specific data
  * for its subclasses to use internally.
  */
class SMTKOPENFOAMSESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  SessionPtr activeSession();
};

} // namespace openfoam
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_openfoam_Operator_h
