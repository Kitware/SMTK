//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_PhraseContent_h
#define smtk_project_PhraseContent_h

#include "smtk/view/PhraseContent.h"

#include "smtk/project/Project.h"

#include "smtk/view/DescriptivePhrase.h"

namespace smtk
{
namespace project
{

/**\brief Describe a project for user presentation.
  *
  */
class SMTKCORE_EXPORT PhraseContent : public smtk::view::PhraseContent
{
public:
  smtkTypeMacro(smtk::project::PhraseContent);
  smtkSharedPtrCreateMacro(smtk::view::PhraseContent);
  Ptr setup(const smtk::project::ProjectPtr& project, int mutability = 0);
  virtual ~PhraseContent();

  static smtk::view::DescriptivePhrasePtr createPhrase(
    const smtk::project::ProjectPtr& project,
    int mutability = 0,
    smtk::view::DescriptivePhrase::Ptr parent = smtk::view::DescriptivePhrasePtr());

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

  bool editStringValue(ContentType contentType, const std::string& val) override;
  bool editFlagValue(ContentType contentType, int val) override;

  smtk::resource::PersistentObjectPtr relatedObject() const override;
  smtk::resource::ResourcePtr relatedResource() const override;
  virtual smtk::project::ProjectPtr relatedProject() const;

  void setMutability(int whatsMutable);

  bool operator==(const smtk::view::PhraseContent& other) const override
  {
    return this->equalTo(other) &&
      !(m_project.owner_before(static_cast<const PhraseContent&>(other).m_project)) &&
      !(static_cast<const PhraseContent&>(other).m_project.owner_before(m_project));
  }

protected:
  PhraseContent();

  std::weak_ptr<smtk::project::Project> m_project;
  int m_mutability;
};
} // namespace project
} // namespace smtk

#endif
