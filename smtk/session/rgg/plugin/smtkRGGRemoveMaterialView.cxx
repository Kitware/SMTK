//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/rgg/plugin/smtkRGGRemoveMaterialView.h"
#include "smtk/session/rgg/plugin/ui_smtkRGGRemoveMaterialParameters.h"

#include "smtkRGGViewHelper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Model.h"
#include "smtk/operation/Operation.h"

#include "smtk/session/rgg/Material.h"
#include "smtk/session/rgg/operators/CreateModel.h"
#include "smtk/session/rgg/operators/RemoveMaterial.h"

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

#include <cassert>
#include <sstream>

using namespace smtk::extension;
using namespace smtk::session::rgg;

class smtkRGGRemoveMaterialViewInternals : public Ui::RGGRemoveMaterialParameters
{
public:
  smtkRGGRemoveMaterialViewInternals() {}

  ~smtkRGGRemoveMaterialViewInternals()
  {
    if (CurrentAtt)
    {
      delete CurrentAtt;
    }
  }

  qtAttribute* createAttUI(smtk::attribute::AttributePtr att,
    const smtk::view::View::Component& comp, QWidget* pw, qtBaseView* view)
  {
    if (att && att->numberOfItems() > 0)
    {
      qtAttribute* attInstance = new qtAttribute(att, comp, pw, view);
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

  QPointer<qtAttribute> CurrentAtt;

  smtk::weak_ptr<smtk::operation::Operation> CurrentOp;
};

smtkRGGRemoveMaterialView::smtkRGGRemoveMaterialView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkRGGRemoveMaterialViewInternals();
}

smtkRGGRemoveMaterialView::~smtkRGGRemoveMaterialView()
{
  delete this->Internals;
}

qtBaseView* smtkRGGRemoveMaterialView::createViewWidget(const ViewInfo& info)
{
  smtkRGGRemoveMaterialView* view = new smtkRGGRemoveMaterialView(info);
  view->buildUI();
  return view;
}

bool smtkRGGRemoveMaterialView::displayItem(smtk::attribute::ItemPtr item)
{
  return this->qtBaseView::displayItem(item);
}

void smtkRGGRemoveMaterialView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkRGGRemoveMaterialView::valueChanged(smtk::attribute::ItemPtr /*optype*/)
{
  this->requestOperation(this->Internals->CurrentOp.lock());
}

void smtkRGGRemoveMaterialView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkRGGRemoveMaterialView::cancelOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentAtt)
  {
    return;
  }
  // Reset widgets here
}

void smtkRGGRemoveMaterialView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkRGGRemoveMaterialView::attributeModified()
{
  // Always enable apply button here
}

bool smtkRGGRemoveMaterialView::ableToOperate()
{
  // Fill the attribute - read all data from UI
  smtk::attribute::StringItemPtr nameI =
    this->Internals->CurrentAtt->attribute()->findString("name");
  nameI->setValue(this->Internals->materialBox->currentText().toStdString());

  auto op = this->Internals->CurrentOp.lock();
  bool valid = op->ableToOperate();
  return valid;
}

void smtkRGGRemoveMaterialView::apply()
{
  if (this->ableToOperate())
  {
    this->requestOperation(this->Internals->CurrentOp.lock());
    this->updateRemoveMaterialPanel();
  }
}

void smtkRGGRemoveMaterialView::updateAttributeData()
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
      if (optype == "remove material")
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

  smtk::operation::OperationPtr removeMaterialOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperation(defName);
  this->Internals->CurrentOp = removeMaterialOp;

  smtk::attribute::AttributePtr att = removeMaterialOp->parameters();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, comp, this->Widget, this);
  this->updateRemoveMaterialPanel();
}

void smtkRGGRemoveMaterialView::createWidget()
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
    &smtkRGGRemoveMaterialView::cancelOperation);
  QObject::connect(
    this->Internals->infoButton, &QPushButton::clicked, this, &smtkRGGRemoveMaterialView::onInfo);
  QObject::connect(
    this->Internals->applyButton, &QPushButton::released, this, &smtkRGGRemoveMaterialView::apply);

  this->updateAttributeData();
  this->updateRemoveMaterialPanel();
}

void smtkRGGRemoveMaterialView::updateRemoveMaterialPanel()
{
  smtk::model::EntityRefArray ents = this->Internals->CurrentAtt->attribute()
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
  }
}

void smtkRGGRemoveMaterialView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}

void smtkRGGRemoveMaterialView::setupMaterialComboBox(QComboBox* box)
{
  box->clear();

  smtk::model::Model model = qtActiveObjects::instance().activeModel();
  if (!model.hasStringProperty(smtk::session::rgg::Material::label) ||
    model.stringProperty(smtk::session::rgg::Material::label).empty())
  {
    this->Internals->applyButton->setEnabled(false);
    return;
  }

  smtk::model::StringList& materialDescriptions =
    model.stringProperty(smtk::session::rgg::Material::label);

  for (auto& materialDescription : materialDescriptions)
  {
    smtk::session::rgg::Material material(materialDescription);
    box->addItem(QString::fromStdString(material.name()));
  }

  this->Internals->applyButton->setEnabled(true);
}
