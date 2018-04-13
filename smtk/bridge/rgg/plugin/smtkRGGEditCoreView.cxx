//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/rgg/plugin/smtkRGGEditCoreView.h"
#include "smtk/bridge/rgg/plugin/ui_smtkRGGEditCoreParameters.h"

#include "smtkRGGViewHelper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/operators/CreateAssembly.h"
#include "smtk/bridge/rgg/operators/CreateDuct.h"
#include "smtk/bridge/rgg/operators/CreateModel.h"
#include "smtk/bridge/rgg/operators/EditCore.h"

#include "smtk/bridge/rgg/qt/qtDraw2DLattice.h"
#include "smtk/bridge/rgg/qt/rggNucCore.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/View.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqPresetDialog.h"

#include <QComboBox>
#include <QMap>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <QDockWidget>

using namespace smtk::model;
using namespace smtk::extension;
using namespace smtk::bridge::rgg;
static const double cos30 = 0.86602540378443864676372317075294;
static const double cos60 = 0.5;
// static const int degreesHex[6] = { -120, -60, 0, 60, 120, 180 };
// static const int degreesRec[4] = { -90, 0, 90, 180 };
// 0', 60', 120, 180', 240', 300'
static const double cosSinAngles[6][2] = { { 1.0, 0.0 }, { cos60, -cos30 }, { -cos60, -cos30 },
  { -1.0, 0.0 }, { -cos60, cos30 }, { cos60, cos30 } };

namespace
{

// Calculate the x,y coordinates of the current assy in the hex grid
void calculateHexAssyCoordinate(
  double& x, double& y, const double& spacing, const int& ring, const int& layer)
{
  // The index order of layer is clockwise, starting from upper left corner of the hex.
  // It's RGG's order and not ideal...
  if (ring == 0)
  {
    x = y = 0;
  }
  else
  {
    int remainder = layer % ring;
    int modulus = layer / ring;

    double eL = ring * spacing;                       // edge length
    double xBT = -eL * cos60 + eL / (ring)*remainder; // x before transform
    double yBT = eL * cos30;                          // y befor transform
    // Apply rotation if needed. In order to avoid sin/cos calculation, we use
    // predefined values;
    double cosValue = cosSinAngles[modulus][0];
    double sinValue = cosSinAngles[modulus][1];

    double x0 = xBT * cosValue - yBT * sinValue;
    double y0 = yBT * cosValue + xBT * sinValue;

    // Rotate 330 degree due to the fact that the orientations do not match in
    // the render view and schema planner
    // sin330 = -cos60 and cos330 = cos30;
    x = x0 * cos30 - y0 * (-cos60);
    y = y0 * cos30 + x0 * (-cos60);
  }
}
}

class smtkRGGEditCoreViewInternals : public Ui::RGGEditCoreParameters
{
public:
  smtkRGGEditCoreViewInternals()
  {
    this->CurrentCMBCore = nullptr;
    this->Current2DLattice = new qtDraw2DLattice(this->SchemaPlanner);
    this->Current2DLattice->setFrameShape(QFrame::NoFrame);
  }

  ~smtkRGGEditCoreViewInternals()
  {
    if (CurrentAtt)
    {
      delete CurrentAtt;
      CurrentAtt = nullptr;
    }
    auto mapIter = this->SMTKCoreToCMBCore.begin();
    while (mapIter != this->SMTKCoreToCMBCore.end())
    {
      if (mapIter.value())
      {
        delete mapIter.value();
      }
      mapIter++;
    }
    if (this->SchemaPlanner)
    {
      this->SchemaPlanner->setVisible(false);
    }
  }

  qtAttribute* createAttUI(smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    if (att && att->numberOfItems() > 0)
    {
      qtAttribute* attInstance = new qtAttribute(att, pw, view);
      attInstance->setUseSelectionManager(view->useSelectionManager());
      if (attInstance && attInstance->widget())
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("RGGCoreEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
      }
      return attInstance;
    }
    return NULL;
  }

