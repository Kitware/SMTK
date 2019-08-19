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

  bool displayable(ContentType contentType) const override
  {
    return contentType != VISIBILITY ? true : false;
  }
  bool editable(ContentType contentType) const override
  {
    return (m_mutability & static_cast<int>(contentType)) ? true : false;
  }

  std::string stringValue(ContentType contentType) const override;
  int flagValue(ContentType contentType) const override;
  resource::FloatList colorValue(ContentType contentType) const override;

  bool editStringValue(ContentType contentType, const std::string& val) override;
  bool editFlagValue(ContentType contentType, int val) override;
  bool editColorValue(ContentType contentType, const resource::FloatList& val) override;

  smtk::resource::PersistentObjectPtr relatedObject() const override;
  smtk::resource::ResourcePtr relatedResource() const override;

  void setMutability(int whatsMutable);

  bool operator==(const PhraseContent& other) const override
  {
    return this->equalTo(other) &&
      !(m_resource.owner_before(static_cast<const ResourcePhraseContent&>(other).m_resource)) &&
      !(static_cast<const ResourcePhraseContent&>(other).m_resource.owner_before(m_resource));
  }

protected:
  ResourcePhraseContent();

  std::weak_ptr<smtk::resource::Resource> m_resource;
  int m_mutability;
};

} // view namespace
} // smtk namespace

#endif
