#ifndef __smtk_model_Storage_h
#define __smtk_model_Storage_h

#include "smtk/model/BRepModel.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Arrangement.h"
#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Tessellation.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

namespace smtk {
  namespace model {

/**\brief Store information about solid models.
  *
  * This adds information about arrangements and tessellations
  * of entities to its BRepModel base class.
  */
class SMTKCORE_EXPORT Storage : public BRepModel
{
public:
  typedef UUIDsToTessellations::iterator tess_iter_type;

  Storage();
  Storage(
    shared_ptr<UUIDsToEntities> topology,
    shared_ptr<UUIDsToArrangements> arrangements,
    shared_ptr<UUIDsToTessellations> tess,
    shared_ptr<UUIDsToAttributeAssignments> attribs);
  virtual ~Storage();

  static smtk::model::StoragePtr New()
    { return smtk::model::StoragePtr(new Storage); }

  UUIDsToArrangements& arrangements();
  const UUIDsToArrangements& arrangements() const;

  UUIDsToTessellations& tessellations();
  const UUIDsToTessellations& tessellations() const;

  UUIDsToAttributeAssignments& attributeAssignments();
  const UUIDsToAttributeAssignments& attributeAssignments() const;

  tess_iter_type setTessellation(const smtk::util::UUID& cellId, const Tessellation& geom);

  int arrangeEntity(const smtk::util::UUID& cellId, ArrangementKind, const Arrangement& arr, int index = -1);

  const Arrangements* hasArrangementsOfKindForEntity(
    const smtk::util::UUID& cellId,
    ArrangementKind) const;
  Arrangements* hasArrangementsOfKindForEntity(
    const smtk::util::UUID& cellId,
    ArrangementKind);

  Arrangements& arrangementsOfKindForEntity(const smtk::util::UUID& cellId, ArrangementKind);

  const Arrangement* findArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index) const;
  Arrangement* findArrangement(const smtk::util::UUID& cellId, ArrangementKind kind, int index);

  smtk::util::UUID cellHasUseOfSense(const smtk::util::UUID& cell, int sense) const;
  smtk::util::UUID findOrCreateCellUseOfSense(const smtk::util::UUID& cell, int sense);

  bool hasAttribute(int attribId, const smtk::util::UUID& toEntity);
  bool attachAttribute(int attribId, const smtk::util::UUID& toEntity);
  bool detachAttribute(int attribId, const smtk::util::UUID& fromEntity, bool reverse = true);

protected:
  shared_ptr<UUIDsToArrangements> m_arrangements;
  shared_ptr<UUIDsToTessellations> m_tessellations;
  shared_ptr<UUIDsToAttributeAssignments> m_attributeAssignments;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Storage_h
