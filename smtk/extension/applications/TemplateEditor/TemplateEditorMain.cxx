//==============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//==============================================================================
#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

#include "smtk/attribute/System.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "AttDefDataModel.h"
#include "AttDefInformation.h"
#include "AttributeBrowser.h"
#include "PreviewPanel.h"
#include "TemplateEditorMain.h"
#include "ui_TemplateEditorMain.h"

#define APP_NAME "Template Editor"
#define TE_TYPES                                                                                   \
  "Simulation Baseline Template (*.sbt);; CMB Resource File (*.crf);; XML Files (*.xml)"

// -----------------------------------------------------------------------------
TemplateEditorMain::TemplateEditorMain()
  : Ui(new Ui::TemplateEditorMain)
{
  this->Ui->setupUi(this);
  this->connectActions();
}

// -----------------------------------------------------------------------------
TemplateEditorMain::~TemplateEditorMain() = default;

// -----------------------------------------------------------------------------
void TemplateEditorMain::connectActions()
{
  QObject::connect(this->Ui->actNew, SIGNAL(triggered()), this, SLOT(onNew()));
  QObject::connect(this->Ui->actLoad, SIGNAL(triggered()), this, SLOT(onLoad()));
  QObject::connect(this->Ui->actSave, SIGNAL(triggered()), this, SLOT(onSave()));
  QObject::connect(this->Ui->actSaveAs, SIGNAL(triggered()), this, SLOT(onSaveAs()));
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::initialize()
{
  if (this->AttDefInfo == nullptr)
  {
    this->AttDefInfo = new AttDefInformation(this);
  }

  if (this->AttDefBrowser == nullptr)
  {
    this->AttDefBrowser = new AttributeBrowser(this);
    this->Ui->menuView->addAction(this->AttDefBrowser->toggleViewAction());
  }

  if (this->AttPreviewPanel == nullptr)
  {
    this->AttPreviewPanel = new PreviewPanel(this, this->AttributeSystem);
    this->Ui->menuView->addAction(this->AttPreviewPanel->toggleViewAction());
  }

  connect(this->AttDefBrowser, SIGNAL(attDefChanged(const QModelIndex&, const QModelIndex&)),
    this->AttDefInfo, SLOT(onAttDefChanged(const QModelIndex&, const QModelIndex&)));
  connect(this->AttDefBrowser, SIGNAL(attDefChanged(const QModelIndex&, const QModelIndex&)),
    this->AttPreviewPanel, SLOT(updateCurrentView(const QModelIndex&, const QModelIndex&)));

  this->AttDefBrowser->populate(this->AttributeSystem);

  /// TODO centralWidget -> make it a QStackedWidget and use a page per
  // Tab (Analysis, Categories, Definitions, Views, etc.)
  QMainWindow::setCentralWidget(this->AttDefInfo);
  QMainWindow::addDockWidget(Qt::LeftDockWidgetArea, this->AttDefBrowser);
  QMainWindow::addDockWidget(Qt::RightDockWidgetArea, this->AttPreviewPanel);

  this->Ui->actSave->setEnabled(true);
  this->Ui->actSaveAs->setEnabled(true);
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::reset()
{
  this->ActiveFilePath = QString();
  this->setWindowTitle(APP_NAME);

  delete this->AttDefBrowser;
  this->AttDefBrowser = nullptr;

  delete this->AttDefInfo;
  this->AttDefInfo = nullptr;

  delete this->AttPreviewPanel;
  this->AttPreviewPanel = nullptr;

  this->AttributeSystem = nullptr;

  this->Ui->actSave->setEnabled(false);
  this->Ui->actSaveAs->setEnabled(false);
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::onNew()
{
  this->reset();
  this->AttributeSystem = smtk::attribute::System::create();
  this->initialize();
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::onLoad()
{
  QFileDialog dialog(this, tr("Open Template"), ".", tr(TE_TYPES));
  dialog.setFileMode(QFileDialog::ExistingFile);

  if (dialog.exec())
  {
    QStringList names = dialog.selectedFiles();
    QString fileName = names.at(0);
    this->load(fileName.toStdString().c_str());
  }
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::load(char const* fileName)
{
  this->reset();
  this->AttributeSystem = smtk::attribute::System::create();

  smtk::io::AttributeReader reader;
  smtk::io::Logger logger;
  const bool err = reader.read(this->AttributeSystem, fileName, true, logger);

  if (err)
  {
    const QString text = "Failed to load template file! " + QString::fromStdString(fileName) +
      "\n" + QString::fromStdString(logger.convertToString()) + "\n";
    QMessageBox::critical(this, "Error", text);

    this->reset();
    return;
  }

  this->ActiveFilePath = QString(fileName);
  this->setWindowTitle(APP_NAME + QString(": ") + this->ActiveFilePath);
  this->initialize();
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::onSave()
{
  this->save(this->ActiveFilePath);
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::onSaveAs()
{
  QFileDialog dialog(this, tr("Save File As..."), ".", tr(TE_TYPES));
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setAcceptMode(QFileDialog::AcceptSave);

  if (dialog.exec())
  {
    const QStringList names = dialog.selectedFiles();
    QString fileName = names.at(0);
    const QString ext = dialog.selectedNameFilter().right(5).left(4);
    fileName += ext;

    this->save(fileName);
  }
}

// -----------------------------------------------------------------------------
void TemplateEditorMain::save(const QString& filePath)
{
  smtk::io::AttributeWriter writer;
  smtk::io::Logger logger;
  const bool err = writer.write(this->AttributeSystem, filePath.toStdString(), logger);

  if (err)
  {
    const QString text = "Error writing simulation file:\n" + filePath + "\n" +
      QString::fromStdString(logger.convertToString()) + "\n";
    QMessageBox::critical(this, "Error", text);
  }
}
