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

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QEvent>
#include <QObject>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

using namespace smtk::extension;

namespace
{
// Internal class for constraining QScrollArea to vertical direction
class qtVerticalScrollArea : public QScrollArea
{
public:
  qtVerticalScrollArea(QWidget* parent = nullptr)
    : QScrollArea(parent)
  {
  }

protected:
  // Override eventFilter on resize events to set width
  bool eventFilter(QObject* obj, QEvent* event) override
  {
    if (obj && obj == this->widget() && event->type() == QEvent::Resize)
    {
      // Get width of contents
      int contentsWidth =
        this->widget()->minimumSizeHint().width() + this->verticalScrollBar()->width();

      // Get width of screen
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
      const QRect screenRect = QApplication::desktop()->screenGeometry(this->widget());
      int screenWidth = screenRect.width();
#else
      int screenWidth = this->widget()->screen()->size().width();
#endif
      // If contents width is less than 1/2 screen width, expand
      // so that horizontal scrolling is not needed.
      if (contentsWidth < screenWidth / 2)
      {
        this->setMinimumWidth(contentsWidth);
      }
    }
    return QScrollArea::eventFilter(obj, event);
  }
};

} // namespace

class qtOperationDialogInternals
{
public:
  QPushButton* m_applyButton = nullptr;
  QPushButton* m_cancelButton = nullptr;
  QPushButton* m_applyCloseButton = nullptr;
  QTabWidget* m_tabWidget = nullptr;

  QSharedPointer<smtk::extension::qtUIManager> m_uiManager;
  smtk::extension::qtOperationView* m_smtkView = nullptr;
  smtk::operation::OperationPtr m_operation;

  bool m_closeAfterOperate = false;

  qtOperationDialogInternals() = default;
  ~qtOperationDialogInternals() = default;
};

qtOperationDialog::qtOperationDialog(
  smtk::operation::OperationPtr op,
  QSharedPointer<smtk::extension::qtUIManager> uiManager,
  QWidget* parentWidget,
  bool showApplyAndClose)
  : QDialog(parentWidget)
{
  this->buildUI(op, uiManager, false, showApplyAndClose);
}

qtOperationDialog::qtOperationDialog(
  smtk::operation::OperationPtr op,
  smtk::resource::ManagerPtr resManager,
  smtk::view::ManagerPtr viewManager,
  QWidget* parentWidget,
  bool showApplyAndClose)
  : QDialog(parentWidget)
{
  auto uiManager = QSharedPointer<smtk::extension::qtUIManager>(
    new smtk::extension::qtUIManager(op, resManager, viewManager));
  this->buildUI(op, uiManager, false, showApplyAndClose);
}

qtOperationDialog::qtOperationDialog(
  smtk::operation::OperationPtr op,
  smtk::resource::ManagerPtr resManager,
  smtk::view::ManagerPtr viewManager,
  bool scrollable,
  QWidget* parentWidget)
  : QDialog(parentWidget)
{
  auto uiManager = QSharedPointer<smtk::extension::qtUIManager>(
    new smtk::extension::qtUIManager(op, resManager, viewManager));
  this->buildUI(op, uiManager, scrollable, false);
}

