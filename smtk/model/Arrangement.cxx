//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Arrangement.h"

namespace smtk
{
namespace model
{

/** @name Methods to construct arrangement records.
  *
  * These should be preferred to manual construction of arrangements so that
  * conventions can be changed in the future.
  *
  * Each method starts with the name of entity type the arrangement will
  * be associated with (e.g., an arrangement that specifies how a relation
  * on a cell is related to that cell starts with "Cell").
  * This is followed by the name of the ArrangementKind enumerant
  * describing the relationship (e.g., HAS_USE becomes "HasUse").
  * Finally, the parameters are called out as part of the method name
  * where appropriate to prevent misinterpretation when calling (e.g.,
  * "CellHasUseWithIndexSenseAndOrientation" is used to note the order of the relation
  * index and sense in the function call -- this does not necessarily reflect
  * their order in the arrangement).
  */
///@{

/**\brief Construct the "parent" end of a bidirectional arrangement of kind \a k.
  *
  * Note that \a sense and \a orientation may be ignored depending on \a k;
  * in these cases, you may pass any value.
  */
Arrangement Arrangement::Construct(
  EntityTypeBits t, ArrangementKind k, int relationIdx, int sense, Orientation orientation)
{
  // Do not construct an arrangement if its inverse is invalid:
  if (Dual(t, k) == KINDS_OF_ARRANGEMENTS)
    return Arrangement(); // INVALID!

  // We use these variable names as aliases to make the code more legible:
  int childIdx = relationIdx;
  int parentIdx = relationIdx;

  switch (k)
  {
    /* ===    PARENT-CHILD RELATIONSHIPS === */
    case INCLUDES:
      if (t & CELL_ENTITY)
        return Arrangement::CellIncludesEntityWithIndex(childIdx);
      else if (t & SHELL_ENTITY || t & USE_ENTITY)
        return Arrangement::UseOrShellIncludesShellWithIndex(childIdx);
      else if (t & MODEL_ENTITY || t & GROUP_ENTITY)
        return Arrangement::SimpleIndex(childIdx);
      break;
    case HAS_USE:
      if (t & CELL_ENTITY)
        return Arrangement::CellHasUseWithIndexSenseAndOrientation(childIdx, sense, orientation);
      else if (t & SHELL_ENTITY)
        return Arrangement::ShellHasUseWithIndexRange(parentIdx, parentIdx + 1);
      break;
    case HAS_SHELL:
      if (t & CELL_ENTITY)
        return Arrangement::CellHasShellWithIndex(childIdx);
      else if (t & USE_ENTITY)
        return Arrangement::UseHasShellWithIndex(parentIdx);
      break;
    case INSTANCED_BY:
      return Arrangement::EntityInstancedByWithIndex(childIdx);
      break;
    case SUPERSET_OF:
      return Arrangement::EntitySupersetOfWithIndex(childIdx);
      break;

    /* ===    CHILD-PARENT RELATIONSHIPS === */
    case EMBEDDED_IN:
      if (t & CELL_ENTITY)
        return Arrangement::CellEmbeddedInEntityWithIndex(parentIdx);
      else if (t & SHELL_ENTITY)
        return Arrangement::ShellEmbeddedInUseOrShellWithIndex(parentIdx);
      break;
    case HAS_CELL:
      if (t & USE_ENTITY)
        return Arrangement::UseHasCellWithIndexAndSense(parentIdx, sense);
      else if (t & SHELL_ENTITY)
        return Arrangement::ShellHasCellWithIndex(parentIdx);
      break;
    //case HAS_SHELL: // See parent-child relations above
    //case HAS_USE: // See parent-child relations above
    case INSTANCE_OF:
      if (t & INSTANCE_ENTITY)
        return Arrangement::InstanceInstanceOfWithIndex(parentIdx);
      break;
    case SUBSET_OF:
      return Arrangement::EntitySubsetOfWithIndex(parentIdx);
      break;
    default:
      break;
  }

  return Arrangement(); // INVALID!
}

/**\brief Construct an arrangement record to add to a cell, indicating a use of that cell.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the USE_ENTITY.
  * The \a sense is an arbitrary integer, but for faces should be either 0 (when \a orient
  * is NEGATIVE) or 1 (when \a orient is POSITIVE).
  * the values of the enum should be used.
  */
Arrangement Arrangement::CellHasUseWithIndexSenseAndOrientation(
  int relationIdx, int sense, Orientation orient)
{
  Arrangement result;
  result.details().push_back(relationIdx);
  result.details().push_back(sense);
  result.details().push_back(orient);
  return result;
}

/**\brief Construct an arrangement record to add to a cell, indicating its parent entity.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the CELL_ENTITY.
  *
  * When the parent entity is a topological entity and the cell is dimension d,
  * the parent entity must be of dimension greater than or equal to d and the cell must
  * be completely interior to its parent.
  * (Example: you may embed a point, edge, or face in a face, but not a volume within a face.)
  */
Arrangement Arrangement::CellEmbeddedInEntityWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/**\brief Construct an arrangement record to add to a cell, indicating that it contains an entity.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the CELL_ENTITY.
  *
  * When the entity to be included is a topological entity and the cell is dimension d,
  * the included entity must be of dimension than or equal to d and must be
  * completely geometrically interior to the cell.
  * (Example: a face may include a point or edge or face, but not a volume.)
  */
Arrangement Arrangement::CellIncludesEntityWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/**\brief Construct an arrangement record to add to a cell, indicating a boundary shell.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the CELL_ENTITY.
  * Only toplevel shells (not contained inside other shells of this cell) should be added
  * directly to the cell. Other shells should be added to the *shell* entities which
  * contain them.
  * In this way, the shells form a tree.
  */
Arrangement Arrangement::CellHasShellWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/**\brief Construct an arrangement record to add to a cell-use, indicating its parent cell.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the CELL_ENTITY.
  * The \a sense is an arbitrary integer, but for edges and faces (not vertices),
  * the values of the CellUseSenses enum should be used.
  */
Arrangement Arrangement::UseHasCellWithIndexAndSense(int relationIdx, int sense)
{
  Arrangement result;
  result.details().push_back(relationIdx);
  result.details().push_back(sense);
  return result;
}

/**\brief Construct an arrangement record to add to a cell-use, indicating its parent shell.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of a single parent SHELL_ENTITY.
  * The *parent* shell must span a dimension higher than the cell-use.
  * A cell-use may also have an EMBEDDED_IN arrangement to indicate child shell(s).
  */
Arrangement Arrangement::UseHasShellWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/**\brief Construct an arrangement record to add to a cell-use, indicating a child shell.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the SHELL_ENTITY.
  */
Arrangement Arrangement::UseOrShellIncludesShellWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/**\brief Construct an arrangement to add to a shell, indicating its parent cell.
  *
  * This relationship indicates that the shell forms part of the boundary of its parent cell.
  * The \a relationIdx is the offset in the Entity::relations() array of the SHELL_ENTITY.
  */
Arrangement Arrangement::ShellHasCellWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}
/**\brief Construct an arrangement to add to a shell, indicating the uses that compose it.
  *
  * The \a relationStart and \a relationEnd specify a range of offsets in the Entity::relations()
  * array of the SHELL_ENTITY.
  * The range is half-open: \a relationBegin is included but \a relationEnd is not.
  */
Arrangement Arrangement::ShellHasUseWithIndexRange(int relationBegin, int relationEnd)
{
  Arrangement result;
  result.details().push_back(relationBegin);
  result.details().push_back(relationEnd);
  return result;
}

/// Create a record for a shell indicating the entity it is embedded in.
Arrangement Arrangement::ShellEmbeddedInUseOrShellWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/// Create a record for an instance indicating the prototype entity it instantiates.
Arrangement Arrangement::InstanceInstanceOfWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/// Create a record for an entity indicating that it serves as a prototype for an instance.
Arrangement Arrangement::EntityInstancedByWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/// Create a record for an entity indicating that it groups other entities beneath it (as children).
Arrangement Arrangement::EntitySupersetOfWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

/// Create a record for an entity indicating that is a group member of another (its parent).
Arrangement Arrangement::EntitySubsetOfWithIndex(int relationIdx)
{
  return Arrangement::SimpleIndex(relationIdx);
}

///@}

/** @name Methods to interpret arrangements.
  *
  * Use these methods to obtain integers assocated with a semantic meaning
  * from the arrangement vector.
  * If a vector is not sized properly, these methods will return false.
  */
///@{
/**\brief Obtain the index (\a relationIdx), \a sense, and
  *       orientation (\a orient) from a cell's HAS_USE arrangement.
  */
bool Arrangement::IndexSenseAndOrientationFromCellHasUse(
  int& relationIdx, int& sense, Orientation& orient) const
{
  if (this->m_details.size() != 3)
  {
    return false;
  }
  relationIdx = this->m_details[0];
  sense = this->m_details[1];
  orient = static_cast<Orientation>(this->m_details[2]);
  return true;
}

/// Obtain the index of the including entity from a cell EMBEDDED_IN arrangement.
bool Arrangement::IndexFromCellEmbeddedInEntity(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of an embedded entity from a cell INCLUDES arrangement.
bool Arrangement::IndexFromCellIncludesEntity(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of the including entity from an auxiliary geometry EMBEDDED_IN arrangement.
bool Arrangement::IndexFromAuxiliaryGeometryEmbeddedInEntity(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of an embedded entity from an auxiliary geometry INCLUDES arrangement.
bool Arrangement::IndexFromAuxiliaryGeometryIncludesEntity(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of a shell entity from a cell's HAS_SHELL arrangement.
bool Arrangement::IndexFromCellHasShell(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index and sense of a cell entity from a cell-use's HAS_CELL arrangement.
bool Arrangement::IndexAndSenseFromUseHasCell(int& relationIdx, int& sense) const
{
  if (this->m_details.size() != 2)
  {
    return false;
  }
  relationIdx = this->m_details[0];
  sense = this->m_details[1];
  return true;
}

/// Obtain the index of a shell containing this cell-use.
bool Arrangement::IndexFromUseHasShell(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of a child shell included in this use or shell.
bool Arrangement::IndexFromUseOrShellIncludesShell(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

bool Arrangement::IndexFromShellHasCell(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

bool Arrangement::IndexRangeFromShellHasUse(int& relationBegin, int& relationEnd) const
{
  if (this->m_details.size() != 2)
  {
    return false;
  }
  relationBegin = this->m_details[0];
  relationEnd = this->m_details[1];
  return true;
}

/// Obtain the index of the shell or cell-use in which this shell is embedded.
bool Arrangement::IndexFromShellEmbeddedInUseOrShell(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of the prototype of which this entity is an instance.
bool Arrangement::IndexFromInstanceInstanceOf(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of the instance for which this entity is a prototype.
bool Arrangement::IndexFromEntityInstancedBy(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of an entity contained within this entity.
bool Arrangement::IndexFromEntitySupersetOf(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of the entity containing this one as its child.
bool Arrangement::IndexFromEntitySubsetOf(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Create an arrangement holding the index of a single entity ID (a simple arrangement).
Arrangement Arrangement::SimpleIndex(int relationIdx)
{
  Arrangement result;
  result.details().push_back(relationIdx);
  return result;
}

/// Return the index of a related entity from an arrangement holding only this single index.
bool Arrangement::IndexFromSimple(int& relationIdx) const
{
  if (this->m_details.size() != 1)
  {
    return false;
  }
  relationIdx = this->m_details[0];
  return true;
}
///@}

/**\brief Return the UUIDs of any related entities referenced by this arrangement.
  *
  * You must provide the smtk::model::Entity record \a ent and
  * smtk::model::ArrangementKind \a k that define this arrangement.
  * These provide the context in which the arrangement information
  * should be interpreted: \a ent provides the type of entity on
  * which the arrangement is defined, \a k defines the type of
  * relationship, and \a ent then provides the list of UUIDs which
  * some entry in this arrangement references.
  *
  * If this method returns true, then \a relsOut will have the
  * relevant UUID(s) appended. Otherwise, \a relsOut will be unchanged.
  * Note that this method does not clear \a relsOut so that you may
  * accumulate relations from multiple arrangements into a single array
  * for later processing.
  *
  * This method and smtk::model::Manager::findDualArrangements() are
  * the two main methods which determine how arrangements should be
  * interpreted in context without any prior constraints on the
  * context. (Other methods create and interpret arrangements in
  * specific circumstances where the context is known.)
  */
bool Arrangement::relations(
  smtk::common::UUIDArray& relsOut, const Entity* ent, ArrangementKind k) const
{
  if (!ent)
    return false;
  switch (ent->entityFlags() & ENTITY_MASK)
  {
    case CELL_ENTITY:
      switch (k)
      {
        case HAS_USE:
          return CellHasUseRelationHelper()(relsOut, ent, *this);

        case EMBEDDED_IN:
          return CellEmbeddedInEntityRelationHelper()(relsOut, ent, *this);
        case INCLUDES:
          return CellIncludesEntityRelationHelper()(relsOut, ent, *this);

        case HAS_SHELL:
          return CellHasShellRelationHelper()(relsOut, ent, *this);

        // Should cells be allowed to be supersets (i.e., are they groups in addition to geometric cells?)
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(relsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(relsOut, ent, *this);

        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    case USE_ENTITY:
      switch (k)
      {
        case HAS_CELL:
          return UseHasCellRelationHelper()(relsOut, ent, *this);
        case HAS_SHELL:
          return UseHasShellRelationHelper()(relsOut, ent, *this);
        case INCLUDES:
          return UseOrShellIncludesShellRelationHelper()(relsOut, ent, *this);
        // FIXME: Should use-records be allowed to be prototypes for instances?
        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    case SHELL_ENTITY:
      switch (k)
      {
        case EMBEDDED_IN:
          return ShellEmbeddedInUseOrShellRelationHelper()(relsOut, ent, *this);
        case INCLUDES:
          return UseOrShellIncludesShellRelationHelper()(relsOut, ent, *this);

        case HAS_CELL:
          return ShellHasCellRelationHelper()(relsOut, ent, *this);

        case HAS_USE:
          return ShellHasUseRelationHelper()(relsOut, ent, *this);

        // FIXME: Should shells be allowed to be prototypes for instances?
        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    case GROUP_ENTITY:
      switch (k)
      {
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(relsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(relsOut, ent, *this);

        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    case MODEL_ENTITY:
      switch (k)
      {
        case EMBEDDED_IN:
          return CellEmbeddedInEntityRelationHelper()(relsOut, ent, *this);
        case INCLUDES:
          return CellIncludesEntityRelationHelper()(relsOut, ent, *this);

        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(relsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(relsOut, ent, *this);

        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    case AUX_GEOM_ENTITY:
      switch (k)
      {
        case EMBEDDED_IN:
          return AuxiliaryGeometryEmbeddedInEntityRelationHelper()(relsOut, ent, *this);
        case INCLUDES:
          return AuxiliaryGeometryIncludesEntityRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    case INSTANCE_ENTITY:
      switch (k)
      {
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(relsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(relsOut, ent, *this);

        // Note that we do not allow an instance to be a prototype for another
        // instance... for now. Infinite recursion could result and would be hard
        // to detect.
        case INSTANCE_OF:
          return InstanceInstanceOfRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    case SESSION:
      switch (k)
      {
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(relsOut, ent, *this);
        default:
          break;
      }
      break;
    default:
      break;
  }
  std::cerr << "Unknown relationship context:"
            << " arrangement of kind " << NameForArrangementKind(k) << " for entity of type "
            << ent->flagSummary() << "\n";
  return false;
}

/**\brief Return the indices (into the entity's relations) of any entities referenced by this arrangement.
  *
  * You must provide the smtk::model::Entity record \a ent and
  * smtk::model::ArrangementKind \a k that define this arrangement.
  * These provide the context in which the arrangement information
  * should be interpreted: \a ent provides the type of entity on
  * which the arrangement is defined, \a k defines the type of
  * relationship, and \a ent then provides the list of UUIDs which
  * some entry in this arrangement references.
  *
  * If this method returns true, then \a idxsOut will have the
  * relevant UUID(s) appended. Otherwise, \a idxsOut will be unchanged.
  * Note that this method does not clear \a idxsOut so that you may
  * accumulate relations from multiple arrangements into a single array
  * for later processing.
  */
bool Arrangement::relationIndices(
  std::vector<int>& idxsOut, const Entity* ent, ArrangementKind k) const
{
  if (!ent)
    return false;
  switch (ent->entityFlags() & ENTITY_MASK)
  {
    case CELL_ENTITY:
      switch (k)
      {
        case HAS_USE:
          return CellHasUseRelationHelper()(idxsOut, ent, *this);

        case EMBEDDED_IN:
          return CellEmbeddedInEntityRelationHelper()(idxsOut, ent, *this);
        case INCLUDES:
          return CellIncludesEntityRelationHelper()(idxsOut, ent, *this);

        case HAS_SHELL:
          return CellHasShellRelationHelper()(idxsOut, ent, *this);

        // Should cells be allowed to be supersets (i.e., are they groups in addition to geometric cells?)
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(idxsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(idxsOut, ent, *this);

        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(idxsOut, ent, *this);
        default:
          break;
      }
      break;
    case USE_ENTITY:
      switch (k)
      {
        case HAS_CELL:
          return UseHasCellRelationHelper()(idxsOut, ent, *this);
        case HAS_SHELL:
          return UseHasShellRelationHelper()(idxsOut, ent, *this);
        case INCLUDES:
          return UseOrShellIncludesShellRelationHelper()(idxsOut, ent, *this);
        // FIXME: Should use-records be allowed to be prototypes for instances?
        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(idxsOut, ent, *this);
        default:
          break;
      }
      break;
    case SHELL_ENTITY:
      switch (k)
      {
        case EMBEDDED_IN:
          return ShellEmbeddedInUseOrShellRelationHelper()(idxsOut, ent, *this);
        case INCLUDES:
          return UseOrShellIncludesShellRelationHelper()(idxsOut, ent, *this);

        case HAS_CELL:
          return ShellHasCellRelationHelper()(idxsOut, ent, *this);

        case HAS_USE:
          return ShellHasUseRelationHelper()(idxsOut, ent, *this);

        // FIXME: Should shells be allowed to be prototypes for instances?
        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(idxsOut, ent, *this);
        default:
          break;
      }
      break;
    case GROUP_ENTITY:
      switch (k)
      {
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(idxsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(idxsOut, ent, *this);

        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(idxsOut, ent, *this);
        default:
          break;
      }
      break;
    case MODEL_ENTITY:
      switch (k)
      {
        case EMBEDDED_IN:
          return CellEmbeddedInEntityRelationHelper()(idxsOut, ent, *this);
        case INCLUDES:
          return CellIncludesEntityRelationHelper()(idxsOut, ent, *this);

        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(idxsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(idxsOut, ent, *this);

        case INSTANCED_BY:
          return InstanceInstancedByRelationHelper()(idxsOut, ent, *this);
        default:
          break;
      }
      break;
    case INSTANCE_ENTITY:
      switch (k)
      {
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(idxsOut, ent, *this);
        case SUBSET_OF:
          return EntitySubsetOfRelationHelper()(idxsOut, ent, *this);

        // Note that we do not allow an instance to be a prototype for another
        // instance... for now. Infinite recursion could result and would be hard
        // to detect.
        case INSTANCE_OF:
          return InstanceInstanceOfRelationHelper()(idxsOut, ent, *this);
        default:
          break;
      }
      break;
    case SESSION:
      switch (k)
      {
        case SUPERSET_OF:
          return EntitySupersetOfRelationHelper()(idxsOut, ent, *this);
        default:
          break;
      }
      break;
    default:
      break;
  }
  std::cerr << "Unknown relationship context:"
            << " arrangement of kind " << NameForArrangementKind(k) << " for entity of type "
            << ent->flagSummary() << "\n";
  return false;
}

} //namespace model
} // namespace smtk
