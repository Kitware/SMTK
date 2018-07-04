//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_vtk_Operation_h
#define __smtk_session_vtk_Operation_h

#include "smtk/bridge/vtk/Exports.h"
#include "smtk/operation/XMLOperation.h"

class vtkDataObject;

namespace smtk
{
namespace bridge
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
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_vtk_Operation_h
