//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtInstancedView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/common/View.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

using namespace smtk::attribute;
using namespace smtk::extension;

class qtInstancedViewInternals
{
public:
  qtInstancedViewInternals() {}
  //QScrollArea *ScrollArea;
  QList<QPointer<qtAttribute> > AttInstances;
};

qtBaseView* qtInstancedView::createViewWidget(const ViewInfo& info)
{
  qtInstancedView* view = new qtInstancedView(info);
  view->buildUI();
  return view;
}

qtInstancedView::qtInstancedView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new qtInstancedViewInternals;
}

qtInstancedView::~qtInstancedView()
{
  delete this->Internals;
}

void qtInstancedView::createWidget()
{
  if (!this->getObject())
  {
    return;
  }
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  if (this->Widget)
  {
    if (parentlayout)
    {
      parentlayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  this->Widget = new QFrame(this->parentWidget());
  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);

  this->updateAttributeData();
}

void qtInstancedView::updateAttributeData()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  smtk::attribute::System* sys = this->uiManager()->attSystem();
  std::string attName, defName;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;
  foreach (qtAttribute* qatt, this->Internals->AttInstances)
  {
    delete qatt;
  }
  this->Internals->AttInstances.clear();

  std::vector<smtk::attribute::AttributePtr> atts;
  int longLabelWidth = 0;
  // Lets find the InstancedAttributes Infomation
  int index = view->details().findChild("InstancedAttributes");
  if (index < 0)
  {
    // Should present error message
    return;
  }

  smtk::common::View::Component& comp = view->details().child(index);
  std::size_t i, n = comp.numberOfChildren();
  for (i = 0; i < n; i++)
  {
    smtk::common::View::Component& attComp = comp.child(i);
    if (attComp.name() != "Att")
    {
      continue;
    }

    if (!attComp.attribute("Name", attName))
    {
      return; // No name set
    }

    // See if the attribute exists and if not then create it
    att = sys->findAttribute(attName);
    if (!att)
    {
      if (!attComp.attribute("Type", defName))
      {
        // No attribute definition name
        continue;
      }
      attDef = sys->findDefinition(defName);
      if (!attDef)
      {
        continue;
      }
      else
      {
        att = sys->createAttribute(attName, attDef);
      }
    }
    else
    {
      attDef = att->definition();
    }

    atts.push_back(att);
    int labelWidth =
      this->uiManager()->getWidthOfAttributeMaxLabel(attDef, this->uiManager()->advancedFont());
    longLabelWidth = std::max(labelWidth, longLabelWidth);
  }
  this->setFixedLabelWidth(longLabelWidth);
  n = atts.size();

  for (i = 0; i < n; i++)
  {
    if (atts[i]->numberOfItems() > 0)
    {
      qtAttribute* attInstance = new qtAttribute(atts[i], this->widget(), this);
      if (attInstance)
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        this->Internals->AttInstances.push_back(attInstance);
        if (attInstance->widget())
        {
          this->Widget->layout()->addWidget(attInstance->widget());
        }
        QObject::connect(attInstance, SIGNAL(modified()), this, SIGNAL(modified()));
      }
    }
  }
}

void qtInstancedView::showAdvanceLevelOverlay(bool show)
{
  foreach (qtAttribute* att, this->Internals->AttInstances)
  {
    if (att->widget())
    {
      att->showAdvanceLevelOverlay(show);
    }
  }
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

bool qtInstancedView::isValid() const
{
  foreach (qtAttribute* att, this->Internals->AttInstances)
  {
    if (!att->attribute()->isValid())
    {
      return false;
    }
  }
  return true;
}

void qtInstancedView::requestModelEntityAssociation()
{
  foreach (qtAttribute* att, this->Internals->AttInstances)
  {
    att->onRequestEntityAssociation();
  }
}
