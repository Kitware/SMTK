//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkExportModelView.h"
#include "smtk/extension/paraview/operators/ui_smtkExportModelParameters.h"
#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtOperationView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/view/View.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/io/SaveJSON.txx"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/model/SessionRef.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqPresetDialog.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSettings.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTableWidget>
#include <QVBoxLayout>

#include "boost/filesystem.hpp"

#include <sstream>

using namespace smtk::extension;

class smtkExportActions
{
public:
  std::ostringstream m_saveErrors;
  std::string m_smtkFilename;
  std::string m_embedDir;
  std::map<smtk::model::EntityRef, smtk::model::StringData> m_modelChanges;
  std::map<std::string, std::string> m_copyFiles;
  std::map<std::string, std::string> m_saveModels;
  std::map<smtk::mesh::CollectionPtr, std::string> m_saveMeshes;
  std::string m_errorColor;
  bool m_enabled;

  smtkExportActions()
    : m_errorColor("#aa7777")
    , m_enabled(true)
  {
  }

  void reset()
  {
    this->m_saveErrors.str("");
    this->m_saveErrors.clear();
    this->m_enabled = true;
    this->m_modelChanges.clear();
    this->m_copyFiles.clear();
    this->m_saveModels.clear();
    this->m_saveMeshes.clear();
  }
};

class smtkExportModelViewInternals : public Ui::smtkExportModelParameters
{
public:
  smtkExportModelViewInternals()
    : FileItem(nullptr)
    , SummaryMode("save")
    , Fini(false)
  {
  }

  ~smtkExportModelViewInternals() { delete this->FileItem; }

  qtFileItem* FileItem;
  QString UserFilename;
  std::string SummaryMode;

  smtkExportActions SaveActions;
  smtkExportActions SaveAsActions;
  smtkExportActions ExportActions;

  smtk::weak_ptr<smtk::operation::Operation> CurrentOp;

  bool Fini; // Prevent access to child widgets after parent is destroyed.
};

template <typename T>
bool smtkExportModelView::updateOperationFromUI(const std::string& mode, const T& action)
{
  using namespace ::boost::filesystem;
  using namespace smtk::model;

  smtk::shared_ptr<smtk::operation::Operation> op = this->Internals->CurrentOp.lock();
  if (!op || (mode != "save" && mode != "save as" && mode != "save a copy"))
  {
    return false;
  }

  /*
  smtk::attribute::ModelEntityItem::Ptr assocSrc = this->Internals->AssocModels->modelEntityItem();
  smtk::attribute::ModelEntityItem::Ptr assocDst = op->parameters()->associations();
  if (assocSrc != assocDst)
  {
    assocDst->setValues(assocSrc->begin(), assocSrc->end(), 0);
  }
  */
  path fullSMTKPath = action.m_embedDir.empty() ? path(action.m_smtkFilename)
                                                : path(action.m_embedDir) / action.m_smtkFilename;

  op->parameters()->findFile("filename")->setValue(fullSMTKPath.string());

  op->parameters()->findVoid("undo edits")->setIsEnabled(mode == "save a copy");

  smtk::attribute::GroupItemPtr propEdits = op->parameters()->findGroup("property edits");
  propEdits->setNumberOfGroups(action.m_modelChanges.size());
  int grp = 0;
  for (auto mcit = action.m_modelChanges.begin(); mcit != action.m_modelChanges.end();
       ++mcit, ++grp)
  {
    propEdits->findAs<smtk::attribute::ModelEntityItem>(grp, "edit entity")->setValue(mcit->first);
    smtk::attribute::GroupItemPtr valPairs =
      propEdits->findAs<smtk::attribute::GroupItem>(grp, "value pairs");
    valPairs->setNumberOfGroups(mcit->second.size());
    int prop = 0;
    for (auto kit = mcit->second.begin(); kit != mcit->second.end(); ++kit, ++prop)
    {
      valPairs->findAs<smtk::attribute::StringItem>(prop, "edit property")->setValue(kit->first);
      valPairs->findAs<smtk::attribute::StringItem>(prop, "edit values")
        ->setValues(kit->second.begin(), kit->second.end());
    }
  }

  smtk::attribute::StringItemPtr copyFilesItem = op->parameters()->findString("copy files");
  copyFilesItem->setNumberOfValues(2 * action.m_copyFiles.size());
  int cfn = 0;
  for (auto cfit = action.m_copyFiles.begin(); cfit != action.m_copyFiles.end(); ++cfit, ++cfn)
  {
    copyFilesItem->setValue(2 * cfn, cfit->first);
    copyFilesItem->setValue(2 * cfn + 1, cfit->second);
  }

  smtk::attribute::StringItemPtr saveModelsItem = op->parameters()->findString("save models");
  saveModelsItem->setNumberOfValues(2 * action.m_saveModels.size());
  int smn = 0;
  for (auto smit = action.m_saveModels.begin(); smit != action.m_saveModels.end(); ++smit, ++smn)
  {
    saveModelsItem->setValue(2 * smn, smit->first);
    saveModelsItem->setValue(2 * smn + 1, smit->second);
  }

  smtk::attribute::MeshItemPtr saveMeshesItem = op->parameters()->findMesh("save meshes");
  smtk::attribute::StringItemPtr saveMeshURLsItem = op->parameters()->findString("save mesh urls");
  saveMeshesItem->setNumberOfValues(action.m_saveMeshes.size());
  saveMeshURLsItem->setNumberOfValues(action.m_saveMeshes.size());
  smn = 0;
  for (auto smit = action.m_saveMeshes.begin(); smit != action.m_saveMeshes.end(); ++smit, ++smn)
  {
    saveMeshesItem->setValue(smn, smtk::mesh::MeshSet(smit->first, 0));
    saveMeshURLsItem->setValue(smn, smit->second);
  }

  return true;
}

