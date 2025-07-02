//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/Manager.h"
#include "smtk/view/ObjectGroupPhraseContent.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseContent.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/UseEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/view/SubphraseGenerator.txx"

#include <algorithm>
//required for insert_iterator on VS2010+
#include <iterator>

namespace smtk
{
namespace view
{

std::string SubphraseGenerator::getType(const smtk::view::ConfigurationPtr& viewSpec)
{
  std::string typeName;
  if (!viewSpec || viewSpec->name().empty() || viewSpec->name() != "ResourceBrowser")
  {
    return typeName;
  }
  // Look at viewSpec child, should be "PhraseModel", look for its child
  if (
    (viewSpec->details().numberOfChildren() != 1) ||
    (viewSpec->details().child(0).name() != "PhraseModel"))
  {
    return typeName;
  }
  auto model = viewSpec->details().child(0);
  if (model.numberOfChildren() == 0)
  {
    typeName = "default";
  }
  else if (model.child(0).name() != "SubphraseGenerator")
  {
    return typeName;
  }
  model.child(0).attribute("Type", typeName);
  return typeName;
}

SubphraseGeneratorPtr SubphraseGenerator::create(
  const std::string& typeName,
  const smtk::view::ManagerPtr& manager)
{
  if (!manager || typeName.empty())
  {
    return nullptr;
  }

  // find things that match the typeName, and create one.
  return manager->subphraseGeneratorFactory().createFromName(typeName);
}

SubphraseGenerator::SubphraseGenerator()
{
  m_directLimit = -1;
  m_skipAttributes = false;
  m_skipProperties = false;
}

smtk::resource::PersistentObjectSet SubphraseGenerator::parentObjects(
  const smtk::resource::PersistentObjectPtr& obj) const
{
  smtk::resource::PersistentObjectSet result;

  auto res = dynamic_pointer_cast<smtk::resource::Resource>(obj);
  if (res)
  {
    return result; // Resources have no parents
  }

  auto attribute = dynamic_pointer_cast<smtk::attribute::Attribute>(obj);
  if (attribute)
  {
    // The parent of an attribute is its resource
    result.insert(attribute->resource());
    return result;
  }

  // If this is not a model entity then just assume it's parent is its resource else
  // return nothing (i.e. put it under the root)
  auto ment = dynamic_pointer_cast<smtk::model::Entity>(obj);
  if (!ment)
  {
    auto comp = dynamic_pointer_cast<smtk::resource::Component>(obj);
    if (comp)
    {
      result.insert(comp->resource());
    }
    return result;
  }
  // If its the model then add it's parent or the resource if it doesn't have it
  if (ment->isModel())
  {
    if (smtk::model::Model(ment).owningModel().isValid())
    {
      result.insert(smtk::model::Model(ment).owningModel().component());
    }
  }
  // If its a group, see if it has a valid parent
  else if (ment->isGroup())
  {
    if (smtk::model::Group(ment).parent().isValid())
    {
      result.insert(smtk::model::Group(ment).parent().component());
    }
  }
  // If its a cell entity, get its bordants
  else if (ment->isCellEntity())
  {
    smtk::model::EntityRefs pEnts = smtk::model::EntityRef(ment).bordantEntities();
    for (const auto& ent : pEnts)
    {
      if (ent.isValid())
      {
        auto comp = ent.component();
        if (comp)
        {
          result.insert(comp);
        }
      }
    }
  }
  // If its Auxiliary Geometry, see if it has a valid model
  else if (ment->isAuxiliaryGeometry())
  {
    auto myModel = ment->owningModel();
    if (myModel)
    {
      result.insert(myModel);
    }
  }
  // If its an Instance, see if it has a valid prototype
  else if (ment->isInstance())
  {
    if (smtk::model::Instance(ment).prototype().isValid())
    {
      result.insert(smtk::model::Instance(ment).prototype().component());
    }
  }
  return result;
}
DescriptivePhrases SubphraseGenerator::subphrases(DescriptivePhrase::Ptr src)
{
  DescriptivePhrases result;
  if (src)
  {
    auto comp = src->relatedComponent();
    if (!comp)
    {
      auto rsrc = src->relatedResource();
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
    else
    {
      auto attr = dynamic_pointer_cast<smtk::attribute::Attribute>(comp);
      auto modelEnt = dynamic_pointer_cast<smtk::model::Entity>(comp);
      if (attr)
      {
        SubphraseGenerator::itemsOfAttribute(src, attr, result);
      }
      else if (modelEnt)
      {
        SubphraseGenerator::childrenOfModelEntity(src, modelEnt, result);
      }
    }
  }
  return result;
}

bool SubphraseGenerator::hasChildren(const DescriptivePhrase& src) const
{
  auto comp = src.relatedComponent();
  if (!comp)
  {
    auto rsrc = src.relatedResource();
    if (!rsrc)
    {
      PhraseContentPtr content = src.content();
      auto ogpc = std::dynamic_pointer_cast<smtk::view::ObjectGroupPhraseContent>(content);
      if (ogpc)
      {
        return ogpc->hasChildren();
      }
      return false; // Don't know what this is
    }
    return SubphraseGenerator::resourceHasChildren(rsrc);
  }

  // OK we have a component
  auto attr = dynamic_pointer_cast<smtk::attribute::Attribute>(comp);
  if (attr)
  {
    // Currently we don't show an Attribute's Items - if we did we would
    // return attr->numberOfItems() > 0
    return false;
  }
  auto modelEnt = dynamic_pointer_cast<smtk::model::Entity>(comp);
  if (modelEnt)
  {
    return SubphraseGenerator::modelEntityHasChildren(modelEnt);
  }
  return false;
}

bool SubphraseGenerator::setModel(PhraseModelPtr model)
{
  auto existing = m_model.lock();
  if (model == existing)
  {
    return false;
  }

  m_model = model;
  return true;
}

template<typename T>
int MutabilityOfComponent(const T& comp)
{
  constexpr int modelMutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
    static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);
  constexpr int attrMutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE);

