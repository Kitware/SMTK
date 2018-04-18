//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/rgg/plugin/smtkRGGEditMaterialView.h"
#include "smtk/bridge/rgg/plugin/ui_smtkRGGEditMaterialParameters.h"

#include "smtk/bridge/rgg/qt/nuclides/NuclideTable.h"

#include "smtkRGGViewHelper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/Material.h"
#include "smtk/bridge/rgg/operators/CreateModel.h"
#include "smtk/bridge/rgg/operators/EditMaterial.h"

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
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSettings.h"

#include <QDockWidget>
#include <QGraphicsView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cassert>
#include <sstream>

using namespace smtk::extension;
using namespace smtk::bridge::rgg;

class smtkRGGEditMaterialViewInternals : public Ui::RGGEditMaterialParameters
{
public:
  smtkRGGEditMaterialViewInternals() {}

  ~smtkRGGEditMaterialViewInternals()
  {
    if (m_currentAtt)
    {
      delete m_currentAtt;
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
        attInstance->widget()->setObjectName("RGGMaterialEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
      }
      return attInstance;
    }
    return NULL;
  }

  QPointer<qtAttribute> m_currentAtt;
  QPointer<NuclideTable> m_nuclideTable;
  QPointer<QDockWidget> m_nuclideWidget;
  smtk::weak_ptr<smtk::model::Operator> m_currentOp;
};

smtkRGGEditMaterialView::smtkRGGEditMaterialView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkRGGEditMaterialViewInternals();
}

smtkRGGEditMaterialView::~smtkRGGEditMaterialView()
{
  delete this->Internals;
}

qtBaseView* smtkRGGEditMaterialView::createViewWidget(const ViewInfo& info)
{
  smtkRGGEditMaterialView* view = new smtkRGGEditMaterialView(info);
  view->buildUI();
  return view;
}

bool smtkRGGEditMaterialView::displayItem(smtk::attribute::ItemPtr item)
{
  return this->qtBaseView::displayItem(item);
}

void smtkRGGEditMaterialView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkRGGEditMaterialView::launchNuclideTable()
{
  if (!this->Internals->m_nuclideWidget)
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

    this->Internals->m_nuclideWidget =
      dockP->findChild<QDockWidget*>("NuclideTable", Qt::FindDirectChildrenOnly);

    if (this->Internals->m_nuclideWidget)
    {
      this->Internals->m_nuclideTable =
        this->Internals->m_nuclideWidget->findChild<NuclideTable*>();
    }
    else
    {
      this->Internals->m_nuclideWidget = new QDockWidget(dockP);
      this->Internals->m_nuclideWidget->setObjectName("NuclideTable");
      this->Internals->m_nuclideWidget->setWindowTitle("Table of Nuclides");
      this->Internals->m_nuclideWidget->setFloating(true);
      this->Internals->m_nuclideTable = new NuclideTable(this->Internals->m_nuclideWidget);
      this->Internals->m_nuclideWidget->setWidget(this->Internals->m_nuclideTable);
      this->Internals->m_nuclideWidget->resize(500, 500);
    }
  }

  this->Internals->m_nuclideWidget->raise();
  this->Internals->m_nuclideWidget->show();
}

void smtkRGGEditMaterialView::valueChanged(smtk::attribute::ItemPtr /*optype*/)
{
  this->requestOperation(this->Internals->m_currentOp.lock());
}

void smtkRGGEditMaterialView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !op->specification())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkRGGEditMaterialView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !this->Widget || !this->Internals->m_currentAtt)
  {
    return;
  }
  // Reset widgets here
}

void smtkRGGEditMaterialView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkRGGEditMaterialView::attributeModified()
{
  // Always enable apply button here
}