smtkExportModelView::smtkExportModelView(const ViewInfo& info)
  : smtkModelIOView(info)
{
  this->Internals = new smtkExportModelViewInternals;
  auto opinfo = dynamic_cast<const OperationViewInfo*>(&info);
  if (opinfo)
  {
    this->Internals->CurrentOp = opinfo->m_operator;
  }
}

smtkExportModelView::~smtkExportModelView()
{
  delete this->Internals;
}

bool smtkExportModelView::displayItem(smtk::attribute::ItemPtr item)
{
  (void)item;
  // We display everything ourselves:
  return false;
}

qtBaseView* smtkExportModelView::createViewWidget(const ViewInfo& info)
{
  smtkExportModelView* view = new smtkExportModelView(info);
  view->buildUI();
  return view;
}

void smtkExportModelView::attributeModified()
{
  // Always enable the apply button here.
  this->updateSummary("unhovered");
}

void smtkExportModelView::refreshSummary()
{
  this->updateSummary("unhovered");
}

void smtkExportModelView::widgetDestroyed(QObject* w)
{
  if (this->Widget == w)
  {
    this->Internals->Fini = true;
  }
}

void smtkExportModelView::updateAttributeData()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
  {
    return;
  }

  int i = view->details().findChild("AttributeTypes");
  if (i < 0)
  {
    return;
  }
  smtk::view::View::Component& comp = view->details().child(i);
  std::string defName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  {
    smtk::view::View::Component& attComp = comp.child(ci);
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      if (optype == "save smtk model")
      {
        defName = optype;
        break;
      }
    }
  }
  if (defName.empty())
  {
    return;
  }

  smtk::operation::OperationPtr saveModelOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperation(defName);
  this->Internals->CurrentOp = saveModelOp;

  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = saveModelOp->parameters();
  //this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
}

