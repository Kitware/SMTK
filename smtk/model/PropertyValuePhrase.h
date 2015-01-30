//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_PropertyValuePhrase_h
#define __smtk_model_PropertyValuePhrase_h

#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/BRepModel.h" // For PropertyType enum.

#include <string>
#include <vector>

namespace smtk {
  namespace model {

/**\brief Describe the value of a named property for user presentation.
  *
  */
class SMTKCORE_EXPORT PropertyValuePhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(PropertyValuePhrase);
  smtkCreateMacro(DescriptivePhrase);
  PropertyValuePhrase();
  Ptr setup(PropertyType propType, const std::string& propName, DescriptivePhrase::Ptr parent);

  virtual std::string title();
  virtual std::string subtitle();

  virtual smtk::common::UUID relatedEntityId() const;
  virtual EntityRef relatedEntity() const;
  virtual std::string relatedPropertyName() const;
  virtual PropertyType relatedPropertyType() const;

  static DescriptivePhraseType propertyToPhraseType(PropertyType p);

protected:
  PropertyType m_propertyType;
  std::string m_propertyName;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_DescriptivePhrase_h
