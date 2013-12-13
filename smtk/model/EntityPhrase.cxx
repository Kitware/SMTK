#include "smtk/model/EntityPhrase.h"

#include "smtk/model/PropertyListPhrase.h"

namespace smtk {
  namespace model {

EntityPhrase::EntityPhrase()
{
}

/**\brief Prepare an EntityPhrase for display.
  */
EntityPhrase::Ptr EntityPhrase::setup(const Cursor& entity, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(ENTITY_SUMMARY, parnt);
  this->m_entity = entity;
  return this->shared_from_this();
}

/// Show the entity name (or a default name) in the title
std::string EntityPhrase::title()
{
  return this->m_entity.name();
}

/// Show the entity type in the subtitle
std::string EntityPhrase::subtitle()
{
  return this->m_entity.flagSummary();
}

/// Return the entity for additional context the UI might wish to present.
Cursor EntityPhrase::relatedEntity() const
{
  return this->m_entity;
}


bool EntityPhrase::buildSubphrasesInternal()
{
  if (this->m_entity.hasStringProperties())
    { // TODO: If m_entity.stringProperties().size() < N, add PropValuePhrases instead of a list.
    this->m_subphrases.push_back(
      PropertyListPhrase::create()->setup(
        this->m_entity, STRING_PROPERTY, shared_from_this()));
    }
  if (this->m_entity.hasFloatProperties())
    { // TODO: If m_entity.floatProperties().size() < N, add PropValuePhrases instead of a list.
    this->m_subphrases.push_back(
      PropertyListPhrase::create()->setup(
        this->m_entity, FLOAT_PROPERTY, shared_from_this()));
    }
  if (this->m_entity.hasIntegerProperties())
    { // TODO: If m_entity.integerProperties().size() < N, add PropValuePhrases instead of a list.
    this->m_subphrases.push_back(
      PropertyListPhrase::create()->setup(
        this->m_entity, INTEGER_PROPERTY, shared_from_this()));
    }
  return true;
}

  } // model namespace
} // smtk namespace
