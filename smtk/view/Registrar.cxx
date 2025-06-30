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
#include "smtk/model/Resource.h"

#include "smtk/plugin/Manager.h"

#include "smtk/view/AssociationBadge.h"
#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/DefaultOperationIcon.h"
#include "smtk/view/EmptySubphraseGenerator.h"
#include "smtk/view/HandleManager.h"
#include "smtk/view/LockedResourceBadge.h"
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
using PhraseModelList = std::
  tuple<ResourcePhraseModel, ComponentPhraseModel, ReferenceItemPhraseModel, SelectionPhraseModel>;
using SubphraseGeneratorList = std::tuple<
  SubphraseGenerator,
  TwoLevelSubphraseGenerator,
  EmptySubphraseGenerator,
  QueryFilterSubphraseGenerator>;
using BadgeList = std::tuple<AssociationBadge, LockedResourceBadge, ObjectIconBadge>;
} // namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  managers->insert(smtk::view::Manager::create());
  smtk::plugin::Manager::instance()->registerPluginsTo(managers->get<smtk::view::Manager::Ptr>());

  // NB: selection should never be overwritten with a different instance
  //     of smtk::view::Selection once it is created; consumers of the
  //     selection assume it is constant, will add observers, etc..
  managers->insert(smtk::view::Selection::create());
  managers->get<smtk::view::Selection::Ptr>()->setDefaultAction(
    smtk::view::SelectionAction::FILTERED_REPLACE);
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::view::Manager::Ptr>();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Add some string hashes to the token manager to avoid
  // "Hash does not exist in database" errors.
  (void)smtk::string::Token("smtk::resource::PersistentObject");
  (void)smtk::string::Token("smtk::resource::Component");
  (void)smtk::string::Token("smtk::resource::Resource");
  (void)smtk::string::Token("smtk::geometry::Resource");
  (void)smtk::string::Token("smtk::attribute::Resource");
  (void)smtk::string::Token("smtk::attribute::Attribute");
  (void)smtk::string::Token("smtk::attribute::Item");
  (void)smtk::string::Token("smtk::attribute::ValueItem");
  (void)smtk::string::Token("smtk::attribute::ValueItemTemplate<int>");
  (void)smtk::string::Token("smtk::attribute::ValueItemTemplate<std::string>");
  (void)smtk::string::Token("smtk::attribute::ValueItemTemplate<double>");
  (void)smtk::string::Token("smtk::attribute::IntItem");
  (void)smtk::string::Token("smtk::attribute::StringItem");
  (void)smtk::string::Token("smtk::attribute::DoubleItem");
  (void)smtk::string::Token("smtk::attribute::VoidItem");
  (void)smtk::string::Token("smtk::attribute::ReferenceItem");
  (void)smtk::string::Token("smtk::attribute::ResourceItem");
  (void)smtk::string::Token("smtk::attribute::ComponentItem");
  (void)smtk::string::Token("smtk::attribute::FileSystemItem");
  (void)smtk::string::Token("smtk::attribute::FileItem");
  (void)smtk::string::Token("smtk::attribute::DirectoryItem");
  (void)smtk::string::Token("smtk::attribute::DateTimeItem");
  (void)smtk::string::Token("smtk::model::Resource");
  (void)smtk::string::Token("smtk::view::Configuration");
  (void)smtk::string::Token("smtk::view::View");

  auto handleManager = smtk::view::HandleManager::instance();
  handleManager->registerOperationManager(operationManager);
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  (void)operationManager;
  auto handleManager = smtk::view::HandleManager::instance();
  handleManager->registerOperationManager(nullptr);
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().registerTypes<PhraseModelList>();
  viewManager->subphraseGeneratorFactory().registerTypes<SubphraseGeneratorList>();

  // Per-object icons
  viewManager->objectIcons().registerIconConstructor<smtk::resource::Resource>(
    ResourceIconConstructor());
  viewManager->objectIcons().registerIconConstructor<smtk::attribute::Resource>(
    AttributeIconConstructor());
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
} // namespace view
} // namespace smtk
