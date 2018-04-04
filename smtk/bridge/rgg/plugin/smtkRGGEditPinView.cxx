//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/rgg/plugin/smtkRGGEditPinView.h"
#include "smtk/bridge/rgg/plugin/ui_smtkRGGEditPinParameters.h"

#include "smtkRGGViewHelper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/operators/CreateModel.h"
#include "smtk/bridge/rgg/operators/CreatePin.h"
#include "smtk/bridge/rgg/operators/EditPin.h"

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

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

using namespace smtk::extension;
using namespace smtk::bridge::rgg;
const int numberOfLayersTableColumns = 2;
const int numberOfPieceTableColumns = 4;

class smtkRGGEditPinViewInternals : public Ui::RGGEditPinParameters
{
public:
  smtkRGGEditPinViewInternals() {}

  ~smtkRGGEditPinViewInternals()
  {
    if (CurrentAtt)
      delete CurrentAtt;
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
        attInstance->widget()->setObjectName("RGGPinEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
      }
      return attInstance;
    }
    return NULL;
  }

  QPointer<qtAttribute> CurrentAtt;

  smtk::weak_ptr<smtk::model::Operator> CurrentOp;
};

smtkRGGEditPinView::smtkRGGEditPinView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkRGGEditPinViewInternals();
}

smtkRGGEditPinView::~smtkRGGEditPinView()
{
  delete this->Internals;
}

qtBaseView* smtkRGGEditPinView::createViewWidget(const ViewInfo& info)
{
  smtkRGGEditPinView* view = new smtkRGGEditPinView(info);
  view->buildUI();
  return view;
}

bool smtkRGGEditPinView::displayItem(smtk::attribute::ItemPtr item)
{
  return this->qtBaseView::displayItem(item);
}

void smtkRGGEditPinView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkRGGEditPinView::valueChanged(smtk::attribute::ItemPtr /*optype*/)
{
  this->requestOperation(this->Internals->CurrentOp.lock());
}

void smtkRGGEditPinView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !op->specification())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkRGGEditPinView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentAtt)
  {
    return;
  }
  // Reset widgets here
}

void smtkRGGEditPinView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkRGGEditPinView::attributeModified()
{
  // Always enable apply button here
}

void smtkRGGEditPinView::onAttItemModified(smtk::extension::qtItem* item)
{
  smtk::attribute::ItemPtr itemPtr = item->getObject();
  // only changing pin would update edit pin panel
  if (itemPtr->name() == "pin" && itemPtr->type() == smtk::attribute::Item::Type::ModelEntityType)
  {
    this->updateEditPinPanel();
  }
}

