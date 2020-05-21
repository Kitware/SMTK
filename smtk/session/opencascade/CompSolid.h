//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_CompSolid_h
#define smtk_session_opencascade_CompSolid_h
/*!\file */

#include "smtk/graph/Component.h"

#include "smtk/resource/Properties.h"

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;

class SMTKOPENCASCADESESSION_EXPORT CompSolid : public Shape
{
public:
  smtkTypeMacro(CompSolid);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);
  CompSolid(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_CompSolid_h
