//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/rgg/plugin/smtkRGGEditAssemblyView.h"
#include "smtk/bridge/rgg/plugin/ui_smtkRGGEditAssemblyParameters.h"

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
#include "smtk/bridge/rgg/operators/EditAssembly.h"

#include "smtk/bridge/rgg/qt/qtDraw2DLattice.h"
#include "smtk/bridge/rgg/qt/rggNucAssembly.h"

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
static const int degreesHex[6] = { -120, -60, 0, 60, 120, 180 };
static const int degreesRec[4] = { -90, 0, 90, 180 };
// 0', 60', 120, 180', 240', 300'
static const double cosSinAngles[6][2] = { { 1.0, 0.0 }, { cos60, -cos30 }, { -cos60, -cos30 },
  { -1.0, 0.0 }, { -cos60, cos30 }, { cos60, cos30 } };

namespace
{

void calculateDuctMinimimThickness(
  const smtk::model::EntityRef& duct, double& thickness0, double& thickness1)
{
  smtk::model::FloatList pitches, thicknesses;
  if (duct.owningModel().hasFloatProperty("duct thickness"))
  {
    pitches = duct.owningModel().floatProperty("duct thickness");
    if (pitches.size() != 2)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "duct "
          << duct.name() << "'s owning model does not have a valid pitch");
      return;
    }
  }
  if (duct.hasFloatProperty("thicknesses(normalized)"))
  {
    thicknesses = duct.floatProperty("thicknesses(normalized)");
    if (thicknesses.size() / 2 < 1)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "duct " << duct.name() << " does not have valid thicknesses");
      return;
    }
  }
  thickness0 = std::numeric_limits<double>::max();
  thickness1 = std::numeric_limits<double>::max();
  for (auto i = 0; i < thicknesses.size() / 2; i++)
  {
    double currentT0 = pitches[0] * thicknesses[i * 2];
    double currentT1 = pitches[1] * thicknesses[i * 2 + 1];
    thickness0 = (currentT0 < thickness0) ? currentT0 : thickness0;
    thickness1 = (currentT1 < thickness1) ? currentT1 : thickness1;
  }
}

// Calculate the x,y coordinates of the current pin in the hex grid
void calculateHexPinCoordinate(
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

    x = xBT * cosValue - yBT * sinValue;
    y = yBT * cosValue + xBT * sinValue;
  }
}
}

class smtkRGGEditAssemblyViewInternals : public Ui::RGGEditAssemblyParameters
{
public:
  smtkRGGEditAssemblyViewInternals()
  {
    this->CurrentCMBAssy = nullptr;
    this->Current2DLattice = new qtDraw2DLattice(this->SchemaPlanner);
    this->Current2DLattice->setFrameShape(QFrame::NoFrame);
  }

  ~smtkRGGEditAssemblyViewInternals()
  {
    if (CurrentAtt)
    {
      delete CurrentAtt;
      CurrentAtt = nullptr;
    }
    auto mapIter = this->SMTKAssyToCMBAssy.begin();
    while (mapIter != this->SMTKAssyToCMBAssy.end())
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
        attInstance->widget()->setObjectName("RGGAssemblyEditor");
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
  // Map smtk assembly(smtk::model::AuxiliaryGeometry) to CMB assembly rggNucAssembly which is used for schema planning
  QMap<smtk::model::EntityRef, rggNucAssembly*> SMTKAssyToCMBAssy;
  rggNucAssembly* CurrentCMBAssy;
  // Used for deciding when to enable the edit panel
  smtk::model::EntityRef CurrentSMTKAssy;
  smtk::model::EntityRef CurrentSMTKDuct;

  smtk::weak_ptr<smtk::model::Operator> CreateInstanceOp;
  smtk::weak_ptr<smtk::model::Operator> DeleteOp;
  smtk::weak_ptr<smtk::model::Operator> EditAssyOp;
};

smtkRGGEditAssemblyView::smtkRGGEditAssemblyView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkRGGEditAssemblyViewInternals();
}

smtkRGGEditAssemblyView::~smtkRGGEditAssemblyView()
{
  delete this->Internals;
}

qtBaseView* smtkRGGEditAssemblyView::createViewWidget(const ViewInfo& info)
{
  smtkRGGEditAssemblyView* view = new smtkRGGEditAssemblyView(info);
  view->buildUI();
  return view;
}

