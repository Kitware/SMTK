//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Compound_h
#define smtk_session_opencascade_Compound_h
/*!\file */

#include "smtk/graph/Component.h"

#include "smtk/resource/Properties.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;

class SMTKOPENCASCADESESSION_EXPORT Compound : public Shape
{
public:
  smtkTypeMacro(Compound);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);
  Compound(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_Compound_h
