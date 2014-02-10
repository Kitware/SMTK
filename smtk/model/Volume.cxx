#include "smtk/model/Volume.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Face.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Volume.h"

namespace smtk {
  namespace model {

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
  Cursors all = this->bordantEntities(/*dim = */ 2);
  for (Cursors::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isFace())
      result.push_back(*it);
    }
  return result;
}

Volume& Volume::setVolumeUse(const VolumeUse& volUse)
{
  if (volUse.isValid() && this->isValid())
    {
    this->m_storage->findCreateOrReplaceCellUseOfSenseAndOrientation(
      this->m_entity, 0, POSITIVE, volUse.entity());
    }
  return *this;
}

  } // namespace model
} // namespace smtk
