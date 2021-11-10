//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/QueryFilterSubphraseGenerator.h"

#include "smtk/model/Entity.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/resource/Component.h"
#include "smtk/view/ObjectGroupPhraseContent.h"

#include "smtk/view/SubphraseGenerator.txx"

namespace smtk
{
namespace view
{

QueryFilterSubphraseGenerator::QueryFilterSubphraseGenerator() = default;

QueryFilterSubphraseGenerator::~QueryFilterSubphraseGenerator() = default;

DescriptivePhrases QueryFilterSubphraseGenerator::subphrases(DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  if (src)
  {
    smtk::resource::ComponentPtr comp = src->relatedComponent();
    if (!comp)
    {
      smtk::resource::ResourcePtr rsrc = src->relatedResource();
      // Check if it's a resource descriptive phrase or a component descriptive phrase
      if (!rsrc)
      {
        PhraseContentPtr content = src->content();
        auto ogpc = std::dynamic_pointer_cast<smtk::view::ObjectGroupPhraseContent>(content);
        if (ogpc)
        {
          ogpc->children(result);
        }
      }
      else
      {
        SubphraseGenerator::componentsOfResource(src, rsrc, result);
      }
    }
  }
  return result;
}

SubphraseGenerator::Path QueryFilterSubphraseGenerator::indexOfObjectInParent(
  const smtk::resource::PersistentObjectPtr& obj,
  const smtk::view::DescriptivePhrasePtr& parent,
  const Path& parentPath)
{
  // The query filter phrase generator will never have resource as children of anything. So unless
  // obj is a component, we do not assign it a path.
  Path result;
  auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);

  if (!parent || !comp)
  {
    return result;
  }
  // Check if obj passes parent descriptive phrases filter. If so, prepare path.
  auto content = parent->content();
  auto objectGroupParent = std::dynamic_pointer_cast<smtk::view::ObjectGroupPhraseContent>(content);
  if (!objectGroupParent)
  {
    return result;
  }
  resource::ComponentSet components =
    comp->resource()->filter(objectGroupParent->componentFilter());
  if (components.count(comp))
  {
    this->PreparePath(result, parentPath, this->IndexFromTitle(comp->name(), parent->subphrases()));
  }
  return result;
}

} // namespace view
} // namespace smtk
