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

#include "smtk/session/opencascade/arcs/ChildrenAs.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

#include "smtk/session/opencascade/CompSolid.h"
#include "smtk/session/opencascade/Compound.h"
#include "smtk/session/opencascade/Edge.h"
#include "smtk/session/opencascade/Face.h"
#include "smtk/session/opencascade/Shape.h"
#include "smtk/session/opencascade/Shell.h"
#include "smtk/session/opencascade/Solid.h"
#include "smtk/session/opencascade/Vertex.h"
#include "smtk/session/opencascade/Wire.h"

#include <tuple>

namespace smtk
{
namespace session
{
namespace opencascade
{

/**\brief Traits that describe OpenCASCADE node and arc types.
  *
  */
struct SMTKOPENCASCADESESSION_EXPORT Traits
{
  typedef std::tuple<Shape, Compound, CompSolid, Solid, Shell, Face, Wire, Edge, Vertex> NodeTypes;
  typedef std::tuple<ChildrenAs<CompSolid>, ChildrenAs<Solid>, ChildrenAs<Shell>, ChildrenAs<Face>,
    ChildrenAs<Wire>, ChildrenAs<Edge>, ChildrenAs<Vertex>, Children, ParentsAs<Compound>,
    ParentsAs<CompSolid>, ParentsAs<Solid>, ParentsAs<Shell>, ParentsAs<Face>, ParentsAs<Wire>,
    ParentsAs<Edge>, Parents>
    ArcTypes;
};
}
}
}

#endif