bool smtkRGGEditAssemblyView::displayItem(smtk::attribute::ItemPtr item)
{
  return this->qtBaseView::displayItem(item);
}

void smtkRGGEditAssemblyView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkRGGEditAssemblyView::valueChanged(smtk::attribute::ItemPtr /*optype*/)
{
  //this->requestOperation(this->Internals->EditAssyOp.lock());
}

void smtkRGGEditAssemblyView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !op->specification())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkRGGEditAssemblyView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentAtt)
  {
    return;
  }
  // Reset widgets here
}

void smtkRGGEditAssemblyView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkRGGEditAssemblyView::attributeModified()
{
  // Always enable apply button here
}

void smtkRGGEditAssemblyView::onAttItemModified(smtk::extension::qtItem* item)
{
  smtk::attribute::ItemPtr itemPtr = item->getObject();
  // only changing assembly and duct would update edit assembly panel
  if ((itemPtr->name() == "assembly" || itemPtr->name() == "associated duct") &&
    itemPtr->type() == smtk::attribute::Item::Type::ModelEntityType)
  {
    // If the assembly has a duct assocated with it, update the duct comboBox
    if (itemPtr->name() == "assembly")
    {
      smtk::attribute::AttributePtr att = this->Internals->CurrentAtt->attribute();
      smtk::model::EntityRefArray ents =
        att->associatedModelEntities<smtk::model::EntityRefArray>();
      if ((ents.size() > 0) && ents[0].hasStringProperty("associated duct"))
      {
        smtk::model::EntityRef duct =
          smtk::model::EntityRef(ents[0].manager(), ents[0].stringProperty("associated duct")[0]);
        smtk::attribute::ModelEntityItemPtr ductI =
          this->Internals->CurrentAtt->attribute()->findModelEntity("associated duct");
        ductI->setValue(duct);
        // FIXME: Even though I set the right duct, the qtComboBox would not
        // update its text to reflect the change.
      }
    }
    this->updateEditAssemblyPanel();
  }
}

