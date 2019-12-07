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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/view/Configuration.h"

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
  qtInstancedViewInternals() = default;

  ~qtInstancedViewInternals()
  {
    foreach (qtAttribute* qatt, this->AttInstances)
    {
      delete qatt;
    }
  }

  //QScrollArea *ScrollArea;
  QList<QPointer<qtAttribute> > AttInstances;
  bool m_isEmpty{ true };
  smtk::operation::Observers::Key m_observerKey;
};

qtBaseView* qtInstancedView::createViewWidget(const ViewInfo& info)
{
  qtInstancedView* view = new qtInstancedView(info);
  view->buildUI();
  return view;
}

qtInstancedView::qtInstancedView(const ViewInfo& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new qtInstancedViewInternals;
}

qtInstancedView::~qtInstancedView()
{
  if (this->Internals->m_observerKey.assigned())
  {
    auto opManager = this->uiManager()->operationManager();
    if (opManager != nullptr)
    {
      opManager->observers().erase(this->Internals->m_observerKey);
    }
  }
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

  this->updateUI();

  auto opManager = uiManager()->operationManager();
  QPointer<qtInstancedView> guardedObject(this);
  if (opManager != nullptr)
  {
    this->Internals->m_observerKey = opManager->observers().insert(
      [guardedObject](const smtk::operation::Operation& oper, smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        if (guardedObject == nullptr)
        {
          return 0;
        }
        return guardedObject->handleOperationEvent(oper, event, result);
      },
      "qtInstancedView: Refresh qtInstancedView when components are modified.");
  }
}

void qtInstancedView::onShowCategory()
{
  this->updateUI();
}

void qtInstancedView::updateUI()
{
  smtk::view::ConfigurationPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  smtk::attribute::ResourcePtr resource = this->uiManager()->attResource();
  std::string attName, defName;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;
  foreach (qtAttribute* qatt, this->Internals->AttInstances)
  {
    delete qatt;
  }
  this->Internals->AttInstances.clear();

  std::vector<smtk::attribute::AttributePtr> atts;
  std::vector<std::reference_wrapper<smtk::view::Configuration::Component> > comps;
  int longLabelWidth = 0;
  // Lets find the InstancedAttributes Infomation
  int index = view->details().findChild("InstancedAttributes");
  if (index < 0)
  {
    // Should present error message
    return;
  }

  smtk::view::Configuration::Component& comp = view->details().child(index);
  std::size_t i, n = comp.numberOfChildren();
  for (i = 0; i < n; i++)
  {
    smtk::view::Configuration::Component& attComp = comp.child(i);
    if (attComp.name() != "Att")
    {
      continue;
    }

    if (!attComp.attribute("Name", attName))
    {
      return; // No name set
    }

    // See if the attribute exists and if not then create it
    att = resource->findAttribute(attName);
    if (!att)
    {
      if (!attComp.attribute("Type", defName))
      {
        // No attribute definition name
        continue;
      }
      attDef = resource->findDefinition(defName);
      if (!attDef)
      {
        continue;
      }
      else
      {
        att = resource->createAttribute(attName, attDef);
        this->attributeCreated(att);
      }
    }
    else
    {
      attDef = att->definition();
    }

    atts.push_back(att);
    comps.push_back(attComp);
    int labelWidth =
      this->uiManager()->getWidthOfAttributeMaxLabel(attDef, this->uiManager()->advancedFont());
    longLabelWidth = std::max(labelWidth, longLabelWidth);
  }
  this->setFixedLabelWidth(longLabelWidth);
  n = atts.size();

  // Assume by default the view is empty
  this->Internals->m_isEmpty = true;
  for (i = 0; i < n; i++)
  {
    if (atts[i]->numberOfItems() > 0)
    {
      qtAttribute* attInstance = new qtAttribute(atts[i], comps[i], this->widget(), this);
      if (attInstance)
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        this->Internals->AttInstances.push_back(attInstance);
        if (attInstance->widget() && !attInstance->isEmpty())
        {
          this->Widget->layout()->addWidget(attInstance->widget());
          this->Internals->m_isEmpty = false;
        }
        QObject::connect(attInstance, SIGNAL(modified()), this, SIGNAL(modified()));
        QObject::connect(
          attInstance, SIGNAL(itemModified(qtItem*)), this, SIGNAL(itemModified(qtItem*)));

        QPointer<qtInstancedView> guardedObject(this);
        connect(attInstance, &qtAttribute::itemModified, [guardedObject](qtItem* item) {
          if (guardedObject != nullptr)
          {
            std::vector<std::string> items;
            items.push_back(item->item()->name());
            guardedObject->attributeChanged(item->item()->attribute(), items);
          }
        });
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
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
}

bool qtInstancedView::isValid() const
{
  foreach (qtAttribute* att, this->Internals->AttInstances)
  {
    if (!this->uiManager()->checkAttributeValidity(att->attribute().get()))
    {
      return false;
    }
  }
  return true;
}

bool qtInstancedView::isEmpty() const
{
  return this->Internals->m_isEmpty;
}

int qtInstancedView::handleOperationEvent(const smtk::operation::Operation& op,
  smtk::operation::EventType event, smtk::operation::Operation::Result result)
{
  if (event != smtk::operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  // Since the Signal Operation originates from a Qt Signal
  // being fired we can ignore this
  if (op.typeName() == smtk::common::typeName<smtk::attribute::Signal>())
  {
    return 0;
  }

  auto compItem = result->findComponent("modified");
  std::size_t i, n = compItem->numberOfValues();
  for (i = 0; i < n; i++)
  {
    if (compItem->isSet(i))
    {
      for (qtAttribute* qatt : this->Internals->AttInstances)
      {
        if (qatt->attribute() == compItem->value(i))
        {
          // Update the attribute's items
          auto items = qatt->items();
          for (auto item : items)
          {
            item->updateItemData();
          }
          break; // we don't have to keep looking for this ComponentPtr
        }
      }
    }
  }
  return 0;
}
