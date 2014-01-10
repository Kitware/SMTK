#include "smtk/model/VolumeUse.h"

#include "smtk/model/Shell.h"
#include "smtk/model/Volume.h"

namespace smtk {
  namespace model {

// The volume bounded by this face use (if any)
Volume VolumeUse::volume() const
{
  return this->relationFromArrangement(HAS_CELL, 0, 0).as<Volume>();
}

// The toplevel boundary shells for this volume (subshells not included)
Shells VolumeUse::shells() const
{
  return this->shellEntities<Shells>();
}

  } // namespace model
} // namespace smtk
