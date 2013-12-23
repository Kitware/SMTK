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
  CursorArray::size_type sz = this->m_entities.size();
  message << sz << " ";

  this->buildSubphrases(); // This sets the m_{common,union}Flags members.
  // Now determine whether all the cursors share a common type or dimension.
  if (this->m_commonFlags == this->m_unionFlags)
    { // All the cursors have exactly the same flags set.
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

bool EntityListPhrase::buildSubphrasesInternal()
{
  this->m_commonFlags = INVALID;
  this->m_unionFlags = 0;
  for (
    CursorArray::iterator it = this->m_entities.begin();
    it != this->m_entities.end();
    ++it)
    {
    this->m_subphrases.push_back(
      EntityPhrase::create()->setup(*it, shared_from_this()));
    this->m_commonFlags &= it->entityFlags();
    this->m_unionFlags |= it->entityFlags();
    }
  return true;
}

  } // model namespace
} // smtk namespace