  QPointer<qtAttribute> CurrentAtt;
  QPointer<qtDraw2DLattice> Current2DLattice;
  QPointer<QDockWidget> SchemaPlanner;
  // Map smtk core(smtk::model::Model) to CMB core rggNucCore which is used for schema planning
  QMap<smtk::model::EntityRef, rggNucCore*> SMTKCoreToCMBCore;
  rggNucCore* CurrentCMBCore;
  QPointer<QWidget> CurrentWidget;
  smtk::model::EntityRef CurrentSMTKCore;

  smtk::weak_ptr<smtk::model::Operator> CreateInstanceOp;
  smtk::weak_ptr<smtk::model::Operator> DeleteOp;
  smtk::weak_ptr<smtk::model::Operator> EditCoreOp;
};

smtkRGGEditCoreView::smtkRGGEditCoreView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkRGGEditCoreViewInternals();
}

smtkRGGEditCoreView::~smtkRGGEditCoreView()
{
  delete this->Internals;
}

qtBaseView* smtkRGGEditCoreView::createViewWidget(const ViewInfo& info)
{
  smtkRGGEditCoreView* view = new smtkRGGEditCoreView(info);
  view->buildUI();
  return view;
}

bool smtkRGGEditCoreView::displayItem(smtk::attribute::ItemPtr item)
{
  return this->qtBaseView::displayItem(item);
}

void smtkRGGEditCoreView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkRGGEditCoreView::valueChanged(smtk::attribute::ItemPtr /*optype*/)
{
}

void smtkRGGEditCoreView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !op->specification())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkRGGEditCoreView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentAtt)
  {
    return;
  }
  // Reset widgets here
}

void smtkRGGEditCoreView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkRGGEditCoreView::attributeModified()
{
  // Always enable apply button here
}

void smtkRGGEditCoreView::onAttItemModified(smtk::extension::qtItem* item)
{
  smtk::attribute::ItemPtr itemPtr = item->getObject();
  // only changing core would update the edit core panel
  if ((itemPtr->name() == "Core") &&
    itemPtr->type() == smtk::attribute::Item::Type::ModelEntityType)
  {
    this->updateEditCorePanel();
  }
}

