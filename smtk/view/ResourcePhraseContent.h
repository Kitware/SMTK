//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ResourcePhraseContent_h
#define smtk_view_ResourcePhraseContent_h

#include "smtk/view/PhraseContent.h"

#include "smtk/resource/PropertyType.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

/**\brief Describe a resource component for user presentation.
  *
  */
class SMTKCORE_EXPORT ResourcePhraseContent : public PhraseContent
{
public:
  smtkTypeMacro(ResourcePhraseContent);
  smtkSharedPtrCreateMacro(PhraseContent);
  Ptr setup(const smtk::resource::ResourcePtr& resource, int mutability = 0);
  virtual ~ResourcePhraseContent();

  static DescriptivePhrasePtr createPhrase(const smtk::resource::ResourcePtr& resource,
    int mutability = 0, DescriptivePhrasePtr parent = DescriptivePhrasePtr());

  bool displayable(ContentType attr) const override { return attr != VISIBILITY ? true : false; }
  bool editable(ContentType attr) const override
  {
    return (m_mutability & static_cast<int>(attr)) ? true : false;
  }

  std::string stringValue(ContentType attr) const override;
  int flagValue(ContentType attr) const override;
  resource::FloatList colorValue(ContentType attr) const override;

  bool editStringValue(ContentType attr, const std::string& val) override;
  bool editFlagValue(ContentType attr, int val) override;
  bool editColorValue(ContentType attr, const resource::FloatList& val) override;

  smtk::resource::PersistentObjectPtr relatedObject() const override;
  smtk::resource::ResourcePtr relatedResource() const override;

  void setMutability(int whatsMutable);

  bool operator==(const PhraseContent& other) const override
  {
    return this->equalTo(other) &&
      m_resource == static_cast<const ResourcePhraseContent&>(other).m_resource;
  }

protected:
  ResourcePhraseContent();

  smtk::resource::ResourcePtr m_resource;
  int m_mutability;
};

} // view namespace
} // smtk namespace

#endif
