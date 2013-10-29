#ifndef __smtk_model_ModelBody_h
#define __smtk_model_ModelBody_h

#include "smtk/model/Arrangement.h"
#include "smtk/model/BRepModel.h"
#include "smtk/model/Link.h"
#include "smtk/model/Tessellation.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT ModelBody : public BRepModel
{
public:
  typedef UUIDsToTessellations::iterator tess_iter_type;

  ModelBody();
  ModelBody(UUIDsToLinks* topology, UUIDsToArrangements* arrangements, UUIDsToTessellations* geometry, bool shouldDelete = false);
  ~ModelBody();

  UUIDsToArrangements& arrangements();
  const UUIDsToArrangements& arrangements() const;

  UUIDsToTessellations& tessellations();
  const UUIDsToTessellations& tessellations() const;

  tess_iter_type setTessellation(const smtk::util::UUID& cellId, const Tessellation& geom);

  int arrangeLink(const smtk::util::UUID& cellId, ArrangementKind, const Arrangement& arr, int index = -1);
  const Arrangement* findArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index) const;
  Arrangement* findArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index);

protected:
  UUIDsToArrangements* m_relationships;
  UUIDsToTessellations* m_geometry;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_ModelBody_h
