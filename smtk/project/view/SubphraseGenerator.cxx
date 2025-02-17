//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/view/SubphraseGenerator.h"

#include "smtk/project/Project.h"
#include "smtk/project/view/PhraseContent.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseContent.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/SubphraseGenerator.txx"

namespace smtk
{
namespace project
{
namespace view
{

SubphraseGenerator::SubphraseGenerator() = default;

SubphraseGenerator::~SubphraseGenerator() = default;

smtk::view::DescriptivePhrases SubphraseGenerator::subphrases(
  smtk::view::DescriptivePhrase::Ptr src)
{
  smtk::view::DescriptivePhrases result;
  if (src)
  {
    auto resource = src->relatedResource();
    auto project = std::dynamic_pointer_cast<smtk::project::Project>(resource);

    if (project)
    {
      smtk::project::view::SubphraseGenerator::childrenOfProject(src, project, result);
    }
  }
  return result;
}

void SubphraseGenerator::childrenOfProject(
  smtk::view::DescriptivePhrase::Ptr src,
  smtk::project::ProjectPtr project,
  smtk::view::DescriptivePhrases& result)
{
  constexpr int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE);
  for (const auto& resource : project->resources())
  {
    result.push_back(smtk::project::view::PhraseContent::createPhrase(resource, mutability, src));
  }
  std::sort(result.begin(), result.end(), smtk::view::DescriptivePhrase::compareByTypeThenTitle);
}

SubphraseGenerator::Path SubphraseGenerator::indexOfObjectInParent(
  const smtk::resource::PersistentObjectPtr& obj,
  const smtk::view::DescriptivePhrasePtr& actualParent,
  const Path& parentPath)
{
  // In the case of Projects - only the Project's resources should be displayed
  Path result;
  auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj);
  if (
    actualParent && rsrc && actualParent->relatedResource() &&
    (rsrc->parentResource() == actualParent->relatedResource().get()))
  {
    PreparePath(result, parentPath, IndexFromTitle(rsrc->name(), actualParent->subphrases()));
  }

  return result;
}

} // namespace view
} // namespace project
} // namespace smtk