  if (std::dynamic_pointer_cast<smtk::model::Entity>(comp))
  {
    return modelMutability;
  }
  else if (std::dynamic_pointer_cast<smtk::attribute::Attribute>(comp))
  {
    return attrMutability;
  }

  // We really should see if NamingGroup(operationManager).matchingOperation(comp) is non-zero.
  // However, fetching an operation manager would be really expensive to do for many phrases
  // because we would have to do shared-ptr locking on the phrase model and operation manager.
  // Currently, all the sessions we know about support renaming components but not all support
  // coloring objects, so avoid this overhead for graph-based models:
  return static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE);
}

template<typename T>
int MutabilityOfObject(const T& obj)
{
  constexpr int resourceMutability =
    static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
    static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);

  if (std::dynamic_pointer_cast<smtk::resource::Resource>(obj))
  {
    return resourceMutability;
  }
  else if (std::dynamic_pointer_cast<smtk::resource::Component>(obj))
  {
    return MutabilityOfComponent(obj);
  }

  return 0;
}

DescriptivePhrasePtr SubphraseGenerator::createSubPhrase(
  const smtk::resource::PersistentObjectPtr& obj,
  const DescriptivePhrasePtr& parent,
  Path& childPath)
{
  smtk::resource::ResourcePtr rsrc;
  smtk::resource::ComponentPtr comp;
  Path parentPath;
  parent->index(parentPath);
  childPath = parent->findDelegate()->indexOfObjectInParent(obj, parent, parentPath);

  if (childPath.empty())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Child object's parent phrase could not position child.");
    return DescriptivePhrasePtr();
  }

  DescriptivePhrasePtr child;
  if ((comp = obj->as<smtk::resource::Component>()))
  {
    // Only add components with valid ids
    if (comp->id())
    {
      return ComponentPhraseContent::createPhrase(comp, MutabilityOfComponent(comp), parent);
    }
  }
  else if ((rsrc = obj->as<smtk::resource::Resource>()))
  {
    return ResourcePhraseContent::createPhrase(rsrc, MutabilityOfObject(rsrc), parent);
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Unsupported object type. Skipping.");
  }
  return DescriptivePhrasePtr();
}

