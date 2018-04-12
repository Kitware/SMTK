//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Entity_h
#define __smtk_model_Entity_h

#include "smtk/CoreExports.h" // for SMTKCORE_EXPORT macro
#include "smtk/SystemConfig.h"

#include "smtk/resource/Component.h"

#include "smtk/model/Arrangement.h"    // for Arrangement, ArrangementKind
#include "smtk/model/EntityTypeBits.h" // for entityFlags values
#include "smtk/model/IntegerData.h"    // for IntegerList
#include "smtk/model/StringData.h"     // for StringList

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace smtk
{
namespace model
{

/**\brief A solid model entity, defined by a type and relations to other entities.
  *
  * A solid model is an smtk::model::Manager instance that maps UUIDs to
  * to records of various types. Every entity (topological cell, geometric,
  * group, submodel, or scene-graph instance) must have an Entity record
  * describing the type of the entity and relating it to other entities.
  * Other records, such as string, integer, or vector properties are optional.
  *
  * The entity type is stored as a bit vector named \a entityFlags().
  * This also encodes the parametric dimension (or dimensions) associated
  * with the entity.
  */
class SMTKCORE_EXPORT Entity : public smtk::resource::Component
{
  friend class smtk::model::Manager;

public:
  using UUID = smtk::common::UUID;
  using ResourcePtr = smtk::resource::ResourcePtr;

  smtkTypeMacro(Entity);
  smtkSharedPtrCreateMacro(smtk::resource::Component);
  virtual ~Entity();

  static EntityPtr create(
    const UUID& uid, BitFlags entityFlags = EntityTypeBits::INVALID, ManagerPtr resource = nullptr);
  static EntityPtr create(BitFlags entityFlags, int dimension, ManagerPtr resource = nullptr);
  EntityPtr setup(
    BitFlags entityFlags, int dimension, ManagerPtr resource = nullptr, bool resetRelations = true);

  const ResourcePtr resource() const override;
  ManagerPtr modelResource() const;

  /// Return the templated object (usually EntityRef or a subclass) that points to this component.
  template <typename T>
  T referenceAs() const
  {
    return T(this->modelResource(), this->id());
  }

  bool reparent(ManagerPtr newParent);

  int dimension() const;
  BitFlags dimensionBits() const;
  BitFlags entityFlags() const;
  bool setEntityFlags(BitFlags flags);

  smtk::common::UUIDArray& relations();
  const smtk::common::UUIDArray& relations() const;

  int appendRelation(const smtk::common::UUID& b, bool useHoles = true);
  EntityPtr pushRelation(const smtk::common::UUID& b);
  EntityPtr removeRelation(const smtk::common::UUID& b);
  void resetRelations();

  int findOrAppendRelation(const smtk::common::UUID& r);
  int invalidateRelation(const smtk::common::UUID& r);
  int invalidateRelationByIndex(int relIdx);

  std::string flagSummary(int form = 0) const
  {
    return Entity::flagSummary(this->entityFlags(), form);
  }
  std::string flagDescription(int form = 0) const
  {
    return Entity::flagDescription(this->entityFlags(), form);
  }

  static std::string flagDimensionList(BitFlags entityFlags, bool& plural);
  static std::string flagSummaryHelper(BitFlags entityFlags, int form = 0);
  static std::string flagSummary(BitFlags entityFlags, int form = 0);
  static std::string flagDescription(BitFlags entityFlags, int form = 0);
  static int countForType(BitFlags flags, IntegerList& counters, bool incr = false);
  static std::string defaultNameFromCounters(
    BitFlags entityFlags, IntegerList& counters, bool incr = true);
  static std::string flagToSpecifierString(BitFlags flagsOrMask, bool textual = true);
  static BitFlags specifierStringToFlag(const std::string& spec);
  static BitFlags dimensionToDimensionBits(int dim);
  static int dimensionBitsToDimension(BitFlags dimBits);

  int arrange(ArrangementKind, const Arrangement& arr, int index = -1);
  int unarrange(ArrangementKind, int index, bool removeIfLast = false);
  bool clearArrangements();

  const Arrangements* hasArrangementsOfKind(ArrangementKind) const;
  Arrangements* hasArrangementsOfKind(ArrangementKind);

  Arrangements& arrangementsOfKind(ArrangementKind);

  const Arrangement* findArrangement(ArrangementKind kind, int index) const;
  Arrangement* findArrangement(ArrangementKind kind, int index);
  int findArrangementInvolvingEntity(ArrangementKind k, const smtk::common::UUID& involved) const;
  bool findDualArrangements(ArrangementKind kind, int index, ArrangementReferences& duals) const;

  const KindsToArrangements& arrangementMap() const { return m_arrangements; }

  const common::UUID& id() const override { return m_id; }
  bool setId(const common::UUID& uid) override
  {
    m_id = uid;
    return true;
  }

  bool isCellEntity() const { return smtk::model::isCellEntity(this->entityFlags()); }
  bool isUseEntity() const { return smtk::model::isUseEntity(this->entityFlags()); }
  bool isShellEntity() const { return smtk::model::isShellEntity(this->entityFlags()); }
  bool isGroup() const { return smtk::model::isGroup(this->entityFlags()); }
  bool isModel() const { return smtk::model::isModel(this->entityFlags()); }
  bool isInstance() const { return smtk::model::isInstance(this->entityFlags()); }
  bool isSessionRef() const { return smtk::model::isSessionRef(this->entityFlags()); }
  bool isAuxiliaryGeometry() const { return smtk::model::isAuxiliaryGeometry(this->entityFlags()); }
  bool isConcept() const { return smtk::model::isConcept(this->entityFlags()); }

  bool isVertex() const { return smtk::model::isVertex(this->entityFlags()); }
  bool isEdge() const { return smtk::model::isEdge(this->entityFlags()); }
  bool isFace() const { return smtk::model::isFace(this->entityFlags()); }
  bool isVolume() const { return smtk::model::isVolume(this->entityFlags()); }
  bool isChain() const { return smtk::model::isChain(this->entityFlags()); }
  bool isLoop() const { return smtk::model::isLoop(this->entityFlags()); }
  bool isShell() const { return smtk::model::isShell(this->entityFlags()); }
  bool isVertexUse() const { return smtk::model::isVertexUse(this->entityFlags()); }
  bool isEdgeUse() const { return smtk::model::isEdgeUse(this->entityFlags()); }
  bool isFaceUse() const { return smtk::model::isFaceUse(this->entityFlags()); }
  bool isVolumeUse() const { return smtk::model::isVolumeUse(this->entityFlags()); }
protected:
  Entity();
  int consumeInvalidIndex(const smtk::common::UUID& uid);

  BitFlags m_entityFlags;
  smtk::common::UUIDArray m_relations;
  smtk::model::WeakManagerPtr m_resource;
  KindsToArrangements m_arrangements;
  int m_firstInvalid;
  smtk::common::UUID m_id;
};

/// An abbreviation for the record type used by maps of Entity records.
typedef std::pair<smtk::common::UUID, Entity> UUIDEntityPair;

} // namespace model
} // namespace smtk

#endif // __smtk_model_Entity_h
