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

class Chain;
class Edge;
class EdgeUse;
class Face;
class FaceUse;
class GroupEntity;
class Loop;
class ModelEntity;
class Shell;
class Vertex;
class VertexUse;
class Volume;

/**\brief Store information about solid models.
  *
  * This adds information about arrangements and tessellations
  * of entities to its BRepModel base class.
  */
class SMTKCORE_EXPORT Storage : public BRepModel
{
public:
  typedef UUIDsToTessellations::iterator tess_iter_type;

  smtkTypeMacro(Storage);
  smtkCreateMacro(Storage);
  smtkSharedFromThisMacro(BRepModel);
  Storage();
  Storage(
    shared_ptr<UUIDsToEntities> topology,
    shared_ptr<UUIDsToArrangements> arrangements,
    shared_ptr<UUIDsToTessellations> tess,
    shared_ptr<UUIDsToAttributeAssignments> attribs);
  virtual ~Storage();

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

  Vertex addVertex();
  Edge addEdge();
  Face addFace();
  Volume addVolume();

  VertexUse addVertexUse();
  EdgeUse addEdgeUse();
  FaceUse addFaceUse();

  Chain addChain();
  Loop addLoop();
  Shell addShell();

  GroupEntity addGroup(int extraFlags = 0, const std::string& name = std::string());

  ModelEntity addModel(
    int parametricDim = 3, int embeddingDim = 3, const std::string& name = std::string());

protected:
  shared_ptr<UUIDsToArrangements> m_arrangements;
  shared_ptr<UUIDsToTessellations> m_tessellations;
  shared_ptr<UUIDsToAttributeAssignments> m_attributeAssignments;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Storage_h
