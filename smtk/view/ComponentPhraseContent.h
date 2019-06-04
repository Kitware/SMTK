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

  static DescriptivePhrasePtr createPhrase(const smtk::resource::ComponentPtr& component,
    int mutability = 0, DescriptivePhrasePtr parent = DescriptivePhrasePtr());

  bool displayable(ContentType attr) const override { return attr != VISIBILITY ? true : false; }
  bool editable(ContentType attr) const override;

  std::string stringValue(ContentType attr) const override;
  int flagValue(ContentType attr) const override;
  resource::FloatList colorValue(ContentType attr) const override;

  bool editStringValue(ContentType attr, const std::string& val) override;
  bool editFlagValue(ContentType attr, int val) override;
  bool editColorValue(ContentType attr, const resource::FloatList& val) override;

  smtk::resource::PersistentObjectPtr relatedObject() const override;
  smtk::resource::ResourcePtr relatedResource() const override;
  smtk::resource::ComponentPtr relatedComponent() const override;

  void setMutability(int whatsMutable);

  bool operator==(const PhraseContent& other) const override
  {
    return this->equalTo(other) &&
      m_component == static_cast<const ComponentPhraseContent&>(other).m_component;
  }

protected:
  ComponentPhraseContent();

  smtk::resource::ComponentPtr m_component;
  int m_mutability;
};

} // view namespace
} // smtk namespace

#endif
