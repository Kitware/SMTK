//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/rgg/plugin/smtkRGGReadRXFFileView.h"
#include "smtk/session/rgg/plugin/ui_smtkRGGReadRXFFileParameters.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Operation.h"

#include "smtk/session/rgg/operators/CreateModel.h"
#include "smtk/session/rgg/operators/CreatePin.h"
#include "smtk/session/rgg/operators/ReadRXFFile.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/View.h"

#include "pqActiveObjects.h"

#include <fstream>

using namespace smtk::extension;
using namespace smtk::session::rgg;

class smtkRGGReadRXFFileViewInternals : public Ui::RGGReadRXFFileParameters
{
public:
  smtkRGGReadRXFFileViewInternals() {}

  ~smtkRGGReadRXFFileViewInternals()
  {
    if (CurrentAtt)
      delete CurrentAtt;
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
        attInstance->widget()->setObjectName("RGGReadRXLFile");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
      }
      return attInstance;
    }
    return NULL;
  }

  QPointer<qtAttribute> CurrentAtt;

  smtk::weak_ptr<smtk::operation::Operation> CurrentOp;
  smtk::weak_ptr<smtk::operation::Operation> CreateInstanceOp;
};

smtkRGGReadRXFFileView::smtkRGGReadRXFFileView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkRGGReadRXFFileViewInternals();
}

smtkRGGReadRXFFileView::~smtkRGGReadRXFFileView()
{
  delete this->Internals;
}

qtBaseView* smtkRGGReadRXFFileView::createViewWidget(const ViewInfo& info)
{
  smtkRGGReadRXFFileView* view = new smtkRGGReadRXFFileView(info);
  view->buildUI();
  return view;
}

bool smtkRGGReadRXFFileView::displayItem(smtk::attribute::ItemPtr item)
{
  return this->qtBaseView::displayItem(item);
}

void smtkRGGReadRXFFileView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkRGGReadRXFFileView::valueChanged(smtk::attribute::ItemPtr /*optype*/)
{
  //this->requestOperation(this->Internals->CurrentOp.lock());
}

void smtkRGGReadRXFFileView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkRGGReadRXFFileView::cancelOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentAtt)
  {
    return;
  }
  // Reset widgets here
}

void smtkRGGReadRXFFileView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkRGGReadRXFFileView::attributeModified()
{
  // Always enable apply button here
}

void smtkRGGReadRXFFileView::apply()
{
  smtk::model::Model model = qtActiveObjects::instance().activeModel();
  if (!model.isValid())
  {
    smtkErrorMacro(smtk::io::Logger::instance().instance(), "A valid model must be created.\n");
    return;
  }

  // Call readRXFFile op first so that materials' info are cached into model.
  this->requestOperation(this->Internals->CurrentOp.lock());
}

void smtkRGGReadRXFFileView::updateAttributeData()
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
      if (optype == "read rxf file")
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

  // Create instance Op
  if (!this->Internals->CreateInstanceOp.lock())
  {
    std::string ciName = "create instances";
    smtk::operation::OperationPtr createInstancesOp =
      this->uiManager()->activeModelView()->operatorsWidget()->existingOperation(ciName);
    if (!createInstancesOp)
    { // We need to make sure that the "create instances" op has been initialized
      smtkErrorMacro(smtk::io::Logger::instance(), "Fail to create \"create instance\" operator");
    }
    this->Internals->CreateInstanceOp = createInstancesOp;
  }

  smtk::operation::OperationPtr rRXFFOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperation(defName);
  this->Internals->CurrentOp = rRXFFOp;

  smtk::attribute::AttributePtr att = rRXFFOp->parameters();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, comp, this->Widget, this);
  if (this->Internals->CurrentAtt)
  {
    QObject::connect(this->Internals->CurrentAtt, &qtAttribute::modified, this,
      &smtkRGGReadRXFFileView::attributeModified);
  }
}

void smtkRGGReadRXFFileView::createWidget()
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

  QObject::disconnect(this->uiManager()->activeModelView());
  QObject::connect(this->uiManager()->activeModelView(),
    &smtk::extension::qtModelView::operationCancelled, this,
    &smtkRGGReadRXFFileView::cancelOperation);

  // Show help when the info button is clicked.
  QObject::connect(
    this->Internals->infoButton, &QPushButton::released, this, &smtkRGGReadRXFFileView::onInfo);

  QObject::connect(
    this->Internals->applyButton, &QPushButton::released, this, &smtkRGGReadRXFFileView::apply);

  this->updateAttributeData();
}

void smtkRGGReadRXFFileView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}
