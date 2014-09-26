//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/FaceUse.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Volume.h"
#include "smtk/model/VolumeUse.h"

namespace smtk {
  namespace model {

/// The volume bounded by this face use (if any).
Volume FaceUse::volume() const
{
  return this->boundingShell().volume();
}

/// The shell in which this face-use participates (if any).
Shell FaceUse::boundingShell() const
{
  return this->boundingShellEntity().as<Shell>();
}

/// The (parent) underlying face of this use.
Face FaceUse::face() const
{
  return this->relationFromArrangement(HAS_CELL, 0, 0).as<Face>();
}

/// The toplevel boundary loops for this face (hole-loops not included).
Loops FaceUse::loops() const
{
  return this->shellEntities<Loops>();
}

  } // namespace model
} // namespace smtk
