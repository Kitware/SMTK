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

#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/operators/CreateAssembly.h"
#include "smtk/bridge/rgg/operators/CreateDuct.h"
#include "smtk/bridge/rgg/operators/EditAssembly.h"

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
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

using namespace smtk::model;
using namespace smtk::extension;
using namespace smtk::bridge::rgg;
static const double cos30 = 0.86602540378443864676372317075294;
static const int degreesHex[6] = { -120, -60, 0, 60, 120, 180 };
static const int degreesRec[4] = { -90, 0, 90, 180 };

class smtkRGGEditAssemblyViewInternals : public Ui::RGGEditAssemblyParameters
{
public:
  smtkRGGEditAssemblyViewInternals() {}

  ~smtkRGGEditAssemblyViewInternals()
  {
    if (CurrentAtt)
    {
      delete CurrentAtt;
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
  QPointer<QDialog> SchemaPlanner;

  smtk::weak_ptr<smtk::model::Operator> CurrentOp;
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
  this->requestOperation(this->Internals->CurrentOp.lock());
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
  // only changing assembly would update edit assembly panel
  if ((itemPtr->name() == "assembly" || itemPtr->name() == "associated duct") &&
    itemPtr->type() == smtk::attribute::Item::Type::ModelEntityType)
  {
    this->updateEditAssemblyPanel();
  }
}

void smtkRGGEditAssemblyView::apply()
{
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
  smtk::model::FloatList pitches, thicknesses;
  if (duct.hasFloatProperty("pitch"))
  {
    pitches = duct.floatProperty("pitch");
    if (pitches.size() != 2)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "duct " << duct.name() << " does not have a valid pitch");
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
  double thickness0(std::numeric_limits<double>::max()),
    thickness1(std::numeric_limits<double>::max());
  for (auto i = 0; i < thicknesses.size() / 2; i++)
  {
    double currentT0 = pitches[0] * thicknesses[i * 2];
    double currentT1 = pitches[1] * thicknesses[i * 2 + 1];
    thickness0 = (currentT0 < thickness0) ? currentT0 : thickness0;
    thickness1 = (currentT1 < thickness1) ? currentT1 : thickness1;
  }
  // Following the logic in RGG code cmbNucAssembly::calculatePitch function L318
  // According to Juda, the provider is provided by their vendor
  // TODO: Improve or add custom function
  if (isHex)
  {
    const double d =
      thickness0 - thickness0 * 0.035; // make it slightly smaller to make exporting happy
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
    this->Internals->SchemaPlanner = new QDialog;
    this->Internals->SchemaPlanner->exec();
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
  std::string defName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  {
    smtk::view::View::Component& attComp = comp.child(ci);
    //std::cout << "  component " << attComp.name() << "\n";
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      //std::cout << "    component type " << optype << "\n";
      if (optype == "edit assembly")
      {
        defName = optype;
        break;
      }
    }
  }
  if (defName.empty())
  {
    return;
  }

  smtk::model::OperatorPtr createAssemblyOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(defName);
  this->Internals->CurrentOp = createAssemblyOp;

  smtk::attribute::AttributePtr att = createAssemblyOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
  if (this->Internals->CurrentAtt)
  {
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::modified, this,
      &smtkRGGEditAssemblyView::attributeModified);
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::itemModified, this,
      &smtkRGGEditAssemblyView::onAttItemModified);
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

  this->updateAttributeData();

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

  // Show help when the info button is clicked.
  QObject::connect(
    this->Internals->infoButton, &QPushButton::released, this, &smtkRGGEditAssemblyView::onInfo);

  QObject::connect(
    this->Internals->applyButton, &QPushButton::released, this, &smtkRGGEditAssemblyView::apply);

  this->updateEditAssemblyPanel();
}

void smtkRGGEditAssemblyView::updateEditAssemblyPanel()
{
  smtk::attribute::AttributePtr att = this->Internals->CurrentAtt->attribute();
  smtk::model::EntityRefArray ents = att->associatedModelEntities<smtk::model::EntityRefArray>();
  // Need a valid assembly
  bool isEnabled(true);
  if ((ents.size() == 0) || (!ents[0].hasStringProperty("rggType")) ||
    (ents[0].stringProperty("rggType")[0] != SMTK_BRIDGE_RGG_ASSEMBLY))
  { // Its type is not rgg assembly
    isEnabled = false;
  }
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

  if (this->Internals)
  {
    this->Internals->scrollArea->setEnabled(isEnabled);
  }

  if (isEnabled)
  {
    std::cout << "updateEditAssemblyPanel" << std::endl;
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
    if (duct.hasIntegerProperty("hex"))
    {
      bool isHex = duct.integerProperty("hex")[0];
      // Lattice
      this->Internals->latticeYLabel->setHidden(isHex);
      this->Internals->latticeYSpinBox->setHidden(isHex);
      this->Internals->latticeXLabel->setText(
        QString::fromStdString(isHex ? "Number of Layers" : "X"));
      // By default rect assembly is 4x4 and hex is 1x1.
      this->Internals->latticeXSpinBox->setValue(isHex ? 1 : 4);
      this->Internals->latticeYSpinBox->setValue(4);
      // Make sure that the label is expanded properly
      this->Internals->latticeXLabel->setMinimumWidth(isHex ? 120 : 20);
      // Pitch
      this->Internals->pitchYLabel->setHidden(isHex);
      this->Internals->pitchYSpinBox->setHidden(isHex);
      this->Internals->pitchXLabel->setText(
        QString::fromStdString(isHex ? "Pitch: " : "Pitch X: "));
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