bool smtkRGGEditMaterialView::ableToOperate()
{
  if (this->Internals->labelValue->text().isEmpty() ||
    this->Internals->colorRValue->text().isEmpty() ||
    this->Internals->colorGValue->text().isEmpty() ||
    this->Internals->colorBValue->text().isEmpty() ||
    this->Internals->colorAValue->text().isEmpty() ||
    this->Internals->temperatureValue->text().isEmpty() ||
    this->Internals->thermalCoeffValue->text().isEmpty() ||
    this->Internals->densityValue->text().isEmpty() || this->Internals->densityTypeBox == nullptr ||
    this->Internals->compositionTypeBox == nullptr || this->Internals->componentsTable == nullptr ||
    this->Internals->componentsTable->rowCount() == 0)
  {
    this->Internals->applyButton->setEnabled(false);
    return false;
  }

  // Fill the attribute - read all data from UI
  smtk::attribute::StringItemPtr nameI =
    this->Internals->m_currentAtt->attribute()->findString("name");
  nameI->setValue(this->Internals->materialBox->currentText().toStdString());

  smtk::attribute::StringItemPtr labelI =
    this->Internals->m_currentAtt->attribute()->findString("label");
  labelI->setValue(this->Internals->labelValue->text().toStdString());

  smtk::attribute::DoubleItemPtr colorI =
    this->Internals->m_currentAtt->attribute()->findDouble("color");
  colorI->setValue(0, this->Internals->colorRValue->text().toFloat());
  colorI->setValue(1, this->Internals->colorGValue->text().toFloat());
  colorI->setValue(2, this->Internals->colorBValue->text().toFloat());
  colorI->setValue(3, this->Internals->colorAValue->text().toFloat());

  smtk::attribute::DoubleItemPtr temperatureI =
    this->Internals->m_currentAtt->attribute()->findDouble("temperature");
  temperatureI->setValue(this->Internals->temperatureValue->text().toFloat());

  smtk::attribute::DoubleItemPtr thermalExpansionI =
    this->Internals->m_currentAtt->attribute()->findDouble("thermalExpansion");
  thermalExpansionI->setValue(this->Internals->thermalCoeffValue->text().toFloat());

  smtk::attribute::DoubleItemPtr densityI =
    this->Internals->m_currentAtt->attribute()->findDouble("density");
  densityI->setValue(this->Internals->densityValue->text().toFloat());

  smtk::attribute::StringItemPtr densityTypeI =
    this->Internals->m_currentAtt->attribute()->findString("densityType");
  {
    std::size_t index = 0;
    const smtk::attribute::StringItemDefinition* def =
      static_cast<const smtk::attribute::StringItemDefinition*>(densityTypeI->definition().get());
    bool ok =
      def->getEnumIndex(this->Internals->densityTypeBox->currentText().toStdString(), index);
    if (!ok)
    {
      return false;
    }
    densityTypeI->setDiscreteIndex(static_cast<int>(index));
  }

  smtk::attribute::StringItemPtr compositionTypeI =
    this->Internals->m_currentAtt->attribute()->findString("compositionType");
  {
    std::size_t index = 0;
    const smtk::attribute::StringItemDefinition* def =
      static_cast<const smtk::attribute::StringItemDefinition*>(
        compositionTypeI->definition().get());
    bool ok =
      def->getEnumIndex(this->Internals->compositionTypeBox->currentText().toStdString(), index);
    if (!ok)
    {
      return false;
    }
    compositionTypeI->setDiscreteIndex(static_cast<int>(index));
  }

  smtk::attribute::StringItemPtr componentI =
    this->Internals->m_currentAtt->attribute()->findString("component");
  smtk::attribute::DoubleItemPtr contentI =
    this->Internals->m_currentAtt->attribute()->findDouble("content");

  QTableWidget* cT = this->Internals->componentsTable;
  componentI->setNumberOfValues(cT->rowCount());
  contentI->setNumberOfValues(cT->rowCount());
  for (int i = 0; i < cT->rowCount(); i++)
  {
    if (cT->item(i, 0) == nullptr || cT->cellWidget(i, 1) == nullptr ||
      static_cast<QLineEdit*>(cT->cellWidget(i, 1))->text().isEmpty())
    {
      return false;
    }
    componentI->setValue(i, cT->item(i, 0)->data(Qt::UserRole).toString().toStdString());
    contentI->setValue(i, static_cast<QLineEdit*>(cT->cellWidget(i, 1))->text().toFloat());
  }

  bool able = this->Internals->m_currentOp.lock()->ableToOperate();
  this->Internals->applyButton->setEnabled(able);
  return able;
}