void SubphraseGenerator::subphrasesForCreatedObjects(
  const smtk::resource::PersistentObjectArray& objects,
  const DescriptivePhrasePtr& localRoot,
  PhrasesByPath& resultingPhrases)
{
  if (!localRoot || !localRoot->areSubphrasesBuilt())
  {
    return;
  }

  // We only want to add subphrases in portions of the tree that already exist.
  // So we iterate over phrases; any phrase that has children will ask each object
  // if its subject should be an immediate parent of the phrase with children (and,
  // if so, what its index in the existing children should be).
  //
  // This search is O(m*n) where m = number of phrases in tree and n = number of
  // objects created.
  localRoot->visitChildren([&](DescriptivePhrasePtr parent, std::vector<int>& parentPath) -> int {
    // Make sure the parent's subphrases are built
    parent->subphrases();

    smtk::resource::ResourcePtr rsrc;
    smtk::resource::ComponentPtr comp;
    for (const auto& obj : objects)
    {
      Path childPath = parent->findDelegate()->indexOfObjectInParent(obj, parent, parentPath);
      if (childPath.empty())
      {
        continue;
      } // obj is not a direct child of parent

      DescriptivePhrasePtr child;
      if ((comp = obj->as<smtk::resource::Component>()))
      {
        // Only add components with valid ids
        if (comp->id())
        {
          child = ComponentPhraseContent::createPhrase(comp, MutabilityOfComponent(comp), parent);
          resultingPhrases.insert(std::make_pair(childPath, child));
        }
      }
      else if ((rsrc = obj->as<smtk::resource::Resource>()))
      {
        child = ResourcePhraseContent::createPhrase(rsrc, MutabilityOfObject(rsrc), parent);
        resultingPhrases.insert(std::make_pair(childPath, child));
      }
      else
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "Unsupported object type. Skipping.");
      }
    }
    return 0; // 0 => continue iterating, 1 => skip children of parent, 2 => terminate
  });
}

int SubphraseGenerator::directLimit() const
{
  return m_directLimit;
}

bool SubphraseGenerator::setDirectLimit(int val)
{
  if (val != 0)
  {
    m_directLimit = val;
    return true;
  }
  return false;
}

bool SubphraseGenerator::shouldOmitProperty(
  DescriptivePhrase::Ptr parent,
  smtk::resource::PropertyType ptype,
  const std::string& pname) const
{
  (void)parent;
  (void)ptype;
  (void)pname;
  return false;
}

bool SubphraseGenerator::skipProperties() const
{
  return m_skipProperties;
}
void SubphraseGenerator::setSkipProperties(bool val)
{
  m_skipProperties = val;
}

bool SubphraseGenerator::skipAttributes() const
{
  return m_skipAttributes;
}
void SubphraseGenerator::setSkipAttributes(bool val)
{
  m_skipAttributes = val;
}