void smtkExportModelView::createWidget()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());

  // Delete any pre-existing widget
  if (this->Widget)
  {
    if (parentlayout)
    {
      parentlayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  // Create a new frame and lay it out
  this->Widget = new QFrame(this->parentWidget());
  this->Internals->Fini = false;
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  this->updateAttributeData();

  /*
  this->Internals->AssocModels =
    new qtModelEntityItem(this->Internals->CurrentOp.lock()->parameters()->associations(),
      nullptr, this, Qt::Horizontal);
  //this->Internals->AssocModels->setUseSelectionManager(this->useSelectionManager());
  QObject::connect(&qtActiveObjects::instance(), SIGNAL(activeModelChanged()),
    this->Internals->AssocModels, SLOT(clearEntityAssociations()));
  layout->addWidget(this->Internals->AssocModels->widget());
  */

  this->Internals->FileItem = new qtFileItem(
    this->Internals->CurrentOp.lock()->parameters()->findAs<smtk::attribute::FileSystemItem>(
      "filename", smtk::attribute::ACTIVE_CHILDREN),
    nullptr, this, Qt::Horizontal);
  layout->addWidget(this->Internals->FileItem->widget());

  QWidget* wtmp = new QWidget;
  this->Internals->setupUi(wtmp);
  this->Internals->EmbedDataBtn->setChecked(false);
  this->Internals->EmbedDataBtn->hide();
  layout->addWidget(wtmp);

  QObject::connect(
    this->Widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));

  // Signals and slots that run the operator
  //QObject::connect(this->Internals->SaveBtn, SIGNAL(released()), this, SLOT(onSave()));
  //QObject::connect(this->Internals->SaveAsBtn, SIGNAL(released()), this, SLOT(onSaveAs()));
  QObject::connect(this->Internals->ExportBtn, SIGNAL(released()), this, SLOT(onExport()));

  // Respond to focus-in/out events on the save buttons:
  //this->Internals->SaveBtn->installEventFilter(this);
  //this->Internals->SaveBtn->setMouseTracking(true);
  //this->Internals->SaveAsBtn->installEventFilter(this);
  //this->Internals->SaveAsBtn->setMouseTracking(true);
  this->Internals->ExportBtn->installEventFilter(this);
  this->Internals->ExportBtn->setMouseTracking(true);

  QObject::connect(
    this->Internals->RenameModelsBtn, SIGNAL(stateChanged(int)), this, SLOT(refreshSummary()));
  QObject::connect(
    this->Internals->EmbedDataBtn, SIGNAL(stateChanged(int)), this, SLOT(refreshSummary()));

  // Ask the widget showing associations to fetch the
  // currently-selected models:
  // this->Internals->AssocModels->onRequestEntityAssociation();
  this->updateSummary("unhovered");

  // Connect the "info" button to display help
  QObject::connect(this->Internals->InfoBtn, SIGNAL(released()), this, SLOT(onInfo()));
  this->Internals->InfoBtn->setEnabled(true);
}

bool smtkExportModelView::eventFilter(QObject* obj, QEvent* evnt)
{
  if (obj == this->Internals->ExportBtn)
  {
    if (evnt->type() == QEvent::FocusIn || evnt->type() == QEvent::Enter)
    {
      this->updateSummary("save a copy");
    }
  }

  if (obj == this->Internals->ExportBtn)
  {
    if (evnt->type() == QEvent::FocusOut || evnt->type() == QEvent::Leave)
    {
      this->updateSummary("unhovered");
    }
  }
  return this->smtk::extension::qtBaseView::eventFilter(obj, evnt);
}

bool smtkExportModelView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return false;
  }
  return this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkExportModelView::cancelOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentOp.lock())
  {
    return;
  }
  // Reset widgets here
}

void smtkExportModelView::valueChanged(smtk::attribute::ItemPtr valItem)
{
  (void)valItem;
  this->updateSummary("unhovered");
}

bool smtkExportModelView::canSave() const
{
  /*
  smtk::attribute::ModelEntityItem::Ptr assoc = this->Internals->AssocModels->modelEntityItem();
  bool ok = true;
  for (auto ait = assoc->begin(); ait != assoc->end(); ++ait)
  {
    if (!ait->hasStringProperty("smtk_url"))
    {
      ok = false;
      break;
    }
  }
  return ok;
  */
  return true;
}

bool smtkExportModelView::onSave()
{
  if (this->updateOperationFromUI("save as", this->Internals->SaveActions))
  {
    this->requestOperation(this->Internals->CurrentOp.lock());
    return true;
  }
  return false;
}