void smtkRGGEditAssemblyView::apply()
{
  // First we delete pre existing duct and pins, then create new duct
  // and pins as requested and in the final step we call edit assembly operator.
  // By doing so, we make sure that assmembly changes are serialized to smtk.

  // Extract some assembly information first
  smtk::attribute::AttributePtr eaAtt = this->Internals->CurrentAtt->attribute();
  // Clear "instances to be deleted&added" item
  smtk::attribute::ModelEntityItemPtr dI =
    this->Internals->CurrentAtt->attribute()->findModelEntity("instance to be deleted");
  dI->setNumberOfValues(0);
  smtk::attribute::ModelEntityItemPtr aI =
    this->Internals->CurrentAtt->attribute()->findModelEntity("instance to be added");
  aI->setNumberOfValues(0);
  // Ignore geometry type item since it can only be decided at
  // creation time
  smtk::model::EntityRefArray assemblyArray =
    eaAtt->associatedModelEntities<smtk::model::EntityRefArray>();
  smtk::attribute::IntItemPtr latticeSizeI = eaAtt->findInt("lattice size");
  int latticeSize[2];
  if (latticeSizeI)
  {
    latticeSizeI->setNumberOfValues(2);
    latticeSize[0] = this->Internals->latticeXSpinBox->value();
    if (this->Internals->pitchYSpinBox->isHidden())
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
  }

  bool isHex(false);
  if (assemblyArray.size() > 0 && assemblyArray[0].owningModel().hasIntegerProperty("hex"))
  {
    isHex = assemblyArray[0].owningModel().integerProperty("hex")[0];
  }
  std::map<smtk::model::EntityRef, std::vector<int> > entityToLayout;
  // Call apply function on qtDraw2DLattice to update CurrentCMBAssy
  this->Internals->Current2DLattice->apply();
  // Pins and layouts
  // FIXME: This map might be a performance bottleneck if we are considering
  // millions of pins
  auto cmbAssembly = this->Internals->CurrentCMBAssy;
  qtLattice assyLattice = cmbAssembly->getLattice();
  std::pair<size_t, size_t> dimension = assyLattice.GetDimensions();
  for (size_t i = 0; i < dimension.first; i++)
  {
    size_t jMax = isHex ? ((i == 0) ? 1 : dimension.second * i) : dimension.second;
    for (size_t j = 0; j < jMax; j++)
    {
      smtk::model::EntityRef currentPin = assyLattice.GetCell(i, j).getPart();
      // Skip invalid and empty pin cell(defined in qtLattice class)
      if (!currentPin.isValid() ||
        (currentPin.name() == "Empty Cell" && currentPin.hasStringProperty("label") &&
          currentPin.stringProperty("label")[0] == "XX"))
      {
        continue;
      }

      if (entityToLayout.find(currentPin) != entityToLayout.end())
      {
        entityToLayout[currentPin].push_back(i);
        entityToLayout[currentPin].push_back(j);
      }
      else
      {
        std::vector<int> layout;
        layout.push_back(i);
        layout.push_back(j);
        entityToLayout[currentPin] = layout;
      }
    }
  }

  // In order to remove/add instances into the assembly(which is a group under
  // the hood), we would listen
  // to the operationFinished signal from qtModelView. It would be disconnected
  // once the apply process has finished
  // Get needed info from assembly for deleting and creating new instances usage
  if (!this->uiManager()->activeModelView())
  {
    smtkErrorMacro(smtk::io::Logger(), "An active model view is missing!");
  }
  QObject::connect(this->uiManager()->activeModelView()->operatorsWidget(),
    &qtModelOperationWidget::operationFinished, this,
    &smtkRGGEditAssemblyView::onOperationFinished);

  // Delete pre-existing duct and pin instances first
  if (!this->Internals->DeleteOp.lock())
  {
    smtkErrorMacro(smtk::io::Logger(), "RGG Delete Op has not been initialized");
    return;
  }
  smtk::attribute::AttributePtr deleteAtt = this->Internals->DeleteOp.lock()->specification();
  smtk::model::Group assembly = eaAtt->associatedModelEntities<smtk::model::Groups>()[0];
  smtk::model::EntityRefArray assemblyMembers = assembly.members<smtk::model::EntityRefArray>();
  if (assemblyMembers.size() > 0)
  {
    deleteAtt->removeAllAssociations();
    for (const auto& member : assemblyMembers)
    {
      deleteAtt->associateEntity(member);
    }
    this->requestOperation(this->Internals->DeleteOp.lock());
  }

  // FIXME: We have to explictly call create instances op due to bugs in CMB5 that
  // we can not create several instances at the same time
  // Duct then pins
  auto associatedChildAuxsWithAtt = [](
    smtk::attribute::AttributePtr att, smtk::model::EntityRef ent) {
    // Associate with sub auxgeoms
    smtk::model::AuxiliaryGeometry aux = ent.as<smtk::model::AuxiliaryGeometry>();
    smtk::model::AuxiliaryGeometries children = aux.auxiliaryGeometries();
    for (size_t i = 0; i < children.size(); i++)
    {
      smtk::model::EntityRef ent =
        smtk::model::EntityRef(children[i].manager(), children[i].entity());
      att->associateEntity(ent);
    }
  };

  smtk::model::EntityRef duct = eaAtt->findModelEntity("associated duct")->value();
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
  cIAtt->removeAllAssociations();
  associatedChildAuxsWithAtt(cIAtt, duct);
  smtk::attribute::StringItemPtr plaRule = cIAtt->findString("placement rule");
  smtk::attribute::GroupItemPtr placementsI =
    smtk::dynamic_pointer_cast<smtk::attribute::GroupItem>(plaRule->activeChildItem(0));
  placementsI->setNumberOfGroups(1);
  smtk::attribute::DoubleItemPtr coordinatesI =
    smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(placementsI->item(0, 0));
  coordinatesI->setValue(0, 0);
  coordinatesI->setValue(1, 0);
  coordinatesI->setValue(2, 0);
  cIAtt->findModelEntity("snap to entity")->setIsEnabled(false);
  this->requestOperation(this->Internals->CreateInstanceOp.lock());

  // Pins
  double thickness0(std::numeric_limits<double>::max()),
    thickness1(std::numeric_limits<double>::max());
  calculateDuctMinimimThickness(duct, thickness0, thickness1);
  this->calculatePitches(); // In case the user has changed duct properties.
  std::vector<double> spacing = { 0, 0 };
  double baseX, baseY;
  if (!isHex)
  { // Use the cartesian coordinate where the starting point is located
    // at left bottom
    // TODO: Use the pitch defined in the file?
    spacing[0] = thickness0 / static_cast<double>(latticeSize[0]);
    spacing[1] = thickness0 / static_cast<double>(latticeSize[1]);
    baseX = -1 * thickness0 / 2 + spacing[0] / 2;
    baseY = -1 * thickness1 / 2 + spacing[0] / 2;
  }
  else
  { // Spacing is the allowable max distance between two adjacent pin centers
    // Use the cartesian coordinate where the starting point is located at
    // the origin point.
    spacing[0] = spacing[1] = this->Internals->pitchXSpinBox->value();
    baseX = baseY = 0.0; // Ignored by calculateHexPinCoordinate for now
  }

  smtk::attribute::GroupItemPtr plI = eaAtt->findGroup("pins and layouts");
  plI->setNumberOfGroups(0); // Clear existing groups
  int plGroupIndex(0);
  for (auto iter = entityToLayout.begin(); iter != entityToLayout.end(); iter++, plGroupIndex++)
  {
    cIAtt->removeAllAssociations();
    // Associate with sub auxgeoms
    associatedChildAuxsWithAtt(cIAtt, iter->first);

    std::vector<int> layout = iter->second;
    std::vector<double> coordinates;
    coordinates.reserve(layout.size() / 2 * 3);
    smtk::attribute::StringItemPtr plaRule = cIAtt->findString("placement rule");
    smtk::attribute::GroupItemPtr placementsI =
      smtk::dynamic_pointer_cast<smtk::attribute::GroupItem>(plaRule->activeChildItem(0));
    placementsI->setNumberOfGroups(1);
    size_t numberOfPair = layout.size() / 2;
    for (size_t index = 0; index < numberOfPair; index++)
    {
      if (index > 0)
      {
        placementsI->appendGroup();
      }
      smtk::attribute::DoubleItemPtr coordinatesI =
        smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(placementsI->item(index, 0));
      double x, y;
      if (isHex)
      {
        calculateHexPinCoordinate(x, y, spacing[0], layout[2 * index], layout[2 * index + 1]);
      }
      else
      { // Question
        // In schema planner, x and y axis are exchanged. Here we just follow the traditional coordinate convension
        x = baseX + spacing[0] * layout[2 * index];
        y = baseY + spacing[1] * layout[2 * index + 1];
      }
      coordinatesI->setValue(0, x);
      coordinatesI->setValue(1, y);
      coordinatesI->setValue(2, 0);

      coordinates.push_back(x);
      coordinates.push_back(y);
      coordinates.push_back(0);
    }
    cIAtt->findModelEntity("snap to entity")->setIsEnabled(false);
    this->requestOperation(this->Internals->CreateInstanceOp.lock());
    // Update pins and layouts
    plI->appendGroup();
    smtk::attribute::StringItemPtr pinUUIDI =
      smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(plI->item(plGroupIndex, 0));
    smtk::attribute::IntItemPtr schemaPlanI =
      smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(plI->item(plGroupIndex, 1));
    smtk::attribute::DoubleItemPtr coordinatesI =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(plI->item(plGroupIndex, 2));
    pinUUIDI->setValue(iter->first.entity().toString());
    schemaPlanI->setValues(iter->second.begin(), iter->second.end());
    coordinatesI->setValues(coordinates.begin(), coordinates.end());
  }
  QObject::disconnect(this->uiManager()->activeModelView()->operatorsWidget(),
    &qtModelOperationWidget::operationFinished, this,
    &smtkRGGEditAssemblyView::onOperationFinished);

  // Populate the op attribute and invoke the operation
  // Associating assembly and duct is taken care of by the qtAttribute
  smtk::attribute::StringItemPtr nameI = eaAtt->findString("name");
  if (nameI)
  {
    nameI->setValue(this->Internals->nameLineEdit->text().toStdString());
  }
  smtk::attribute::StringItemPtr labelI = eaAtt->findString("label");
  if (labelI)
  {
    labelI->setValue(this->Internals->labelLineEdit->text().toStdString());
  }

  smtk::attribute::VoidItemPtr centerPinsI = eaAtt->findVoid("center pins");
  if (centerPinsI)
  {
    centerPinsI->setIsEnabled(this->Internals->centerPinsCheckbox->isChecked());
  }
  smtk::attribute::DoubleItemPtr pitchesI = eaAtt->findDouble("pitches");
  if (pitchesI)
  {
    pitchesI->setNumberOfValues(2);
    if (this->Internals->pitchYSpinBox->isHidden())
    { // Geometry type is hex.
      pitchesI->setValue(0, this->Internals->pitchXSpinBox->value());
      pitchesI->setValue(1, this->Internals->pitchXSpinBox->value());
    }
    else
    {
      pitchesI->setValue(0, this->Internals->pitchXSpinBox->value());
      pitchesI->setValue(1, this->Internals->pitchYSpinBox->value());
    }
  }
  smtk::attribute::IntItemPtr zAxisI = eaAtt->findInt("z axis");
  if (zAxisI)
  {
    zAxisI->setValue(this->Internals->zAxisRotationComboBox->currentText().toDouble());
  }
  this->requestOperation(this->Internals->EditAssyOp.lock());
}