void smtkRGGEditCoreView::apply()
{
  // First we delete pre existing core related instances, then create new core related
  // instances as requested and in the final step we call edit core operator.
  // By doing so, we make sure that core changes are serialized to smtk.
  // Due to the limitation of PV 3DGlyphMapper that it cannot create instances
  // based on an instance, I choose to decompose an assembly to pins and a duct.
  // In other words, a rgg core also consists of pin instances and duct instances.

  // Extract some assembly information first
  smtk::attribute::AttributePtr ecAtt = this->Internals->CurrentAtt->attribute();
  // Clear "instances to be deleted&added" item
  smtk::attribute::ModelEntityItemPtr dI =
    this->Internals->CurrentAtt->attribute()->findModelEntity("instance to be deleted");
  dI->setNumberOfValues(0);
  smtk::attribute::ModelEntityItemPtr aI =
    this->Internals->CurrentAtt->attribute()->findModelEntity("instance to be added");
  aI->setNumberOfValues(0);

  smtk::model::EntityRefArray coreArray =
    ecAtt->associatedModelEntities<smtk::model::EntityRefArray>();
  smtk::model::Group core;
  smtk::model::Model model;
  bool isHex(false);
  if (coreArray.size() > 0 && coreArray[0].owningModel().hasIntegerProperty("hex"))
  {
    isHex = coreArray[0].owningModel().integerProperty("hex")[0];
    core = coreArray[0].as<smtk::model::Group>();
    model = core.owningModel();
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger(), "An invalid core is provided to the op. Stop"
                                       "the operation.");
    return;
  }

  // Ignore geometry type item since it can only be decided at
  // the creation time
  smtk::attribute::IntItemPtr latticeSizeI = ecAtt->findInt("lattice size");
  int latticeSize[2];
  if (latticeSizeI)
  {
    latticeSizeI->setNumberOfValues(2);
    latticeSize[0] = this->Internals->latticeXSpinBox->value();
    if (isHex)
    { // Geometry type is hex.
      latticeSizeI->setValue(0, this->Internals->latticeXSpinBox->value());
      latticeSizeI->setValue(1, this->Internals->latticeXSpinBox->value());
      latticeSize[1] = this->Internals->latticeXSpinBox->value();
    }
    else
    { // Geometry type is rect
      latticeSizeI->setValue(0, this->Internals->latticeXSpinBox->value());
      latticeSizeI->setValue(1, this->Internals->latticeYSpinBox->value());
      latticeSize[1] = this->Internals->latticeYSpinBox->value();
    }
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "The "
                                                 "lattice size item is not valid");
    return;
  }

  // Assembly and its corresponding layouts
  std::map<smtk::model::EntityRef, std::vector<int> > assemblyToLayout;
  // Call apply function on qtDraw2DLattice to update CurrentCMBCore
  this->Internals->Current2DLattice->apply();

  // FIXME: This map might be a performance bottleneck if we are considering
  // millions of assemblies
  qtLattice coreLattice = this->Internals->CurrentCMBCore->getLattice();
  std::pair<size_t, size_t> dimension = coreLattice.GetDimensions();
  for (size_t i = 0; i < dimension.first; i++)
  {
    size_t jMax = isHex ? ((i == 0) ? 1 : dimension.second * i) : dimension.second;
    for (size_t j = 0; j < jMax; j++)
    {
      smtk::model::EntityRef currentAssy = coreLattice.GetCell(i, j).getPart();
      // Skip invalid and empty assy cell(defined in qtLattice class)
      if (!currentAssy.isValid() ||
        (currentAssy.name() == "Empty Cell" && currentAssy.hasStringProperty("label") &&
          currentAssy.stringProperty("label")[0] == "XX"))
      {
        continue;
      }

      if (assemblyToLayout.find(currentAssy) != assemblyToLayout.end())
      {
        assemblyToLayout[currentAssy].push_back(i);
        assemblyToLayout[currentAssy].push_back(j);
      }
      else
      {
        std::vector<int> layout;
        layout.push_back(i);
        layout.push_back(j);
        assemblyToLayout[currentAssy] = layout;
      }
    }
  }

  // In order to remove/add instances into the core(which is a group under
  // the hood), we would listen
  // to the operationFinished signal from qtModelView. It would be disconnected
  // once the apply process has finished
  // Get needed info from core for deleting and creating new instances usage
  if (!this->uiManager()->activeModelView())
  {
    smtkErrorMacro(smtk::io::Logger(), "An active model view is missing!");
  }
  QObject::connect(this->uiManager()->activeModelView()->operatorsWidget(),
    &qtModelOperationWidget::operationFinished, this, &smtkRGGEditCoreView::onOperationFinished);

  // Delete pre-existing duct and pin instances first
  if (!this->Internals->DeleteOp.lock())
  {
    smtkErrorMacro(smtk::io::Logger(), "RGG Delete Op has not been initialized");
    return;
  }
  smtk::attribute::AttributePtr deleteAtt = this->Internals->DeleteOp.lock()->specification();
  smtk::model::EntityRefArray coreMembers = core.members<smtk::model::EntityRefArray>();
  if (coreMembers.size() > 0)
  {
    deleteAtt->removeAllAssociations();
    for (const auto& member : coreMembers)
    {
      deleteAtt->associateEntity(member);
    }
    this->requestOperation(this->Internals->DeleteOp.lock());
  }

  // Pins and ducts
  // Calculate the starting point first
  auto associatedChildAuxsWithAtt = [](
    smtk::attribute::AttributePtr att, smtk::model::EntityRef ent) {
    // Associate with sub auxgeoms
    smtk::model::AuxiliaryGeometry aux = ent.as<smtk::model::AuxiliaryGeometry>();
    smtk::model::AuxiliaryGeometries children = aux.auxiliaryGeometries();
    for (size_t i = 0; i < children.size(); i++)
    {
      smtk::model::EntityRef e =
        smtk::model::EntityRef(children[i].manager(), children[i].entity());
      att->associateEntity(e);
    }
  };
  smtk::model::FloatList spacing = model.floatProperty("duct thickness");
  double baseX, baseY;
  if (!isHex)
  { // Use the cartesian coordinate where the starting point is located
    // at left bottom
    baseX = -1 * spacing[0] * (static_cast<double>(latticeSize[0]) / 2 - 0.5);
    baseY = -1 * spacing[1] * (static_cast<double>(latticeSize[1]) / 2 - 0.5);
  }
  else
  { // Spacing is the allowable max distance between two adjacent assembly centers
    // Use the cartesian coordinate where the starting point is located at
    // the origin point.
    baseX = baseY = 0.0; // Ignored by calculateHexAssyCoordinate for now
  }

  // Map pins&ducts to their placements
  std::map<smtk::model::EntityRef, std::vector<double> > pDToPs;
  // Cache assembly layouts and coordinates in the attribute
  smtk::attribute::GroupItemPtr alI = ecAtt->findGroup("assemblies and layouts");
  alI->setNumberOfGroups(0); // Clear existing groups
  int alGroupIndex(0);
  for (auto iter = assemblyToLayout.begin(); iter != assemblyToLayout.end(); iter++, alGroupIndex++)
  { // For each assembly, retrieve its pins&duct info, apply the right transformation
    // then add it into pinDuctToLayout map

    smtk::model::EntityRef assy = iter->first;
    std::vector<int> layout = iter->second;
    std::string ductUUID;
    if (assy.hasStringProperty("associated duct"))
    {
      ductUUID = assy.stringProperty("associated duct")[0];
    }
    else
    {
      smtkErrorMacro(smtk::io::Logger(), "The assembly "
          << assy.name() << "does"
                            "not have a valid assocated duct, skipping it in the core");
      continue;
    }

    smtk::model::StringList pinIds;
    if (!assy.hasStringProperty("pins") && ductUUID.empty())
    {
      smtkErrorMacro(smtk::io::Logger(), "The assembly "
          << assy.name() << "does"
                            "not have pin ids and duct id cached, skipping it in the core");
      continue;
    }
    else
    {
      pinIds = assy.stringProperty("pins");
    }

    size_t pointSize = layout.size() / 2;
    std::vector<double> coordinates;
    coordinates.reserve(pointSize * 3);
    for (size_t index = 0; index < pointSize; index++)
    {
      double x, y;
      if (isHex)
      {
        calculateHexAssyCoordinate(x, y, spacing[0], layout[2 * index], layout[2 * index + 1]);
      }
      else
      {
        // In schema planner, x and y axis are following Qt's pattern.
        // Here we just follow the traditional coordinate convension
        x = baseX + spacing[0] * layout[2 * index];
        y = baseY + spacing[1] * layout[2 * index + 1];
      }
      coordinates.push_back(x);
      coordinates.push_back(y);
      coordinates.push_back(0);
      // For each (x,y) pair, add it to every pin and duct in the current assy
      auto addTransformCoordsToMap = [&pDToPs, &x, &y](
        const smtk::model::EntityRef& ent, std::vector<double>& coords) {
        // Apply transformation
        for (size_t i = 0; i < coords.size(); i++)
        {
          if (i % 3 == 0)
          { // X
            coords[i] += x;
          }
          if (i % 3 == 1)
          { // Y
            coords[i] += y;
          }
        }

        if (pDToPs.find(ent) != pDToPs.end())
        { // TODO: Possible performance bottleneck
          pDToPs[ent].insert(pDToPs[ent].end(), coords.begin(), coords.end());
        }
        else
        {
          pDToPs[ent] = coords;
        }
      };
      // Duct
      smtk::model::EntityRef duct = smtk::model::EntityRef(assy.manager(), ductUUID);
      std::vector<double> ductCoords = { 0, 0, 0 };
      addTransformCoordsToMap(duct, ductCoords);
      // Pins
      for (const auto pinId : pinIds)
      {
        smtk::model::EntityRef pin = smtk::model::EntityRef(assy.manager(), pinId);
        if (!assy.hasFloatProperty(pinId))
        {
          smtkErrorMacro(smtk::io::Logger(), "Assembly "
              << assy.name() << "does"
                                "not have pin "
              << pin.name() << "'s coordinates, skipping it in the core");
          continue;
        }
        std::vector<double> pinCoords = assy.floatProperty(pinId);
        addTransformCoordsToMap(pin, pinCoords);
      }
    }
    // Update pins and layouts
    alI->appendGroup();
    smtk::attribute::StringItemPtr assyUUIDI =
      smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(alI->item(alGroupIndex, 0));
    smtk::attribute::IntItemPtr schemaPlanI =
      smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(alI->item(alGroupIndex, 1));
    smtk::attribute::DoubleItemPtr coordinatesI =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(alI->item(alGroupIndex, 2));
    assyUUIDI->setValue(iter->first.entity().toString());
    schemaPlanI->setValues(iter->second.begin(), iter->second.end());
    coordinatesI->setValues(coordinates.begin(), coordinates.end());
  }

  if (!this->Internals->CreateInstanceOp.lock())
  {
    smtkErrorMacro(smtk::io::Logger(), "Create instances Op has not been initialized");
    return;
  }
  smtk::attribute::AttributePtr cIAtt = this->Internals->CreateInstanceOp.lock()->specification();
  if (!cIAtt)
  {
    smtkErrorMacro(smtk::io::Logger(), "Create instances Op does not have a valid attribute");
    return;
  }

  // Create instances for each pin and duct
  for (auto iter = pDToPs.begin(); iter != pDToPs.end(); iter++)
  {
    std::vector<double>& coords = iter->second;

    cIAtt->removeAllAssociations();
    associatedChildAuxsWithAtt(cIAtt, iter->first);

    smtk::attribute::StringItemPtr plaRule = cIAtt->findString("placement rule");
    smtk::attribute::GroupItemPtr placementsI =
      smtk::dynamic_pointer_cast<smtk::attribute::GroupItem>(plaRule->activeChildItem(0));
    placementsI->setNumberOfGroups(1);
    size_t pointSize = coords.size() / 3;
    for (size_t index = 0; index < pointSize; index++)
    {
      if (index > 0)
      {
        placementsI->appendGroup();
      }
      smtk::attribute::DoubleItemPtr coordinatesI =
        smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(placementsI->item(index, 0));
      coordinatesI->setValue(0, coords[index * 3]);
      coordinatesI->setValue(1, coords[index * 3 + 1]);
      coordinatesI->setValue(2, coords[index * 3 + 2]);
    }
    cIAtt->findModelEntity("snap to entity")->setIsEnabled(false);
    this->requestOperation(this->Internals->CreateInstanceOp.lock());
  }

  QObject::disconnect(this->uiManager()->activeModelView()->operatorsWidget(),
    &qtModelOperationWidget::operationFinished, this, &smtkRGGEditCoreView::onOperationFinished);

  // Populate the op attribute and invoke the operation
  // Associating assembly and duct is taken care of by the qtAttribute
  smtk::attribute::StringItemPtr nameI = ecAtt->findString("name");
  if (nameI)
  {
    nameI->setValue(this->Internals->nameLineEdit->text().toStdString());
  }

  smtk::attribute::StringItemPtr geometryTypeI = ecAtt->findString("geometry type");
  if (geometryTypeI)
  {
    geometryTypeI->setValue(isHex ? "Hex" : "Rect");
  }

  smtk::attribute::DoubleItemPtr heightI = ecAtt->findDouble("height");
  if (heightI)
  {
    heightI->setValue(this->Internals->heightLineEdit->text().toDouble());
  }
  smtk::attribute::DoubleItemPtr zOI = ecAtt->findDouble("z origin");
  if (zOI)
  {
    zOI->setValue(this->Internals->zOriginLineEdit->text().toDouble());
  }
  if (isHex)
  {
    smtk::attribute::DoubleItemPtr dtI = ecAtt->findDouble("duct thickness");
    if (dtI)
    {
      dtI->setValue(this->Internals->ductXThicknessSpinBox->value());
    }
  }
  else
  {
    smtk::attribute::DoubleItemPtr dtXI = ecAtt->findDouble("duct thickness X");
    if (dtXI)
    {
      dtXI->setValue(this->Internals->ductXThicknessSpinBox->value());
    }
    smtk::attribute::DoubleItemPtr dtYI = ecAtt->findDouble("duct thickness Y");
    if (dtYI)
    {
      dtYI->setValue(this->Internals->ductYThicknessSpinBox->value());
    }
  }
  this->requestOperation(this->Internals->EditCoreOp.lock());
}

