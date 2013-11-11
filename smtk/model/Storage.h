#ifndef __smtk_model_Storage_h
#define __smtk_model_Storage_h

#include "smtk/model/Arrangement.h"
#include "smtk/model/BRepModel.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Tessellation.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT Storage : public BRepModel
{
public:
  typedef UUIDsToTessellations::iterator tess_iter_type;

  Storage();
  Storage(UUIDsToEntities* topology, UUIDsToArrangements* arrangements, UUIDsToTessellations* geometry, bool shouldDelete = false);
  ~Storage();

  UUIDsToArrangements& arrangements();
  const UUIDsToArrangements& arrangements() const;

  UUIDsToTessellations& tessellations();
  const UUIDsToTessellations& tessellations() const;

  tess_iter_type setTessellation(const smtk::util::UUID& cellId, const Tessellation& geom);

  int arrangeEntity(const smtk::util::UUID& cellId, ArrangementKind, const Arrangement& arr, int index = -1);
  const Arrangement* findArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index) const;
  Arrangement* findArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index);

protected:
  UUIDsToArrangements* m_relationships;
  UUIDsToTessellations* m_geometry;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Storage_h
