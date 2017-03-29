//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ShellEntity_txx
#define __smtk_model_ShellEntity_txx

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/ShellEntity.h"

namespace smtk {
  namespace model {


/**\brief Return the cells of all the uses composing this shell.
  */
template<typename T>
T ShellEntity::cellsOfUses() const
{
  T result;
  UseEntities useRecs = this->uses<UseEntities>();
  for (UseEntities::const_iterator it = useRecs.begin(); it != useRecs.end(); ++it)
    {
    result.insert(result.end(), it->cell());
    }
  return result;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ShellEntity_txx
