#include "smtk/model/EntityPhrase.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/InstanceEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

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

/// Return a color associated with the related entity.
FloatList EntityPhrase::relatedColor() const
{
  return this->m_entity.color();
}

bool EntityPhrase::isRelatedColorMutable() const
{
  return this->m_entity.isValid();
}

bool EntityPhrase::setRelatedColor(const FloatList& rgba)
{
  if (this->m_entity.isValid())
    {
    bool colorValid = rgba.size() == 4;
    for (int i = 0; colorValid && i < 4; ++i)
      colorValid &= (rgba[i] >= 0. && rgba[i] <= 1.);
    if (colorValid)
      {
      this->m_entity.setColor(rgba);
      return true;
      }
    }
  return false;
}

DescriptivePhrases EntityPhrase::PhrasesFromUUIDs(
  smtk::model::StoragePtr storage, const smtk::util::UUIDs& uids)
{
  DescriptivePhrases result;
  smtk::util::UUIDs::const_iterator it;
  for (it = uids.begin(); it != uids.end(); ++it)
    {
    result.push_back(
      EntityPhrase::create()->setup(Cursor(storage, *it)));
    }
  return result;
}

  } // model namespace
} // smtk namespace
