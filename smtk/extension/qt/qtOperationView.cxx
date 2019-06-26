//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtInstancedView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/extension/qt/qtOperationLauncher.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/view/View.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <memory>

using namespace smtk::attribute;
using namespace smtk::extension;

class qtOperationViewInternals
{
public:
  qtOperationViewInternals()
    : m_instancedView(nullptr)
    , m_activeOperations(0)
  {
  }
  smtk::operation::OperationPtr m_operator;
  std::unique_ptr<qtInstancedView> m_instancedView;
  smtk::view::ViewPtr m_instancedViewDef;
  QPointer<QPushButton> m_applyButton;
  QPointer<QPushButton> m_infoButton;
  qtOperationLauncher* m_launcher;
  std::atomic<std::size_t> m_activeOperations;
};

qtBaseView* qtOperationView::createViewWidget(const ViewInfo& info)
{
  const OperationViewInfo* opinfo = dynamic_cast<const OperationViewInfo*>(&info);
  qtOperationView* view;
  if (!opinfo)
  {
    return NULL;
  }
  view = new qtOperationView(*opinfo);
  view->buildUI();
  return view;
}

qtOperationView::qtOperationView(const OperationViewInfo& info)
  : qtBaseAttributeView(info)
  , m_applied(false)
{
  this->Internals = new qtOperationViewInternals;
  this->Internals->m_operator = info.m_operator;
  // We need to create a new View for the internal instanced View
  this->Internals->m_instancedViewDef = smtk::view::View::New("Instanced", "Parameters");
  smtk::view::ViewPtr view = this->getObject();
  if (view)
  {
    this->Internals->m_instancedViewDef->copyContents(*view);
    // We need to remove the TopLevel attribute (if there is one)
    this->Internals->m_instancedViewDef->details().unsetAttribute("TopLevel");
    // The default top-level behavior is that filter by category and advance level is on by default.
    // For Operation View they need to be explicilty set to turn them on
    if (!view->details().attribute("FilterByAdvanceLevel"))
    {
      view->details().setAttribute("FilterByAdvanceLevel", "false");
    }
    if (!view->details().attribute("FilterByCategory"))
    {
      view->details().setAttribute("FilterByCategory", "false");
    }
  }
  if (auto manager = this->Internals->m_operator->manager())
  {
    auto launcher = manager->launchers()[qtOperationLauncher::type_name].target<qt::Launcher>();
    if (launcher == nullptr)
    {
      manager->launchers()[qtOperationLauncher::type_name] = qt::Launcher();
      launcher = manager->launchers()[qtOperationLauncher::type_name].target<qt::Launcher>();
    }
    assert(launcher != nullptr);
    this->Internals->m_launcher = launcher->get();
    connect(this->Internals->m_launcher, &qtOperationLauncher::resultReady, this,
      &qtOperationView::operationExecuted);
  }

  connect(this, &qtOperationView::operationExecuted, this, &qtOperationView::onOperationExecuted);
}

qtOperationView::~qtOperationView()
{
  delete this->Internals;
}

QPointer<QPushButton> qtOperationView::applyButton() const
{
  return this->Internals->m_applyButton;
}

smtk::operation::OperationPtr qtOperationView::operation() const
{
  return this->Internals->m_operator;
}

void qtOperationView::createWidget()
{
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
  this->Widget->setObjectName("OpViewFrame");
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  ViewInfo v(this->Internals->m_instancedViewDef, this->Widget, this->uiManager());
  qtInstancedView* iview = dynamic_cast<qtInstancedView*>(qtInstancedView::createViewWidget(v));
  this->Internals->m_instancedView.reset(iview);

  QObject::connect(iview, SIGNAL(modified()), this, SLOT(onModifiedParameters()));
  QObject::connect(iview, SIGNAL(itemModified(qtItem*)), this, SLOT(onModifiedParameter(qtItem*)));

  this->Internals->m_applyButton = new QPushButton("Apply", this->Widget);
  this->Internals->m_applyButton->setObjectName("OpViewApplyButton");
  this->Internals->m_applyButton->setMinimumHeight(32);
  this->Internals->m_applyButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->Internals->m_applyButton->setDefault(true);
  QObject::connect(this->Internals->m_applyButton, SIGNAL(clicked()), this, SLOT(onOperate()));
  //auto bbox = new QDialogButtonBox(this->Widget);
  //bbox->addButton(this->Internals->m_applyButton, QDialogButtonBox::AcceptRole);
  layout->addWidget(this->Internals->m_applyButton);
  //layout->addWidget( bbox);
  this->Internals->m_infoButton = new QPushButton("Info", this->Widget);
  this->Internals->m_infoButton->setObjectName("OpViewApplyButton");
  this->Internals->m_infoButton->setMinimumHeight(32);
  this->Internals->m_infoButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QObject::connect(this->Internals->m_infoButton, SIGNAL(clicked()), this, SLOT(onInfo()));
  //auto bbox = new QDialogButtonBox(this->Widget);
  //bbox->addButton(this->Internals->m_applyButton, QDialogButtonBox::AcceptRole);
  layout->addWidget(this->Internals->m_infoButton);
  //layout->addWidget( bbox);
  this->Internals->m_applyButton->setEnabled((!m_applied) && iview->isValid());
}

void qtOperationView::onModifiedParameters()
{
  m_applied = false;
  if (this->Internals->m_applyButton)
  {
    this->Internals->m_applyButton->setEnabled(this->Internals->m_instancedView->isValid());
  }
}

void qtOperationView::onModifiedParameter(qtItem* uiItem)
{
  auto op = this->operation();
  if (op && uiItem)
  {
    // Signal the operation that an item's value has been changed in the UI.
    // The attribute pointer is null; that is reserved for when the UI creates
    // or destroys an attribute instance.
    if (op->configure(nullptr, uiItem->item()))
    {
      this->attributeChanged(op->parameters());
    }
  }
}

void qtOperationView::showAdvanceLevelOverlay(bool show)
{
  this->Internals->m_instancedView->showAdvanceLevelOverlay(show);
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
}

void qtOperationView::requestModelEntityAssociation()
{
  this->Internals->m_instancedView->requestModelEntityAssociation();
}

void qtOperationView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->Internals->m_operator->parameters());
}

void qtOperationView::onOperate()
{
  if ((!m_applied) && this->Internals->m_instancedView->isValid())
  {
    auto future = (*this->Internals->m_launcher)(this->Internals->m_operator);
    emit this->operationRequested(this->Internals->m_operator);
    if (this->Internals->m_applyButton)
    { // The button may disappear when a session is closed by an operator.
      this->Internals->m_applyButton->setEnabled(false);
    }
    m_applied = true;

    if ((this->Internals->m_activeOperations)++ == 0)
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
      QApplication::processEvents();
    }
  }
}

void qtOperationView::onOperationExecuted(const smtk::operation::Operation::Result&)
{
  if (--(this->Internals->m_activeOperations) == 0)
  {
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
  }
}
