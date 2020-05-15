//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Import_h
#define smtk_session_opencascade_Import_h

#include "smtk/common/UUID.h"
#include "smtk/graph/Component.h"
#include "smtk/operation/XMLOperation.h"
#include "smtk/session/opencascade/Exports.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;
class Shape;

/**\brief Import native OpenCASCADE models as well as STEP and IGES data.
  *
  * As with most import operations, this one allows imports into an existing
  * resource or a new one.
  */
class SMTKOPENCASCADESESSION_EXPORT Import : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::opencascade::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  /// Recursively add topological model entities to the resource.
  void iterateChildren(Shape& parent, Result& result, int& numShapes);
  /// This method does the bulk of the work importing model data.
  Result operateInternal() override;
  /// Return XML describing the operation inputs and outputs as attributes.
  virtual const char* xmlDescription() const override;
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_Import_h
