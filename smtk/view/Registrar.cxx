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

#include "smtk/view/AssociationBadge.h"
#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/DefaultOperationIcon.h"
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
using PhraseModelList = std::tuple<ResourcePhraseModel, ComponentPhraseModel,
  ReferenceItemPhraseModel, SelectionPhraseModel>;
using SubphraseGeneratorList = std::tuple<SubphraseGenerator, TwoLevelSubphraseGenerator,
  EmptySubphraseGenerator, QueryFilterSubphraseGenerator>;
using BadgeList = std::tuple<AssociationBadge, ObjectIconBadge>;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().registerTypes<PhraseModelList>();
  viewManager->subphraseGeneratorFactory().registerTypes<SubphraseGeneratorList>();

  // Per-object icons
  viewManager->objectIcons().registerIconConstructor<smtk::attribute::Resource>(
    AttributeIconConstructor());
  viewManager->objectIcons().registerIconConstructor<smtk::mesh::Resource>(MeshIconConstructor());
  viewManager->objectIcons().registerIconConstructor<smtk::model::Resource>(ModelIconConstructor());

  // Per-operation icons
  viewManager->operationIcons().registerDefaultIconConstructor(DefaultOperationIcon);

  viewManager->badgeFactory().registerTypes<BadgeList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().unregisterTypes<PhraseModelList>();
  viewManager->subphraseGeneratorFactory().unregisterTypes<SubphraseGeneratorList>();

  viewManager->badgeFactory().unregisterTypes<BadgeList>();
}
}
}