void smtkRGGEditAssemblyView::calculatePitches()
{
  bool isHex = this->Internals->latticeYLabel->isHidden();
  double pitchX, pitchY;
  int latticeX = this->Internals->latticeXSpinBox->value(),
      latticeY = this->Internals->latticeYSpinBox->value();
  // Get the inner duct size
  smtk::attribute::ModelEntityItemPtr ductItem =
    this->Internals->CurrentAtt->attribute()->findModelEntity("associated duct");
  smtk::model::EntityRef duct;
  if (ductItem)
  {
    duct = ductItem->value(0);
    if (!duct.hasStringProperty("rggType") ||
      duct.stringProperty("rggType")[0] != SMTK_BRIDGE_RGG_DUCT)
    {
      return;
    }
  }
  double thickness0(std::numeric_limits<double>::max()),
    thickness1(std::numeric_limits<double>::max());
  calculateDuctMinimimThickness(duct, thickness0, thickness1);

  // Following the logic in RGG code cmbNucAssembly::calculatePitch function L318
  // According to Juda, the formula is provided by their vendor
  // TODO: Improve or add custom function
  if (isHex)
  {
    const double d =
      thickness0 - thickness0 * 0.035; // @juda Make it slightly smaller to make exporting happy
    pitchX = pitchY = (cos30 * d) / (latticeX + 0.5 * (latticeX - 1));
  }
  else
  {
    pitchX = (thickness0) / (latticeX + 0.5);
    pitchY = (thickness1) / (latticeY + 0.5);
  }
  this->Internals->pitchXSpinBox->setValue(pitchX);
  this->Internals->pitchYSpinBox->setValue(pitchY);
}

