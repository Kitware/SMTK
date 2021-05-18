//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_view_PhraseModel_h
#define smtk_project_view_PhraseModel_h

#include "smtk/project/Observer.h"
#include "smtk/project/Project.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/ResourcePhraseModel.h"

namespace smtk
{
namespace project
{
namespace view
{

/**\brief Present phrases describing a set of projects held by one or more project managers.
  *
  */
class SMTKCORE_EXPORT PhraseModel : public smtk::view::ResourcePhraseModel
{
public:
  smtkTypeMacro(smtk::project::view::PhraseModel);
  smtkSuperclassMacro(smtk::view::ResourcePhraseModel);
  smtkSharedPtrCreateMacro(smtk::view::PhraseModel);

  PhraseModel();
  PhraseModel(const smtk::view::Configuration*, smtk::view::Manager*);
  ~PhraseModel() override;

  static smtk::view::PhraseModelPtr create(const smtk::view::ConfigurationPtr& view);

  /// A method called when a project manager adds or removes a project.
  virtual void handleProjectEvent(const smtk::project::Project&, smtk::project::EventType);

  /// Indicate a project manager that should be monitored for changes.
  bool addSource(const smtk::common::TypeContainer&) override;
  /// Indicate a project manager that should no longer be monitored for changes.
  bool removeSource(const smtk::common::TypeContainer&) override;

protected:
  void processResource(const smtk::resource::Resource::Ptr& rsrc, bool adding) override;
  virtual void processProject(const smtk::project::Project::Ptr& project, bool adding);
  virtual void updateProject(const Project::Ptr& project);

  std::list<smtk::project::Observers::Key> m_projectHandles;
};
} // namespace view
} // namespace project
} // namespace smtk

#endif