SubphraseGenerator::Path SubphraseGenerator::indexOfObjectInParent(
  const smtk::resource::PersistentObjectPtr& obj,
  const smtk::view::DescriptivePhrasePtr& actualParent,
  const Path& parentPath)
{
  // The default subphrase generator will never have resources as children
  // of anything, so unless obj is a component, we do not assign it a path:
  Path result;
  auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
  if (!actualParent || !comp)
  {
    return result;
  }

  smtk::resource::ResourcePtr rsrc;
  smtk::attribute::AttributePtr attr;
  smtk::model::EntityPtr ment;
  bool added = false;
  // Determine if the component is a direct-ish child of parent
  if (!actualParent->relatedComponent() && actualParent->relatedResource())
  {
    rsrc = comp->resource();
    // Attribute resources own all their components directly.
    // Model resources have only _free_ models as direct children.
    if (rsrc == actualParent->relatedResource())
    {
      if (
        std::dynamic_pointer_cast<smtk::attribute::Attribute>(comp) ||
        ((ment = std::dynamic_pointer_cast<smtk::model::Entity>(comp)) && ment->isModel() &&
         !smtk::model::Model(ment).owningModel().isValid()))
      {
        PreparePath(result, parentPath, IndexFromTitle(comp->name(), actualParent->subphrases()));
        added = true;
      }
      else if (
        !std::dynamic_pointer_cast<smtk::attribute::Resource>(rsrc) &&
        !std::dynamic_pointer_cast<smtk::model::Resource>(rsrc))
      {
        // Resources of unknown type... plop all the components in the top-level
        PreparePath(result, parentPath, IndexFromTitle(comp->name(), actualParent->subphrases()));
        added = true;
      }
    }
  }
  if (
    !added &&
    (ment = std::dynamic_pointer_cast<smtk::model::Entity>(actualParent->relatedComponent())))
  {
    bool shouldAdd = false;
    const auto& parentEntity = ment;
    auto childEntity = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
    if (childEntity)
    {
      smtk::model::EntityRef childRef(childEntity);
      if (childEntity->isCellEntity())
      {
        // Is the parent an owning group?
        if (parentEntity->isGroup())
        {
          auto groups = childRef.containingGroups();
          for (const auto& group : groups)
          {
            if (group.entity() == parentEntity->id())
            {
              shouldAdd = true;
              break;
            }
          }
        }
        else if (parentEntity->isCellEntity())
        {
          auto bordants = childRef.bordantEntities();
          if (bordants.find(smtk::model::EntityRef(parentEntity)) != bordants.end())
          {
            shouldAdd = true;
          }
        }
        else if (parentEntity->isModel())
        {
          smtk::model::Model parentModel(parentEntity);
          auto cells = parentModel.cells();
          for (const auto& cell : cells)
          {
            if (cell.entity() == childRef.entity())
            {
              shouldAdd = true;
              break;
            }
          }
        }
      }
      else if (childEntity->isGroup())
      {
        // Is the parent a group that owns this group?
        auto groups = childRef.containingGroups();
        for (const auto& group : groups)
        {
          if (group.entity() == parentEntity->id())
          {
            shouldAdd = true;
            break;
          }
        }
        // Is the parent a model that owns this group?
        if (!shouldAdd && parentEntity->isModel())
        {
          groups = smtk::model::Model(parentEntity).groups();
          for (const auto& group : groups)
          {
            if (group.entity() == childRef.entity())
            {
              shouldAdd = true;
              break;
            }
          }
        }
      }
      else if (childEntity->isAuxiliaryGeometry())
      {
        if (parentEntity->isModel())
        {
          auto auxGeoms = parentEntity->referenceAs<smtk::model::Model>().auxiliaryGeometry();
          std::set<smtk::model::AuxiliaryGeometry> searchable(auxGeoms.begin(), auxGeoms.end());
          if (
            searchable.find(childEntity->referenceAs<smtk::model::AuxiliaryGeometry>()) !=
            searchable.end())
          {
            shouldAdd = true;
          }
        }
        else if (parentEntity->isAuxiliaryGeometry())
        {
          auto auxGeoms =
            parentEntity->referenceAs<smtk::model::AuxiliaryGeometry>().auxiliaryGeometries();
          std::set<smtk::model::AuxiliaryGeometry> searchable(auxGeoms.begin(), auxGeoms.end());
          if (
            searchable.find(childEntity->referenceAs<smtk::model::AuxiliaryGeometry>()) !=
            searchable.end())
          {
            shouldAdd = true;
          }
        }
        else if (parentEntity->isGroup())
        {
          auto members =
            parentEntity->referenceAs<smtk::model::Group>().members<smtk::model::EntityRefs>();
          if (
            members.find(childEntity->referenceAs<smtk::model::AuxiliaryGeometry>()) !=
            members.end())
          {
            shouldAdd = true;
          }
        }
      }
      else if (
        childEntity->isInstance() &&
        smtk::model::Instance(childEntity).prototype().entity() == parentEntity->id())
      {
        shouldAdd = true;
      }
    }
    if (shouldAdd)
    {
      added = true;
      PreparePath(result, parentPath, IndexFromTitle(comp->name(), actualParent->subphrases()));
    }
  }
  return result;
}

int SubphraseGenerator::findResourceLocation(
  smtk::resource::ResourcePtr rsrc,
  const DescriptivePhrase::Ptr& root) const
{
  if (!root || !rsrc)
  {
    return -1;
  }

  int ii = 0;
  for (const auto& phrase : root->subphrases())
  {
    if (phrase->relatedResource() == rsrc)
    {
      return ii;
    }
    ++ii;
  }
  return -1;
}

bool SubphraseGenerator::findSortedLocation(
  Path& pathOut,
  smtk::attribute::AttributePtr attr,
  DescriptivePhrase::Ptr& phr,
  const DescriptivePhrase::Ptr& parent) const
{
  (void)phr;
  if (!attr || !parent || !parent->areSubphrasesBuilt())
  {
    // If the user has not opened the parent phrase, do not
    // add to it; when subphrases are generated later (on demand)
    // they should include \a attr.
    return false;
  }

  const auto& phrases = parent->subphrases();
  int ii = 0;

  // For now, attributes are flat, so pathOut is easy.
  for (const auto& phrase : phrases)
  {
    if (phrase->title() > attr->name())
    {
      pathOut.back() = ii;
      return true;
    }
    ++ii;
  }
  pathOut.back() = ii;
  return true;
}

