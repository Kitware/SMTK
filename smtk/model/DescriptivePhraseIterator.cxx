#include "smtk/model/DescriptivePhraseIterator.h"

namespace smtk {
  namespace model {

DescriptivePhraseIterator::DescriptivePhraseIterator(const Cursor& entity) :
  m_parent(NULL),
  m_context(entity),
  m_currentPhraseType(ENTITY_SUMMARY),
  m_currentPhrase(-1)
{
}

DescriptivePhraseIterator::DescriptivePhraseIterator(
  const DescriptivePhraseIterator& other) :
  m_parent(other.m_parent),
  m_context(other.m_context),
  m_currentPhraseType(other.m_currentPhraseType),
  m_currentPhrase(other.m_currentPhrase)
{
}

DescriptivePhraseIterator::DescriptivePhraseIterator(
  DescriptivePhraseIterator* parent, DescriptivePhraseType phraseType, int phraseNum) :
  m_parent(parent),
  m_context(parent->m_context),
  m_currentPhraseType(phraseType),
  m_currentPhrase(phraseNum)
{
}

void DescriptivePhraseIterator::init()
{
  if (this->m_parent)
    {
    *this = *this->m_parent;
    }
  else
    {
    this->m_currentPhraseType = ENTITY_SUMMARY;
    this->m_numberOfPhrasesOfCurrentType = 1;
    this->m_currentPhrase = 0;
    this->m_currentChild.reset();
    }
}

bool DescriptivePhraseIterator::advance()
{
  return false;
}

bool DescriptivePhraseIterator::done()
{
  return true;
}

std::string DescriptivePhraseIterator::phrase() const
{
  std::ostringstream message;
  switch (this->m_currentPhraseType)
    {
  case ENTITY_SUMMARY:
    message << this->m_context.name() << " (" << this->m_context.flagSummary() << ")";
    break;
  case MODEL_INCLUDES_ENTITY:
    message
      << "contains " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case MODEL_EMBEDDED_IN_MODEL:
    message
      << "parent " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case CELL_INCLUDES_CELL:
    message
      << "includes " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case CELL_EMBEDDED_IN_CELL:
    message
      << "embedded in " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case CELL_HAS_SHELL:
    message
      << "shell " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case CELL_HAS_USE:
    message
      << "sense " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case SHELL_HAS_CELL:
    message
      << "cell " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case SHELL_HAS_USE:
    message
      << "has use " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case USE_HAS_CELL:
    message
      << "cell " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case USE_HAS_SHELL:
    message
      << "cell " << this->relatedEntity().name()
      << " (" << this->relatedEntity().flagSummary() << ")";
    break;
  case ENTITY_HAS_FLOAT_PROPERTY:
      {
      std::string propName = this->relatedPropertyName();
      const FloatList& propVal(this->m_context.floatProperty(propName));
      message << "float property " << propName << " ";
      if (propVal.size() < 1)
        {
        message << "(empty)";
        }
      else if (propVal.size() == 1)
        {
        message << propVal[0];
        }
      else
        {
        message << propVal.size();
        }
      }
    break;
  case ENTITY_HAS_STRING_PROPERTY:
      {
      std::string propName = this->relatedPropertyName();
      const StringList& propVal(this->m_context.stringProperty(propName));
      message << "string property " << propName << " ";
      if (propVal.size() < 1)
        {
        message << "(empty)";
        }
      else if (propVal.size() == 1)
        {
        message << propVal[0];
        }
      else
        {
        message << propVal.size();
        }
      }
    break;
  case ENTITY_HAS_INTEGER_PROPERTY:
      {
      std::string propName = this->relatedPropertyName();
      const IntegerList& propVal(this->m_context.integerProperty(propName));
      message << "integer property " << propName << " ";
      if (propVal.size() < 1)
        {
        message << "(empty)";
        }
      else if (propVal.size() == 1)
        {
        message << propVal[0];
        }
      else
        {
        message << propVal.size();
        }
      }
    break;
  case ENTITY_HAS_ATTRIBUTE:
    message
      << " attribute " << this->relatedAttributeId();
    break;
  case ENTITY_HAS_SUBPHRASES:
    message
      << " has " << this->numberOfSubphrases() << " subentries";
    break;
  case INVALID_DESCRIPTION:
  default:
    message << "error";
    break;
    }
  return message.str();
}

DescriptivePhraseType DescriptivePhraseIterator::phraseType() const
{
  return this->m_currentPhraseType;
}

int DescriptivePhraseIterator::numberOfSubphrases() const
{
  switch (this->m_currentPhraseType)
    {
  case ENTITY_SUMMARY:
    return 0;//this->numberOfPhrasesForEntity(this->m_context);
  case ENTITY_HAS_FLOAT_PROPERTY:
    return this->m_currentFloatProperty->second.size();
  case ENTITY_HAS_STRING_PROPERTY:
    return this->m_currentStringProperty->second.size();
  case ENTITY_HAS_INTEGER_PROPERTY:
    return this->m_currentIntegerProperty->second.size();
  case ENTITY_HAS_ATTRIBUTE:
    return 0;
  case ENTITY_HAS_SUBPHRASES:
    return this->m_currentChild->numberOfSubphrases();
    break;
  case MODEL_INCLUDES_ENTITY:
  case CELL_INCLUDES_CELL:
    return this->m_context.findArrangement(INCLUDES, this->m_currentPhrase)->details().size();
    break;
  case MODEL_EMBEDDED_IN_MODEL:
  case CELL_EMBEDDED_IN_CELL:
    return this->m_context.findArrangement(EMBEDDED_IN, this->m_currentPhrase)->details().size();
    break;
  case CELL_HAS_SHELL:
  case USE_HAS_SHELL:
    return this->m_context.findArrangement(HAS_SHELL, this->m_currentPhrase)->details().size();
    break;
  case CELL_HAS_USE:
  case SHELL_HAS_USE:
    return this->m_context.findArrangement(HAS_USE, this->m_currentPhrase)->details().size();
    break;
  case SHELL_HAS_CELL:
  case USE_HAS_CELL:
    return this->m_context.findArrangement(HAS_CELL, this->m_currentPhrase)->details().size();
    break;
  default:
    break;
    }
  return 0;
}

smtk::common::UUID DescriptivePhraseIterator::relatedEntityId() const
{
  return this->relatedEntity().entity();
}

Cursor DescriptivePhraseIterator::relatedEntity() const
{
  switch (this->m_currentPhraseType)
    {
  case ENTITY_SUMMARY:
  case ENTITY_HAS_FLOAT_PROPERTY:
  case ENTITY_HAS_STRING_PROPERTY:
  case ENTITY_HAS_INTEGER_PROPERTY:
  case ENTITY_HAS_ATTRIBUTE:
  case ENTITY_HAS_SUBPHRASES:
    return this->m_context;
    break;
  case MODEL_INCLUDES_ENTITY:
  case CELL_INCLUDES_CELL:
    return this->m_context.relationFromArrangement(INCLUDES, this->m_currentPhrase, 0);
    break;
  case MODEL_EMBEDDED_IN_MODEL:
  case CELL_EMBEDDED_IN_CELL:
    return this->m_context.relationFromArrangement(EMBEDDED_IN, this->m_currentPhrase, 0);
    break;
  case CELL_HAS_SHELL:
  case USE_HAS_SHELL:
    return this->m_context.relationFromArrangement(HAS_SHELL, this->m_currentPhrase, 0);
    break;
  case CELL_HAS_USE:
  case SHELL_HAS_USE:
    return this->m_context.relationFromArrangement(HAS_USE, this->m_currentPhrase, 0);
    break;
  case SHELL_HAS_CELL:
  case USE_HAS_CELL:
    return this->m_context.relationFromArrangement(HAS_CELL, this->m_currentPhrase, 0);
    break;
  default:
    break;
    }
  return Cursor();
}

smtk::attribute::AttributeId DescriptivePhraseIterator::relatedAttributeId() const
{
  return 0;
}

/// The name of the property related to the current descriptive phrase (or empty if none).
std::string DescriptivePhraseIterator::relatedPropertyName() const
{
  switch (this->m_currentPhraseType)
    {
  case ENTITY_HAS_FLOAT_PROPERTY:
    if (this->m_currentFloatProperty != this->m_context.floatProperties().end())
      return this->m_currentFloatProperty->first;
    break;
  case ENTITY_HAS_STRING_PROPERTY:
    if (this->m_currentStringProperty != this->m_context.stringProperties().end())
      return this->m_currentStringProperty->first;
    break;
  case ENTITY_HAS_INTEGER_PROPERTY:
    if (this->m_currentIntegerProperty != this->m_context.integerProperties().end())
      return this->m_currentIntegerProperty->first;
    break;
  default:
    break;
    }
  return std::string();
}

/// The storage type of the property related to the current descriptive phrase (or INVALID_PROPERTY).
PropertyType DescriptivePhraseIterator::relatedPropertyType() const
{
  switch (this->m_currentPhraseType)
    {
  case ENTITY_HAS_FLOAT_PROPERTY:
    return FLOAT_PROPERTY;
    break;
  case ENTITY_HAS_STRING_PROPERTY:
    return STRING_PROPERTY;
    break;
  case ENTITY_HAS_INTEGER_PROPERTY:
    return INTEGER_PROPERTY;
    break;
  default:
    break;
    }
  return INVALID_PROPERTY;
}

/*
static const char* DescriptivePhraseTypeNamesSingular[] = {
  "has %%%d subentity",         //!< The entity is a model with a "free" entity.
  "has a parent model",         //!< The entity is a model that is the child of another model.
  "has %%%d included %s",       //!< The entity is a cell that includes a lower-dimensional cell.
  "is embedded in %s",          //!< The entity is a cell embedded in a higher-dimensional cell.
  "has %%%d %s",                //!< The entity is a cell with a shell describing its boundary
  "has %%%d uses",              //!< The entity is a cell with a sense that is in use as a boundary.
  "parent %s",                  //!< The entity is a shell and this is its parent cell.
  "has %%%d %s",                //!< The entity is a shell composed of multiple uses.
  "has parent %s",              //!< The entity is a use and this is its parent cell.
  "in shell %%%d",              //!< The entity is a use and it participates in a shell.
  "%d float properties",        //!< The entity has floating-point properties.
  "%d string properties",       //!< The entity has string properties.
  "%d integer properties",      //!< The entity has integer properties.
  "attribute %s",               //!< The entity is associated with an attribute
  "%d %s",                      //!< The entity has many phrases of one type; this phrase summarizes them.
  "invalid"                     //!< This is used to indicate an invalid or empty descriptive phrase.
};
*/

  } // model namespace
} // smtk namespace
