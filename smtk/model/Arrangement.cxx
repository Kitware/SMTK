#include "smtk/model/Arrangement.h"

namespace smtk {
 namespace model {

const char* ArrangementKindName[KINDS_OF_ARRANGEMENTS + 1] = {
  "inclusion",   // INCLUSION
  "cell",        // CELL
  "shell",       // SHELL
  "use",         // USE
  "sense",       // SENSE
  "embedding",   // EMBEDDING
  "invalid"      // KINDS_OF_ARRANGEMENTS
};

const char* ArrangementKindAbbr[KINDS_OF_ARRANGEMENTS + 1] = {
  "i", // INCLUSION
  "c", // CELL
  "s", // SHELL
  "u", // ORIENTATION
  "n", // SENSE
  "e", // EMBEDDING
  "?"  // KINDS_OF_ARRANGEMENTS
};

ArrangementKind ArrangementKindFromName(const std::string& name)
{
  for (int i = 0; i < KINDS_OF_ARRANGEMENTS; ++i)
    {
    if (name == ArrangementKindName[i])
      {
      return ArrangementKind(i);
      }
    }
  return KINDS_OF_ARRANGEMENTS;
}

std::string NameForArrangementKind(ArrangementKind k)
{
  return ArrangementKindName[k];
}

ArrangementKind ArrangementKindFromAbbreviation(const std::string& abbr)
{
  for (int i = 0; i < KINDS_OF_ARRANGEMENTS; ++i)
    {
    if (abbr == ArrangementKindAbbr[i])
      {
      return ArrangementKind(i);
      }
    }
  return KINDS_OF_ARRANGEMENTS;
}

std::string AbbreviationForArrangementKind(ArrangementKind k)
{
  return ArrangementKindAbbr[k];
}

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
  * "CellHasUseWithIndexAndSense" is used to note the order of the relation
  * index and sense in the function call -- this does not necessarily reflect
  * their order in the arrangement).
  */
///@{
/**\brief Construct an arrangement record to add to a cell, indicating a use of that cell.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the USE_ENTITY.
  * The \a sense is an arbitrary integer, but for edges and faces (not vertices),
  * the values of the CellUseSenses enum should be used.
  */
Arrangement Arrangement::CellHasUseWithIndexAndSense(int relationIdx, int sense)
{
  Arrangement result;
  result.details().push_back(relationIdx);
  result.details().push_back(sense);
  return result;
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

/**\brief Construct an arrangement record to add to a cell, indicating its parent entity.
  *
  * The \a relationIdx is the offset in the Entity::relations() array of the CELL_ENTITY.
  *
  * When the parent entity is a topological entity and the cell is dimension d,
  * the parent entity must be of dimension **greater** than d and the cell must
  * be completely interior to its parent.
  * (Example: you may embed a point or edge in a face, but not a face within a face.)
  */
Arrangement Arrangement::CellEmbeddedInEntityWithIndex(int relationIdx)
{
  Arrangement result;
  result.details().push_back(relationIdx);
  return result;
}

/**\brief Construct an arrangement to add to a shell, indicating its parent cell.
  *
  * This relationship indicates that the shell forms part of the boundary of its parent cell.
  * The \a relationIdx is the offset in the Entity::relations() array of the SHELL_ENTITY.
  */
Arrangement Arrangement::ShellHasCellWithIndex(int relationIdx)
{
  Arrangement result;
  result.details().push_back(relationIdx);
  return result;
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
  for (int i = relationBegin; i < relationEnd; ++i)
    {
    result.details().push_back(i);
    }
  return result;
}
///@}

/** @name Methods to interpret arrangements.
  *
  * Use these methods to obtain integers assocated with a semantic meaning
  * from the arrangement vector.
  * If a vector is not sized properly, these methods will return false.
  */
///@{
bool Arrangement::IndexAndSenseFromCellHasUse(int& relationIdx, int& sense)
{
  if (this->m_details.size() != 2)
    {
    return false;
    }
  relationIdx = this->m_details[0];
  sense = this->m_details[1];
  return true;
}

bool Arrangement::IndexAndSenseFromUseHasCell(int& relationIdx, int& sense)
{
  if (this->m_details.size() != 2)
    {
    return false;
    }
  relationIdx = this->m_details[0];
  sense = this->m_details[1];
  return true;
}

bool Arrangement::IndexFromCellEmbeddedInEntity(int& relationIdx)
{
  if (this->m_details.size() != 1)
    {
    return false;
    }
  relationIdx = this->m_details[0];
  return true;
}

bool Arrangement::IndexFromShellHasCell(int& relationIdx)
{
  if (this->m_details.size() != 1)
    {
    return false;
    }
  relationIdx = this->m_details[0];
  return true;
}

bool Arrangement::IndexRangeFromShellHasUse(int& relationBegin, int& relationEnd)
{
  if (this->m_details.size() != 2)
    {
    return false;
    }
  relationBegin = this->m_details[0];
  relationEnd = this->m_details[1];
  return true;
}
///@}

  } //namespace model
} // namespace smtk