void smtkRGGEditCoreView::launchSchemaPlanner()
{
  if (!this->Internals->SchemaPlanner)
  {
    QWidget* dockP = NULL;
    foreach (QWidget* widget, QApplication::topLevelWidgets())
    {
      if (widget->inherits("QMainWindow"))
      {
        dockP = widget;
        break;
      }
    }
    this->Internals->SchemaPlanner = new QDockWidget(dockP);
    this->Internals->SchemaPlanner->setObjectName("rggCoreSchemaPlanner");
    this->Internals->SchemaPlanner->setWindowTitle("Core Schema Planner");
    auto sp = this->Internals->SchemaPlanner;
    sp->setFloating(true);
    sp->setWidget(this->Internals->Current2DLattice);
    sp->raise();
    sp->show();
    this->Internals->Current2DLattice->rebuild();
  }
  else
  {
    this->Internals->SchemaPlanner->raise();
    this->Internals->SchemaPlanner->show();
  }
}

void smtkRGGEditCoreView::onOperationFinished(const OperatorResult& result)
{
  if (result->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    return;
  }
  // Remove expunged entities in the core
  smtk::attribute::ModelEntityItem::Ptr remEntities = result->findModelEntity("expunged");
  smtk::model::EntityRefArray::const_iterator it;
  smtk::attribute::ModelEntityItemPtr dI =
    this->Internals->CurrentAtt->attribute()->findModelEntity("instance to be deleted");
  if (dI)
  {
    for (it = remEntities->begin(); it != remEntities->end(); ++it)
    {
      dI->appendValue(*it);
    }
  }

  smtk::attribute::ModelEntityItem::Ptr newEntities = result->findModelEntity("created");
  smtk::attribute::ModelEntityItemPtr aI =
    this->Internals->CurrentAtt->attribute()->findModelEntity("instance to be added");
  for (it = newEntities->begin(); it != newEntities->end(); ++it)
  {
    aI->appendValue(*it);
  }
}

