//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ComponentPhraseContent_h
#define smtk_view_ComponentPhraseContent_h

#include "smtk/view/PhraseContent.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/PropertyType.h"

namespace smtk
{
namespace view
{

/**\brief Describe a resource component for user presentation.
  *
  */
class SMTKCORE_EXPORT ComponentPhraseContent : public PhraseContent
{
public:
  smtkTypeMacro(ComponentPhraseContent);
  smtkSharedPtrCreateMacro(PhraseContent);
  Ptr setup(const smtk::resource::ComponentPtr& component, int mutability = 0);
  ~ComponentPhraseContent() override;

  static DescriptivePhrasePtr createPhrase(
    const smtk::resource::ComponentPtr& component,
    int mutability = 0,
    DescriptivePhrasePtr parent = DescriptivePhrasePtr());

  bool displayable(ContentType contentType) const override { return contentType != VISIBILITY; }
  bool editable(ContentType contentType) const override;

  std::string stringValue(ContentType contentType) const override;
  int flagValue(ContentType contentType) const override;

  bool editStringValue(ContentType contentType, const std::string& val) override;
  bool editFlagValue(ContentType contentType, int val) override;

  smtk::resource::PersistentObjectPtr relatedObject() const override;
  smtk::resource::ResourcePtr relatedResource() const override;
  smtk::resource::Resource* relatedRawResource() const override { return m_rawResource; }
  smtk::resource::ComponentPtr relatedComponent() const override;
  smtk::resource::Component* relatedRawComponent() const override { return m_rawComponent; }

  void setMutability(int whatsMutable);

  bool operator==(const PhraseContent& other) const override
  {
    return this->equalTo(other) &&
      !(m_component.owner_before(static_cast<const ComponentPhraseContent&>(other).m_component)) &&
      !(static_cast<const ComponentPhraseContent&>(other).m_component.owner_before(m_component));
  }

protected:
  ComponentPhraseContent();

  std::weak_ptr<smtk::resource::Component> m_component;
  smtk::resource::Component* m_rawComponent = nullptr;
  smtk::resource::Resource* m_rawResource = nullptr;
  int m_mutability{ 0 };
};

} // namespace view
} // namespace smtk

#endif
