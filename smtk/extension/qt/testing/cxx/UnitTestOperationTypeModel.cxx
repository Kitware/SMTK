//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Registrar.h"
#include "smtk/common/Environment.h"
#include "smtk/extension/qt/qtOperationAction.h"
#include "smtk/extension/qt/qtOperationPalette.h"
#include "smtk/extension/qt/qtOperationTypeModel.h"
#include "smtk/extension/qt/qtOperationTypeView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtViewRegistrar.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Registrar.h"
#include "smtk/plugin/Manager.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"
#include "smtk/view/OperationDecorator.h"
#include "smtk/view/Registrar.h"
#include "smtk/view/Selection.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/attribute/operators/Associate.h"
#include "smtk/attribute/operators/Dissociate.h"
#include "smtk/attribute/operators/Export.h"
#include "smtk/attribute/operators/Import.h"
#include "smtk/mesh/operators/DeleteMesh.h"
#include "smtk/mesh/operators/ElevateMesh.h"
#include "smtk/mesh/operators/Export.h"
#include "smtk/mesh/operators/ExtractAdjacency.h"
#include "smtk/mesh/operators/ExtractByDihedralAngle.h"
#include "smtk/mesh/operators/ExtractSkin.h"
#include "smtk/mesh/operators/Import.h"
#include "smtk/mesh/operators/InterpolateOntoMesh.h"
#include "smtk/mesh/operators/MergeCoincidentPoints.h"
#include "smtk/mesh/operators/PrintMeshInformation.h"
#include "smtk/mesh/operators/SelectCells.h"
#include "smtk/mesh/operators/SetMeshName.h"
#include "smtk/mesh/operators/Subtract.h"
#include "smtk/mesh/operators/Transform.h"
#include "smtk/mesh/operators/UndoElevateMesh.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"
#include "smtk/model/operators/AddImage.h"
#include "smtk/model/operators/CompositeAuxiliaryGeometry.h"
#include "smtk/model/operators/CreateInstances.h"
#include "smtk/model/operators/Delete.h"
#include "smtk/model/operators/DivideInstance.h"
#include "smtk/model/operators/EntityGroupOperation.h"
#include "smtk/model/operators/GroupAuxiliaryGeometry.h"
#include "smtk/model/operators/MergeInstances.h"
#include "smtk/model/operators/SetInstancePrototype.h"
#include "smtk/operation/operators/AssignColors.h"
#include "smtk/operation/operators/SetProperty.h"

#include "nlohmann/json.hpp"

#include <QApplication>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <cassert>
#include <string>

using namespace smtk::view;

