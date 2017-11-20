//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ResourcePhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseList.h"
#include "smtk/view/ResourcePhrase.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/View.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include <algorithm> // for std::sort

namespace smtk
{
namespace view
{

PhraseModelPtr ResourcePhraseModel::create(const smtk::view::ViewPtr& viewSpec)
{
  (void)viewSpec;
  auto model = PhraseModel::Ptr(new ResourcePhraseModel);
  return model;
}

ResourcePhraseModel::ResourcePhraseModel()
  : m_root(PhraseList::create())
{
  auto generator = smtk::view::SubphraseGenerator::create();
  m_root->setDelegate(generator);
}

ResourcePhraseModel::~ResourcePhraseModel()
{
  this->resetSources();
}

DescriptivePhrasePtr ResourcePhraseModel::root() const
{
  return m_root;
}

void ResourcePhraseModel::handleCreated(
  Operator::Ptr op, Operator::Result res, ComponentItemPtr data)
{
  (void)op;
  if (!res || !data)
  {
    return;
  }

  for (auto it = data->begin(); it != data->end(); ++it)
  {
    smtk::resource::ComponentPtr comp = *it;
    smtk::resource::ResourcePtr rsrc = comp->resource();
    if (m_resources.find(rsrc) == m_resources.end())
    {
      DescriptivePhrases children(m_root->subphrases());
      children.push_back(smtk::view::ResourcePhrase::create()->setup(rsrc, 0, m_root));
      std::sort(children.begin(), children.end(), DescriptivePhrase::compareByTypeThenTitle);
      this->updateChildren(m_root, children, std::vector<int>());
    }
  }
}
}
}

smtkImplementsPhraseModel(
  SMTKCORE_EXPORT, smtk::view::ResourcePhraseModel, resource, ResourcePhrase);