void smtkRGGEditPinView::apply()
{
  // Fill the attribute - read all data from UI
  // It should only takes a pin as input
  smtk::attribute::StringItemPtr nameI =
    this->Internals->CurrentAtt->attribute()->findString("name");
  nameI->setValue(this->Internals->nameLineEdit->text().toStdString());

  smtk::attribute::StringItemPtr labelI =
    this->Internals->CurrentAtt->attribute()->findString("label");
  labelI->setValue(this->Internals->labelLineEdit->text().toStdString());

  smtk::attribute::IntItemPtr materialI =
    this->Internals->CurrentAtt->attribute()->findInt("cell material");
  materialI->setValue(this->Internals->CellMaterial->currentIndex());

  smtk::attribute::VoidItemPtr isCutAway =
    this->Internals->CurrentAtt->attribute()->findVoid("cut away");
  isCutAway->setIsEnabled(this->Internals->cutAwayViewCheckBox->isChecked());

  smtk::attribute::DoubleItemPtr zOriginI =
    this->Internals->CurrentAtt->attribute()->findDouble("z origin");
  zOriginI->setValue(this->Internals->Z0rigin->value());

  smtk::attribute::GroupItemPtr piecesI =
    this->Internals->CurrentAtt->attribute()->findGroup("pieces");
  piecesI->setNumberOfGroups(1); // Clear the existing groups
  // Add pieces
  QTableWidget* pT = this->Internals->piecesTable;
  for (int i = 0; i < pT->rowCount(); i++)
  { // segment part
    int segmentType = dynamic_cast<QComboBox*>(pT->cellWidget(i, 0))->currentIndex();
    if (i > 0)
    { // Attribute would create an empty group since required value is set to 1
      piecesI->appendGroup();
    }
    smtk::attribute::IntItemPtr type =
      smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(piecesI->item(i, 0));
    type->setValue(segmentType);
    smtk::attribute::DoubleItemPtr para =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(piecesI->item(i, 1));
    // type parameters
    double length = pT->item(i, 1)->text().toDouble();
    double baseR = pT->item(i, 2)->text().toDouble();
    double topR = pT->item(i, 3)->text().toDouble();

    std::vector<double> paras = { length, baseR, topR };
    para->setValues(paras.begin(), paras.end());
  }

  smtk::attribute::GroupItemPtr layersI =
    this->Internals->CurrentAtt->attribute()->findGroup("layer materials");
  layersI->setNumberOfGroups(1); // Clear the existing groups
  // Add pieces
  QTableWidget* lT = this->Internals->layersTable;
  for (size_t i = 0; i < lT->rowCount(); i++)
  {
    if (i > 0)
    { // Attribute would create an empty group since required value is set to 1
      layersI->appendGroup();
    }
    // sub material
    int subMaterial = dynamic_cast<QComboBox*>(lT->cellWidget(i, 0))->currentIndex();

    smtk::attribute::IntItemPtr subMI =
      smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(layersI->item(i, 0));
    subMI->setValue(subMaterial);
    smtk::attribute::DoubleItemPtr radiusNI =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(layersI->item(i, 1));
    double radiusN = lT->item(i, 1)->text().toDouble();
    radiusNI->setValue(radiusN);
  }

  this->requestOperation(this->Internals->CurrentOp.lock());
}

void smtkRGGEditPinView::pieceTypeChanged()
{
  QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
  if (!comboBox)
  {
    return;
  }

  if (comboBox->currentIndex() == 0) // Only cylinder needs an update on top radius
  {
    int row = comboBox->itemData(0).toInt();
    radiusItem* baseR = dynamic_cast<radiusItem*>(this->Internals->piecesTable->item(row, 2));
    radiusItem* topRItem = dynamic_cast<radiusItem*>(this->Internals->piecesTable->item(row, 3));
    topRItem->setData(Qt::EditRole, QVariant(baseR->text().toDouble()));
  }
}

void smtkRGGEditPinView::updateAttributeData()
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
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      if (optype == "edit pin")
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

  smtk::model::OperatorPtr createPinOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(defName);
  this->Internals->CurrentOp = createPinOp;

  smtk::attribute::AttributePtr att = createPinOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
  if (this->Internals->CurrentAtt)
  {
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::modified, this,
      &smtkRGGEditPinView::attributeModified);
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::itemModified, this,
      &smtkRGGEditPinView::onAttItemModified);
    // Associate pin combobox with the latest pin
    smtk::attribute::ModelEntityItemPtr pinI =
      this->Internals->CurrentAtt->attribute()->associations();

    if (!pinI)
    {
      smtkErrorMacro(smtk::io::Logger(), "Edit pin operator does not have a pin association");
      return;
    }
    smtk::model::Model model = qtActiveObjects::instance().activeModel();
    if (model.hasStringProperty("latest pin"))
    {
      smtk::model::EntityRef latestPin = smtk::model::EntityRef(
        model.manager(), smtk::common::UUID(model.stringProperty("latest pin")[0]));
      pinI->setValue(latestPin);
    }
    this->updateEditPinPanel();
  }
}

