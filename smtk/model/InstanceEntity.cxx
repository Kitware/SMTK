//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/InstanceEntity.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

Cursor InstanceEntity::prototype() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, INSTANCE_OF);
}

  } // namespace model
} // namespace smtk
