//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/view/Registrar.h"

#include "smtk/attribute/Resource.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/model/Resource.h"

#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/EmptySubphraseGenerator.h"
#include "smtk/view/ObjectIconBadge.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/QueryFilterSubphraseGenerator.h"
#include "smtk/view/ReferenceItemPhraseModel.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SVGIconConstructor.h"
#include "smtk/view/SelectionPhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/TwoLevelSubphraseGenerator.h"

#include <tuple>

namespace smtk
{
namespace view
{
namespace
{
typedef std::tuple<ResourcePhraseModel, ComponentPhraseModel, ReferenceItemPhraseModel,
  SelectionPhraseModel>
  PhraseModelList;
typedef std::tuple<SubphraseGenerator, TwoLevelSubphraseGenerator, EmptySubphraseGenerator,
  QueryFilterSubphraseGenerator>
  SubphraseGeneratorList;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().registerTypes<PhraseModelList>();
  viewManager->registerSubphraseGenerators<SubphraseGeneratorList>();

  viewManager->iconFactory().registerIconConstructor<smtk::attribute::Resource>(
    AttributeIconConstructor());
  viewManager->iconFactory().registerIconConstructor<smtk::mesh::Resource>(MeshIconConstructor());
  viewManager->iconFactory().registerIconConstructor<smtk::model::Resource>(ModelIconConstructor());

  viewManager->badgeFactory().registerBadge<ObjectIconBadge>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().unregisterTypes<PhraseModelList>();
  viewManager->unregisterSubphraseGenerators<SubphraseGeneratorList>();

  viewManager->badgeFactory().unregisterBadge<ObjectIconBadge>();
}
}
}
