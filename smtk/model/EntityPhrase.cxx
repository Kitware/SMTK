//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EntityPhrase.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"

namespace smtk {
  namespace model {

EntityPhrase::EntityPhrase()
{
  this->m_mutability = 3; // both color and title are mutable.
}

/**\brief Prepare an EntityPhrase for display.
  */
EntityPhrase::Ptr EntityPhrase::setup(const EntityRef& entity, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(ENTITY_SUMMARY, parnt);
  this->m_entity = entity;
  this->m_mutability = 3; // both color and title are mutable by default.
  if(!this->m_entity.hasVisibility())
    this->m_entity.setVisible(true); // set "visible" property to true
  return this->shared_from_this();
}

/// Show the entity name (or a default name) in the title
std::string EntityPhrase::title()
{
  return this->m_entity.name();
}

/// True when the entity is valid and marked as mutable (the default, setMutability(0x1)).
bool EntityPhrase::isTitleMutable() const
{
  return (this->m_mutability & 0x1) && this->m_entity.isValid();
}

bool EntityPhrase::setTitle(const std::string& newTitle)
{
  // The title is the name, so set the name as long as we're allowed.
  if (this->isTitleMutable() && this->m_entity.name() != newTitle)
    {
    if (!newTitle.empty())
      this->m_entity.setName(newTitle);
    else
      {
      this->m_entity.removeStringProperty("name");
      // Don't let name be a blank... assign a default.
      this->m_entity.manager()->assignDefaultName(
        this->m_entity.entity());
      }
    return true;
    }
  return false;
}

/// Show the entity type in the subtitle
std::string EntityPhrase::subtitle()
{
  return this->m_entity.flagSummary();
}

/// Return the entity for additional context the UI might wish to present.
EntityRef EntityPhrase::relatedEntity() const
{
  return this->m_entity;
}

/// Return a color associated with the related entity.
FloatList EntityPhrase::relatedColor() const
{
  return this->m_entity.color();
}

/// True when the entity is valid and marked as mutable (the default, setMutability(0x4)).
bool EntityPhrase::isRelatedColorMutable() const
{
  return (this->m_mutability & 0x4) && this->m_entity.isValid();
}

bool EntityPhrase::setRelatedColor(const FloatList& rgba)
{
  if (this->isRelatedColorMutable())
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

// Set bit vector indicating mutability; title (0x1), subtitle (0x2), color (0x4).
void EntityPhrase::setMutability(int whatsMutable)
{
  this->m_mutability = whatsMutable;
}

DescriptivePhrases EntityPhrase::PhrasesFromUUIDs(
  smtk::model::ManagerPtr manager, const smtk::common::UUIDs& uids)
{
  DescriptivePhrases result;
  smtk::common::UUIDs::const_iterator it;
  for (it = uids.begin(); it != uids.end(); ++it)
    {
    result.push_back(
      EntityPhrase::create()->setup(EntityRef(manager, *it)));
    }
  return result;
}

  } // model namespace
} // smtk namespace
