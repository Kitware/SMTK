//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOperationDialog.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtViewInfoDialog.h"
#include "smtk/model/Registrar.h"
#include "smtk/view/Manager.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

using namespace smtk::extension;

class qtOperationDialogInternals
{
public:
  QPushButton* m_applyButton;
  QPushButton* m_cancelButton;
  QTabWidget* m_tabWidget;

  QSharedPointer<smtk::extension::qtUIManager> m_uiManager;
  smtk::extension::qtOperationView* m_smtkView;
  smtk::operation::OperationPtr m_operation;

  qtOperationDialogInternals()
    : m_applyButton(nullptr)
    , m_cancelButton(nullptr)
    , m_tabWidget(nullptr)
    , m_smtkView(nullptr)
  {
  }
};

qtOperationDialog::qtOperationDialog(
  smtk::operation::OperationPtr op, smtk::view::ManagerPtr viewManager, QWidget* parentWidget)
  : QDialog(parentWidget)
{
  m_internals = new qtOperationDialogInternals();
  m_internals->m_operation = op;

  QVBoxLayout* dialogLayout = new QVBoxLayout(this);
  m_internals->m_tabWidget = new QTabWidget(this);

  // 1. Create the editor tab
  QWidget* editorWidget = new QWidget(this);
  QVBoxLayout* editorLayout = new QVBoxLayout(editorWidget);

  // Create the ui manager
  auto specResource = op->specification();
  m_internals->m_uiManager = QSharedPointer<smtk::extension::qtUIManager>(
    new smtk::extension::qtUIManager(op, specResource->manager(), viewManager));

  // Create the SMTK view
  auto viewConfig = m_internals->m_uiManager->findOrCreateOperationView();
  auto qtView = m_internals->m_uiManager->setSMTKView(viewConfig, editorWidget);
  m_internals->m_smtkView = dynamic_cast<smtk::extension::qtOperationView*>(qtView);

  editorWidget->setLayout(editorLayout);
  m_internals->m_tabWidget->addTab(editorWidget, "Parameters");

  // 2. Create the info tab
  QTextEdit* infoWidget = new QTextEdit(this);
  infoWidget->setReadOnly(true);

  QString html;
  qtViewInfoDialog::formatInfoHtml(op->parameters(), html);
  infoWidget->insertHtml(html);
  infoWidget->moveCursor(QTextCursor::Start);
  m_internals->m_tabWidget->addTab(infoWidget, "Info");
  m_internals->m_tabWidget->setCurrentIndex(0);

  dialogLayout->addWidget(m_internals->m_tabWidget);

  // 3. Add dialog buttons and replace operation view buttons
  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
  dialogLayout->addWidget(buttonBox);
  this->setLayout(dialogLayout);

  m_internals->m_applyButton = buttonBox->button(QDialogButtonBox::Apply);
  m_internals->m_applyButton->setDefault(true);
  m_internals->m_cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

  QObject::connect(m_internals->m_smtkView, &smtk::extension::qtOperationView::operationExecuted,
    this, &qtOperationDialog::onOperationExecuted);
  QObject::connect(m_internals->m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

  m_internals->m_smtkView->setButtons(m_internals->m_applyButton, nullptr, nullptr);
  bool isValid = m_internals->m_operation->parameters()->isValid();
  m_internals->m_applyButton->setEnabled(isValid);

  // 4. And the window title
  std::string title = viewConfig->label();
  title.append(" Information");
  this->setWindowTitle(title.c_str());
}

qtOperationDialog::~qtOperationDialog()
{
  delete m_internals;
}

void qtOperationDialog::onOperationExecuted(const smtk::operation::Operation::Result& result)
{
  emit this->operationExecuted(result);
  this->accept(); // closes the dialog
}

void qtOperationDialog::showEvent(QShowEvent* event)
{
  QDialog::showEvent(event);

  // Call resize() on next tick to resolve/workaround sizing issues
  QTimer::singleShot(0, this, [this]() { this->resize(this->minimumSizeHint()); });
}
