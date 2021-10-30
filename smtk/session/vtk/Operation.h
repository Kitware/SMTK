//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_vtk_Operation_h
#define smtk_session_vtk_Operation_h

#include "smtk/operation/XMLOperation.h"
#include "smtk/session/vtk/Exports.h"

class vtkDataObject;

namespace smtk
{
namespace session
{
namespace vtk
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;
struct EntityHandle;

/**\brief An operator using the VTK "kernel."
  *
  * This is a base class for actual operators.
  * It provides convenience methods for accessing VTK-specific data
  * for its subclasses to use internally.
  */
class SMTKVTKSESSION_EXPORT Operation : public smtk::operation::XMLOperation
{
};

} // namespace vtk
} // namespace session
} // namespace smtk

#endif // smtk_session_vtk_Operation_h
