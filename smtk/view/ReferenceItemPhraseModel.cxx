//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ReferenceItemPhraseModel.h"

#include "smtk/attribute/utility/Queries.h"

#include "smtk/resource/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include "smtk/resource/Component.h"
#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/ResourcePhraseContent.h"
#include "smtk/view/SubphraseGenerator.h"

#include <algorithm> // for std::sort

using namespace smtk::view;

PhraseModelPtr ReferenceItemPhraseModel::create(
  const smtk::view::Configuration::Component& itemViewSpec)
{
  // Currently the itemViewSpec is not used but in the future it will contain information to
  // customize how the model should behave
  (void)itemViewSpec;
  auto model = PhraseModel::Ptr(new ReferenceItemPhraseModel);
  model->root()->findDelegate()->setModel(model);
  return model;
}

ReferenceItemPhraseModel::ReferenceItemPhraseModel()
{
  m_useAttributeAssociatons = false;
}

ReferenceItemPhraseModel::ReferenceItemPhraseModel(const Configuration* config, Manager* manager)
  : Superclass(config, manager)
{
  m_useAttributeAssociatons = false;
}

ReferenceItemPhraseModel::~ReferenceItemPhraseModel() = default;

void ReferenceItemPhraseModel::setReferenceItem(smtk::attribute::ReferenceItemPtr& refItem)
{
  m_refItem = refItem;
  this->populateRoot();
}

void ReferenceItemPhraseModel::populateRoot()
{
  // FIXME: What should happen if m_componentFilters is empty?
  //        It seems like _all_ components (possibly just from the active resource)
  //        should be displayed. If so, we need to handle it here. On the other hand
  //        if nothing should be displayed, then no action is needed.
  smtk::resource::ManagerPtr resourceManager;
  this->visitSources(
    [&resourceManager](
      const smtk::resource::ManagerPtr& rsrcMgr,
      const smtk::operation::ManagerPtr& /*unused*/,
      const smtk::view::ManagerPtr& /*unused*/,
      const smtk::view::SelectionPtr&
      /*unused*/) -> smtk::common::Visit {
      if (rsrcMgr)
      {
        resourceManager = rsrcMgr;
        return smtk::common::Visit::Halt;
      }
      return smtk::common::Visit::Continue;
    });

  auto objSet = smtk::attribute::utility::associatableObjects(
    m_refItem, resourceManager, m_useAttributeAssociatons);

  // Turn each entry of comps into a decorated phrase, sort, and update.
  DescriptivePhrases children;
  for (const auto& obj : objSet)
  {
    const auto& comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
    if (comp != nullptr)
    {
      children.push_back(
        smtk::view::ComponentPhraseContent::createPhrase(comp, m_mutableAspects, m_root));
    }
    else
    {
      const auto& rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj);
      if (rsrc != nullptr)
      {
        children.push_back(
          smtk::view::ResourcePhraseContent::createPhrase(rsrc, m_mutableAspects, m_root));
      }
    }
  }
  std::sort(children.begin(), children.end(), DescriptivePhrase::compareByTypeThenTitle);
  this->updateChildren(m_root, children, std::vector<int>());
}

void ReferenceItemPhraseModel::handleModified(
  const smtk::resource::PersistentObjectSet& modifiedObjects)
{
  this->Superclass::handleModified(modifiedObjects);
  this->populateRoot();
}