void smtkRGGEditAssemblyView::launchSchemaPlanner()
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
    this->Internals->SchemaPlanner->setObjectName("rggAssemblySchemaPlanner");
    this->Internals->SchemaPlanner->setWindowTitle("Assembly Schema Planner");
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

void smtkRGGEditAssemblyView::onOperationFinished(const OperatorResult& result)
{
  if (result->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    return;
  }
  // Remove expunged entities in the assembly
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

void smtkRGGEditAssemblyView::updateAttributeData()
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
      if (optype == "edit assembly")
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

  smtk::model::OperatorPtr editAssemblyOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(eaName);
  this->Internals->EditAssyOp = editAssemblyOp;

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

  smtk::attribute::AttributePtr att = editAssemblyOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
  if (this->Internals->CurrentAtt)
  {
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::modified, this,
      &smtkRGGEditAssemblyView::attributeModified);
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::itemModified, this,
      &smtkRGGEditAssemblyView::onAttItemModified);
    // Associate assembly combobox with the latest assembly
    smtk::attribute::ModelEntityItemPtr assemblyI =
      this->Internals->CurrentAtt->attribute()->associations();
    if (!assemblyI)
    {
      smtkErrorMacro(
        smtk::io::Logger(), "Edit assembly operator does not have an assembly association");
      return;
    }
    smtk::model::Model model = qtActiveObjects::instance().activeModel();
    if (model.hasStringProperty("latest assembly"))
    {
      smtk::model::EntityRef latestAssembly = smtk::model::EntityRef(
        model.manager(), smtk::common::UUID(model.stringProperty("latest assembly")[0]));
      assemblyI->setValue(latestAssembly);
    }

    // Associate duct combobox with the latest duct
    smtk::attribute::ModelEntityItemPtr ductI =
      this->Internals->CurrentAtt->attribute()->findModelEntity("associated duct");
    if (!ductI)
    {
      smtkErrorMacro(smtk::io::Logger(), "Edit assembly operator does not have a duct association");
      return;
    }
    if (model.hasStringProperty("latest duct"))
    {
      smtk::model::EntityRef latestDuct = smtk::model::EntityRef(
        model.manager(), smtk::common::UUID(model.stringProperty("latest duct")[0]));
      ductI->setValue(latestDuct);
    }
    this->updateEditAssemblyPanel();
  }
}