void smtkRGGEditPinView::createWidget()
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
    &smtk::extension::qtModelView::operationCancelled, this, &smtkRGGEditPinView::cancelOperation);
  // Edit name
  this->Internals->nameLineEdit->setToolTip(QString::fromStdString("Set the "
                                                                   "name of the pin"));
  // Edit label
  this->Internals->labelLineEdit->setToolTip(QString::fromStdString("Set the "
                                                                    "label of the pin"));
  // Edit cell material
  this->setupMaterialComboBox(this->Internals->CellMaterial);
  this->Internals->CellMaterial->setToolTip(QString::fromStdString("Set the "
                                                                   " material on the cell"));
  // Edit z origin
  // z origin would be updated when applied
  this->Internals->Z0rigin->setToolTip(QString::fromStdString("Set the base z value of the pin"));

  // Set cut away or not
  this->Internals->cutAwayViewCheckBox->setToolTip(
    QString::fromStdString("If checked, a clipping plane would cut the"
                           "pin half through center axis"));

  // Add/edit/remove pieces table
  this->createPiecesTable();

  // Add/edit/remove layers table
  this->createLayersTable();

  this->updateButtonStatus();

  // Show help when the info button is clicked.
  QObject::connect(
    this->Internals->infoButton, &QPushButton::released, this, &smtkRGGEditPinView::onInfo);

  QObject::connect(
    this->Internals->applyButton, &QPushButton::released, this, &smtkRGGEditPinView::apply);

  this->updateAttributeData();
}

void smtkRGGEditPinView::updateEditPinPanel()
{
  smtk::model::EntityRefArray ents = this->Internals->CurrentAtt->attribute()
                                       ->associatedModelEntities<smtk::model::EntityRefArray>();
  bool isEnabled(true);
  if ((ents.size() == 0) || (!ents[0].hasStringProperty("rggType")) ||
    (ents[0].stringProperty("rggType")[0] != SMTK_BRIDGE_RGG_PIN) ||
    !(ents[0].embeddedEntities<smtk::model::EntityRefArray>().size() > 0))
  { // Its type is not rgg pin
    isEnabled = false;
  }
  if (this->Internals)
  {
    this->Internals->scrollArea->setEnabled(isEnabled);
  }
  if (isEnabled)
  {
    // Update material combo box
    this->setupMaterialComboBox(this->Internals->CellMaterial);

    smtk::model::AuxiliaryGeometry pin = ents[0].as<smtk::model::AuxiliaryGeometry>();
    // Name
    this->Internals->nameLineEdit->setText(QString::fromStdString(pin.name()));
    // Label
    this->Internals->labelLineEdit->setText(QString::fromStdString(pin.stringProperty("label")[0]));
    // Cell material
    this->Internals->CellMaterial->setCurrentIndex(pin.integerProperty("cell material")[0]);
    // Hex
    this->Internals->isHexCheckbox->setChecked(pin.owningModel().integerProperty("hex")[0]);
    // Cut away
    this->Internals->cutAwayViewCheckBox->setChecked(pin.integerProperty("cut away")[0]);
    // Z origin
    this->Internals->Z0rigin->setValue(pin.floatProperty("z origin")[0]);
    // Populate the piece table
    this->Internals->piecesTable->clearSpans();
    this->Internals->piecesTable->model()->removeRows(0, this->Internals->piecesTable->rowCount());
    smtk::model::IntegerList pieceTypes = pin.integerProperty("pieces");
    smtk::model::FloatList pieceparas = pin.floatProperty("pieces");
    if (pieceTypes.size() > 0 && (pieceTypes.size() * 3 == pieceparas.size()))
    {
      size_t numParts = pieceTypes.size();
      for (std::size_t index = 0; index < numParts; index++)
      {
        RGGType type = static_cast<RGGType>(pieceTypes[index]);
        this->addPieceToTable(static_cast<int>(index), type, pieceparas[3 * index],
          pieceparas[3 * index + 1], pieceparas[3 * index + 2]);
      }
    }
    else
    {
      std::cerr << "For pieces table, segment type and type parameters' default "
                   "value has wrong size"
                << std::endl;
    }
    // Populate the layers table
    this->Internals->layersTable->clearSpans();
    this->Internals->layersTable->model()->removeRows(0, this->Internals->layersTable->rowCount());
    smtk::model::IntegerList layerMs = pin.integerProperty("layer materials");
    smtk::model::FloatList layerRadiusNs = pin.floatProperty("layer materials");
    if (layerMs.size() == layerRadiusNs.size())
    {
      size_t numLayers = layerMs.size();
      for (std::size_t index = 0; index < numLayers; index++)
      {
        this->addLayerToTable(index, layerMs[index], layerRadiusNs[index]);
      }
    }
    else
    {
      std::cerr << "For layers table, sub material and radius(normalized)'s default "
                   "value has wrong size"
                << std::endl;
      return;
    }
  }
}

