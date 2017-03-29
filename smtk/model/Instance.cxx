//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Instance.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/Manager.h"

namespace smtk {
  namespace model {

EntityRef Instance::prototype() const
{
  return EntityRefArrangementOps::firstRelation<EntityRef>(*this, INSTANCE_OF);
}

  } // namespace model
} // namespace smtk
