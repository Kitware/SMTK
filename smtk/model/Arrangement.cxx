#include "smtk/model/Arrangement.h"

namespace smtk {
 namespace model {

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

/// Create a record for an entity indicating that is serves as a prototype for an instance.
Arrangement Arrangement::EntityInstancedByWithIndex(int relationIdx)
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

/// Obtain the index of an included entity from a cell EMBEDDED_IN arrangement.
bool Arrangement::IndexFromCellEmbeddedInEntity(int& relationIdx) const
{
  return this->IndexFromSimple(relationIdx);
}

/// Obtain the index of an included entity from a cell INCLUDES arrangement.
bool Arrangement::IndexFromCellIncludesEntity(int& relationIdx) const
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

  } //namespace model
} // namespace smtk
