//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ObjectGroupPhraseContent_h
#define smtk_view_ObjectGroupPhraseContent_h

#include "smtk/view/PhraseContent.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/PropertyType.h"

namespace smtk
{
namespace view
{

/**\brief Group a set of persistent objects for user presentation.
  *
  * This takes resource and component filter strings plus a title
  * and populates its children with all objects that match the
  * filters.
  *
  * The phrase title is not mutable (although a decorator can be
  * used to change this).
  */
class SMTKCORE_EXPORT ObjectGroupPhraseContent : public PhraseContent
{
public:
  smtkTypeMacro(smtk::view::ObjectGroupPhraseContent);
  smtkSharedPtrCreateMacro(smtk::view::PhraseContent);
  Ptr setup(
    const std::string& title,
    const std::string& resourceFilter,
    const std::string& componentFilter);
  ~ObjectGroupPhraseContent() override;

  static DescriptivePhrasePtr createPhrase(
    const std::string& title,
    const std::string& resourceFilter,
    const std::string& componentFilter,
    DescriptivePhrasePtr parent = DescriptivePhrasePtr());

  const std::string& resourceFilter() const { return m_resourceFilter; }
  const std::string& componentFilter() const { return m_componentFilter; }

  /// Add this phrase's children to the container.
  void children(DescriptivePhrases& container) const;

  bool displayable(ContentType attr) const override { return attr == TITLE ? true : false; }
  bool editable(ContentType) const override { return false; }

  std::string stringValue(ContentType) const override { return m_title; }

  smtk::resource::PersistentObjectPtr relatedObject() const override { return nullptr; }
  smtk::resource::ResourcePtr relatedResource() const override { return nullptr; }
  smtk::resource::ComponentPtr relatedComponent() const override { return nullptr; }

  bool operator==(const PhraseContent& other) const override
  {
    return /* this->equalTo(other) && */
      m_resourceFilter == static_cast<const ObjectGroupPhraseContent&>(other).m_resourceFilter &&
      m_componentFilter == static_cast<const ObjectGroupPhraseContent&>(other).m_componentFilter &&
      m_title == static_cast<const ObjectGroupPhraseContent&>(other).m_title;
  }

protected:
  ObjectGroupPhraseContent();

  std::string m_resourceFilter;
  std::string m_componentFilter;
  std::string m_title;
};

} // namespace view
} // namespace smtk

#endif
