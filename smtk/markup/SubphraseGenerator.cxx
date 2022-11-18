//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/SubphraseGenerator.h"

#include "smtk/markup/Resource.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/NodeGroupPhraseContent.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseContent.h"

#include "smtk/common/StringUtil.h"

#include <iostream>

using smtk::view::ComponentPhraseContent;
using smtk::view::ResourcePhraseContent;

static int phraseMutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
  static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);

namespace smtk
{
namespace markup
{

SubphraseGenerator::SubphraseGenerator() = default;

smtk::view::DescriptivePhrases SubphraseGenerator::subphrases(
  smtk::view::DescriptivePhrase::Ptr parent)
{
  smtk::view::DescriptivePhrases result;
  if (!parent)
  {
    return result;
  }
  auto* content = parent->content().get();
  const int mutability =
    static_cast<int>(smtk::view::PhraseContent::TITLE | smtk::view::PhraseContent::VISIBILITY);
  if (auto* componentContent = dynamic_cast<ComponentPhraseContent*>(content))
  {
    if (auto* baseComp = dynamic_cast<Component*>(componentContent->relatedRawComponent()))
    {
      // All components may be labeled:
      baseComp->incoming<arcs::LabelsToSubjects>().visit(
        [&result, &parent, &mutability](const Label* constLabel) {
          auto* label = const_cast<Label*>(constLabel);
          result.push_back(
            ComponentPhraseContent::createPhrase(label->shared_from_this(), mutability, parent));
        });
      if (auto* group = dynamic_cast<Group*>(baseComp))
      {
        // Groups have members underneath them
        group->outgoing<arcs::GroupsToMembers>().visit(
          [&result, &parent, &mutability](const Component* constMember) {
            auto* member = const_cast<Component*>(constMember);
            result.push_back(
              ComponentPhraseContent::createPhrase(member->shared_from_this(), mutability, parent));
          });
      }
      else if (auto* label = dynamic_cast<Label*>(baseComp))
      {
        // Labels have no children.
        (void)label;
      }

      if (auto* shape = dynamic_cast<SpatialData*>(baseComp))
      {
        // Spatial data (meshes, images) may have Fields as children.
        shape->incoming<arcs::FieldsToShapes>().visit(
          [&result, &parent, &mutability](const Field* constField) {
            auto* field = const_cast<Field*>(constField);
            result.push_back(
              ComponentPhraseContent::createPhrase(field->shared_from_this(), mutability, parent));
          });
      }
    }
  }
  else if (auto* resourceContent = dynamic_cast<ResourcePhraseContent*>(content))
  {
    if (std::dynamic_pointer_cast<smtk::markup::Resource>(resourceContent->relatedResource()))
    {
      // Top-level resources have node groups underneath them
      result.push_back(
        smtk::view::NodeGroupPhraseContent::createPhrase(parent, "smtk::markup::Group"));
      result.push_back(smtk::view::NodeGroupPhraseContent::createPhrase(
        parent, "smtk::markup::ImageData", "image"));
      result.push_back(smtk::view::NodeGroupPhraseContent::createPhrase(
        parent, "smtk::markup::UnstructuredData", "mesh", "meshes"));
      result.push_back(
        smtk::view::NodeGroupPhraseContent::createPhrase(parent, "smtk::markup::Field", "field"));
      result.push_back(smtk::view::NodeGroupPhraseContent::createPhrase(
        parent, "smtk::markup::Box", "box", "boxes"));
      result.push_back(
        smtk::view::NodeGroupPhraseContent::createPhrase(parent, "smtk::markup::Sphere", "sphere"));
      result.push_back(
        smtk::view::NodeGroupPhraseContent::createPhrase(parent, "smtk::markup::Label"));
    }
    else
    {
      result = this->Superclass::subphrases(parent);
    }
  }
  else if (auto* nodeGroupContent = dynamic_cast<smtk::view::NodeGroupPhraseContent*>(content))
  {
    // Node groups hold "free" nodes of a given type underneath them.
    // If type is AnalyticShape, then AnalyticShape not owned by a Group appear.
    // If type is Group, then Groups not owned by a Group appear.
    // If type is Label, then all labels appear (though no Label should ever be "free").
    std::shared_ptr<smtk::view::DescriptivePhrase> ancestor = parent->parent();
    std::shared_ptr<smtk::resource::Resource> resource =
      ancestor ? ancestor->relatedResource() : nullptr;
    while (ancestor && !resource)
    {
      ancestor = ancestor->parent();
      resource = ancestor ? ancestor->relatedResource() : nullptr;
    }
    if (resource)
    {
      auto childObjects = resource->filter(nodeGroupContent->childType());
      for (const auto& child : childObjects)
      {
        result.push_back(ComponentPhraseContent::createPhrase(child, mutability, parent));
      }
      std::sort(result.begin(), result.end(), smtk::view::DescriptivePhrase::compareByTitle);
    }
  }
  return result;
}

bool SubphraseGenerator::hasChildren(const smtk::view::DescriptivePhrase& parent) const
{
  bool result = false;
  auto* content = parent.content().get();
  if (auto* componentContent = dynamic_cast<ComponentPhraseContent*>(content))
  {
    if (auto* baseComp = dynamic_cast<Component*>(componentContent->relatedRawComponent()))
    {
      // Components have labels underneath them
      baseComp->incoming<arcs::LabelsToSubjects>().visit(
        [&result, &parent](const Label* constLabel) {
          if (constLabel)
          {
            result = true;
            return smtk::common::Visit::Halt;
          }
          return smtk::common::Visit::Continue;
        });
      if (auto* group = dynamic_cast<Group*>(baseComp))
      {
        // Groups have members underneath them
        group->outgoing<arcs::GroupsToMembers>().visit(
          [&result, &parent](const Component* constMember) {
            if (constMember)
            {
              result = true;
              return smtk::common::Visit::Halt;
            }
            return smtk::common::Visit::Continue;
          });
      }
      else if (auto* label = dynamic_cast<Label*>(baseComp))
      {
        (void)label;
        // Labels have no children.
        result = false;
      }

      if (auto* shape = dynamic_cast<SpatialData*>(baseComp))
      {
        // Spatial data (meshes, images) may have Fields as children.
        shape->incoming<arcs::FieldsToShapes>().visit([&result](const Field* constField) {
          if (constField)
          {
            result = true;
            return smtk::common::Visit::Halt;
          }
          return smtk::common::Visit::Continue;
        });
      }
    }
  }
  else if (auto* resourceContent = dynamic_cast<ResourcePhraseContent*>(content))
  {
    if (std::dynamic_pointer_cast<smtk::markup::Resource>(resourceContent->relatedResource()))
    {
      // Top-level resources have node groups underneath them
      result = true;
    }
    else
    {
      result = this->Superclass::hasChildren(parent);
    }
  }
  else if (auto* nodeGroupContent = dynamic_cast<smtk::view::NodeGroupPhraseContent*>(content))
  {
    // Node groups hold "free" nodes of a given type underneath them.
    // If type is AnalyticShape, then AnalyticShape not owned by a Group appear.
    // If type is Group, then Groups not owned by a Group appear.
    // If type is Label, then all labels appear (though no Label should ever be "free").
    std::shared_ptr<smtk::view::DescriptivePhrase> ancestor = parent.parent();
    std::shared_ptr<smtk::resource::Resource> resource =
      ancestor ? ancestor->relatedResource() : nullptr;
    while (ancestor && !resource)
    {
      ancestor = ancestor->parent();
      resource = ancestor ? ancestor->relatedResource() : nullptr;
    }
    if (resource)
    {
      auto childObjects = resource->filter(nodeGroupContent->childType());
      result = !childObjects.empty();
    }
  }
  return result;
}

smtk::resource::PersistentObjectSet SubphraseGenerator::parentObjects(
  const smtk::resource::PersistentObjectPtr& obj) const
{
  smtk::resource::PersistentObjectSet result;
  if (auto* baseComp = dynamic_cast<Component*>(obj.get()))
  {
    if (auto* label = dynamic_cast<Label*>(baseComp))
    {
      // If the baseComp is a Label, it appears underneath the "Labels" NodeGroup
      // and zero or more Components.
      label->outgoing<arcs::LabelsToSubjects>().visit([&result](const Component* constSubject) {
        auto* subject = const_cast<Component*>(constSubject);
        result.insert(subject->shared_from_this());
      });
    }
    else if (auto* group = dynamic_cast<Group*>(obj.get()))
    {
      (void)group;
      // Groups appear underneath the Groups NodeGroup
      // and 0 or 1 Groups (if hierarchical groups are allowed; they aren't currently).
    }
    else if (auto* field = dynamic_cast<Field*>(obj.get()))
    {
      // Fields appear underneath spatial data (meshes, images)
      // which define their domain.
      field->outgoing<arcs::FieldsToShapes>().visit([&result](const SpatialData* constParent) {
        auto* parent = const_cast<SpatialData*>(constParent);
        result.insert(parent->shared_from_this());
      });
    }
    // Any baseComp may appear under zero or more Groups.
    baseComp->incoming<arcs::GroupsToMembers>().visit([&result](const Group* constParent) {
      auto* parent = const_cast<Group*>(constParent);
      result.insert(parent->shared_from_this());
    });
  }
  else
  {
    result = this->Superclass::parentObjects(obj);
  }
  return result;
}

void SubphraseGenerator::subphrasesForCreatedObjects(
  const smtk::resource::PersistentObjectArray& objects,
  const smtk::view::DescriptivePhrasePtr& localRoot,
  PhrasesByPath& resultingPhrases)
{
  // Add phrases underneath the fixed NodeGroupPhraseContent instances.
  // These nodes are under each resource, so we build a map of their locations first.
  struct Entry
  {
    SubphraseGenerator::Path path;
    std::shared_ptr<smtk::view::DescriptivePhrase> phrase;
  };
  std::map<smtk::markup::Resource*, std::map<std::string, Entry>> compTypes;

  // Scan for NodeGroupPhraseContent entries and add them to the map.
  int ii = -1;
  for (const auto& resourcePhrase : localRoot->subphrases())
  {
    ++ii;
    auto* resource = dynamic_cast<smtk::markup::Resource*>(resourcePhrase->relatedResource().get());
    if (!resource)
    {
      continue;
    }
    std::map<std::string, Entry> blank;
    compTypes[resource] = blank;
    int jj = -1;
    for (const auto& subphrase : resourcePhrase->subphrases())
    {
      ++jj;
      if (
        auto* nodeGroup =
          dynamic_cast<smtk::view::NodeGroupPhraseContent*>(subphrase->content().get()))
      {
        compTypes[resource][nodeGroup->childType()] = Entry{ { ii, jj }, subphrase };
      }
    }
  }

  // Split objects into markup vs non-markup objects:
  smtk::resource::PersistentObjectArray markupObjects;
  smtk::resource::PersistentObjectArray otherObjects;
  for (const auto& object : objects)
  {
    if (
      dynamic_cast<smtk::markup::Component*>(object.get()) ||
      dynamic_cast<smtk::markup::Resource*>(object.get()))
    {
      markupObjects.push_back(object);
    }
    else
    {
      otherObjects.push_back(object);
    }
  }

  // Use the superclass to process non-markup objects.
  this->Superclass::subphrasesForCreatedObjects(otherObjects, localRoot, resultingPhrases);

  // Now we can look up where to add each markup object.
  auto phrasesForObject(
    localRoot->phraseModel()->uuidPhraseMap()); // TODO: Fix SMTK to return const ref?
  std::vector<int> objectPath;
  for (const auto& object : markupObjects)
  {
    auto comp = std::dynamic_pointer_cast<Component>(object);
    if (!comp && !dynamic_cast<smtk::markup::Resource*>(object.get()))
    {
      this->subphrasesForCreatedObjects(objects, localRoot, resultingPhrases);
      continue;
    }
    auto* resource = dynamic_cast<smtk::markup::Resource*>(comp->parentResource());
    if (!resource)
    {
      continue;
    }
    auto resourceIt = compTypes.find(resource);
    if (resourceIt == compTypes.end())
    {
      continue;
    }
    std::string quotedTypeName = "'" + comp->typeName() + "'";
    auto compTypeIt = resourceIt->second.find(quotedTypeName);
    if (compTypeIt == resourceIt->second.end())
    {
      continue;
    }
    objectPath =
      this->indexOfObjectInParent(object, compTypeIt->second.phrase, compTypeIt->second.path);
    resultingPhrases.insert(std::make_pair(
      objectPath,
      ComponentPhraseContent::createPhrase(comp, phraseMutability, compTypeIt->second.phrase)));

    // Now, in addition to the object's entry under its proper NodeGroupPhraseContent,
    // we need to consider when an object must also appear as the child of another
    // object. Examples: labels are children of the component they annotate; any
    // components may be of a group.
    comp->incoming<arcs::LabelsToSubjects>().visit([&](const Label* parent) {
      for (const auto& weakPhrase : phrasesForObject[parent->id()])
      {
        if (auto parentPhrase = weakPhrase.lock())
        {
          objectPath = this->indexOfObjectInParent(comp, parentPhrase, parentPhrase->index());
          if (!objectPath.empty())
          {
            resultingPhrases.insert(std::make_pair(
              objectPath,
              ComponentPhraseContent::createPhrase(comp, phraseMutability, parentPhrase)));
          }
        }
      }
    });
    comp->incoming<arcs::GroupsToMembers>().visit([&](const Group* parent) {
      for (const auto& weakPhrase : phrasesForObject[parent->id()])
      {
        if (auto parentPhrase = weakPhrase.lock())
        {
          objectPath = this->indexOfObjectInParent(comp, parentPhrase, parentPhrase->index());
          if (!objectPath.empty())
          {
            resultingPhrases.insert(std::make_pair(
              objectPath,
              ComponentPhraseContent::createPhrase(comp, phraseMutability, parentPhrase)));
          }
        }
      }
    });
    // TODO: Add shape as a child to any relevant groups/styles.
    //       This should follow the pattern used for cusps above.
  }
}

SubphraseGenerator::PhrasePath SubphraseGenerator::indexOfObjectInParent(
  const smtk::resource::PersistentObjectPtr& obj,
  const smtk::view::DescriptivePhrasePtr& parent,
  const PhrasePath& parentPath)
{
  if (
    !dynamic_cast<smtk::markup::Component*>(obj.get()) &&
    !dynamic_cast<smtk::markup::Resource*>(obj.get()))
  {
    return this->Superclass::indexOfObjectInParent(obj, parent, parentPath);
  }

  const auto& subphrases = parent->subphrases();
  // TODO: We could bisect here (since phrases are assumed ordered).
  //       That would be much faster than this O(N) algorithm.

  SubphraseGenerator::PhrasePath result = parentPath;
  std::string objName = obj->name();
  int ii = 0;
  for (const auto& phrase : subphrases)
  {
    if (smtk::common::StringUtil::mixedAlphanumericComparator(objName, phrase->title()))
    {
      result.push_back(ii);
      return result;
    }
    ++ii;
  }
  result.push_back(static_cast<int>(subphrases.size()));
  return result;
}

} // namespace markup
} // namespace smtk
