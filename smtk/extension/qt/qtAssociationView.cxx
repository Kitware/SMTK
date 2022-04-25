//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAssociationView.h"

#include "smtk/extension/qt/qtAssociation2ColumnWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"

#include "smtk/view/Configuration.h"
#include "ui_qtAssociationView.h"

#include <QMessageBox>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPointer>
#include <QStyleOptionViewItem>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QVariant>

#include <iostream>
#include <set>
using namespace smtk::attribute;
using namespace smtk::extension;

class qtAssociationViewInternals : public Ui::qtAssociationView
{
public:
  QList<smtk::attribute::DefinitionPtr> getCurrentDefs(const ResourcePtr& attResource) const
  {
    if (!(attResource && attResource->activeCategoriesEnabled()))
    {
      // There is no filtering - return everything
      return this->AllDefs;
    }

    if (attResource->activeCategories().size() == 1)
    {
      std::string theCategory = *(attResource->activeCategories().begin());
      if (this->AttDefMap.keys().contains(theCategory.c_str()))
      {
        return this->AttDefMap[theCategory.c_str()];
      }
      return this->AllDefs;
    }
    QList<smtk::attribute::DefinitionPtr> defs;
    Q_FOREACH (DefinitionPtr attDef, this->AllDefs)
    {
      if (attDef->categories().passes(attResource->activeCategories()))
      {
        defs.push_back(attDef);
      }
    }
    return defs;
  }

  bool currentDefsIsEmpty(const ResourcePtr& attResource) const
  {
    if (attResource && attResource->activeCategoriesEnabled())
    {
      if (attResource->activeCategories().empty())
      {
        return (this->AllDefs.empty());
      }
      if (attResource->activeCategories().size() == 1)
      {
        std::string theCategory = *(attResource->activeCategories().begin());
        if (this->AttDefMap.keys().contains(theCategory.c_str()))
        {
          return (this->AttDefMap[theCategory.c_str()].empty());
        }
        return (this->AllDefs.empty());
      }
      QList<smtk::attribute::DefinitionPtr> defs;
      Q_FOREACH (DefinitionPtr attDef, this->AllDefs)
      {
        if (attResource->passActiveCategoryCheck(attDef->categories()))
        {
          return false;
        }
      }
    }
    return true;
  }

  QPointer<qtAssociationWidget> AssociationsWidget;

  // <category, AttDefinitions>
  QMap<QString, QList<smtk::attribute::DefinitionPtr>> AttDefMap;

  // All definitions list
  QList<smtk::attribute::DefinitionPtr> AllDefs;

  std::vector<smtk::attribute::DefinitionPtr> m_attDefinitions;
  smtk::model::BitFlags m_modelEntityMask;
  std::map<std::string, smtk::view::Configuration::Component> m_attCompMap;
};

qtBaseView* qtAssociationView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtAssociationView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtAssociationView::qtAssociationView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new qtAssociationViewInternals;
}

qtAssociationView::~qtAssociationView()
{
  delete this->Internals;
}

const QMap<QString, QList<smtk::attribute::DefinitionPtr>>& qtAssociationView::attDefinitionMap()
  const
{
  return this->Internals->AttDefMap;
}

smtk::extension::qtAssociationWidget* qtAssociationView::createAssociationWidget(
  QWidget* parent,
  qtBaseView* view)
{
  return new qtAssociation2ColumnWidget(parent, view);
}

void qtAssociationView::createWidget()
{
  // this->Internals->AttDefMap has to be initialized before getAllDefinitions()
  // since the getAllDefinitions() call needs the categories list in AttDefMap
  // Create a map for all catagories so we can cluster the definitions
  this->Internals->AttDefMap.clear();
  const ResourcePtr attResource = this->attributeResource();
  std::set<std::string>::const_iterator it;
  const std::set<std::string>& cats = attResource->categories();

  for (it = cats.begin(); it != cats.end(); it++)
  {
    QList<smtk::attribute::DefinitionPtr> attdeflist;
    this->Internals->AttDefMap[it->c_str()] = attdeflist;
  }

  // Initialize definition info
  this->getAllDefinitions();

  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName("associations");
  this->Internals->setupUi(this->Widget);

  // the association widget
  this->Internals->AssociationsWidget =
    this->createAssociationWidget(this->Internals->associations, this);
  this->Internals->mainLayout->addWidget(this->Internals->AssociationsWidget);
  // signals/slots
  QObject::connect(
    this->Internals->AssociationsWidget,
    SIGNAL(attAssociationChanged()),
    this,
    SLOT(associationsChanged()));

  QObject::connect(
    this->Internals->attributes,
    SIGNAL(currentIndexChanged(int)),
    this,
    SLOT(onAttributeChanged(int)));

  this->updateModelAssociation();
}

