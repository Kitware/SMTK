//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vxl/operators/smtkTerrainExtractionView.h"
#include "smtk/extension/vxl/operators/ui_smtkTerrainExtractionParameters.h"

#include "smtk/model/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/View.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QtWidgets/QWidget>

using namespace smtk::extension;

class smtkTerrainExtractionViewInternals : public Ui::TerrainExtractionParameters
{
public:
  smtkTerrainExtractionViewInternals() {}
  ~smtkTerrainExtractionViewInternals()
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
      if (attInstance && attInstance->widget())
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("terrainExtractionEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
      }
      return attInstance;
    }
    return NULL;
  }

  QPointer<qtAttribute> CurrentAtt;
  smtk::weak_ptr<smtk::model::Operator> CurrentOp;
  QPointer<QWidget> terrainExtraction;
};

smtkTerrainExtractionView::smtkTerrainExtractionView(const smtk::extension::ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkTerrainExtractionViewInternals;
}

smtkTerrainExtractionView::~smtkTerrainExtractionView()
{
  delete this->Internals;
}

qtBaseView* smtkTerrainExtractionView::createViewWidget(const smtk::extension::ViewInfo& info)
{
  smtkTerrainExtractionView* view = new smtkTerrainExtractionView(info);
  view->buildUI();
  return view;
}

Ui::TerrainExtractionParameters* smtkTerrainExtractionView::terrainExtractionParameterUI()
{
  return this->Internals;
}

void smtkTerrainExtractionView::attributeModified()
{
  // enable when user has picked a point cloud
  this->Internals->terrainExtraction->setEnabled(
    this->Internals->CurrentAtt->attribute()->isValid());
}

void smtkTerrainExtractionView::createWidget()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());

  // Delete any pre-existing widget
  if (this->Widget)
  {
    if (parentlayout)
    {
      parentlayout->removeWidget(this->Widget);
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

  this->Internals->terrainExtraction = new QWidget;
  this->Internals->setupUi(this->Internals->terrainExtraction);
  layout->addWidget(
    this->Internals
      ->terrainExtraction); // ui must have a default layout other wise it would not work
  this->Internals->terrainExtraction->setEnabled(false);

  // Signals and slots
}

void smtkTerrainExtractionView::updateAttributeData()
{
  smtk::common::ViewPtr view = this->getObject();
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
  smtk::common::View::Component& comp = view->details().child(i);
  std::string defName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  {
    smtk::common::View::Component& attComp = comp.child(ci);
    std::cout << "  component " << attComp.name() << "\n";
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      std::cout << "    component type " << optype << "\n";
      if (optype == "terrain extraction")
      {
        defName = optype;
        std::cout << "match terrain extraction!" << std::endl;
        break;
      }
    }
  }
  if (defName.empty())
  {
    return;
  }

  smtk::model::OperatorPtr terrainExtractionOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(defName);
  this->Internals->CurrentOp = terrainExtractionOp;

  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = terrainExtractionOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
  if (this->Internals->CurrentAtt)
  {
    QObject::connect(
      this->Internals->CurrentAtt, SIGNAL(modified()), this, SLOT(attributeModified()));
  }
}

void smtkTerrainExtractionView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !op->specification())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkTerrainExtractionView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentAtt)
  {
    return;
  }
  // Reset widgets here
}

void smtkTerrainExtractionView::valueChanged(smtk::attribute::ItemPtr /*valItem*/)
{
  this->requestOperation(this->Internals->CurrentOp.lock());
}

void smtkTerrainExtractionView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkTerrainExtractionView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}