bool smtkExportModelView::onSaveAs()
{
  if (this->updateOperationFromUI("save as", this->Internals->SaveAsActions))
  {
    this->requestOperation(this->Internals->CurrentOp.lock());
    return true;
  }
  return false;
}

bool smtkExportModelView::onExport()
{
  if (this->updateOperationFromUI("save a copy", this->Internals->ExportActions))
  {
    this->requestOperation(this->Internals->CurrentOp.lock());
    return true;
  }
  return false;
}

bool smtkExportModelView::chooseFile(const std::string& mode)
{
  if (this->Internals->FileItem)
  {
    if (this->Internals->FileItem->onLaunchFileBrowser())
    { // User picked a file... try to perform the save
      return this->attemptSave(mode);
    }
  }
  return false;
}

bool smtkExportModelView::attemptSave(const std::string& mode)
{
  bool shouldSave = false;
  if (mode == "save")
  {
    if (this->canSave())
    {
      shouldSave = true;
      // Plop the "smtk_url" into the filename item
      ::boost::filesystem::path embedDir(this->Internals->SaveActions.m_embedDir);
      this->Internals->FileItem->setInputValue(
        QString::fromStdString((embedDir / this->Internals->SaveActions.m_smtkFilename).string()));
    }
  }
  else
  {
    shouldSave = true;
  }

  if (shouldSave)
  {
    if (mode == "save")
    {
      shouldSave = this->updateOperationFromUI("save", this->Internals->SaveActions);
    }
    else if (mode == "save as")
    {
      shouldSave = this->updateOperationFromUI("save as", this->Internals->SaveAsActions);
    }
    else if (mode == "save a copy")
    {
      shouldSave = this->updateOperationFromUI("save a copy", this->Internals->ExportActions);
    }
    else
    {
      shouldSave = false;
    }
    if (shouldSave)
    {
      return this->requestOperation(this->Internals->CurrentOp.lock());
    }
  }
  return false;
}

void smtkExportModelView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkExportModelView::setEmbedData(bool doEmbed)
{
  this->Internals->EmbedDataBtn->setCheckState(doEmbed ? Qt::Checked : Qt::Unchecked);
}

void smtkExportModelView::setRenameModels(bool doRename)
{
  this->Internals->RenameModelsBtn->setCheckState(doRename ? Qt::Checked : Qt::Unchecked);
}

void smtkExportModelView::updateUI()
{
  this->qtBaseView::updateUI();
}

void smtkExportModelView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

void smtkExportModelView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkExportModelView::setModeToPreview(const std::string& mode)
{
  this->updateSummary(mode);
}

void smtkExportModelView::setModelToSave(const smtk::model::Model& model)
{
  smtk::model::EntityRefs assoc;
  assoc.insert(model);
  // this->Internals->AssocModels->associateEntities(assoc, /*resetExisting*/ true);
  this->updateActions();
}