bool SubphraseGenerator::findSortedLocation(
  Path& pathInOut,
  smtk::model::EntityPtr entity,
  DescriptivePhrase::Ptr& phr,
  const DescriptivePhrase::Ptr& parent) const
{
  (void)phr;
  if (!entity || !parent || !parent->areSubphrasesBuilt())
  {
    // If the user has not opened the parent phrase, do not
    // add to it; when subphrases are generated later (on demand)
    // they should include \a attr.
    return false;
  }
  smtk::model::BitFlags entityType = entity->entityFlags() & smtk::model::ENTITY_MASK;
  int rsrcIdx = pathInOut[0];
  switch (entityType)
  {
    case smtk::model::CELL_ENTITY:
      pathInOut = Path{ rsrcIdx, 0 };
      phr->reparent(parent);
      return true;
      break;
    default:
      return false;
      break;
  }
  (void)pathInOut;
  return false;
}

void SubphraseGenerator::componentsOfResource(
  DescriptivePhrase::Ptr src,
  smtk::resource::ResourcePtr rsrc,
  DescriptivePhrases& result)
{
  auto modelRsrc = dynamic_pointer_cast<smtk::model::Resource>(rsrc);
  auto attrRsrc = dynamic_pointer_cast<smtk::attribute::Resource>(rsrc);
  if (modelRsrc)
  {
    // By default, make model component names and colors editable but not visibility
    // as that is handled by modelbuilder/paraview on a per-view basis.
    constexpr int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
      static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);
    auto models =
      modelRsrc->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);
    for (const auto& model : models)
    {
      result.push_back(ComponentPhraseContent::createPhrase(model.component(), mutability, src));
    }
  }
  else if (attrRsrc)
  {
    constexpr int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);
    std::vector<smtk::attribute::AttributePtr> attrs;
    attrRsrc->attributes(attrs);
    for (const auto& attr : attrs)
    {
      result.push_back(ComponentPhraseContent::createPhrase(attr, mutability, src));
    }
  }
  else
  { // Some random resource...
    // By default, make names and colors editable but not visibility
    // as that is handled by modelbuilder/paraview on a per-view basis.
    constexpr int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
      static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);

// MSVC cannot access constexpr PODs inside a lambda without explicit
// capture rules. Other compilers consider the explicit rules unnecessary,
// and they warn about it.
#ifdef SMTK_MSVC
    smtk::resource::Component::Visitor visitor =
      [mutability, &result, &src](
#else
    smtk::resource::Component::Visitor visitor =
      [&result, &src](
#endif
        const smtk::resource::Component::Ptr& component) {
        result.push_back(ComponentPhraseContent::createPhrase(component, mutability, src));
      };
    rsrc->visit(visitor);
  }
  std::sort(result.begin(), result.end(), DescriptivePhrase::compareByTitle);
}

bool SubphraseGenerator::resourceHasChildren(const smtk::resource::ResourcePtr& rsrc) const
{
  auto modelRsrc = dynamic_pointer_cast<smtk::model::Resource>(rsrc);
  if (modelRsrc)
  {
    auto models =
      modelRsrc->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);
    return !models.empty();
  }

  auto attrRsrc = dynamic_pointer_cast<smtk::attribute::Resource>(rsrc);
  if (attrRsrc)
  {
    return attrRsrc->hasAttributes();
  }

  // Some random resource...
  bool hasChild = false;
  smtk::resource::Component::Visitor visitor = [&hasChild](const smtk::resource::Component::Ptr&) {
    hasChild = true;
  };
  rsrc->visit(visitor);
  return hasChild;
}

void SubphraseGenerator::itemsOfAttribute(
  DescriptivePhrase::Ptr src,
  smtk::attribute::AttributePtr att,
  DescriptivePhrases& result)
{
  (void)att;
  (void)src;
  (void)result;
  // TODO: Need an AttributeItemPhrase
}

