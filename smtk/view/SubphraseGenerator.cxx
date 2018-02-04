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

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseContent.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/UseEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"

#include "smtk/view/SubphraseGenerator.txx"

#include <algorithm>
//required for insert_iterator on VS2010+
#include <iterator>

namespace smtk
{
namespace view
{

SubphraseGenerator::SubphraseGenerator()
{
  m_directLimit = 20;
  m_skipAttributes = false;
  m_skipProperties = false;
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
      if (rsrc)
      {
        SubphraseGenerator::componentsOfResource(src, rsrc, result);
      }
    }
    else
    {
      auto attr = dynamic_pointer_cast<smtk::attribute::Attribute>(comp);
      auto modelEnt = dynamic_pointer_cast<smtk::model::Entity>(comp);
      // auto meshSet = dynamic_pointer_cast<smtk::mesh::Component>(comp);
      if (attr)
      {
        SubphraseGenerator::itemsOfAttribute(src, attr, result);
      }
      else if (modelEnt)
      {
        SubphraseGenerator::childrenOfModelEntity(src, modelEnt, result);
      }
      // else if (meshSet)
      // {
      //   SubphraseGenerator::subsetsOfMeshSet(src, meshSet, result);
      // }
    }
  }
  this->decoratePhrases(result);
  return result;
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

void SubphraseGenerator::decoratePhrases(DescriptivePhrases& phrases)
{
  PhraseModelPtr mod = this->model();
  if (!mod)
  {
    return;
  }

  for (auto phrase : phrases)
  {
    mod->decoratePhrase(phrase);
  }
}

int SubphraseGenerator::directLimit() const
{
  return m_directLimit;
}

bool SubphraseGenerator::setDirectLimit(int val)
{
  if (val != 0)
  {
    this->m_directLimit = val;
    return true;
  }
  return false;
}

bool SubphraseGenerator::shouldOmitProperty(
  DescriptivePhrase::Ptr parent, smtk::resource::PropertyType ptype, const std::string& pname) const
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

void SubphraseGenerator::componentsOfResource(
  DescriptivePhrase::Ptr src, smtk::resource::ResourcePtr rsrc, DescriptivePhrases& result)
{
  auto modelRsrc = dynamic_pointer_cast<smtk::model::Manager>(rsrc);
  auto attrRsrc = dynamic_pointer_cast<smtk::attribute::Collection>(rsrc);
  //auto meshRsrc = dynamic_pointer_cast<smtk::mesh::Resource>(rsrc);
  if (modelRsrc)
  {
    auto models =
      modelRsrc->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);
    for (auto model : models)
    {
      result.push_back(ComponentPhraseContent::createPhrase(model.component(), 0, src));
    }
  }
  else if (attrRsrc)
  {
    std::vector<smtk::attribute::AttributePtr> attrs;
    attrRsrc->attributes(attrs);
    for (auto attr : attrs)
    {
      result.push_back(ComponentPhraseContent::createPhrase(attr, 0, src));
    }
  }
  // else if (meshRsrc)
  // {
  // }
}

void SubphraseGenerator::itemsOfAttribute(
  DescriptivePhrase::Ptr src, smtk::attribute::AttributePtr att, DescriptivePhrases& result)
{
  (void)att;
  (void)src;
  (void)result;
  // TODO: Need an AttributeItemPhrase
}

void SubphraseGenerator::childrenOfModelEntity(
  DescriptivePhrase::Ptr src, smtk::model::EntityPtr entity, DescriptivePhrases& result)
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
  this->instancesOfModelEntity(src, ref, result);
  // Any entity may have associated attributes
  this->instancesOfModelEntity(src, ref, result);
}

void SubphraseGenerator::freeSubmodelsOfModel(
  DescriptivePhrase::Ptr src, const smtk::model::Model& mod, DescriptivePhrases& result)
{
  auto freeSubmodelsInModel = mod.submodels();
  this->addModelEntityPhrases(freeSubmodelsInModel, src, this->directLimit(), result);
}

void SubphraseGenerator::freeGroupsOfModel(
  DescriptivePhrase::Ptr src, const smtk::model::Model& mod, DescriptivePhrases& result)
{
  auto freeGroups = mod.groups();
  this->addModelEntityPhrases(freeGroups, src, this->directLimit(), result);
}

