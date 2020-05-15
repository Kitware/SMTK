//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_IconConstructor_h
#define smtk_session_opencascade_IconConstructor_h

#include "smtk/session/opencascade/Exports.h"
#include "smtk/view/SVGIconConstructor.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

/**\brief Register icons for opencascade component types.
  *
  */
class SMTKOPENCASCADESESSION_EXPORT IconConstructor : public smtk::view::SVGIconConstructor
{
  std::string svg(const smtk::resource::PersistentObject&) const override;
};
}
}
}

#endif