void smtkRGGEditPinView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}

void smtkRGGEditPinView::createPiecesTable()
{
  QTableWidget* pt = this->Internals->piecesTable;
  pt->clear();
  pt->resizeRowsToContents();
  pt->setMinimumHeight(200);
  // type, length, base radius and top radius
  pt->setColumnCount(numberOfPieceTableColumns);
  pt->setHorizontalHeaderLabels(QStringList() << "Segment\nType"
                                              << "Length"
                                              << "Base\nRadius"
                                              << "Top\nRadius");
  // Add picece button
  this->Internals->addPieceButton->setToolTip("Add a new piece after the lastest piece");
  QObject::connect(this->Internals->addPieceButton, &QPushButton::clicked, this, [this]() {
    // Get the current last piece
    QTableWidget* pt = this->Internals->piecesTable;
    int lastRowIndex = pt->rowCount() - 1;
    if (lastRowIndex < 0)
    {
      std::cerr << "pieces table does not have initial value" << std::endl;
      return;
    }
    QWidget* tmpWidget = pt->cellWidget(lastRowIndex, 0);
    QComboBox* comboBox = dynamic_cast<QComboBox*>(tmpWidget);
    RGGType type = static_cast<RGGType>(comboBox->currentIndex());
    double height = pt->item(lastRowIndex, 1)->text().toDouble();
    // If last row's type is frustum, we would flip the baseR and topR here
    double baseR = type ? pt->item(lastRowIndex, 3)->text().toDouble()
                        : pt->item(lastRowIndex, 2)->text().toDouble();
    double topR = type ? pt->item(lastRowIndex, 2)->text().toDouble()
                       : pt->item(lastRowIndex, 3)->text().toDouble();
    this->addPieceToTable(lastRowIndex + 1, type, height, baseR, topR);
  });
  // Remove picece button
  this->Internals->deletePieceButton->setToolTip("Delete the current selected piece");
  QObject::connect(this->Internals->deletePieceButton, &QPushButton::clicked, this, [this]() {
    auto pT = this->Internals->piecesTable;
    pT->blockSignals(true);
    int row = pT->currentRow();
    // Check radius status, making sure that it's forming a smooth surface
    if (row > 0 || row < (pT->rowCount() - 1))
    {
      radiusItem* baseRBefore = dynamic_cast<radiusItem*>(pT->item(row - 1, 2));
      radiusItem* baseRAfter = dynamic_cast<radiusItem*>(pT->item(row + 1, 2));
      if (baseRAfter && baseRBefore && baseRBefore != baseRAfter)
      {
        baseRBefore->setData(Qt::EditRole, QVariant(baseRAfter->text().toDouble()));
      }
    }
    pT->removeRow(row);
    pT->blockSignals(false);
    this->updateButtonStatus();
  });
}

