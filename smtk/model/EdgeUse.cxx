#include "smtk/model/EdgeUse.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Loop.h"

namespace smtk {
  namespace model {

/**\brief The loop of the face associated with this edge use (if any) or an invalid entity.
  *
  */
Loop EdgeUse::loop() const
{
  return this->relationFromArrangement(HAS_SHELL, 0, 0).as<Loop>();
}

  } // namespace model
} // namespace smtk