void SubphraseGenerator::freeCellsOfModel(
  DescriptivePhrase::Ptr src, const smtk::model::Model& mod, DescriptivePhrases& result)
{
  auto freeCellsInModel = mod.cells();
  this->addModelEntityPhrases(freeCellsInModel, src, this->directLimit(), result);
}

void SubphraseGenerator::freeAuxiliaryGeometriesOfModel(
  DescriptivePhrase::Ptr src, const smtk::model::Model& mod, DescriptivePhrases& result)
{
  auto freeAuxGeom = mod.auxiliaryGeometry();
  this->addModelEntityPhrases(freeAuxGeom, src, this->directLimit(), result);
}

void SubphraseGenerator::cellOfModelUse(
  DescriptivePhrase::Ptr src, const smtk::model::UseEntity& ent, DescriptivePhrases& result)
{
  auto parentCell = ent.cell();
  if (parentCell.isValid())
  {
    result.push_back(ComponentPhraseContent::createPhrase(parentCell.component(), 0, src));
  }
}

void SubphraseGenerator::boundingShellsOfModelUse(
  DescriptivePhrase::Ptr src, const smtk::model::UseEntity& ent, DescriptivePhrases& result)
{
  smtk::model::ShellEntities boundingShells =
    ent.boundingShellEntities<smtk::model::ShellEntities>();
  this->addModelEntityPhrases(boundingShells, src, this->directLimit(), result);
}

void SubphraseGenerator::toplevelShellsOfModelUse(
  DescriptivePhrase::Ptr src, const smtk::model::UseEntity& ent, DescriptivePhrases& result)
{
  auto toplevelShells = ent.shellEntities<smtk::model::ShellEntities>();
  this->addModelEntityPhrases(toplevelShells, src, this->directLimit(), result);
}

void SubphraseGenerator::usesOfModelCell(
  DescriptivePhrase::Ptr src, const smtk::model::CellEntity& ent, DescriptivePhrases& result)
{
  auto cellUses = ent.uses<smtk::model::UseEntities>();
  this->addModelEntityPhrases(cellUses, src, this->directLimit(), result);
}

void SubphraseGenerator::inclusionsOfModelCell(
  DescriptivePhrase::Ptr src, const smtk::model::CellEntity& ent, DescriptivePhrases& result)
{
  auto inclusions = ent.inclusions<smtk::model::EntityRefs>();
  auto boundingCells = ent.boundingCells();
  smtk::model::EntityRefs strictInclusions;
  std::set_difference(inclusions.begin(), inclusions.end(), boundingCells.begin(),
    boundingCells.end(), std::inserter(strictInclusions, strictInclusions.end()));
  this->addModelEntityPhrases(strictInclusions, src, this->directLimit(), result);
}

void SubphraseGenerator::boundingCellsOfModelCell(
  DescriptivePhrase::Ptr src, const smtk::model::CellEntity& ent, DescriptivePhrases& result)
{
  auto boundingCells = ent.boundingCells();
  this->addModelEntityPhrases(boundingCells, src, this->directLimit(), result);
}

void SubphraseGenerator::usesOfModelShell(
  DescriptivePhrase::Ptr src, const smtk::model::ShellEntity& ent, DescriptivePhrases& result)
{
  auto shellUses = ent.uses<smtk::model::UseEntities>();
  this->addModelEntityPhrases(shellUses, src, this->directLimit(), result);
}

void SubphraseGenerator::membersOfModelGroup(
  DescriptivePhrase::Ptr src, const smtk::model::Group& grp, DescriptivePhrases& result)
{
  auto members = grp.members<smtk::model::EntityRefArray>();
  this->addModelEntityPhrases(members, src, this->directLimit(), result);
  // TODO: Sort by entity type, name, etc.?
}

void SubphraseGenerator::childrenOfModelAuxiliaryGeometry(
  DescriptivePhrase::Ptr src, const smtk::model::AuxiliaryGeometry& aux, DescriptivePhrases& result)
{
  auto children = aux.embeddedEntities<smtk::model::AuxiliaryGeometries>();
  for (auto child : children)
  {
    result.push_back(ComponentPhraseContent::createPhrase(child.component(), 0, src));
  }
}