void smtkRGGEditAssemblyView::createWidget()
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
  this->Internals->setupUi(tempWidget);
  tempWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  layout->addWidget(tempWidget, 1);

  QObject::disconnect(this->uiManager()->activeModelView());
  QObject::connect(this->uiManager()->activeModelView(),
    &smtk::extension::qtModelView::operationCancelled, this,
    &smtkRGGEditAssemblyView::cancelOperation);

  QObject::connect(
    this->Internals->centerPinsCheckbox, &QCheckBox::stateChanged, this, [=](int isChecked) {
      this->Internals->pitchXSpinBox->setEnabled(!isChecked);
      this->Internals->pitchYSpinBox->setEnabled(!isChecked);
      this->Internals->calculatePitchButton->setEnabled(!isChecked);
      if (isChecked)
      { // Modifying lattice X and lattice Y should automatically trigger pitch calculation
        QObject::connect(this->Internals->latticeXSpinBox,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          &smtkRGGEditAssemblyView::calculatePitches);
        QObject::connect(this->Internals->latticeYSpinBox,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          &smtkRGGEditAssemblyView::calculatePitches);
      }
      else
      {
        QObject::disconnect(this->Internals->latticeXSpinBox,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          &smtkRGGEditAssemblyView::calculatePitches);
        QObject::disconnect(this->Internals->latticeYSpinBox,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          &smtkRGGEditAssemblyView::calculatePitches);
      }
    });

  // By default, pins are centered so that modifying lattice X and lattice Y
  // should automatically trigger pitch calculation
  QObject::connect(this->Internals->latticeXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
    this, &smtkRGGEditAssemblyView::calculatePitches);
  QObject::connect(this->Internals->latticeYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
    this, &smtkRGGEditAssemblyView::calculatePitches);

  QObject::connect(this->Internals->calculatePitchButton, &QPushButton::clicked, this,
    &smtkRGGEditAssemblyView::calculatePitches);

  QObject::connect(this->Internals->launchSchemaButton, &QPushButton::clicked, this,
    &smtkRGGEditAssemblyView::launchSchemaPlanner);
  // Modify layers/ x and y value would update the schema planner
  QObject::connect(this->Internals->latticeXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
    this->Internals->Current2DLattice, &qtDraw2DLattice::setLatticeXorLayers);
  QObject::connect(this->Internals->latticeYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
    this->Internals->Current2DLattice, &qtDraw2DLattice::setLatticeY);
  // Reset schema
  QObject::connect(this->Internals->resetSchemaPlannerButton, &QPushButton::clicked,
    this->Internals->Current2DLattice, &qtDraw2DLattice::reset);

  // Show help when the info button is clicked.
  QObject::connect(
    this->Internals->infoButton, &QPushButton::released, this, &smtkRGGEditAssemblyView::onInfo);

  QObject::connect(
    this->Internals->applyButton, &QPushButton::released, this, &smtkRGGEditAssemblyView::apply);

  this->updateAttributeData();
}