void qtOperationDialog::buildUI(
  smtk::operation::OperationPtr op,
  QSharedPointer<smtk::extension::qtUIManager> uiManager,
  bool scrollable,
  bool showApplyAndClose)
{
  this->setObjectName("ExportDialog");
  m_internals = new qtOperationDialogInternals();
  m_internals->m_uiManager = uiManager;
  m_internals->m_operation = op;

  QVBoxLayout* dialogLayout = new QVBoxLayout(this);
  m_internals->m_tabWidget = new QTabWidget(this);
  m_internals->m_tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; }");

  // 1. Create the editor tab
  auto viewConfig = m_internals->m_uiManager->findOrCreateOperationView();
  if (scrollable)
  {
    qtVerticalScrollArea* scroll = new qtVerticalScrollArea();
    QWidget* viewport = new QWidget();
    scroll->setWidget(viewport);
    scroll->setWidgetResizable(true);

    QVBoxLayout* viewportLayout = new QVBoxLayout(viewport);
    viewport->setLayout(viewportLayout);

    // Create the SMTK view
    auto* qtView = m_internals->m_uiManager->setSMTKView(viewConfig, viewport);
    m_internals->m_smtkView = dynamic_cast<smtk::extension::qtOperationView*>(qtView);
    m_internals->m_tabWidget->addTab(scroll, "Parameters");
  }
  else
  {
    QWidget* editorWidget = new QWidget(this);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorWidget);
    auto* qtView = m_internals->m_uiManager->setSMTKView(viewConfig, editorWidget);
    editorWidget->setLayout(editorLayout);
    m_internals->m_smtkView = dynamic_cast<smtk::extension::qtOperationView*>(qtView);
    m_internals->m_tabWidget->addTab(editorWidget, "Parameters");
  }

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
  auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
  dialogLayout->addWidget(buttonBox);
  this->setLayout(dialogLayout);

  m_internals->m_applyButton = buttonBox->button(QDialogButtonBox::Apply);
  m_internals->m_cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
  if (showApplyAndClose)
  {
    m_internals->m_applyCloseButton = buttonBox->addButton(QDialogButtonBox::Yes);
    m_internals->m_applyCloseButton->setText("Apply && Close");
  }

  // don't set a default button, so "Enter" won't dismiss the dialog. But
  // make Apply come first it tab order, so tabbing to Apply then "Enter" works,
  // if the dialog is modal.
  QWidget::setTabOrder(m_internals->m_applyButton, m_internals->m_cancelButton);

  QObject::connect(
    m_internals->m_smtkView,
    &smtk::extension::qtOperationView::operationExecuted,
    this,
    &qtOperationDialog::onOperationExecuted);
  QObject::connect(m_internals->m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
  if (m_internals->m_applyCloseButton)
  {
    QObject::connect(m_internals->m_applyCloseButton, &QPushButton::clicked, this, [this]() {
      // flag that we want the dialog to close after the operation finishes
      this->m_internals->m_closeAfterOperate = true;
      // perform operation
      this->m_internals->m_smtkView->onOperate();
    });
  }

  m_internals->m_smtkView->setButtons(m_internals->m_applyButton, nullptr, nullptr);
  bool isValid = m_internals->m_operation->parameters()->isValid();
  m_internals->m_applyButton->setEnabled(isValid);

  // 4. And the window title
  std::string title = viewConfig->label();
  this->setWindowTitle(title.c_str());

  if (scrollable)
  {
    // Set min height for reasonable display
    // Application can always override this
    this->setMinimumHeight(480);
  }
}

qtOperationDialog::~qtOperationDialog()
{
  delete m_internals;
}

void qtOperationDialog::updateUI()
{
  if (m_internals->m_smtkView)
  {
    m_internals->m_smtkView->updateUI();
  }
}

const smtk::operation::OperationPtr& qtOperationDialog::operation() const
{
  return m_internals->m_smtkView->operation();
}

void qtOperationDialog::onOperationExecuted(const smtk::operation::Operation::Result& result)
{
  Q_EMIT this->operationExecuted(result);
  if (this->isModal() || this->m_internals->m_closeAfterOperate)
  {
    this->m_internals->m_closeAfterOperate = false;
    // closes the dialog, and deletes if setAttribute(Qt::WA_DeleteOnClose) as called.
    this->done(QDialog::Accepted);
  }
  else
  {
    // Let the user re-execute the operation
    m_internals->m_smtkView->onModifiedParameters();
  }
}

void qtOperationDialog::showEvent(QShowEvent* event)
{
  QDialog::showEvent(event);

  // Call resize() on next tick to resolve/workaround sizing issues
  QTimer::singleShot(0, this, [this]() { this->resize(this->minimumSizeHint()); });
}