void smtkRGGEditPinView::addPieceToTable(
  int row, smtk::bridge::rgg::RGGType type, double height, double baseR, double topR)
{
  QTableWidget* pt = this->Internals->piecesTable;
  pt->setRowCount(pt->rowCount() + 1);
  pt->blockSignals(true);
  QTableWidgetItem* item(nullptr);
  // Type
  { //drop box
    QWidget* tmpWidget = pt->cellWidget(row, 0);
    QComboBox* comboBox = dynamic_cast<QComboBox*>(tmpWidget);
    if (comboBox == NULL)
    {
      comboBox = new QComboBox(this->Internals->piecesTable);
      comboBox->addItem("Cylinder");
      comboBox->addItem("Frustum");
      comboBox->setObjectName("PincellPartBox_" + QString::number(row));
      pt->setCellWidget(row, 0, comboBox);
      item = new QTableWidgetItem;
      pt->setItem(row, 0, item);
      QObject::connect(comboBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
        this, &smtkRGGEditPinView::pieceTypeChanged);
    }
    // Cache row number on comboBox
    QVariant vdata;
    vdata.setValue(row);
    comboBox->setItemData(0, vdata);
    comboBox->blockSignals(true);
    if (type == RGGType::FRUSTUM)
    {
      comboBox->setCurrentIndex(1);
    }
    else
    {
      comboBox->setCurrentIndex(0);
    }
    comboBox->blockSignals(false);
  }

  // length
  {
    item = new generalItem();
    item->setText(QString::number(height));
    pt->setItem(row, 1, item);
  }

  // radius (base)
  item = new radiusItem(RADIUSType::BASE);
  item->setText(QString::number(baseR));
  pt->setItem(row, 2, item);

  // radius (top)
  item = new radiusItem(RADIUSType::TOP);
  item->setText(QString::number(topR));
  pt->setItem(row, 3, item);

  pt->blockSignals(false);

  this->updateButtonStatus();
}

void smtkRGGEditPinView::createLayersTable()
{
  QTableWidget* pt = this->Internals->layersTable;
  pt->clear();
  this->Internals->layersTable->setRowCount(0);
  this->Internals->layersTable->setColumnCount(numberOfLayersTableColumns);
  this->Internals->layersTable->setHorizontalHeaderLabels(QStringList() << "Material"
                                                                        << "Radius\n(normalized)");
  this->Internals->layersTable->horizontalHeader()->setStretchLastSection(true);
  this->Internals->layersTable->horizontalHeader()->setSizeAdjustPolicy(
    QAbstractScrollArea::AdjustToContents);

  // Add layer before button
  QObject::connect(this->Internals->addLayerBeforeButton, &QPushButton::clicked, this,
    &smtkRGGEditPinView::addlayerBefore);
  // Add layer after button
  QObject::connect(this->Internals->addLayerAfterButton, &QPushButton::clicked, this,
    &smtkRGGEditPinView::addlayerAfter);

  // Delete layer button
  this->Internals->deleteLayerButton->setToolTip("Delete the current selected layer");
  QObject::connect(this->Internals->deleteLayerButton, &QPushButton::clicked, this,
    &smtkRGGEditPinView::deletelayer);
  QObject::connect(this->Internals->layersTable, &QTableWidget::itemSelectionChanged, this,
    &smtkRGGEditPinView::updateButtonStatus);
}

void smtkRGGEditPinView::addlayerBefore()
{
  QTableWidget* lT = this->Internals->layersTable;
  int row;
  if (lT->selectedItems().count() != 0)
  {
    row = lT->selectedItems().value(0)->row();
  }
  // Generate the new rN. Input is assumed to be valid
  double beforeRN, afterRN;
  beforeRN = (row == 0) ? 0 : lT->item(row - 1, 1)->text().toDouble();
  afterRN = lT->item((row), 1)->text().toDouble();
  double rN = (beforeRN + afterRN) / 2;
  this->addLayerToTable(row, 0, rN);
}

