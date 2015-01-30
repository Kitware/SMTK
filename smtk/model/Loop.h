//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Loop_h
#define __smtk_model_Loop_h

#include "smtk/model/ShellEntity.h"

namespace smtk {
  namespace model {

class Face;
class FaceUse;
class Loop;
class EdgeUse;
typedef std::vector<Loop> Loops;
typedef std::vector<EdgeUse> EdgeUses;

/**\brief A entityref subclass with methods specific to edge-loops.
  *
  * A loop is a collection of oriented edge-uses that form a
  * subset of the boundary of a face cell.
  * A loop may contain other loops.
  */
class SMTKCORE_EXPORT Loop : public ShellEntity
{
public:
  SMTK_ENTITYREF_CLASS(Loop,ShellEntity,isLoop);

  Face face() const;
  FaceUse faceUse() const;
  EdgeUses edgeUses() const;
  Loop containingLoop() const;
  Loops containedLoops() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Loop_h
