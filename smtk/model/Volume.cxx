//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Volume.h"

#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/Face.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Tessellation.h"

namespace smtk
{
namespace model
{

/// Return the volume use associated with this volume (or an invalid use).
VolumeUse Volume::use() const
{
  VolumeUses vus = this->uses<VolumeUses>();
  if (vus.empty())
    return VolumeUse();
  return vus[0];
}

/**\brief Return the top-level shells bounding this volume.
  *
  * Note that only top-level shells are returned; if there are
  * voids in the volume, the top-level shells will contain
  * subshells.
  */
Shells Volume::shells() const
{
  return this->use().shells();
}

smtk::model::Faces Volume::faces() const
{
  Faces result;
  EntityRefs all = this->boundaryEntities(/*ofDimension = */ 2);
  for (EntityRefs::iterator it = all.begin(); it != all.end(); ++it)
  {
    if (it->isFace())
      result.push_back(*it);
  }
  return result;
}

Volume& Volume::setVolumeUse(const VolumeUse& volUse)
{
  ResourcePtr resource = this->resource();
  if (volUse.isValid() && this->isValid())
  {
    resource->findCreateOrReplaceCellUseOfSenseAndOrientation(
      m_entity, 0, POSITIVE, volUse.entity());
  }
  return *this;
}

} // namespace model
} // namespace smtk
