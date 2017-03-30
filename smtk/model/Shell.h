//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Shell_h
#define __smtk_model_Shell_h

#include "smtk/model/ShellEntity.h"

namespace smtk
{
namespace model
{

class Volume;
class Shell;
class FaceUse;
typedef std::vector<Shell> Shells;
typedef std::vector<FaceUse> FaceUses;

/**\brief A entityref subclass with methods specific to face-shells.
  *
  * A shell is a collection of oriented face-uses that form a
  * subset of the boundary of a volume cell.
  * A shell may contain other shells.
  */
class SMTKCORE_EXPORT Shell : public ShellEntity
{
public:
  SMTK_ENTITYREF_CLASS(Shell, ShellEntity, isShell);

  Volume volume() const;
  FaceUses faceUses() const;
  Shell containingShell() const;
  Shells containedShells() const;
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_Shell_h
