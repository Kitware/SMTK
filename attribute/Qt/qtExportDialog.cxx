/*=========================================================================

  Program:   ERDC Hydro
  Module:    qtExportDialog.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME Represents a dialog for texturing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtExportDialog.h"

#include "ui_qtqtExportDialog.h"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QEventLoop>

#include <pqFileDialog.h>
#include "pqServer.h"

#include "vtkXMLDataParser.h"
#include "vtkSBDataContainer.h"
#include "vtkSBAnalysisContainer.h"

#include "../erdcCMBModel.h"
#include "qtADHExporter.h"
#include "qtPT123Exporter.h"
#include "../qtModelBuilderOptions.h"

QString qtExportDialog::LastExtension = "All files (*)";
QString qtExportDialog::LastFormatFileParsed = "";
//-----------------------------------------------------------------------------
qtExportDialog::qtExportDialog(vtkSBAnalysisContainer *container) :
  Status(-1), ActiveServer(0), Parser(0),RootDataContainer(0), ParserStatus(0),
  CMBModel(0)
{

  this->MainDialog = new QDialog();
  this->Parser = vtkXMLDataParser::New();
  this->Dialog = new Ui::qtqtExportDialog;
  this->Dialog->setupUi(MainDialog);

  // Turn off Accept
  this->setAcceptable(false);

  QVBoxLayout* frameLayout = new QVBoxLayout(this->Dialog->groupBox_AnalysisType);
  int i, n = container->GetNumberOfAnalysisGroups();
  for (i = 0; i < n; i++)
    {
    QCheckBox* typeCheck = new QCheckBox(this->Dialog->groupBox_AnalysisType);
    typeCheck->setText(container->GetAnalysisGroup(i));
    // TODO: Need to check for active groups
    // TODO: General catagory is always active
    typeCheck->setChecked(1);
    frameLayout->addWidget(typeCheck);
    }

  QObject::connect(this->Dialog->FileNameText, SIGNAL(textChanged(const QString &)),
    this, SLOT(validate()));

  QObject::connect(this->Dialog->FormatFileNameText, SIGNAL(editingFinished()),
    this, SLOT(formatFileChanged()));

  QObject::connect(this->Dialog->FileBrowserButton, SIGNAL(clicked()),
    this, SLOT(displayFileBrowser()));

  QObject::connect(this->Dialog->FormatFileBrowserButton, SIGNAL(clicked()),
    this, SLOT(displayFormatFileBrowser()));



  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));

  // Lets restore the last format file we used
  this->Dialog->FormatFileNameText->setText(this->LastFormatFileParsed);
  this->LastFormatFileParsed = "";
  this->formatFileChanged();
}

//-----------------------------------------------------------------------------
qtExportDialog::~qtExportDialog()
{
  if (this->Parser)
    {
    this->Parser->Delete();
    }
  if (this->Dialog)
    {
    delete Dialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}

//-----------------------------------------------------------------------------
int qtExportDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();

  QEventLoop loop;
  QObject::connect(this->MainDialog, SIGNAL(finished(int)), &loop, SLOT(quit()));
  loop.exec();

  return this->Status;
}
///-----------------------------------------------------------------------------
void qtExportDialog::formatFileChanged()
{
  QString formatName = this->getFormatFileName();
  // If the format name is "" make sure we deactivate the accept button

  if (formatName == "")
    {
    this->setAcceptable(false);
    return;
    }
  // if the format file name has not changed then nothing needs to be done
 if (formatName == this->LastFormatFileParsed)
    {
    return;
    }
  // See if we can parse the file
  this->Parser->SetFileName(formatName.toStdString().c_str());
  this->ParserStatus = this->Parser->Parse();
  if (!this->ParserStatus)
    {
    QString message("Could not parse format file: ");
    message += formatName;
    QMessageBox::warning(NULL, "Exporter Warning!",
                         message);
    this->Dialog->FormatFileNameText->setText("");
    // Turn off Accept
    this->setAcceptable(false);
    return;
    }
  this->LastFormatFileParsed = formatName;
  // Get the Extension
  vtkXMLDataElement* rootFormatElement = this->Parser->GetRootElement();
  const char *str = rootFormatElement->GetAttribute("Extension");
  // If there is a default extension then see if it matches the last one we had
  if (str)
    {
    if (this->LastExtension != str)
      {
      // The extension has changed so clear out the output file name
      this->Dialog->FileNameText->setText("");
      this->LastExtension = str;
      }
    }
  else
    {
    // there is no set extension to use - default to all files
    this->LastExtension = "All files (*)";
    }
  // See if we are in a valid state
  this->validate();
}
//-----------------------------------------------------------------------------
void qtExportDialog::accept()
{
  // Get the type of exporter needed
  vtkXMLDataElement* rootFormatElement = this->Parser->GetRootElement();
  const char *str = rootFormatElement->GetAttribute("ExporterType");
  qtExporter *exporter;
  // If there is no Exporter Type assume we are using the generic one
  if (!str)
    {
    exporter = new qtExporter;
    }
  else
    {
    QString etype = str;
    if (etype == "ADH")
      {
      exporter = new qtADHExporter;
      }
    else if (etype == "PT123")
      {
      exporter = new qtPT123Exporter;
      }
    else if (etype == "Generic")
      {
      exporter = new qtExporter;
      }
    else
      {
      QString txt("Exporter Type: ");
      txt += etype;
      txt += " is not supported.  Using Generic Exporter!";
      QMessageBox::warning(NULL, "Export  Warning!", txt);
      exporter = new qtExporter;
      }
    }

  // There should be at least 1 analysis type (typically the General catagory)
  // to be processed

  int numAnalysisTypes = 0;
  QLayout* typeLayout = this->Dialog->groupBox_AnalysisType->layout();
  for(int i=0; i<typeLayout->count(); i++)
    {
    QCheckBox* typeBox = static_cast<QCheckBox*>
      (typeLayout->itemAt(i)->widget());
    if(typeBox && typeBox->isChecked())
      {
      exporter->addAnalysisToExport(typeBox->text().toStdString());
      ++numAnalysisTypes;
      }
    }
  if (numAnalysisTypes == 0)
    {
    QMessageBox::warning(NULL, "Export  Warning!",
      "There are no analysis types selected!");
    }
  else
    {
    exporter->setRootDataContainer(this->RootDataContainer);
    exporter->setActiveServer(this->ActiveServer);
    vtkSMProxy* serverModelProxy = 0;
    if(this->CMBModel)
      {
      exporter->setModel(this->CMBModel->getModel());
      serverModelProxy = this->CMBModel->getModelWrapper();
      }
    exporter->write(this->getFileName().toStdString(), rootFormatElement,
                   serverModelProxy);
    }
  delete exporter;
}
//-----------------------------------------------------------------------------
void qtExportDialog::cancel()
{
  this->Status = 0;
}

//-----------------------------------------------------------------------------
QString qtExportDialog::getFileName() const
{
  return  this->Dialog->FileNameText->text();
}

//-----------------------------------------------------------------------------
QString qtExportDialog::getFormatFileName() const
{
  return  this->Dialog->FormatFileNameText->text();
}
//-----------------------------------------------------------------------------
void qtExportDialog::setRootDataContainer(slctk::AttributeItemPtr root)
{
  this->RootDataContainer = root;
}
//-----------------------------------------------------------------------------
void qtExportDialog::setCmbModel(erdcCMBModel* model)
{
  this->CMBModel = model;
}
//-----------------------------------------------------------------------------
void qtExportDialog::setActiveServer(pqServer* server)
{
  this->ActiveServer = server;
}

//-----------------------------------------------------------------------------
void qtExportDialog::displayFormatFileBrowser()
{
  QString startDir = qtModelBuilderOptions::instance()->
    defaultqtTemplateDirectory().c_str();

  pqFileDialog file_dialog(
    this->ActiveServer,
    this->MainDialog, tr("Open Format XML:"), startDir, QString("Format XML files (*.xml)"));
  file_dialog.setObjectName("InputDialog");
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.setFileMode(pqFileDialog::ExistingFile); //open
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      this->Dialog->FormatFileNameText->setText(files[0]);
      this->formatFileChanged();
      }
    }
}
//-----------------------------------------------------------------------------
void qtExportDialog::displayFileBrowser()
{
  pqFileDialog file_dialog(
    this->ActiveServer,
    this->MainDialog, tr("Save Simulation Output:"), QString(), this->LastExtension);
  file_dialog.setObjectName("OutputDialog");
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      this->Dialog->FileNameText->setText(files[0]);
      this->validate();
      }
    }
}

//-----------------------------------------------------------------------------
void qtExportDialog::validate()
{
  bool valid;
  // In order to be valid we need both a valid format and output file names
  valid = (this->getFormatFileName() != "") && (this->getFileName() != "");
  this->setAcceptable(valid);
}
//-----------------------------------------------------------------------------
void qtExportDialog::setAcceptable(bool state)
{
  this->Dialog->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(!state);
}
