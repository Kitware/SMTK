//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "PreviewPanel.h"
#include "ui_PreviewPanel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/view/Configuration.h"

#include "AttDefDataModel.h"

// -----------------------------------------------------------------------------
PreviewPanel::PreviewPanel(QWidget* parent, smtk::attribute::ResourcePtr resource)
  : QDockWidget(parent)
  , AttributeResource(resource)
{
  this->setWindowTitle("Preview Panel");
}

// -----------------------------------------------------------------------------
PreviewPanel::~PreviewPanel() = default;

// -----------------------------------------------------------------------------
smtk::view::ConfigurationPtr PreviewPanel::createView(const smtk::attribute::DefinitionPtr& def)
{
  const bool isInvalid = def == nullptr || def->isAbstract();
  if (isInvalid)
  {
    return nullptr;
  }

  const std::string title = def->type();
  const std::string type = "Instanced";
  smtk::view::ConfigurationPtr config = smtk::view::Configuration::New(type, title);
  this->AttributeResource->addView(config);

  smtk::view::Configuration::Component& comp =
    config->details().addChild("InstancedAttributes").addChild("Att");
  comp.setAttribute("Type", title);
  comp.setAttribute("Name", title);

  if (this->CurrentViewAttr)
  {
    this->AttributeResource->removeAttribute(this->CurrentViewAttr);
  }
  this->CurrentViewAttr = nullptr;

  smtk::attribute::AttributePtr attr = this->AttributeResource->createAttribute(title);
  const std::string attrName = attr ? attr->name() : std::string();
  comp.setContents(attrName);

  this->CurrentViewAttr = attr;
  return config;
}

// -----------------------------------------------------------------------------
void PreviewPanel::createViewForAllAttributes(smtk::view::ConfigurationPtr& root)
{
  root = smtk::view::Configuration::New("Group", "RootView");
  root->details().setAttribute("TopLevel", "true");
  this->AttributeResource->addView(root);

  root->details().addChild("Views");
  int viewsIndex = root->details().findChild("Views");

  // Add instances of all non-abstract attribute definitions
  smtk::view::Configuration::Component& viewsComp = root->details().child(viewsIndex);

  using DefinitionPtrVec = std::vector<smtk::attribute::DefinitionPtr>;
  using DefinitionVecIter = DefinitionPtrVec::const_iterator;

  DefinitionPtrVec defs;
  DefinitionPtrVec baseDefs;
  this->AttributeResource->findBaseDefinitions(baseDefs);

  for (DefinitionVecIter baseIt = baseDefs.begin(); baseIt != baseDefs.end(); baseIt++)
  {
    // Add def if not abstract
    if (!(*baseIt)->isAbstract())
    {
      defs.push_back(*baseIt);
    }

    DefinitionPtrVec derivedDefs;
    this->AttributeResource->findAllDerivedDefinitions(*baseIt, true, derivedDefs);
    defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
  }

  // Instantiate attribute for each concrete definition
  for (DefinitionVecIter defIt = defs.begin(); defIt != defs.end(); defIt++)
  {
    smtk::view::ConfigurationPtr instView =
      smtk::view::Configuration::New("Instanced", (*defIt)->type());
    this->AttributeResource->addView(instView);

    smtk::view::Configuration::Component& comp =
      instView->details().addChild("InstancedAttributes").addChild("Att");
    comp.setAttribute("Type", (*defIt)->type());
    comp.setAttribute("Name", (*defIt)->type());

    smtk::attribute::AttributePtr instAttr =
      this->AttributeResource->createAttribute((*defIt)->type());

    comp.setContents(instAttr->name());
    viewsComp.addChild("View").setAttribute("Title", (*defIt)->type());
  }
}

// -----------------------------------------------------------------------------
void PreviewPanel::updateCurrentView(const QModelIndex& current, const QModelIndex& previous)
{
  Q_UNUSED(previous);
  const AttDefDataModel* model = qobject_cast<const AttDefDataModel*>(current.model());
  const auto& def = model->get(current);

  // TODO It might be necessary to remove views (in order to save/export only those
  // views created by the user or already available beforehand).
  auto view = this->createView(def);
  if (!view)
  {
    // No view is created for empty AttributeDefinitions
    this->createViewWidget(nullptr);
    return;
  }

  view->details().setAttribute("TopLevel", "true");
  this->createViewWidget(view);
}

// -----------------------------------------------------------------------------
void PreviewPanel::createViewWidget(const smtk::view::ConfigurationPtr& view)
{
  if (!view)
  {
    this->setWidget(nullptr);
    return;
  }

  // Destroying qtUIManager will cleanup after the older PreviewWidget as it is
  // parented by one of its internal widgets (m_ScrollArea).
  this->UIManager = std::make_unique<smtk::extension::qtUIManager>(this->AttributeResource);

  this->PreviewWidget = new QWidget(this);
  this->PreviewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QVBoxLayout* wlayout = new QVBoxLayout();
  this->PreviewWidget->setLayout(wlayout);
  this->setWidget(this->PreviewWidget);

  this->UIManager->setSMTKView(view, this->PreviewWidget, true);
}