bool SubphraseGenerator::modelEntityHasChildren(const smtk::model::EntityPtr& entity) const
{
  smtk::model::BitFlags entityFlags = entity->entityFlags();
  // WARNING: GROUP_ENTITY must go first since other bits may be set for groups in \a entityFlags
  if (entityFlags & smtk::model::GROUP_ENTITY)
  {
    auto group = entity->referenceAs<smtk::model::Group>();
    auto groups = group.members<smtk::model::EntityRefArray>();
    if (!groups.empty())
    {
      return true;
    }
  }
  else if (entityFlags & smtk::model::MODEL_ENTITY)
  {
    auto model = entity->referenceAs<smtk::model::Model>();
    auto submodelsInModel = model.submodels();
    if (!submodelsInModel.empty())
    {
      return true;
    }
    auto groupsInModel = model.groups();
    if (!groupsInModel.empty())
    {
      return true;
    }
    auto auxGeomsInModel = model.auxiliaryGeometry();
    if (!auxGeomsInModel.empty())
    {
      return true;
    }
    auto cellsInModel = model.cells();
    if (!cellsInModel.empty())
    {
      return true;
    }
  }
  else if (entityFlags & smtk::model::CELL_ENTITY)
  {
    auto cell = entity->referenceAs<smtk::model::CellEntity>();
    auto boundingCells = cell.boundingCells();
    if (!boundingCells.empty())
    {
      return true;
    }
    auto inclusions = cell.inclusions<smtk::model::EntityRefs>();
    if (!inclusions.empty())
    {
      return true;
    }
  }
  else if (entityFlags & smtk::model::AUX_GEOM_ENTITY)
  {
    auto auxGeom = entity->referenceAs<smtk::model::AuxiliaryGeometry>();
    auto auxChildren = auxGeom.embeddedEntities<smtk::model::AuxiliaryGeometries>();
    if (!auxChildren.empty())
    {
      return true;
    }
  }
  // To avoid infinite nesting we currently don't look at the children
  // of instances

  // TODO: Finish handling other model-entity types

  auto ref = entity->referenceAs<smtk::model::EntityRef>();
  // Any entity may have instances
  auto instances = ref.instances<smtk::model::InstanceEntities>();
  return (!instances.empty());
}

void SubphraseGenerator::childrenOfModelEntity(
  DescriptivePhrase::Ptr src,
  smtk::model::EntityPtr entity,
  DescriptivePhrases& result)
{
  smtk::model::BitFlags entityFlags = entity->entityFlags();
  // WARNING: GROUP_ENTITY must go first since other bits may be set for groups in \a entityFlags
  if (entityFlags & smtk::model::GROUP_ENTITY)
  {
    auto group = entity->referenceAs<smtk::model::Group>();
    this->membersOfModelGroup(src, group, result);
  }
  else if (entityFlags & smtk::model::MODEL_ENTITY)
  {
    auto model = entity->referenceAs<smtk::model::Model>();
    this->freeSubmodelsOfModel(src, model, result);
    this->freeGroupsOfModel(src, model, result);
    this->freeAuxiliaryGeometriesOfModel(src, model, result);
    this->freeCellsOfModel(src, model, result);
  }
  else if (entityFlags & smtk::model::CELL_ENTITY)
  {
    auto cell = entity->referenceAs<smtk::model::CellEntity>();
    this->boundingCellsOfModelCell(src, cell, result);
    this->inclusionsOfModelCell(src, cell, result);
  }
  else if (entityFlags & smtk::model::AUX_GEOM_ENTITY)
  {
    auto auxGeom = entity->referenceAs<smtk::model::AuxiliaryGeometry>();
    this->childrenOfModelAuxiliaryGeometry(src, auxGeom, result);
  }
  else if (entityFlags & smtk::model::INSTANCE_ENTITY)
  {
    auto instance = entity->referenceAs<smtk::model::Instance>();
    this->prototypeOfModelInstance(src, instance, result);
  }
  // TODO: Finish handling other model-entity types

  auto ref = entity->referenceAs<smtk::model::EntityRef>();
  // Any entity may have instances
  // TODO:  There is a performance issue when dealing with instances
  // which needs to be addressed.  Since instances are not currently a
  // high priority, this was commented out for the time being.
  //this->instancesOfModelEntity(src, ref, result);
  // Any entity may have associated attributes
  // this->attributesOfModelEntity(src, ref, result);

  // Sort the result
  if (!result.empty())
  {
    std::sort(result.begin(), result.end(), DescriptivePhrase::compareByTypeThenTitle);
  }
}

void SubphraseGenerator::freeSubmodelsOfModel(
  DescriptivePhrase::Ptr src,
  const smtk::model::Model& mod,
  DescriptivePhrases& result)
{
  auto freeSubmodelsInModel = mod.submodels();
  this->filterModelEntityPhraseCandidates(freeSubmodelsInModel);
  this->addModelEntityPhrases(freeSubmodelsInModel, src, this->directLimit(), result);
}

void SubphraseGenerator::freeGroupsOfModel(
  DescriptivePhrase::Ptr src,
  const smtk::model::Model& mod,
  DescriptivePhrases& result)
{
  auto freeGroups = mod.groups();
  this->filterModelEntityPhraseCandidates(freeGroups);
  this->addModelEntityPhrases(freeGroups, src, this->directLimit(), result);
}