void smtkRGGEditPinView::addlayerAfter()
{
  QTableWidget* lT = this->Internals->layersTable;
  int row;
  if (lT->selectedItems().count() != 0)
  {
    row = lT->selectedItems().value(0)->row();
  }
  // Generate the new rN. Input is assumed to be valid
  double beforeRN, afterRN;
  beforeRN = lT->item(row, 1)->text().toDouble();
  afterRN = lT->item((row + 1), 1)->text().toDouble();
  double rN = (beforeRN + afterRN) / 2;
  this->addLayerToTable(row + 1, 0, rN);
}

void smtkRGGEditPinView::deletelayer()
{
  this->Internals->layersTable->blockSignals(true);
  int row = this->Internals->layersTable->currentRow();
  if (row == (this->Internals->layersTable->rowCount() - 1))
  { // Upper bound of normalized radius should always be 1
    this->Internals->layersTable->item(row - 1, 1)->setText(QString::number(1.0));
  }
  this->Internals->layersTable->removeRow(row);
  this->Internals->layersTable->blockSignals(false);
  this->updateButtonStatus();
}

void smtkRGGEditPinView::addLayerToTable(int row, int subMaterial, double rN)
{
  // Caller is reponsible for
  QTableWidget* lT = this->Internals->layersTable;
  // 0 would insert row to the very top and rowCount woud insert row to the very bottom
  if (row < 0 || row > lT->rowCount())
  { // invalid input
    return;
  }
  lT->blockSignals(true);
  lT->insertRow(row);
  // Create the material combo box first
  {
    QWidget* tmp = lT->cellWidget(row, 0);
    QComboBox* materialCom = dynamic_cast<QComboBox*>(tmp);
    if (materialCom == nullptr)
    {
      materialCom = new QComboBox(this->Internals->layersTable);
      materialCom->setObjectName("PinMaterialBox_" + QString::number(row));
      lT->setCellWidget(row, 0, materialCom);
    }
    materialCom->blockSignals(true);
    this->setupMaterialComboBox(materialCom);
    materialCom->blockSignals(false);
    materialCom->setCurrentIndex(subMaterial);
  }
  // normalRadius
  rangeItem* radiusNItem = new rangeItem();
  radiusNItem->setText(QString::number(rN));
  lT->setItem(row, 1, radiusNItem);
  //
  lT->blockSignals(false);
}

void smtkRGGEditPinView::updateButtonStatus()
{
  // Delete piece
  int rc = this->Internals->piecesTable->rowCount();
  this->Internals->deletePieceButton->setEnabled(rc > 1);
  // Delete layer
  rc = this->Internals->layersTable->rowCount();
  this->Internals->deleteLayerButton->setEnabled(rc > 1);
  // Add layer after
  if (this->Internals->layersTable->currentRow() == (rc - 1) ||
    this->Internals->layersTable->selectedItems().count() == 0)
  {
    this->Internals->addLayerAfterButton->setEnabled(false);
  }
  else
  {
    this->Internals->addLayerAfterButton->setEnabled(true);
  }
  if (this->Internals->layersTable->selectedItems().count() == 0)
  {
    this->Internals->addLayerBeforeButton->setEnabled(false);
  }
  else
  {
    this->Internals->addLayerBeforeButton->setEnabled(true);
  }
}

void smtkRGGEditPinView::setupMaterialComboBox(QComboBox* box, bool isCell)
{
  box->clear();
  smtk::model::Model model = qtActiveObjects::instance().activeModel();
  size_t matN = smtk::bridge::rgg::CreateModel::materialNum(model);
  for (size_t i = 0; i < matN; i++)
  {
    std::string name;
    smtk::bridge::rgg::CreateModel::getMaterial(i, name, model);
    box->addItem(QString::fromStdString(name));
  }
  if (isCell)
  {
    // In this condition, the part does not need to have a material. Change the item text
  }
}