void smtkRGGEditAssemblyView::updateEditAssemblyPanel()
{
  smtk::attribute::AttributePtr att = this->Internals->CurrentAtt->attribute();
  smtk::model::EntityRefArray ents = att->associatedModelEntities<smtk::model::EntityRefArray>();
  bool isEnabled(true);

  // Need a valid duct
  smtk::attribute::ModelEntityItemPtr ductItem = att->findModelEntity("associated duct");
  smtk::model::EntityRef duct;
  if (ductItem)
  {
    duct = ductItem->value(0);
    if (!duct.hasStringProperty("rggType") ||
      duct.stringProperty("rggType")[0] != SMTK_BRIDGE_RGG_DUCT)
    {
      isEnabled = false;
    }
  }

  // Need a valid assembly
  if ((ents.size() == 0) || (!ents[0].hasStringProperty("rggType")) ||
    (ents[0].stringProperty("rggType")[0] != SMTK_BRIDGE_RGG_ASSEMBLY))
  { // Its type is not rgg assembly
    isEnabled = false;
    // Invalid the current smtk assy
    this->Internals->CurrentSMTKAssy = smtk::model::EntityRef();
  }
  else
  {
    if (this->Internals->CurrentSMTKAssy == ents[0] && this->Internals->CurrentSMTKDuct == duct)
    { // If it's the same, do not reset the schema planner
      // since it might surprise the user
      return;
    }
    this->Internals->CurrentSMTKAssy = ents[0]; // Update current smtk assy
  }
  this->Internals->CurrentSMTKDuct = duct; // Update current smtk duct

  if (this->Internals)
  {
    this->Internals->scrollArea->setEnabled(isEnabled);
  }

  if (isEnabled)
  {
    // Create/update current rggNucAssembly
    if (this->Internals->SMTKAssyToCMBAssy.contains(ents[0]))
    {
      this->Internals->CurrentCMBAssy = this->Internals->SMTKAssyToCMBAssy.value(ents[0]);
    }
    else
    {
      rggNucAssembly* assy = new rggNucAssembly(ents[0]);
      this->Internals->SMTKAssyToCMBAssy[ents[0]] = assy;
      this->Internals->CurrentCMBAssy = assy;
    }
    this->Internals->Current2DLattice->setLattice(this->Internals->CurrentCMBAssy);
    // Fulfill/ Update info of current assembly
    // TODO: Handle switching duct condition
    this->Internals->CurrentCMBAssy->setAssyDuct(duct);

    // Populate the panel
    AuxiliaryGeometry assembly = ents[0].as<AuxiliaryGeometry>();
    if (assembly.hasStringProperty("name"))
    {
      this->Internals->nameLineEdit->setText(
        QString::fromStdString(assembly.stringProperty("name")[0]));
    }
    if (assembly.hasStringProperty("label"))
    {
      this->Internals->labelLineEdit->setText(
        QString::fromStdString(assembly.stringProperty("label")[0]));
    }
    if (assembly.owningModel().hasIntegerProperty("hex"))
    {
      bool isHex = assembly.owningModel().integerProperty("hex")[0];
      // Lattice
      this->Internals->latticeYLabel->setHidden(isHex);
      this->Internals->latticeYSpinBox->setHidden(isHex);
      this->Internals->latticeXLabel->setText(
        QString::fromStdString(isHex ? "Number of Layers" : "X"));
      // By default rect assembly is 4x4 and hex is 1x1.
      if (assembly.hasIntegerProperty("lattice size") &&
        assembly.integerProperty("lattice size").size() == 2)
      {
        smtk::model::IntegerList lSize = assembly.integerProperty("lattice size");
        this->Internals->latticeXSpinBox->setValue(lSize[0]);
        this->Internals->latticeYSpinBox->setValue(lSize[1]);
      }
      else
      {
        smtkErrorMacro(smtk::io::Logger(), "Assembly " << assembly.name()
                                                       << " does not have a valid lattice size");
      }
      // Make sure that the label is expanded properly
      this->Internals->latticeXLabel->setMinimumWidth(isHex ? 120 : 20);
      // Pitch
      this->Internals->pitchYLabel->setHidden(isHex);
      this->Internals->pitchYSpinBox->setHidden(isHex);
      this->Internals->pitchXLabel->setText(
        QString::fromStdString(isHex ? "Pitch: " : "Pitch X: "));

      // Center pins and pitches
      if (assembly.hasIntegerProperty("center pins"))
      {
        this->Internals->centerPinsCheckbox->setChecked(assembly.integerProperty("center pins")[0]);
      }
      if (assembly.hasFloatProperty("pitches") && (assembly.floatProperty("pitches").size() == 2))
      {
        smtk::model::FloatList pitches = assembly.floatProperty("pitches");
        this->Internals->pitchXSpinBox->setValue(pitches[0]);
        this->Internals->pitchYSpinBox->setValue(pitches[1]);
      }

      this->Internals->zAxisRotationComboBox->clear();
      QStringList rotationOptions;
      if (isHex)
      {
        for (auto item : degreesHex)
        {
          rotationOptions << QString::number(item);
        }
      }
      else
      {
        for (auto item : degreesRec)
        {
          rotationOptions << QString::number(item);
        }
      }
      this->Internals->zAxisRotationComboBox->addItems(rotationOptions);
      // Set default rotation to be 0
      this->Internals->zAxisRotationComboBox->setCurrentIndex(isHex ? 2 : 1);
    }
  }
}

void smtkRGGEditAssemblyView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}