void SubphraseGenerator::freeCellsOfModel(
  DescriptivePhrase::Ptr src,
  const smtk::model::Model& mod,
  DescriptivePhrases& result)
{
  auto freeCellsInModel = mod.cells();
  this->filterModelEntityPhraseCandidates(freeCellsInModel);
  this->addModelEntityPhrases(freeCellsInModel, src, this->directLimit(), result);
}

void SubphraseGenerator::freeAuxiliaryGeometriesOfModel(
  DescriptivePhrase::Ptr src,
  const smtk::model::Model& mod,
  DescriptivePhrases& result)
{
  auto freeAuxGeom = mod.auxiliaryGeometry();
  this->filterModelEntityPhraseCandidates(freeAuxGeom);
  this->addModelEntityPhrases(freeAuxGeom, src, this->directLimit(), result);
}

void SubphraseGenerator::cellOfModelUse(
  DescriptivePhrase::Ptr src,
  const smtk::model::UseEntity& ent,
  DescriptivePhrases& result)
{
  auto parentCell = ent.cell();
  if (parentCell.isValid())
  {
    constexpr int mutability = static_cast<int>(smtk::view::PhraseContent::ContentType::TITLE) |
      static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);
    result.push_back(ComponentPhraseContent::createPhrase(parentCell.component(), mutability, src));
  }
}

void SubphraseGenerator::boundingShellsOfModelUse(
  DescriptivePhrase::Ptr src,
  const smtk::model::UseEntity& ent,
  DescriptivePhrases& result)
{
  smtk::model::ShellEntities boundingShells =
    ent.boundingShellEntities<smtk::model::ShellEntities>();
  this->filterModelEntityPhraseCandidates(boundingShells);
  this->addModelEntityPhrases(boundingShells, src, this->directLimit(), result);
}

void SubphraseGenerator::toplevelShellsOfModelUse(
  DescriptivePhrase::Ptr src,
  const smtk::model::UseEntity& ent,
  DescriptivePhrases& result)
{
  auto toplevelShells = ent.shellEntities<smtk::model::ShellEntities>();
  this->filterModelEntityPhraseCandidates(toplevelShells);
  this->addModelEntityPhrases(toplevelShells, src, this->directLimit(), result);
}

void SubphraseGenerator::usesOfModelCell(
  DescriptivePhrase::Ptr src,
  const smtk::model::CellEntity& ent,
  DescriptivePhrases& result)
{
  auto cellUses = ent.uses<smtk::model::UseEntities>();
  this->filterModelEntityPhraseCandidates(cellUses);
  this->addModelEntityPhrases(cellUses, src, this->directLimit(), result);
}

void SubphraseGenerator::inclusionsOfModelCell(
  DescriptivePhrase::Ptr src,
  const smtk::model::CellEntity& ent,
  DescriptivePhrases& result)
{
  auto inclusions = ent.inclusions<smtk::model::EntityRefs>();
  auto boundingCells = ent.boundingCells();
  smtk::model::EntityRefArray strictInclusions;
  std::set_difference(
    inclusions.begin(),
    inclusions.end(),
    boundingCells.begin(),
    boundingCells.end(),
    std::inserter(strictInclusions, strictInclusions.end()));
  this->filterModelEntityPhraseCandidates(strictInclusions);
  this->addModelEntityPhrases(strictInclusions, src, this->directLimit(), result);
}

void SubphraseGenerator::boundingCellsOfModelCell(
  DescriptivePhrase::Ptr src,
  const smtk::model::CellEntity& ent,
  DescriptivePhrases& result)
{
  auto boundingCells = ent.boundingCells();
  this->filterModelEntityPhraseCandidates(boundingCells);
  this->addModelEntityPhrases(boundingCells, src, this->directLimit(), result);
}

void SubphraseGenerator::usesOfModelShell(
  DescriptivePhrase::Ptr src,
  const smtk::model::ShellEntity& ent,
  DescriptivePhrases& result)
{
  auto shellUses = ent.uses<smtk::model::UseEntities>();
  this->filterModelEntityPhraseCandidates(shellUses);
  this->addModelEntityPhrases(shellUses, src, this->directLimit(), result);
}

void SubphraseGenerator::membersOfModelGroup(
  DescriptivePhrase::Ptr src,
  const smtk::model::Group& grp,
  DescriptivePhrases& result)
{
  auto members = grp.members<smtk::model::EntityRefArray>();
  this->filterModelEntityPhraseCandidates(members);
  this->addModelEntityPhrases(members, src, this->directLimit(), result);
  // TODO: Sort by entity type, name, etc.?
}

