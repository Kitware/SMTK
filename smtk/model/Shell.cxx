//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Shell.h"

#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Volume.h"

namespace smtk
{
namespace model
{

/**\brief Return the high-dimensional cell whose interior is bounded by this shell.
  *
  */
Volume Shell::volume() const
{
  return this->ShellEntity::boundingCell().as<Volume>();
}

/**\brief Return the face-uses composing this shell.
  *
  */
FaceUses Shell::faceUses() const
{
  return this->ShellEntity::uses<FaceUses>();
}

/**\brief Return the parent shell of this shell (or an invalid shell if unbounded).
  *
  */
Shell Shell::containingShell() const
{
  return this->ShellEntity::containingShellEntity().as<Shell>();
}

/**\brief Return the child shells of this shell, if any.
  *
  */
Shells Shell::containedShells() const
{
  return this->ShellEntity::containedShellEntities<Shells>();
}

} // namespace model
} // namespace smtk