void SubphraseGenerator::prototypeOfModelInstance(
  DescriptivePhrase::Ptr src, const smtk::model::Instance& ent, DescriptivePhrases& result)
{
  auto instanceOf = ent.prototype();
  if (instanceOf.isValid())
  {
    result.push_back(ComponentPhraseContent::createPhrase(instanceOf.component(), 0, src));
  }
}

void SubphraseGenerator::instancesOfModelEntity(
  DescriptivePhrase::Ptr src, const smtk::model::EntityRef& ent, DescriptivePhrases& result)
{
  auto instances = ent.instances<smtk::model::InstanceEntities>();
  this->addModelEntityPhrases(instances, src, this->directLimit(), result);
}

#if 0

void SubphraseGenerator::attributesOfModelEntity(
  DescriptivePhrase::Ptr src, const smtk::model::EntityPtr& ent, DescriptivePhrases& result)
{
  if (!m_skipAttributes && ent.hasAttributes())
  {
    result.push_back(AttributeListPhraseContent::createPhrase(ent, ent.attributes(), src));
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
  if (!this->m_skipProperties)
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

void SubphraseGenerator::meshesOfModelModel(
  DescriptivePhrase::Ptr src, const Model& mod, DescriptivePhrases& result)
{
  std::vector<smtk::mesh::CollectionPtr> meshCollections =
    mod.manager()->meshes()->associatedCollections(mod);
  // We need to sort the meshes before we add them to the result since if
  // we sort the result itself we could be intermixing the mesh and model
  // information
  DescriptivePhrases meshPhrases;
  addMeshPhrases(meshCollections, src, this->directLimit(), meshPhrases);
  std::sort(meshPhrases.begin(), meshPhrases.end(), DescriptivePhrase::compareByTitle);
  result.insert(result.end(), meshPhrases.begin(), meshPhrases.end());
}

void SubphraseGenerator::meshsetsOfMesh(MeshPhrase::Ptr meshphr, DescriptivePhrases& result)
{
  smtk::mesh::MeshSet meshes = meshphr->relatedMesh();
  // if this is a mesh collection
  if (meshphr->isCollection())
  {
    this->meshsetsOfCollectionByDim(meshphr, smtk::mesh::Dims3, result);
    this->meshsetsOfCollectionByDim(meshphr, smtk::mesh::Dims2, result);
    this->meshsetsOfCollectionByDim(meshphr, smtk::mesh::Dims1, result);
    this->meshsetsOfCollectionByDim(meshphr, smtk::mesh::Dims0, result);
  }
  // if this is a MeshSet
  else if (meshes.size() > 1)
  {
    // if the MeshSet contains more than one mesh, we need to create subphrases for
    // each subset, otherwise the meshphr will represent the relatedMesh.
    for (std::size_t i = 0; i < meshes.size(); ++i)
    {
      result.push_back(MeshPhraseContent::createPhrase(meshes.subset(i), meshphr));
    }
  }
}

void SubphraseGenerator::meshsetsOfCollectionByDim(
  MeshPhrase::Ptr meshphr, smtk::mesh::DimensionType dim, DescriptivePhrases& result)
{
  if (meshphr->isCollection())
  {
    smtk::mesh::CollectionPtr meshcollection = meshphr->relatedMeshCollection();
    smtk::mesh::MeshSet dimMeshes = meshcollection->meshes(dim);
    if (!dimMeshes.is_empty())
    {
      result.push_back(MeshPhraseContent::createPhrase(dimMeshes, meshphr));
    }
  }
}

void SubphraseGenerator::meshesOfMeshList(MeshListPhrase::Ptr src, DescriptivePhrases& result)
{
  if (src->relatedCollections().size() > 0)
  {
    addMeshPhrases(src->relatedCollections(), src, this->directLimit(), result);
  }
  else if (src->relatedMeshes().size() > 0)
  {
    addMeshPhrases(src->relatedMeshes(), src, this->directLimit(), result);
  }
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
