//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EntityListPhrase_h
#define __smtk_model_EntityListPhrase_h

#include "smtk/model/DescriptivePhrase.h"

namespace smtk
{
namespace model
{

/**\brief Describe a list of entities for user presentation.
  *
  */
class SMTKCORE_EXPORT EntityListPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(EntityListPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  template <typename T>
  Ptr setup(const T& entities, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  virtual ~EntityListPhrase() {}

  virtual std::string title();
  virtual std::string subtitle();

  EntityRefArray relatedEntities() const;
  virtual BitFlags relatedBitFlags() const { return this->m_commonFlags; }
  EntityRefArray& relatedEntities();
  virtual void setFlags(BitFlags commonFlags, BitFlags unionFlags);

  virtual bool isRelatedColorMutable() const;
  virtual FloatList relatedColor() const;

protected:
  EntityListPhrase();

  EntityRefArray m_entities;
  BitFlags m_commonFlags;
  BitFlags m_unionFlags;
};

/**\brief Initialize an entity list with an iterable container of entityrefs.
  *
  * This templated method is provided so that arrays of **subclasses** of
  * EntityRefs are also accepted.
  */
template <typename T>
EntityListPhrase::Ptr EntityListPhrase::setup(const T& entities, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(ENTITY_LIST, parnt);
  for (typename T::const_iterator it = entities.begin(); it != entities.end(); ++it)
  {
    this->m_entities.push_back(*it);
  }
  this->m_commonFlags = INVALID;
  this->m_unionFlags = 0;
  return shared_from_this();
}

} // model namespace
} // smtk namespace

#endif // __smtk_model_EntityListPhrase_h
