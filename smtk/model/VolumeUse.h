//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_VolumeUse_h
#define __smtk_model_VolumeUse_h

#include "smtk/model/UseEntity.h"

#include <vector>

namespace smtk {
  namespace model {

class Shell;
class VolumeUse;
class Volume;
typedef std::vector<Shell> Shells;
typedef std::vector<VolumeUse> VolumeUses;

/**\brief A entityref subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT VolumeUse : public UseEntity
{
public:
  SMTK_ENTITYREF_CLASS(VolumeUse,UseEntity,isVolumeUse);

  Volume volume() const; // The volume bounded by this face use (if any)
  Shells shells() const; // The toplevel boundary loops for this face (hole-loops not included)
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_VolumeUse_h
