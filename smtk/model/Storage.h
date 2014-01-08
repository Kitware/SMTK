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
class InstanceEntity;
class Loop;
class ModelEntity;
class Shell;
class Vertex;
class VertexUse;
class Volume;
class VolumeUse;

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
  int findCellHasUseWithSense(const smtk::util::UUID& cellId, int sense) const;
  std::set<int> findCellHasUsesWithOrientation(const smtk::util::UUID& cellId, Orientation orient) const;

  smtk::util::UUID cellHasUseOfSenseAndOrientation(const smtk::util::UUID& cell, int sense, Orientation o) const;
  smtk::util::UUID findOrCreateCellUseOfSenseAndOrientation(const smtk::util::UUID& cell, int sense, Orientation o);

  smtk::util::UUIDs useOrShellIncludesShells(const smtk::util::UUID& cellUseOrShell) const;
  smtk::util::UUID createIncludedShell(const smtk::util::UUID& cellUseOrShell);

  //bool shellHasUse(const smtk::util::UUID& shell, const smtk::util::UUID& use) const;
  //smtk::util::UUIDs shellHasUses(const smtk::util::UUID& shell) const;
  bool findOrAddUseToShell(const smtk::util::UUID& shell, const smtk::util::UUID& use);

  bool findOrAddInclusionToCell(const smtk::util::UUID& cell, const smtk::util::UUID& inclusion);

  bool hasAttribute(int attribId, const smtk::util::UUID& toEntity);
  bool attachAttribute(int attribId, const smtk::util::UUID& toEntity);
  bool detachAttribute(int attribId, const smtk::util::UUID& fromEntity, bool reverse = true);

  Vertex addVertex();
  Edge addEdge();
  Face addFace();
  Volume addVolume();

  VertexUse addVertexUse();
  VertexUse addVertexUse(const Vertex& src, int sense);
  EdgeUse addEdgeUse();
  EdgeUse addEdgeUse(const Edge& src, int sense, Orientation o);
  FaceUse addFaceUse();
  FaceUse addFaceUse(const Face& src, int sense, Orientation o);
  VolumeUse addVolumeUse();
  VolumeUse addVolumeUse(const Volume& src);

  Chain addChain();
  Chain addChain(const EdgeUse&);
  Chain addChain(const Chain&);
  Loop addLoop();
  Loop addLoop(const FaceUse&);
  Loop addLoop(const Loop&);
  Shell addShell();
  Shell addShell(const Volume& src);
  Shell addShell(const VolumeUse& src);

  GroupEntity addGroup(int extraFlags = 0, const std::string& name = std::string());

  ModelEntity addModel(
    int parametricDim = 3, int embeddingDim = 3, const std::string& name = std::string());

  InstanceEntity addInstance();
  InstanceEntity addInstance(const Cursor& instanceOf);

protected:
  shared_ptr<UUIDsToArrangements> m_arrangements;
  shared_ptr<UUIDsToTessellations> m_tessellations;
  shared_ptr<UUIDsToAttributeAssignments> m_attributeAssignments;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Storage_h
