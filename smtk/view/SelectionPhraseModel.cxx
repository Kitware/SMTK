//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/SelectionPhraseModel.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/EmptySubphraseGenerator.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/ResourcePhraseContent.h"
#include "smtk/view/View.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include "smtk/io/Logger.h"

#include <algorithm> // for std::sort

using namespace smtk::view;

PhraseModelPtr SelectionPhraseModel::create(const smtk::view::ViewPtr& viewSpec)
{
  (void)viewSpec;
  auto model = SelectionPhraseModel::Ptr(new SelectionPhraseModel);
  model->root()->findDelegate()->setModel(model);
  int selnBit;
  if (!viewSpec->details().attributeAsInt("SelectionValue", selnBit))
  {
    selnBit = ~0; // Listen to all selection bits by default.
  }
  model->setSelectionBit(selnBit);
  return model;
}

SelectionPhraseModel::SelectionPhraseModel()
  : m_root(DescriptivePhrase::create())
  , m_selectionBit(~0)
  , m_componentMutability(0)
  , m_resourceMutability(0)
{
  // By default, do not show children of selected objects.
  auto generator = smtk::view::EmptySubphraseGenerator::create();
  m_root->setDelegate(generator);
}

SelectionPhraseModel::~SelectionPhraseModel()
{
  this->resetSources();
}

DescriptivePhrasePtr SelectionPhraseModel::root() const
{
  return m_root;
}

void SelectionPhraseModel::handleSelectionEvent(const std::string& src, Selection::Ptr seln)
{
  this->populateRoot(src, seln);
}

void SelectionPhraseModel::populateRoot(const std::string& src, Selection::Ptr seln)
{
  (void)src;

  if (!seln)
  {
    return;
  }

  // NB: By starting with children empty, we do not handle the
  //     case where the PhraseModel has been told to listen
  //     to multiple sources.
  DescriptivePhrases children;
  smtk::resource::Component::Ptr comp;
  smtk::resource::Resource::Ptr rsrc;
  for (auto entry : seln->currentSelection())
  {
    if ((entry.second & m_selectionBit) == 0)
    { // Not part of the selection we're interested in.
      continue;
    }

    auto obj = entry.first;
    if ((comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj)))
    {
      children.push_back(
        smtk::view::ComponentPhraseContent::createPhrase(comp, m_componentMutability, m_root));
    }
    else if ((rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj)))
    {
      children.push_back(
        smtk::view::ResourcePhraseContent::createPhrase(rsrc, m_resourceMutability, m_root));
    }
    else
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "Cannot present unknown object type.");
    }
  }
  std::sort(children.begin(), children.end(), DescriptivePhrase::compareByTypeThenTitle);
  this->root()->findDelegate()->decoratePhrases(children);
  this->updateChildren(m_root, children, std::vector<int>());
}