int UnitTestOperationTypeModel(int argc, char** const argv)
{
  QApplication app(argc, argv);
  // Passing "-d" (or any other argument) will run the event loop and allow interaction.
  bool interactive =
    argc > 1 && !smtk::common::Environment::hasVariable("DASHBOARD_TEST_FROM_CTEST");

  // Initialize view and UI managers
  auto managers = smtk::common::Managers::create();
  auto operationManager = smtk::operation::Manager::create();
  auto resourceManager = smtk::resource::Manager::create();
  auto viewManager = smtk::view::Manager::create();
  auto selection = smtk::view::Selection::create();
  managers->insertOrAssign(operationManager);
  managers->insertOrAssign(viewManager);
  managers->insertOrAssign(selection);
  // auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  // auto viewManager = managers->get<smtk::view::Manager::Ptr>();
  // auto selection = managers->get<smtk::view::Selection::Ptr>();

  auto modelRsrcRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager);
  auto attrRsrcRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager);
  auto modelOpRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(operationManager);
  auto modelViewRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(viewManager);
  auto operationOpRegistry =
    smtk::plugin::addToManagers<smtk::operation::Registrar>(operationManager);
  auto operationViewRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(viewManager);
  auto viewViewRegistry = smtk::plugin::addToManagers<smtk::view::Registrar>(viewManager);
  auto qtViewRegistry = smtk::plugin::addToManagers<smtk::extension::qtViewRegistrar>(viewManager);

  smtk::plugin::Manager::instance()->registerPluginsTo(managers);
  smtk::plugin::Manager::instance()->registerPluginsTo(operationManager);
  smtk::plugin::Manager::instance()->registerPluginsTo(viewManager);

  // Instantiate widget as container for qtUIManager
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout();
  widget->setLayout(layout);
  auto* uiMgr = new smtk::extension::qtUIManager(resourceManager, viewManager);
  uiMgr->setOperationManager(operationManager);
  // auto view = uiMgr->createView("OperationPalette");
  nlohmann::json jsonConfig = {
    { "Name", "Operations" },
    { "Type", "qtOperationPalette" },
    { "Component",
      { { "Name", "Details" },
        { "Attributes",
          { { "Title", "Tools" }, { "SearchBar", true }, { "EditMode", "LongClick" } } } } }
  };
  std::shared_ptr<smtk::view::Configuration> viewConfig = jsonConfig;
  //NOLINTNEXTLINE(modernize-make-shared)
  auto opDecor = std::shared_ptr<smtk::view::OperationDecorator>(new OperationDecorator(
    { wrap<smtk::operation::AssignColors>(
        "a color chooser", "Assign lots of colors at once", {}, "a color\nchooser"),
      wrap<smtk::model::Delete>(),
      wrap<smtk::model::EntityGroupOperation>("edit group", "Edit groups", {}, "edit\ngroup"),
      wrap<smtk::model::GroupAuxiliaryGeometry>(),
      wrap<smtk::model::AddAuxiliaryGeometry>(),
      wrap<smtk::model::AddImage>(),
      wrap<smtk::model::CompositeAuxiliaryGeometry>(),
      wrap<smtk::model::CreateInstances>(),
      wrap<smtk::model::DivideInstance>(),
      wrap<smtk::model::MergeInstances>(),
      wrap<smtk::model::SetInstancePrototype>(),
      wrap<smtk::mesh::DeleteMesh>(),
      wrap<smtk::mesh::ElevateMesh>(),
      wrap<smtk::mesh::Export>(),
      wrap<smtk::mesh::ExtractAdjacency>(),
      wrap<smtk::mesh::ExtractByDihedralAngle>(),
      wrap<smtk::mesh::ExtractSkin>(),
      wrap<smtk::mesh::Import>(),
      wrap<smtk::mesh::InterpolateOntoMesh>(),
      wrap<smtk::mesh::MergeCoincidentPoints>(),
      wrap<smtk::mesh::PrintMeshInformation>(),
      wrap<smtk::mesh::SelectCells>(),
      wrap<smtk::mesh::SetMeshName>(),
      wrap<smtk::mesh::Subtract>(),
      wrap<smtk::mesh::Transform>(),
      wrap<smtk::mesh::UndoElevateMesh>(),
      wrap<smtk::operation::SetProperty>(),
      wrap<smtk::attribute::Associate>(),
      wrap<smtk::attribute::Dissociate>(),
      wrap<smtk::attribute::Export>(),
      wrap<smtk::attribute::Import>() }));
  managers->insertOrAssign(opDecor);
  smtk::view::Information viewInfo;

  viewInfo.insert(widget);
  viewInfo.insert(viewConfig);
  viewInfo.insert(managers);
  viewInfo.insert(uiMgr);
  viewInfo.insert(opDecor);
  auto* view = uiMgr->setSMTKView(viewInfo);
  widget->setGeometry(0, 0, 400, 500);
  widget->show();

  test(!!view, "Could not create view.");
  auto* paletteView = dynamic_cast<smtk::extension::qtOperationPalette*>(view);
  test(!!paletteView, "Unexpected view type.");

  auto* model = paletteView->model();
  test(!!model, "View has no model.");
  std::cout << "Model has"
            << " " << model->rowCount(QModelIndex()) << " rows"
            << " " << model->columnCount(QModelIndex()) << " cols"
            << "\n";

  std::cout << "Row  Associability  Editability — TypeName — Label – Center – Index\n";
  auto* opView = paletteView->operationView();
  test(!!opView, "Expected a non-empty view.");
  QModelIndex assignColorsIndex;
  QModelIndex createGroupIndex;
  for (int ii = 0; ii < model->rowCount(QModelIndex()); ++ii)
  {
    // Test indexAt(), visualRect(), and (in theory) scrollTo().
    auto opIdx = model->index(ii, 0);
    auto opRect = opView->visualRect(opIdx);
    int opRowFromRect = opView->indexAt(opRect.center()).row();
    std::string opTypeName =
      model->index(ii, static_cast<int>(qtOperationTypeModel::Column::TypeName))
        .data()
        .toString()
        .toStdString();

    std::cout << ii << " "
              << model->index(ii, static_cast<int>(qtOperationTypeModel::Column::Associability))
                   .data()
                   .toInt()
              << " "
              << model->index(ii, static_cast<int>(qtOperationTypeModel::Column::Editability))
                   .data()
                   .toInt()
              << " — " << opTypeName << " — "
              << model->index(ii, static_cast<int>(qtOperationTypeModel::Column::Label))
                   .data()
                   .toString()
                   .toStdString()
              << " – x " << opRect.center().x() << " y " << opRect.center().y() << " – "
              << opRowFromRect << "\n";

    if (opTypeName == "smtk::operation::AssignColors")
    {
      assignColorsIndex = model->index(ii, 0);
    }
    else if (opTypeName == "smtk::model::EntityGroupOperation")
    {
      createGroupIndex = model->index(ii, 0);
    }

    test(opRowFromRect == ii, "Expected view->indexAt(view->visualRect(idx)) == idx.");
  }

  // Verify that the label was in fact overridden by the OperationDecorator.
  test(assignColorsIndex.isValid(), "Expected to find the assign-colors operation.");
  test(
    assignColorsIndex
        .sibling(assignColorsIndex.row(), static_cast<int>(qtOperationTypeModel::Column::Label))
        .data()
        .toString()
        .toStdString() == "a color chooser",
    "Expected label to be an override value.");

  // NB: The operation decorator above has 32 entries, but only 14 of the
  //     operations whitelisted are registered with the operation manager.
  test(model->rowCount(QModelIndex()) == 14, "Wrong number of rows (expected 14).");
  test(model->columnCount(QModelIndex()) == 9, "Wrong number of columns.");

  if (!interactive)
  {
    opView->scrollTo(assignColorsIndex, QAbstractItemView::EnsureVisible);
    opView->scrollTo(createGroupIndex, QAbstractItemView::PositionAtCenter);
    opView->scrollTo(createGroupIndex, QAbstractItemView::PositionAtTop);
    opView->scrollTo(assignColorsIndex, QAbstractItemView::PositionAtBottom);
  }
  else
  {
    QTimer::singleShot(500, [&opView, &assignColorsIndex]() {
      opView->scrollTo(assignColorsIndex, QAbstractItemView::EnsureVisible);
    });
    QTimer::singleShot(1500, [&opView, &createGroupIndex]() {
      opView->scrollTo(createGroupIndex, QAbstractItemView::PositionAtCenter);
    });
    QTimer::singleShot(2500, [&opView, &createGroupIndex]() {
      opView->scrollTo(createGroupIndex, QAbstractItemView::PositionAtTop);
    });
    QTimer::singleShot(3500, [&opView, &assignColorsIndex]() {
      opView->scrollTo(assignColorsIndex, QAbstractItemView::PositionAtBottom);
    });
  }

  auto* operationModel = paletteView->operationModel();
  QObject::connect(
    operationModel,
    &qtOperationTypeModel::runOperation,
    [](smtk::operation::Operation::Index index) { std::cout << "Run " << index << "\n"; });
  QObject::connect(
    operationModel,
    &qtOperationTypeModel::editOperationParameters,
    [](smtk::operation::Operation::Index index) { std::cout << "Edit " << index << "\n"; });

  if (interactive)
  {
    QApplication::exec();
  }
  return 0;
}
