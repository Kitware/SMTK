//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_AttributeListPhrase_h
#define __smtk_model_AttributeListPhrase_h

#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/AttributeAssignments.h"

#include <string>
#include <vector>

namespace smtk {
  namespace model {

/**\brief Describe a list of property (name,value) pairs
  * associated with an entity for user presentation.
  *
  * This enumerates properties of a single primitive storage
  * type (e.g. only string properties or only integer properties),
  * not all of an entity's properties at once.
  */
class SMTKCORE_EXPORT AttributeListPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(AttributeListPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const EntityRef& ent, DescriptivePhrasePtr parent);
  Ptr setup(const EntityRef& ent, const AttributeSet& subset, DescriptivePhrasePtr parent);

  virtual std::string title();
  virtual std::string subtitle();

  virtual smtk::common::UUID relatedEntityId() const;
  virtual EntityRef relatedEntity() const;

protected:
  AttributeListPhrase();

  virtual bool buildSubphrasesInternal();

  EntityRef m_entity;
  AttributeSet m_attributes; // Subset to be presented, modifications do not affect storage!
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_DescriptivePhrase_h
