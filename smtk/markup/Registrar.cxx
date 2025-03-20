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
#include "smtk/markup/Registrar.h"

#include "smtk/markup/operators/ChangeUnits.h"
#include "smtk/markup/operators/Create.h"
#include "smtk/markup/operators/CreateAnalyticShape.h"
#include "smtk/markup/operators/CreateArc.h"
#include "smtk/markup/operators/CreateGroup.h"
#include "smtk/markup/operators/Delete.h"
#include "smtk/markup/operators/DumpGraph.h"
#include "smtk/markup/operators/EditComment.h"
#include "smtk/markup/operators/Import.h"
#include "smtk/markup/operators/Read.h"
#include "smtk/markup/operators/SetName.h"
#include "smtk/markup/operators/TagIndividual.h"
#include "smtk/markup/operators/Ungroup.h"
#include "smtk/markup/operators/Write.h"

#include "smtk/markup/Resource.h"

#include "smtk/markup/SubphraseGenerator.h"

#include "smtk/view/NameManager.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/DeleterGroup.h"
#include "smtk/operation/groups/GroupingGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/NamingGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/UngroupingGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace markup
{

namespace
{
using OperationList = std::tuple<
  ChangeUnits,
  Create,
  CreateArc,
  CreateAnalyticShape,
  CreateGroup,
  Delete,
  DumpGraph,
  EditComment,
  Import,
  Read,
  SetName,
  TagIndividual,
  Ungroup,
  Write>;

using SubphraseGeneratorList = std::tuple<smtk::markup::SubphraseGenerator>;
} // anonymous namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  // The markup resource uses a NameManager to ensure names are easy to identify
  // without visualizing the entire hierarchy above a component. If one isn't
  // already provided by the application, create one here.
  if (!managers->contains<smtk::view::NameManager::Ptr>())
  {
    managers->insert(smtk::view::NameManager::create());
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  (void)managers;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  // Providing a write method to the resource manager is what allows
  // modelbuilder's pqSMTKSaveResourceBehavior to determine how to write
  // the resource when users click "Save Resource" or ⌘S/⌃S.
  resourceManager->registerResource<smtk::markup::Resource>(read, write, create);

  // Beyond registering the resource type, we now prepare the resource by
  // registering domain and node types to their respective factories.
  smtk::markup::Resource::domainFactory().registerTypes<smtk::markup::Traits::DomainTypes>();

  auto& typeLabels = resourceManager->objectTypeLabels();
  // clang-format off
  // Resource name
  typeLabels[smtk::common::typeName<smtk::markup::Resource>()] = "markup resource";

  // Component names
  typeLabels[smtk::common::typeName<smtk::markup::AnalyticShape>()] = "analytic shape";
  typeLabels[smtk::common::typeName<smtk::markup::Box>()] = "box";
  typeLabels[smtk::common::typeName<smtk::markup::Comment>()] = "comment";
  typeLabels[smtk::common::typeName<smtk::markup::Component>()] = "markup component";
  typeLabels[smtk::common::typeName<smtk::markup::Cone>()] = "cone";
  typeLabels[smtk::common::typeName<smtk::markup::DiscreteGeometry>()] = "discrete geometry";
  typeLabels[smtk::common::typeName<smtk::markup::Feature>()] = "feature";
  typeLabels[smtk::common::typeName<smtk::markup::Field>()] = "field";
  typeLabels[smtk::common::typeName<smtk::markup::Group>()] = "group";
  typeLabels[smtk::common::typeName<smtk::markup::ImageData>()] = "image";
  typeLabels[smtk::common::typeName<smtk::markup::Label>()] = "label";
  typeLabels[smtk::common::typeName<smtk::markup::NodeSet>()] = "node set";
  typeLabels[smtk::common::typeName<smtk::markup::Ontology>()] = "ontology";
  typeLabels[smtk::common::typeName<smtk::markup::OntologyIdentifier>()] = "ontology id";
  typeLabels[smtk::common::typeName<smtk::markup::Plane>()] = "plane";
  typeLabels[smtk::common::typeName<smtk::markup::SideSet>()] = "side set";
  typeLabels[smtk::common::typeName<smtk::markup::SpatialData>()] = "spatial data";
  typeLabels[smtk::common::typeName<smtk::markup::Sphere>()] = "sphere";
  typeLabels[smtk::common::typeName<smtk::markup::Subset>()] = "subset";
  typeLabels[smtk::common::typeName<smtk::markup::UnstructuredData>()] = "unstructured mesh";
  typeLabels[smtk::common::typeName<smtk::markup::URL>()] = "url";

  // Arc names
  typeLabels[smtk::common::typeName<smtk::markup::arcs::BoundariesToShapes>()] = "boundaries of shape";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::FieldsToShapes>()] = "shapes";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::GroupsToMembers>()] = "members";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::LabelsToSubjects>()] = "subjects";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::OntologyIdentifiersToIndividuals>()] = "individuals";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::OntologyIdentifiersToSubtypes>()] = "ontological subtypes";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::OntologyToIdentifiers>()] = "identifiers";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::ReferencesToPrimaries>()] = "primaries";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::URLsToData>()] = "source URL for";
  typeLabels[smtk::common::typeName<smtk::markup::arcs::URLsToImportedData>()] = "import URL for";
  // clang-format on
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  // Add operations to groups
  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Create>();

  smtk::operation::DeleterGroup(operationManager).registerOperation<smtk::markup::Delete>();

  smtk::operation::GroupingGroup(operationManager).registerOperation<smtk::markup::CreateGroup>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Import>();

  smtk::operation::NamingGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::SetName>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Read>();

  smtk::operation::UngroupingGroup(operationManager).registerOperation<smtk::markup::Ungroup>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::markup::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::CreatorGroup(operationManager).unregisterOperation<smtk::markup::Create>();

  smtk::operation::DeleterGroup(operationManager).unregisterOperation<smtk::markup::Delete>();

  smtk::operation::GroupingGroup(operationManager).unregisterOperation<smtk::markup::CreateGroup>();

  smtk::operation::UngroupingGroup(operationManager).unregisterOperation<smtk::markup::Ungroup>();

  smtk::operation::ImporterGroup(operationManager).unregisterOperation<smtk::markup::Import>();

  smtk::operation::NamingGroup(operationManager).unregisterOperation<smtk::markup::SetName>();

  smtk::operation::ReaderGroup(operationManager).unregisterOperation<smtk::markup::Read>();

  smtk::operation::WriterGroup(operationManager).unregisterOperation<smtk::markup::Write>();

  operationManager->unregisterOperations<OperationList>();
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->subphraseGeneratorFactory().registerTypes<SubphraseGeneratorList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->subphraseGeneratorFactory().unregisterTypes<SubphraseGeneratorList>();
}

} // namespace markup
} // namespace smtk
