//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/EntityPhrase.h"

#include "smtk/model/PropertyListPhrase.h"

namespace smtk {
  namespace model {

EntityListPhrase::EntityListPhrase()
  : m_commonFlags(INVALID), m_unionFlags(0)
{
}

/// Show the entity name (or a default name) in the title
std::string EntityListPhrase::title()
{
  std::ostringstream message;
  EntityRefArray::size_type sz = this->m_entities.size();
  message << sz << " ";

  this->buildSubphrases(); // This sets the m_{common,union}Flags members.
  // Now determine whether all the entityrefs share a common type or dimension.
  if (this->m_commonFlags == this->m_unionFlags)
    { // All the entityrefs have exactly the same flags set.
    message << Entity::flagSummary(this->m_commonFlags, sz == 1 ? 0 : 1);
    }
  else
    {
    // Do the common flags specify a particular entity type?
    BitFlags etype = this->m_commonFlags & ENTITY_MASK;
    BitFlags edims = this->m_commonFlags & ANY_DIMENSION;
    bool pluralDims;
    std::string dimPhrase =
      Entity::flagDimensionList( edims ?
        edims :
        this->m_unionFlags & ANY_DIMENSION, pluralDims);
    message
      << (etype ?
        Entity::flagSummary(etype, sz == 1 ? 0 : 1) :
        std::string("entities"))
      << " of "
      << (pluralDims ? "dimensions" : "dimension")
      << " "
      << dimPhrase;
    }
  return message.str();
}

/// Show the entity type in the subtitle
std::string EntityListPhrase::subtitle()
{
  return std::string();
}

/// The list of entities to be presented.
EntityRefArray EntityListPhrase::relatedEntities() const
{
  return this->m_entities;
}

/**\brief Inform the descriptor what bits in entityFlags() are set across the list.
  *
  * The \a commonFlags argument is the logical-AND of all entityFlags()
  * of all entities in the list. The \a unionFlags argument is the
  * logical-OR of all entityFlags().
  *
  * These are used by EntityListPhrase to choose an appropriate summary
  * description of the entities.
  */
void EntityListPhrase::setFlags(BitFlags commonFlags, BitFlags unionFlags)
{
  this->m_commonFlags = commonFlags;
  this->m_unionFlags = unionFlags;
}

  } // model namespace
} // smtk namespace
