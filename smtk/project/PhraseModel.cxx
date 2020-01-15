//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/PhraseModel.h"

#include "smtk/project/Manager.h"
#include "smtk/project/PhraseContent.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace project
{

smtk::view::PhraseModel::Ptr PhraseModel::create(const smtk::view::ConfigurationPtr& view)
{
  (void)view;
  auto model = PhraseModel::Ptr(new PhraseModel);
  model->root()->findDelegate()->setModel(model);
  return model;
}

PhraseModel::PhraseModel()
  : Superclass()
{
}

PhraseModel::PhraseModel(const smtk::view::Configuration* config, smtk::view::Manager* manager)
  : Superclass(config, manager)
{
}

PhraseModel::~PhraseModel() {}

bool PhraseModel::addSource(const smtk::common::TypeContainer& managers)
{
  if (!smtk::view::PhraseModel::addSource(managers))
  {
    return false;
  }

  auto& projectMgr =
    (managers.contains<smtk::project::ManagerPtr>() ? managers.get<smtk::project::ManagerPtr>()
                                                    : smtk::project::ManagerPtr());

  std::ostringstream s;
  s << "PhraseModel " << this << ": Update phrases when projects change.";
  auto projectHandle = projectMgr
    ? projectMgr->observers().insert(
        [this](const smtk::project::Project& project, const smtk::project::EventType& event) {
          this->handleProjectEvent(project, event);
          return 0;
        },
        0,    // assign a neutral priority
        true, // observeImmediately
        s.str())
    : smtk::project::Observers::Key();

  return true;
}

bool PhraseModel::removeSource(const smtk::common::TypeContainer& managers)
{
  return smtk::view::PhraseModel::removeSource(managers);
}

void PhraseModel::handleProjectEvent(const Project& proj, smtk::project::EventType event)
{
  // The PhraseModle system has been designed to handle shared pointers to
  // non-const projects and components. We const-cast here to accommodate
  // this pattern.
  smtk::project::ProjectPtr project = const_cast<smtk::project::Project&>(proj).shared_from_this();

  // if (event == smtk::project::EventType::MODIFIED)
  // {
  //   this->triggerModified(project);
  // }
  // else
  {
    this->processProject(project, event == smtk::project::EventType::ADDED);
  }
}

void PhraseModel::processResource(const smtk::resource::Resource::Ptr& resource, bool adding)
{
  if (auto project = std::dynamic_pointer_cast<smtk::project::Project>(resource))
  {
    processProject(project, adding);
  }
}

void PhraseModel::processProject(const smtk::project::Project::Ptr& project, bool adding)
{
  if (adding)
  {
    const auto& subphrases(m_root->subphrases());
    smtk::view::DescriptivePhrases::const_iterator it;
    for (it = subphrases.begin(); it != subphrases.end(); ++it)
    {
      if ((*it)->relatedResource() && (*it)->relatedResource()->id() == project->id())
      {
        return; // Already have the project listed.
      }
    }

    // Resource was not there; need to add it:
    smtk::view::DescriptivePhrases children(subphrases);
    children.push_back(
      smtk::project::PhraseContent::createPhrase(project, m_mutableAspects, m_root));
    std::sort(
      children.begin(), children.end(), smtk::view::DescriptivePhrase::compareByTypeThenTitle);
    this->updateChildren(m_root, children, std::vector<int>());
  }
  else
  {
    smtk::view::DescriptivePhrases children(m_root->subphrases());
    std::weak_ptr<smtk::project::Project> weakProjectPtr = project;
    children.erase(
      std::remove_if(
        children.begin(),
        children.end(),
        [weakProjectPtr](const smtk::view::DescriptivePhrase::Ptr& phr) -> bool {
          return phr->relatedResource() == weakProjectPtr.lock();
        }),
      children.end());
    this->updateChildren(m_root, children, std::vector<int>());
  }
}
} // namespace project
} // namespace smtk