void smtkRGGEditMaterialView::apply()
{
  if (this->ableToOperate())
  {
    this->requestOperation(this->Internals->m_currentOp.lock());
    this->Internals->applyButton->setEnabled(false);
  }
}

void smtkRGGEditMaterialView::updateAttributeData()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
  {
    return;
  }

  if (this->Internals->m_currentAtt)
  {
    delete this->Internals->m_currentAtt;
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
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      if (optype == "edit material")
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

  smtk::model::OperatorPtr editMaterialOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(defName);
  this->Internals->m_currentOp = editMaterialOp;

  smtk::attribute::AttributePtr att = editMaterialOp->specification();
  this->Internals->m_currentAtt = this->Internals->createAttUI(att, this->Widget, this);

  this->updateEditMaterialPanel();
}

void smtkRGGEditMaterialView::createWidget()
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
  // Make sure that we have enough space for the custom widget
  this->Internals->scrollArea->setMinimumHeight(650);

  QObject::disconnect(this->uiManager()->activeModelView());
  QObject::connect(this->uiManager()->activeModelView(),
    &smtk::extension::qtModelView::operationCancelled, this,
    &smtkRGGEditMaterialView::cancelOperation);

  QObject::connect(this->Internals->labelValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  QObject::connect(this->Internals->colorRValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  QObject::connect(this->Internals->colorGValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  QObject::connect(this->Internals->colorBValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  QObject::connect(this->Internals->colorAValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  QObject::connect(this->Internals->temperatureValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  {
    auto validator = new QDoubleValidator();
    validator->setBottom(0.);
    this->Internals->temperatureValue->setValidator(validator);
  }
  QObject::connect(this->Internals->thermalCoeffValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  {
    auto validator = new QDoubleValidator();
    validator->setBottom(0.);
    this->Internals->thermalCoeffValue->setValidator(validator);
  }
  QObject::connect(this->Internals->densityValue, &QLineEdit::textChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  {
    auto validator = new QDoubleValidator();
    validator->setBottom(0.);
    this->Internals->densityValue->setValidator(validator);
  }
  QObject::connect(this->Internals->densityTypeBox, &QComboBox::currentTextChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  QObject::connect(this->Internals->compositionTypeBox, &QComboBox::currentTextChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);
  QObject::connect(this->Internals->componentsTable, &QTableWidget::cellChanged, this,
    &smtkRGGEditMaterialView::ableToOperate);

  QObject::connect(
    this->Internals->infoButton, &QPushButton::clicked, this, &smtkRGGEditMaterialView::onInfo);
  QObject::connect(
    this->Internals->applyButton, &QPushButton::released, this, &smtkRGGEditMaterialView::apply);
  this->Internals->addComponentButton->setToolTip("Add a component");
  QObject::connect(
    this->Internals->componentsTable, &QTableWidget::cellClicked, [=](int row, int col) {
      if (col == 0)
      {
        this->launchNuclideTable();
        this->Internals->m_nuclideTable->disconnect();
        QObject::connect(
          this->Internals->m_nuclideTable, &NuclideTable::nuclideSelected, [=](Nuclide* nuclide) {
            auto item = this->Internals->componentsTable->item(row, col);
            item->setData(Qt::DisplayRole, nuclide->prettyName());
            item->setData(Qt::UserRole, nuclide->name());
            item->setData(Qt::DecorationRole, nuclide->toPixmap());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled |
              Qt::ItemIsEnabled);
          });
      }
    });
  QObject::connect(this->Internals->addComponentButton, &QPushButton::clicked, this, [this]() {
    auto cT = this->Internals->componentsTable;
    cT->blockSignals(true);
    int rowCount = cT->rowCount();
    cT->setRowCount(rowCount + 1);
    auto item = new QTableWidgetItem(QString::fromStdString("<name>"));
    item->setFlags(
      Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    cT->setItem(rowCount, 0, item);
    QLineEdit* edit = new QLineEdit(cT);
    edit->setFrame(false);
    auto validator = new QDoubleValidator(edit);
    validator->setBottom(0.);
    edit->setValidator(validator);
    edit->setText(QString::number(0.));
    cT->setCellWidget(rowCount, 1, edit);
    cT->blockSignals(false);
    cT->cellClicked(rowCount, 0);
  });
  this->Internals->deleteComponentButton->setToolTip("Delete the current selected component");
  QObject::connect(this->Internals->deleteComponentButton, &QPushButton::clicked, this, [this]() {
    if (this->Internals->m_nuclideTable != nullptr)
    {
      this->Internals->m_nuclideTable->disconnect();
    }
    auto cT = this->Internals->componentsTable;
    cT->blockSignals(true);
    int row = cT->currentRow();
    cT->removeRow(row);
    cT->blockSignals(false);
  });

  this->updateAttributeData();
}

void smtkRGGEditMaterialView::updateEditMaterialPanel()
{
  smtk::model::EntityRefArray ents = this->Internals->m_currentAtt->attribute()
                                       ->associatedModelEntities<smtk::model::EntityRefArray>();
  bool isEnabled(true);
  if (ents.size() == 0)
  {
    isEnabled = false;
  }
  if (this->Internals)
  {
    this->Internals->scrollArea->setEnabled(isEnabled);
  }
  if (isEnabled)
  {
    this->setupMaterialComboBox(this->Internals->materialBox);
    this->setupDensityTypeComboBox(this->Internals->densityTypeBox);
    this->setupCompositionTypeComboBox(this->Internals->compositionTypeBox);
    this->materialChanged(this->Internals->materialBox->currentText());
    this->Internals->applyButton->setEnabled(false);
  }
}

void smtkRGGEditMaterialView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}

void smtkRGGEditMaterialView::setupMaterialComboBox(QComboBox* box)
{
  box->disconnect();
  box->clear();
  smtk::model::Model model = qtActiveObjects::instance().activeModel();
  if (!model.hasStringProperty(smtk::bridge::rgg::Material::label) ||
    model.stringProperty(smtk::bridge::rgg::Material::label).empty())
  {
    return;
  }

  smtk::model::StringList& materialDescriptions =
    model.stringProperty(smtk::bridge::rgg::Material::label);

  for (auto& materialDescription : materialDescriptions)
  {
    smtk::bridge::rgg::Material material(materialDescription);
    box->addItem(QString::fromStdString(material.name()));
  }

  QObject::connect(
    box, &QComboBox::currentTextChanged, this, &smtkRGGEditMaterialView::materialChanged);
}

void smtkRGGEditMaterialView::setupDensityTypeComboBox(QComboBox* box)
{
  box->clear();
  const smtk::attribute::StringItemDefinition* def =
    static_cast<const smtk::attribute::StringItemDefinition*>(
      this->Internals->m_currentAtt->attribute()->findString("densityType")->definition().get());

  for (std::size_t i = 0; i < def->numberOfDiscreteValues(); i++)
  {
    box->addItem(QString::fromStdString(def->discreteEnum(i)));
  }
}

void smtkRGGEditMaterialView::setupCompositionTypeComboBox(QComboBox* box)
{
  box->clear();
  const smtk::attribute::StringItemDefinition* def = static_cast<
    const smtk::attribute::StringItemDefinition*>(
    this->Internals->m_currentAtt->attribute()->findString("compositionType")->definition().get());

  for (std::size_t i = 0; i < def->numberOfDiscreteValues(); i++)
  {
    box->addItem(QString::fromStdString(def->discreteEnum(i)));
  }
}

void smtkRGGEditMaterialView::clear()
{
  this->Internals->labelValue->clear();
  this->Internals->colorRValue->clear();
  this->Internals->colorGValue->clear();
  this->Internals->colorBValue->clear();
  this->Internals->colorAValue->clear();
  this->Internals->temperatureValue->clear();
  this->Internals->thermalCoeffValue->clear();
  this->Internals->densityValue->clear();
  this->Internals->componentsTable->clear();
  this->Internals->componentsTable->setRowCount(0);
}

void smtkRGGEditMaterialView::setEnabled(bool choice)
{
  if (choice == false)
  {
    this->Internals->applyButton->setEnabled(false);
    this->clear();
  }

  this->Internals->labelValue->setEnabled(choice);
  this->Internals->colorRValue->setEnabled(choice);
  this->Internals->colorGValue->setEnabled(choice);
  this->Internals->colorBValue->setEnabled(choice);
  this->Internals->colorAValue->setEnabled(choice);
  this->Internals->temperatureValue->setEnabled(choice);
  this->Internals->thermalCoeffValue->setEnabled(choice);
  this->Internals->densityValue->setEnabled(choice);

  this->Internals->densityTypeBox->setEnabled(choice);
  this->Internals->compositionTypeBox->setEnabled(choice);
  this->Internals->addComponentButton->setEnabled(choice);
  this->Internals->deleteComponentButton->setEnabled(choice);
}

void smtkRGGEditMaterialView::materialChanged(const QString& text)
{
  smtk::model::Model model = qtActiveObjects::instance().activeModel();
  smtk::model::StringList& materialDescriptions =
    model.stringProperty(smtk::bridge::rgg::Material::label);

  std::stringstream ss;
  ss << "material ( " << text.toStdString() << " )";
  std::size_t id = 0;
  for (; id < materialDescriptions.size(); id++)
  {
    if (materialDescriptions[id].find(ss.str()) != std::string::npos)
    {
      break;
    }
  }

  if (id >= materialDescriptions.size())
  {
    this->setEnabled(false);
    return;
  }

  smtk::bridge::rgg::Material material(materialDescriptions[id]);

  auto label = model.stringProperty(material.name())[0];
  auto color = model.floatProperty(material.name());

  this->Internals->labelValue->setText(QString::fromStdString(label));

  this->Internals->colorRValue->setText(QString::number(color[0]));
  this->Internals->colorGValue->setText(QString::number(color[1]));
  this->Internals->colorBValue->setText(QString::number(color[2]));
  this->Internals->colorAValue->setText(QString::number(color[3]));

  this->Internals->temperatureValue->setText(QString::number(material.temperature()));

  this->Internals->thermalCoeffValue->setText(QString::number(material.thermalExpansion()));

  this->Internals->densityValue->setText(QString::number(material.density()));

  this->Internals->densityTypeBox->setCurrentIndex(
    this->Internals->densityTypeBox->property(material.densityType().c_str()).toInt());

  this->Internals->compositionTypeBox->setCurrentIndex(
    this->Internals->compositionTypeBox->property(material.compositionType().c_str()).toInt());

  smtk::attribute::StringItemPtr componentI =
    this->Internals->m_currentAtt->attribute()->findString("component");
  smtk::attribute::DoubleItemPtr contentI =
    this->Internals->m_currentAtt->attribute()->findDouble("content");

  if (this->Internals->m_nuclideTable == nullptr)
  {
    this->launchNuclideTable();
  }

  QTableWidget* cT = this->Internals->componentsTable;
  cT->blockSignals(true);
  cT->setRowCount(static_cast<int>(material.numberOfComponents()));
  cT->setColumnCount(2);
  for (std::size_t i = 0; i < material.numberOfComponents(); i++)
  {
    Nuclide* nuclide = this->Internals->m_nuclideTable->nuclide(material.component(i).c_str());
    auto item = new QTableWidgetItem(
      nuclide ? nuclide->prettyName() : QString::fromStdString(material.component(i)));
    if (nuclide != nullptr)
    {
      item->setData(Qt::DisplayRole, nuclide->prettyName());
      item->setData(Qt::UserRole, nuclide->name());
      item->setData(Qt::DecorationRole, nuclide->toPixmap());
    }
    cT->setItem(static_cast<int>(i), 0, item);
    QLineEdit* edit = new QLineEdit(cT);
    edit->setFrame(false);
    auto validator = new QDoubleValidator(edit);
    validator->setBottom(0.);
    edit->setValidator(validator);
    edit->setText(QString::number(material.content(i)));
    cT->setCellWidget(static_cast<int>(i), 1, edit);
    cT->blockSignals(false);
  }
  this->setEnabled(true);
}
