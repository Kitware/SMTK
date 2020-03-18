//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ComponentItemPhraseModel.h"

#include "smtk/attribute/Utilities.h"

#include "smtk/resource/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"
#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/SubphraseGenerator.h"

#include <algorithm> // for std::sort

using namespace smtk::view;

PhraseModelPtr ComponentItemPhraseModel::create(
  const smtk::view::Configuration::Component& itemViewSpec)
{
  // Currently the itemViewSpec is not used but in the future it will contain information to
  // customize how the model should behave
  (void)itemViewSpec;
  auto model = PhraseModel::Ptr(new ComponentItemPhraseModel);
  model->root()->findDelegate()->setModel(model);
  return model;
}

ComponentItemPhraseModel::ComponentItemPhraseModel()
{
  m_useAttributeAssociatons = false;
}

ComponentItemPhraseModel::~ComponentItemPhraseModel() = default;

void ComponentItemPhraseModel::setComponentItem(smtk::attribute::ComponentItemPtr& compItem)
{
  m_compItem = compItem;
  this->populateRoot();
}

void ComponentItemPhraseModel::populateRoot()
{
  // FIXME: What should happen if m_componentFilters is empty?
  //        It seems like _all_ components (possibly just from the active resource)
  //        should be displayed. If so, we need to handle it here. On the other hand
  //        if nothing should be displayed, then no action is needed.
  smtk::resource::ManagerPtr resourceManager;
  this->visitSources([&resourceManager](const smtk::resource::ManagerPtr& rsrcMgr,
                       const smtk::operation::ManagerPtr& /*unused*/,
                       const smtk::view::ManagerPtr& /*unused*/, const smtk::view::SelectionPtr &
                       /*unused*/) -> bool {
    if (rsrcMgr)
    {
      resourceManager = rsrcMgr;
      return false;
    }
    return true;
  });

  auto objSet = smtk::attribute::Utilities::associatableObjects(
    m_compItem, resourceManager, m_useAttributeAssociatons);

  // Turn each entry of comps into a decorated phrase, sort, and update.
  DescriptivePhrases children;
  for (const auto& obj : objSet)
  {
    const auto& comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
    if (comp == nullptr)
    {
      continue;
    }
    children.push_back(
      smtk::view::ComponentPhraseContent::createPhrase(comp, m_mutableAspects, m_root));
  }
  std::sort(children.begin(), children.end(), DescriptivePhrase::compareByTypeThenTitle);
  this->root()->findDelegate()->decoratePhrases(children);
  this->updateChildren(m_root, children, std::vector<int>());
}

void ComponentItemPhraseModel::handleModified(
  const Operation& op, const Operation::Result& res, const ComponentItemPtr& data)
{
  this->Superclass::handleModified(op, res, data);
  this->populateRoot();
}