void smtkExportModelView::updateSummary(const std::string& mode)
{
  if (this->Internals->Fini)
  {
    return;
  }
  this->updateActions();
  QPushButton* focusBtn;
  if (mode != "unhovered")
  {
    this->Internals->SummaryMode = mode;
  }
  smtkExportActions* action = (this->Internals->SummaryMode == "save"
      ? &this->Internals->SaveActions
      : (this->Internals->SummaryMode == "save as"
            ? &this->Internals->SaveAsActions
            : (this->Internals->SummaryMode == "save a copy" ? &this->Internals->ExportActions
                                                             : nullptr)));

  this->Internals->FileItem->widget()->setEnabled(mode != "save");

  if (action)
  {
    std::string summary;
    std::ostringstream builder;
    builder << "<h2> Summary (export)</h2><br>";

    std::map<std::string, std::string>::const_iterator ssit;
    if (action->m_enabled)
    {
      builder << "<b>Saving</b><ul>\n"
              << "<li> " << action->m_smtkFilename << " to " << action->m_embedDir << "</li>\n";
      ;
      for (ssit = action->m_saveModels.begin(); ssit != action->m_saveModels.end(); ++ssit)
      {
        builder << "<li> " << ssit->first << " to " << ssit->second << "</li>\n";
      }
      std::map<smtk::mesh::CollectionPtr, std::string>::const_iterator smit;
      for (smit = action->m_saveMeshes.begin(); smit != action->m_saveMeshes.end(); ++smit)
      {
        builder << "<li> " << smit->first->name() << " to " << smit->second << "</li>\n";
      }
      builder << "</ul>\n";
    }
    else
    {
      builder << action->m_saveErrors.str();
    }
    summary += builder.str();
    builder.str("");
    builder.clear();

    bool didRename = false;
    builder << "<b>Renaming:</b><ul>\n";
    std::map<smtk::model::EntityRef, smtk::model::StringData>::const_iterator mocit;
    for (mocit = action->m_modelChanges.begin(); mocit != action->m_modelChanges.end(); ++mocit)
    {
      if (mocit->second.find("name") != mocit->second.end())
      {
        builder << "<li>" << mocit->first.name() << " to " << mocit->second.find("name")->second[0]
                << "</li>\n";
        didRename = true;
      }
    }
    builder << "</ul>\n";
    if (didRename)
    {
      summary += builder.str();
    }
    builder.str("");
    builder.clear();

    bool didCopy = false;
    builder << "<b>Copying:</b><ul>\n";
    for (ssit = action->m_copyFiles.begin(); ssit != action->m_copyFiles.end(); ++ssit)
    {
      builder << "<li> " << ssit->first << " to " << ssit->second << "</li>\n";
      didCopy = true;
    }
    builder << "</ul>\n";
    if (didCopy)
    {
      summary += builder.str();
    }
    builder.str("");
    builder.clear();

    this->Internals->ExportSummaryLabel->setText(summary.c_str());
  }

  QPushButton* btns[1] = {
    //this->Internals->SaveBtn, this->Internals->SaveAsBtn,
    this->Internals->ExportBtn,
  };
  smtkExportActions* act[1] = {
    // &this->Internals->SaveActions, &this->Internals->SaveAsActions,
    &this->Internals->ExportActions,
  };
  for (int ii = 0; ii < 1; ++ii)
  {
    focusBtn = btns[ii];
    action = act[ii];
    focusBtn->setEnabled(action->m_enabled);
  }
}

void smtkExportModelView::updateActions()
{
  if (this->Internals->Fini)
  {
    return;
  }
  smtk::model::Models models;
  smtk::model::Model active = qtActiveObjects::instance().activeModel();
  if (active.isValid())
  {
    models.push_back(active);
  }
  std::string filename;
  auto fileItem =
    smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(this->Internals->FileItem->getObject());
  if (fileItem->isSet(0))
  {
    filename = fileItem->value(0);
  }
  this->Internals->SaveActions.reset();
  this->Internals->SaveActions.m_enabled = smtk::io::SaveJSON::prepareToSave(models, "save",
    filename, this->Internals->RenameModelsBtn->isChecked() ? "only default" : "none",
    this->Internals->EmbedDataBtn->isChecked(), this->Internals->SaveActions);
  this->Internals->SaveAsActions.reset();
  this->Internals->SaveAsActions.m_enabled = smtk::io::SaveJSON::prepareToSave(models, "save as",
    filename, this->Internals->RenameModelsBtn->isChecked() ? "only default" : "none",
    this->Internals->EmbedDataBtn->isChecked(), this->Internals->SaveAsActions);
  this->Internals->ExportActions.reset();
  this->Internals->ExportActions.m_enabled =
    smtk::io::SaveJSON::prepareToSave(models, "save a copy", filename,
      this->Internals->RenameModelsBtn->isChecked() ? "only default" : "none",
      this->Internals->EmbedDataBtn->isChecked(), this->Internals->ExportActions);
}

void smtkExportModelView::setInfoToBeDisplayed()
{
  this->m_infoDialog->displayInfo(this->getObject());
}