void smtkRGGEditCoreView::updateAttributeData()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
  {
    return;
  }

  if (this->Internals->CurrentAtt)
  {
    delete this->Internals->CurrentAtt;
  }

  int i = view->details().findChild("AttributeTypes");
  if (i < 0)
  {
    return;
  }
  smtk::view::View::Component& comp = view->details().child(i);
  std::string eaName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  {
    smtk::view::View::Component& attComp = comp.child(ci);
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      if (optype == "edit core")
      {
        eaName = optype;
        break;
      }
    }
  }
  if (eaName.empty())
  {
    return;
  }

  smtk::model::OperatorPtr editCoreOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(eaName);
  this->Internals->EditCoreOp = editCoreOp;

  // Delete Op
  if (!this->Internals->DeleteOp.lock())
  {
    std::string dName = "delete";
    smtk::model::OperatorPtr deleteOp =
      this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(dName);
    if (!deleteOp)
    { // We need to make sure that the "delete" op has been initialized
      smtkErrorMacro(smtk::io::Logger::instance(), "Fail to create \"delete\" operator");
    }
    this->Internals->DeleteOp = deleteOp;
  }

  // Create instance Op
  if (!this->Internals->CreateInstanceOp.lock())
  {
    std::string ciName = "create instances";
    smtk::model::OperatorPtr createInstancesOp =
      this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(ciName);
    if (!createInstancesOp)
    { // We need to make sure that the "create instances" op has been initialized
      smtkErrorMacro(smtk::io::Logger::instance(), "Fail to create \"create instance\" operator");
    }
    this->Internals->CreateInstanceOp = createInstancesOp;
  }

  smtk::attribute::AttributePtr att = editCoreOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
  if (this->Internals->CurrentAtt)
  {
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::modified, this,
      &smtkRGGEditCoreView::attributeModified);
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::itemModified, this,
      &smtkRGGEditCoreView::onAttItemModified);

    this->updateEditCorePanel();
  }
}

