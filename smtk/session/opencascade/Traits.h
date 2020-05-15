//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Traits_h
#define smtk_session_opencascade_Traits_h
/*!\file */

#include "smtk/session/opencascade/Exports.h"

#include <tuple>

namespace smtk
{
namespace session
{
namespace opencascade
{

class Shape;

/**\brief Traits that describe OpenCASCADE node and arc types.
  *
  */
struct SMTKOPENCASCADESESSION_EXPORT Traits
{
  typedef std::tuple<Shape> NodeTypes;
  typedef std::tuple<> ArcTypes;
};
}
}
}

#endif