void SubphraseGenerator::childrenOfModelAuxiliaryGeometry(
  DescriptivePhrase::Ptr src,
  const smtk::model::AuxiliaryGeometry& aux,
  DescriptivePhrases& result)
{
  auto children = aux.embeddedEntities<smtk::model::AuxiliaryGeometries>();
  for (const auto& child : children)
  {
    result.push_back(ComponentPhraseContent::createPhrase(child.component(), 0, src));
  }
}

void SubphraseGenerator::prototypeOfModelInstance(
  DescriptivePhrase::Ptr /*src*/,
  const smtk::model::Instance& /*ent*/,
  DescriptivePhrases& /*result*/)
{
  // For now do nothing here to prevent the generation of infinite nested prototype/glyph phrases.
}

void SubphraseGenerator::instancesOfModelEntity(
  DescriptivePhrase::Ptr src,
  const smtk::model::EntityRef& ent,
  DescriptivePhrases& result)
{
  auto instances = ent.instances<smtk::model::InstanceEntities>();
  this->filterModelEntityPhraseCandidates(instances);
  this->addModelEntityPhrases(instances, src, this->directLimit(), result);
}

#if 0

void SubphraseGenerator::attributesOfModelEntity(
  DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result)
{
  if (!m_skipAttributes && ent.hasAttributes())
  {
    result.push_back(AttributeListPhraseContent::createPhrase(ent, ent.attributeIds(), src));
  }
}


/**\brief Set the maximum number of direct children before a summary phrase is inserted.
  *
  * This is used to add a layer of indirection to the hierarchy so that
  * long lists are not inadvertently opened and so that a parent which would
  * otherwise have many children of many different kinds can group its
  * children to allow easier browsing.
  *
  * A negative value indicates that no limit should be imposed (no summary
  * phrases will ever be generated).
  */

void SubphraseGenerator::propertiesOfComponent(
  DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result)
{
  if (!m_skipProperties)
  {
    this->stringPropertiesOfComponent(src, ent, result);
    this->integerPropertiesOfComponent(src, ent, result);
    this->floatPropertiesOfComponent(src, ent, result);
  }
}

void SubphraseGenerator::floatPropertiesOfComponent(
  DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result)
{
  std::set<std::string> pnames = ent.floatPropertyNames();
  if (!pnames.empty())
  {
    this->addEntityProperties(smtk::resource::FLOAT_PROPERTY, pnames, src, result);
  }
}

void SubphraseGenerator::stringPropertiesOfComponent(
  DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result)
{
  std::set<std::string> pnames = ent.stringPropertyNames();
  if (!pnames.empty())
  {
    this->addEntityProperties(smtk::resource::STRING_PROPERTY, pnames, src, result);
  }
}

void SubphraseGenerator::integerPropertiesOfComponent(
  DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result)
{
  std::set<std::string> pnames = ent.integerPropertyNames();
  if (!pnames.empty())
  {
    this->addEntityProperties(smtk::resource::INTEGER_PROPERTY, pnames, src, result);
  }
}

void SubphraseGenerator::modelsOfModelSession(
  DescriptivePhrase::Ptr src, const SessionRef& sess, DescriptivePhrases& result)
{
  // We need the models to be unique, no duplicated entries.
  std::set<smtk::model::Model> modelsOf = sess.models<std::set<smtk::model::Model> >();
  this->addModelEntityPhrases(modelsOf, src, this->directLimit(), result);
}

/// Add subphrases (or a list of them) to \a result for the specified properties.
void SubphraseGenerator::addEntityProperties(
  smtk::resource::PropertyType ptype, std::set<std::string>& props,
  DescriptivePhrase::Ptr parnt, DescriptivePhrases& result)
{
  std::set<std::string>::const_iterator it;
  for (it = props.begin(); it != props.end();)
  {
    std::string pname = *it;
    ++it;
    if (this->shouldOmitProperty(parnt, ptype, pname))
      props.erase(pname);
  }
  // First, use the generator to prune entries we do not wish to display.
  // Now, add either the properties directly or as a list.
  if (this->directLimit() < 0 || static_cast<int>(props.size()) < this->directLimit())
  {
    for (it = props.begin(); it != props.end(); ++it)
    {
      result.push_back(PropertyValuePhraseContent::createPhrase(ptype, *it, parnt));
    }
  }
  else
  {
    result.push_back(
      PropertyListPhraseContent::createPhrase(parnt->relatedEntity(), ptype, props, parnt));
  }
}

#endif // 0

} // namespace view
} // namespace smtk