void smtkRGGEditCoreView::createWidget()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  QVBoxLayout* parentLayout = dynamic_cast<QVBoxLayout*>(this->parentWidget()->layout());

  // Delete any pre-existing widget
  if (this->Widget)
  {
    if (parentLayout)
    {
      parentLayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  // Create a new frame and lay it out
  this->Widget = new QFrame(this->parentWidget());
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  // QUESTION: You might need to keep tracking of the widget
  QWidget* tempWidget = new QWidget(this->parentWidget());
  this->Internals->CurrentWidget = tempWidget;
  this->Internals->setupUi(tempWidget);
  tempWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  layout->addWidget(tempWidget, 1);

  QObject::disconnect(this->uiManager()->activeModelView());
  QObject::connect(this->uiManager()->activeModelView(),
    &smtk::extension::qtModelView::operationCancelled, this, &smtkRGGEditCoreView::cancelOperation);

  QObject::connect(this->Internals->launchSchemaPlannerButton, &QPushButton::clicked, this,
    &smtkRGGEditCoreView::launchSchemaPlanner);
  // Modify layers/ x and y value would update the schema planner
  QObject::connect(this->Internals->latticeXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
    this->Internals->Current2DLattice, &qtDraw2DLattice::setLatticeXorLayers);
  QObject::connect(this->Internals->latticeYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
    this->Internals->Current2DLattice, &qtDraw2DLattice::setLatticeY);
  // Reset schema
  QObject::connect(this->Internals->resetSchemaPlannerButton, &QPushButton::clicked,
    this->Internals->Current2DLattice, &qtDraw2DLattice::reset);

  // Show help when the info button is clicked. //
  QObject::connect(
    this->Internals->infoButton, &QPushButton::released, this, &smtkRGGEditCoreView::onInfo);

  QObject::connect(
    this->Internals->applyButton, &QPushButton::released, this, &smtkRGGEditCoreView::apply);

  this->updateAttributeData();
}

void smtkRGGEditCoreView::updateEditCorePanel()
{
  smtk::attribute::AttributePtr att = this->Internals->CurrentAtt->attribute();
  smtk::model::EntityRefArray ents = att->associatedModelEntities<smtk::model::EntityRefArray>();
  auto populateThePanel = [this, &ents]() {
    // Populate the panel
    smtk::model::Model model = ents[0].owningModel();
    smtk::model::EntityRef core = ents[0];
    if (core.hasStringProperty("name"))
    {
      this->Internals->nameLineEdit->setText(
        QString::fromStdString(core.stringProperty("name")[0]));
    }

    if (model.hasFloatProperty("duct height") && (model.floatProperty("duct height").size() == 2))
    {
      smtk::model::FloatList ductHeight = model.floatProperty("duct height");
      this->Internals->zOriginLineEdit->setText(QString::number(ductHeight[0]));
      this->Internals->heightLineEdit->setText(QString::number(ductHeight[1] - ductHeight[0]));
    }
    else
    {
      smtkErrorMacro(
        smtk::io::Logger(), "Core " << model.name() << " does not have a valid duct height");
    }

    if (model.hasFloatProperty("duct thickness") &&
      (model.floatProperty("duct thickness").size() == 2))
    {
      smtk::model::FloatList dt = model.floatProperty("duct thickness");
      this->Internals->ductXThicknessSpinBox->setValue(dt[0]);
      this->Internals->ductYThicknessSpinBox->setValue(dt[1]);
    }
    else
    {
      smtkErrorMacro(
        smtk::io::Logger(), "Core " << core.name() << " does not have a valid duct height");
    }

    if (model.hasIntegerProperty("hex"))
    {
      bool isHex = model.integerProperty("hex")[0];
      // Lattice
      this->Internals->latticeYLabel->setHidden(isHex);
      this->Internals->latticeYSpinBox->setHidden(isHex);
      this->Internals->latticeXLabel->setHidden(isHex);
      // Duct thickness
      this->Internals->ductXThicknessLabel->setHidden(isHex);
      this->Internals->ductYThicknessLabel->setHidden(isHex);
      this->Internals->ductYThicknessSpinBox->setHidden(isHex);

      // By default the rect model is 4x4 and hex is 1x1.
      if (model.hasIntegerProperty("lattice size") &&
        model.integerProperty("lattice size").size() == 2)
      {
        smtk::model::IntegerList lSize = model.integerProperty("lattice size");
        this->Internals->latticeXSpinBox->setValue(lSize[0]);
        this->Internals->latticeYSpinBox->setValue(lSize[1]);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger(), "Core " << core.name() << " does not have a valid lattice size");
      }
    }
  };

  // Need a valid core
  bool isEnabled(true);
  if ((ents.size() == 0) || (!ents[0].hasStringProperty("rggType")) ||
    (ents[0].stringProperty("rggType")[0] != SMTK_BRIDGE_RGG_CORE))
  { // Its type is not rgg core
    isEnabled = false;
    this->Internals->CurrentSMTKCore = smtk::model::EntityRef(); // Invalid the current smtk core
  }
  else
  {
    if (this->Internals->CurrentSMTKCore == ents[0])
    { // If it's the same, do not reset the schema planner
      // since it might surprise the user
      // We still populate the panel incase the user uses the 'read xrf op'
      // to read from files
      populateThePanel();
      return;
    }
    this->Internals->CurrentSMTKCore = ents[0]; // Update the current smtk core
  }

  if (this->Internals)
  {
    this->Internals->CurrentWidget->setEnabled(isEnabled);
  }

  if (isEnabled)
  {
    // Create/update current rggNucCore
    if (this->Internals->SMTKCoreToCMBCore.contains(ents[0]))
    {
      this->Internals->CurrentCMBCore = this->Internals->SMTKCoreToCMBCore.value(ents[0]);
    }
    else
    {
      rggNucCore* assy = new rggNucCore(ents[0]);
      this->Internals->SMTKCoreToCMBCore[ents[0]] = assy;
      this->Internals->CurrentCMBCore = assy;
    }
    this->Internals->Current2DLattice->setLattice(this->Internals->CurrentCMBCore);
    populateThePanel();
  }
}

void smtkRGGEditCoreView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}
