#ifndef __smtk_model_ModelBody_h
#define __smtk_model_ModelBody_h
#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

#include "smtk/model/Item.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/BRepModel.h"
#include "smtk/model/Link.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT ModelBody : public BRepModel<smtk::util::UUID,UUIDs,Link>
{
public:
  typedef UUIDsToTessellations::iterator geom_iter_type;

  ModelBody();
  ModelBody(UUIDsToLinks* topology, UUIDsToArrangements* arrangements, UUIDsToTessellations* geometry, bool shouldDelete = false);
  ~ModelBody();

  UUIDsToArrangements& arrangements();
  const UUIDsToArrangements& arrangements() const;

  UUIDsToTessellations& tessellations();
  const UUIDsToTessellations& tessellations() const;

  geom_iter_type SetTessellation(const smtk::util::UUID& cellId, const Tessellation& geom);

  int ArrangeLink(const smtk::util::UUID& cellId, ArrangementKind, const Arrangement& arr, int index = -1);
  const Arrangement* GetArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index) const;
  Arrangement* GetArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index);

protected:
  UUIDsToArrangements* Relationships;
  UUIDsToTessellations* Geometry;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_ModelBody_h
