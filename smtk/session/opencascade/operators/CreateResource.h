//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_CreateResource_h
#define smtk_session_opencascade_CreateResource_h

#include "smtk/common/UUID.h"
#include "smtk/graph/Component.h"
#include "smtk/session/opencascade/Operation.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;
class Shape;

class SMTKOPENCASCADESESSION_EXPORT CreateResource : public Operation
{
public:
  smtkTypeMacro(smtk::session::opencascade::CreateResource);
  smtkCreateMacro(CreateResource);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_CreateResource_h
