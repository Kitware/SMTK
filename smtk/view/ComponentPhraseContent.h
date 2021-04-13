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
  virtual ~ComponentPhraseContent();

  static DescriptivePhrasePtr createPhrase(
    const smtk::resource::ComponentPtr& component,
    int mutability = 0,
    DescriptivePhrasePtr parent = DescriptivePhrasePtr());

  bool displayable(ContentType contentType) const override
  {
    return contentType != VISIBILITY ? true : false;
  }
  bool editable(ContentType contentType) const override;

  std::string stringValue(ContentType contentType) const override;
  int flagValue(ContentType contentType) const override;

  bool editStringValue(ContentType contentType, const std::string& val) override;
  bool editFlagValue(ContentType contentType, int val) override;

  smtk::resource::PersistentObjectPtr relatedObject() const override;
  smtk::resource::ResourcePtr relatedResource() const override;
  smtk::resource::ComponentPtr relatedComponent() const override;

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
  int m_mutability;
};

} // namespace view
} // namespace smtk

#endif