void qtAssociationView::updateModelAssociation()
{
  this->updateUI();
}

smtk::attribute::AttributePtr qtAssociationView::getAttributeFromIndex(int index)
{
  Attribute* rawPtr = (index >= 0)
    ? static_cast<Attribute*>(
        this->Internals->attributes->itemData(index, Qt::UserRole).value<void*>())
    : nullptr;
  return rawPtr ? rawPtr->shared_from_this() : smtk::attribute::AttributePtr();
}

void qtAssociationView::onAttributeChanged(int index)
{
  auto att = this->getAttributeFromIndex(index);
  this->Internals->AssociationsWidget->showEntityAssociation(att);
}

void qtAssociationView::updateUI()
{
  this->Internals->attributes->blockSignals(true);
  this->Internals->attributes->clear();
  if (this->Internals->m_attDefinitions.empty())
  {
    this->Internals->attributes->blockSignals(false);
    return;
  }

  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->attributeResource());
  std::set<AttributePtr, Attribute::CompareByName> atts;
  // Get all of the attributes that match the list of definitions
  Q_FOREACH (attribute::DefinitionPtr attDef, currentDefs)
  {
    ResourcePtr attResource = attDef->resource();
    std::vector<smtk::attribute::AttributePtr> result;
    attResource->findAttributes(attDef, result);
    if (!result.empty())
    {
      atts.insert(result.begin(), result.end());
    }
  }
  for (const auto& att : atts)
  {
    QVariant vdata;
    vdata.setValue(static_cast<void*>(att.get()));
    this->Internals->attributes->addItem(att->name().c_str(), vdata);
  }
  this->Internals->attributes->blockSignals(false);
  if (!atts.empty())
  {
    this->Internals->attributes->setCurrentIndex(0);
    this->onAttributeChanged(0);
  }
}

void qtAssociationView::onShowCategory()
{
  this->updateUI();
}

void qtAssociationView::getAllDefinitions()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }

  smtk::attribute::ResourcePtr resource = this->attributeResource();

  std::string attName, defName, val;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;

  // The view should have a single internal component called InstancedAttributes
  if (
    (view->details().numberOfChildren() != 1) ||
    (view->details().child(0).name() != "AttributeTypes"))
  {
    // Should present error message
    return;
  }

  if (view->details().attribute("ModelEntityFilter", val))
  {
    smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(val);
    this->Internals->m_modelEntityMask = flags;
  }
  else
  {
    this->Internals->m_modelEntityMask = 0;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  smtk::view::Configuration::Component& attsComp = view->details().child(0);
  std::size_t i, n = attsComp.numberOfChildren();
  for (i = 0; i < n; i++)
  {
    if (attsComp.child(i).name() != "Att")
    {
      continue;
    }
    if (!attsComp.child(i).attribute("Type", defName))
    {
      continue;
    }

    attDef = resource->findDefinition(defName);
    if (attDef == nullptr)
    {
      continue;
    }

    this->Internals->m_attCompMap[defName] = attsComp.child(i);
    this->qtBaseAttributeView::getDefinitions(attDef, this->Internals->AllDefs);
    this->Internals->m_attDefinitions.push_back(attDef);
  }

  // sort the list
  std::sort(
    std::begin(this->Internals->AllDefs),
    std::end(this->Internals->AllDefs),
    [](smtk::attribute::DefinitionPtr a, smtk::attribute::DefinitionPtr b) {
      return a->displayedTypeName() < b->displayedTypeName();
    });

  Q_FOREACH (smtk::attribute::DefinitionPtr adef, this->Internals->AllDefs)
  {
    Q_FOREACH (QString category, this->Internals->AttDefMap.keys())
    {
      if (
        adef->categories().passes(category.toStdString()) &&
        !this->Internals->AttDefMap[category].contains(adef))
      {
        this->Internals->AttDefMap[category].push_back(adef);
      }
    }
  }
}

// The criterea used for this view is the same as that used in the attribute view
// - are there any definitions that pass the category check.  Alternatively it could
// have been - are there any attributes available to select.  However there is a problem
// with this since this view potentially depends on a view to create attributes.  If the view's
// visibility depends on whether it is empty or not, then there would need to be a way to ask
// the top level view to redraw based on an attribute view changing things - in the current
// implementation, both the attribute view and the association view would either be visible or
// hidden at the same time.
bool qtAssociationView::isEmpty() const
{
  return this->Internals->currentDefsIsEmpty(this->attributeResource());
}

void qtAssociationView::associationsChanged()
{
  int index = this->Internals->attributes->currentIndex();
  auto att = this->getAttributeFromIndex(index);
  if (att == nullptr)
  {
    return;
  }

  Q_EMIT this->modified(att->associations());
  Q_EMIT this->attAssociationChanged();
  Q_EMIT qtBaseView::modified();
}
