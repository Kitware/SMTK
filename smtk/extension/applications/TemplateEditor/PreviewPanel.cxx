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
#include "smtk/attribute/System.h"
#include "smtk/common/View.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "AttDefDataModel.h"

// -----------------------------------------------------------------------------
PreviewPanel::PreviewPanel(QWidget* parent, smtk::attribute::SystemPtr system)
  : QDockWidget(parent)
  //, Ui(new Ui::PreviewPanel)
  , AttributeSystem(system)
{
  this->setWindowTitle("Preview Panel");

  // TODO Center SMTK generated UI in panel. For this a proxy central widget
  // is used. SMTK's preview widget is added as a child to it.
  //  QWidget* centralWid = new QWidget(this);
  //  this->Ui->setupUi(centralWid);
  //  this->setWidget(centralWid);
}

// -----------------------------------------------------------------------------
PreviewPanel::~PreviewPanel() = default;

// -----------------------------------------------------------------------------
smtk::common::ViewPtr PreviewPanel::createView(const smtk::attribute::DefinitionPtr& def)
{
  const bool isInvalid = def == nullptr || def->isAbstract();
  if (isInvalid)
  {
    return nullptr;
  }

  const std::string title = def->type();
  const std::string type = "Instanced";
  smtk::common::ViewPtr view = smtk::common::View::New(type, title);
  this->AttributeSystem->addView(view);

  smtk::common::View::Component& comp =
    view->details().addChild("InstancedAttributes").addChild("Att");
  comp.setAttribute("Type", title);
  comp.setAttribute("Name", title);

  if (this->CurrentViewAttr)
  {
    this->AttributeSystem->removeAttribute(this->CurrentViewAttr);
  }
  this->CurrentViewAttr = nullptr;

  smtk::attribute::AttributePtr attr = this->AttributeSystem->createAttribute(title);
  const std::string attrName = attr ? attr->name() : std::string();
  comp.setContents(attrName);

  this->CurrentViewAttr = attr;
  return view;
}

// -----------------------------------------------------------------------------
void PreviewPanel::createViewForAllAttributes(smtk::common::ViewPtr& root)
{
  root = smtk::common::View::New("Group", "RootView");
  root->details().setAttribute("TopLevel", "true");
  this->AttributeSystem->addView(root);

  root->details().addChild("Views");
  int viewsIndex = root->details().findChild("Views");

  // Add instances of all non-abstract attribute definitions
  smtk::common::View::Component& viewsComp = root->details().child(viewsIndex);

  using DefinitionPtrVec = std::vector<smtk::attribute::DefinitionPtr>;
  using DefinitionVecIter = DefinitionPtrVec::const_iterator;

  DefinitionPtrVec defs;
  DefinitionPtrVec baseDefs;
  this->AttributeSystem->findBaseDefinitions(baseDefs);

  for (DefinitionVecIter baseIt = baseDefs.begin(); baseIt != baseDefs.end(); baseIt++)
  {
    // Add def if not abstract
    if (!(*baseIt)->isAbstract())
    {
      defs.push_back(*baseIt);
    }

    DefinitionPtrVec derivedDefs;
    this->AttributeSystem->findAllDerivedDefinitions(*baseIt, true, derivedDefs);
    defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
  }

  // Instantiate attribute for each concrete definition
  for (DefinitionVecIter defIt = defs.begin(); defIt != defs.end(); defIt++)
  {
    smtk::common::ViewPtr instView = smtk::common::View::New("Instanced", (*defIt)->type());
    this->AttributeSystem->addView(instView);

    smtk::common::View::Component& comp =
      instView->details().addChild("InstancedAttributes").addChild("Att");
    comp.setAttribute("Type", (*defIt)->type());
    comp.setAttribute("Name", (*defIt)->type());

    smtk::attribute::AttributePtr instAttr =
      this->AttributeSystem->createAttribute((*defIt)->type());

    comp.setContents(instAttr->name());
    viewsComp.addChild("View").setAttribute("Title", (*defIt)->type());
  }
}

// -----------------------------------------------------------------------------
void PreviewPanel::updateCurrentView(const QModelIndex& current, const QModelIndex& previous)
{
  const AttDefDataModel* model = qobject_cast<const AttDefDataModel*>(current.model());
  const auto def = model->getAttDef(current);

  // TODO Use the API to check whether a given view was already added (by title,
  // etc.). Create it only if necessary (ensure this approach still updates
  // changes to the definition & itemDefs correctly).
  //
  // TODO might be necessary to remove views (in order to save/export only those
  // views created by the user or already available beforehand).
  auto view = this->createView(def);
  //this->Ui->laAttDefType->setText(QString::fromStdString(def->type()));
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
void PreviewPanel::createViewWidget(const smtk::common::ViewPtr& view)
{
  if (!view)
  {
    this->setWidget(nullptr);
    //this->widget()->layout()->removeWidget(this->PreviewWidget);
    return;
  }

  // Destroying qtUIManager will cleanup after the older PreviewWidget as it is
  // parented by one of its internal widgets (m_ScrollArea).
  this->UIManager.reset(new smtk::extension::qtUIManager(this->AttributeSystem));

  //this->widget()->layout()->removeWidget(this->PreviewWidget);
  this->PreviewWidget = new QWidget(this);
  this->PreviewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QVBoxLayout* wlayout = new QVBoxLayout();
  this->PreviewWidget->setLayout(wlayout);
  this->setWidget(this->PreviewWidget);
  //this->widget()->layout()->addWidget(this->PreviewWidget);

  this->UIManager->setSMTKView(view, this->PreviewWidget, true);
}
