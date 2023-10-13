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
#include "smtk/view/Configuration.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
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
  std::unique_ptr<qtInstancedView> m_instancedView;
  smtk::view::ConfigurationPtr m_instancedViewDef;
  QPointer<QPushButton> m_applyButton;
  QPointer<QPushButton> m_infoButton;
  QPointer<QPushButton> m_doneButton;
  qtOperationLauncher* m_launcher;
  std::atomic<std::size_t> m_activeOperations;
  bool m_launchOperationOnApply{ true };
};

bool qtOperationView::validateInformation(const smtk::view::Information& info)
{
  return qtBaseAttributeView::validateInformation(info) &&
    info.contains<smtk::operation::OperationPtr>();
}

qtBaseView* qtOperationView::createViewWidget(const smtk::view::Information& info)
{
  if (qtOperationView::validateInformation(info))
  {
    auto* view = new qtOperationView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtOperationView::qtOperationView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new qtOperationViewInternals;
  // We need to create a new View for the internal instanced View
  this->Internals->m_instancedViewDef = smtk::view::Configuration::New("Instanced", "Parameters");
  smtk::view::ConfigurationPtr view = this->configuration();
  if (view)
  {
    this->Internals->m_instancedViewDef->copyContents(*view);
    // We need to remove the TopLevel attribute (if there is one)
    this->Internals->m_instancedViewDef->details().unsetAttribute("TopLevel");
    view->details().attributeAsBool("LaunchOnApply", this->Internals->m_launchOperationOnApply);
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
  if (auto manager = this->operation()->manager())
  {
    auto* launcher = manager->launchers()[qtOperationLauncher::type_name].target<qt::Launcher>();
    if (launcher == nullptr)
    {
      manager->launchers()[qtOperationLauncher::type_name] = qt::Launcher();
      launcher = manager->launchers()[qtOperationLauncher::type_name].target<qt::Launcher>();
    }
    assert(launcher != nullptr);
    this->Internals->m_launcher = launcher->get();
  }
}

qtOperationView::~qtOperationView()
{
  delete this->Internals;
}

bool qtOperationView::runOperationOnApply() const
{
  return this->Internals->m_launchOperationOnApply;
}

void qtOperationView::setRunOperationOnApply(bool shouldLaunch)
{
  this->Internals->m_launchOperationOnApply = shouldLaunch;
}

QPointer<QPushButton> qtOperationView::applyButton() const
{
  return this->Internals->m_applyButton;
}

QPointer<QPushButton> qtOperationView::doneButton() const
{
  return this->Internals->m_doneButton;
}

const smtk::operation::OperationPtr& qtOperationView::operation() const
{
  return m_viewInfo.get<smtk::operation::OperationPtr>();
}

void qtOperationView::showInfoButton(bool visible)
{
  if (!this->Internals->m_infoButton.isNull())
  {
    this->Internals->m_infoButton->setVisible(visible);
  }
}

void qtOperationView::setButtons(
  QPointer<QPushButton> applyButton,
  QPointer<QPushButton> infoButton,
  QPointer<QPushButton> doneButton)
{
  // Disconnect and hide current buttons
  this->Internals->m_applyButton->disconnect();
  this->Internals->m_infoButton->disconnect();
  this->Internals->m_doneButton->disconnect();
  this->Internals->m_applyButton->hide();
  this->Internals->m_infoButton->hide();
  this->Internals->m_doneButton->hide();

  // Assign new buttons
  this->Internals->m_applyButton = applyButton.data();
  this->Internals->m_infoButton = infoButton.data();
  this->Internals->m_doneButton = doneButton.data();

  if (applyButton.data() != nullptr)
  {
    QObject::connect(
      this->Internals->m_applyButton, &QPushButton::clicked, this, &qtOperationView::onOperate);
  }
  if (infoButton.data() != nullptr)
  {
    QObject::connect(
      this->Internals->m_infoButton, &QPushButton::clicked, this, &qtOperationView::onInfo);
  }
  if (doneButton.data() != nullptr)
  {
    QObject::connect(
      this->Internals->m_doneButton,
      &QAbstractButton::clicked,
      this,
      &qtOperationView::doneEditing);
  }
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

  // Create  the information to create an Instance View
  smtk::view::Information v = m_viewInfo;
  v.insertOrAssign<smtk::view::ConfigurationPtr>(this->Internals->m_instancedViewDef);
  v.insertOrAssign<QWidget*>(this->Widget);
  v.insertOrAssign<qtOperationView*>(this);
  v.insertOrAssign<smtk::operation::Operation::Ptr>(this->operation());
  v.insertOrAssign<smtk::operation::Manager::Ptr>(this->operation()->manager());

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
  this->Internals->m_infoButton->setObjectName("OpViewInfoButton");
  this->Internals->m_infoButton->setMinimumHeight(32);
  this->Internals->m_infoButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QObject::connect(this->Internals->m_infoButton, SIGNAL(clicked()), this, SLOT(onInfo()));
  //auto bbox = new QDialogButtonBox(this->Widget);
  //bbox->addButton(this->Internals->m_applyButton, QDialogButtonBox::AcceptRole);
  layout->addWidget(this->Internals->m_infoButton);
  //layout->addWidget( bbox);
  this->Internals->m_applyButton->setEnabled((!m_applied) && iview->isValid());

  this->Internals->m_doneButton = new QPushButton("Done", this->Widget);
  this->Internals->m_doneButton->setObjectName("OpViewDoneButton");
  this->Internals->m_doneButton->setToolTip("Click to stop editing this operation's parameters. "
                                            "This has no effect on already-running operations.");
  this->Internals->m_doneButton->setMinimumHeight(32);
  this->Internals->m_doneButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QObject::connect(
    this->Internals->m_doneButton, &QAbstractButton::clicked, this, &qtOperationView::doneEditing);
  layout->addWidget(this->Internals->m_doneButton);
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

void qtOperationView::updateUI()
{
  this->Internals->m_instancedView->updateUI();
}

void qtOperationView::onShowCategory()
{
  this->Internals->m_instancedView->onShowCategory();
  this->onModifiedParameters(); // updates Apply button state
}

void qtOperationView::showAdvanceLevelOverlay(bool show)
{
  this->Internals->m_instancedView->showAdvanceLevelOverlay(show);
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
}

void qtOperationView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->operation()->parameters());
}

void qtOperationView::onOperate()
{
  if ((!m_applied) && this->Internals->m_instancedView->isValid())
  {
    const auto& myOperation = this->operation();
    if (this->Internals->m_launchOperationOnApply)
    {
      shared_ptr<smtk::extension::ResultHandler> handler =
        (*this->Internals->m_launcher)(myOperation);

      connect(
        handler.get(),
        &smtk::extension::ResultHandler::resultReady,
        this,
        &qtOperationView::operationExecuted);
    }

    Q_EMIT this->operationRequested(myOperation);
    if (this->Internals->m_applyButton && m_disableApply)
    { // The button may disappear when a session is closed by an operator.
      this->Internals->m_applyButton->setEnabled(false);
    }
    m_applied = true;
    if (!m_disableApply)
    {
      // Rate-limit running the operation to about 10 Hz to
      // prevent accidental multiple-presses of the Apply button.
      QTimer::singleShot(100 /*ms*/, this, &qtOperationView::onModifiedParameters);
    }
  }
}
