//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkSaveModelView.h"
#include "smtk/extension/paraview/operators/ui_smtkSaveModelParameters.h"
#include "smtk/extension/qt/qtActiveObjects.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/View.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/io/SaveJSON.txx"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/model/Operator.h"
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

class smtkSaveActions
{
public:
  std::ostringstream m_saveErrors;
  std::string m_smtkFilename;
  std::string m_embedDir;
  std::map<smtk::model::EntityRef, smtk::model::StringData> m_modelChanges;
  //std::map<smtk::mesh::CollectionPtr,smtk::model::StringData> m_meshChanges;
  std::map<std::string, std::string> m_copyFiles;
  std::map<std::string, std::string> m_saveModels;
  std::map<smtk::mesh::CollectionPtr, std::string> m_saveMeshes;
  std::string m_errorColor;
  bool m_enabled;

  smtkSaveActions()
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
    //this->m_meshChanges.clear();
    this->m_copyFiles.clear();
    this->m_saveModels.clear();
    this->m_saveMeshes.clear();
  }
};

class smtkSaveModelViewInternals : public Ui::smtkSaveModelParameters
{
public:
  smtkSaveModelViewInternals()
    : AssocModels(nullptr)
    , FileItem(nullptr)
    , SummaryMode("save")
    , Fini(false)
  {
  }

  ~smtkSaveModelViewInternals()
  {
    /*
    if (CurrentAtt)
      {
      delete CurrentAtt;
      }
      */
    delete this->AssocModels;
    delete this->FileItem;
  }

  qtModelEntityItem* AssocModels;
  qtFileItem* FileItem;
  QString UserFilename;
  std::string SummaryMode;

  smtkSaveActions SaveActions;
  smtkSaveActions SaveAsActions;
  smtkSaveActions SaveACopyActions;

  smtk::weak_ptr<smtk::model::Operator> CurrentOp;

  bool Fini; // Prevent access to child widgets after parent is destroyed.
};

template <typename T>
bool smtkSaveModelView::updateOperatorFromUI(const std::string& mode, const T& action)
{
  using namespace ::boost::filesystem;
  using namespace smtk::model;

  smtk::shared_ptr<smtk::model::Operator> op = this->Internals->CurrentOp.lock();
  if (!op || (mode != "save" && mode != "save as" && mode != "save a copy"))
  {
    return false;
  }

  smtk::attribute::ModelEntityItem::Ptr assocSrc = this->Internals->AssocModels->modelEntityItem();
  smtk::attribute::ModelEntityItem::Ptr assocDst = op->specification()->associations();
  if (assocSrc != assocDst)
  {
    assocDst->setValues(assocSrc->begin(), assocSrc->end(), 0);
  }
  path fullSMTKPath = action.m_embedDir.empty() ? path(action.m_smtkFilename)
                                                : path(action.m_embedDir) / action.m_smtkFilename;

  op->findFile("filename")->setValue(fullSMTKPath.string());

  op->findVoid("undo edits")->setIsEnabled(mode == "save a copy");

  smtk::attribute::GroupItemPtr propEdits = op->findGroup("property edits");
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

  smtk::attribute::StringItemPtr copyFilesItem = op->findString("copy files");
  copyFilesItem->setNumberOfValues(2 * action.m_copyFiles.size());
  int cfn = 0;
  for (auto cfit = action.m_copyFiles.begin(); cfit != action.m_copyFiles.end(); ++cfit, ++cfn)
  {
    copyFilesItem->setValue(2 * cfn, cfit->first);
    copyFilesItem->setValue(2 * cfn + 1, cfit->second);
  }

  smtk::attribute::StringItemPtr saveModelsItem = op->findString("save models");
  saveModelsItem->setNumberOfValues(2 * action.m_saveModels.size());
  int smn = 0;
  for (auto smit = action.m_saveModels.begin(); smit != action.m_saveModels.end(); ++smit, ++smn)
  {
    saveModelsItem->setValue(2 * smn, smit->first);
    saveModelsItem->setValue(2 * smn + 1, smit->second);
  }

  smtk::attribute::MeshItemPtr saveMeshesItem = op->findMesh("save meshes");
  smtk::attribute::StringItemPtr saveMeshURLsItem = op->findString("save mesh urls");
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

smtkSaveModelView::smtkSaveModelView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkSaveModelViewInternals;
}

smtkSaveModelView::~smtkSaveModelView()
{
  delete this->Internals;
}

bool smtkSaveModelView::displayItem(smtk::attribute::ItemPtr item)
{
  (void)item;
  // We display everything ourselves:
  return false;
}

qtBaseView* smtkSaveModelView::createViewWidget(const ViewInfo& info)
{
  //std::cout << "createViewWidget\n";
  smtkSaveModelView* view = new smtkSaveModelView(info);
  view->buildUI();
  return view;
}

void smtkSaveModelView::attributeModified()
{
  // Always enable the apply button here.
  //std::cout << "attributeModified\n";
  this->updateSummary("unhovered");
}

void smtkSaveModelView::refreshSummary()
{
  /*
  std::cout << "refresh (rename " << (this->Internals->RenameModelsBtn->isChecked() ? "T" : "F") << " embed "
    << (this->Internals->EmbedDataBtn->isChecked() ? "T" : "F") << ")\n";
    */
  this->updateSummary("unhovered");
}

void smtkSaveModelView::widgetDestroyed(QObject* w)
{
  if (this->Widget == w)
  {
    this->Internals->Fini = true;
  }
}

void smtkSaveModelView::updateAttributeData()
{
  //std::cout << "updateAttributeData\n";
  smtk::common::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
  {
    return;
  }

  /*
  if(this->Internals->CurrentAtt)
    {
    delete this->Internals->CurrentAtt;
    }
    */

  int i = view->details().findChild("AttributeTypes");
  if (i < 0)
  {
    return;
  }
  smtk::common::View::Component& comp = view->details().child(i);
  std::string defName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  {
    smtk::common::View::Component& attComp = comp.child(ci);
    //std::cout << "  component " << attComp.name() << "\n";
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      //std::cout << "    component type " << optype << "\n";
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

  smtk::model::OperatorPtr saveModelOp =
    this->uiManager()->activeModelView()->operatorsWidget()->existingOperator(defName);
  this->Internals->CurrentOp = saveModelOp;

  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = saveModelOp->specification();
  //this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
}

void smtkSaveModelView::createWidget()
{
  //std::cout << "createWidget\n";
  smtk::common::ViewPtr view = this->getObject();
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

  this->Internals->AssocModels =
    new qtModelEntityItem(this->Internals->CurrentOp.lock()->specification()->associations(),
      nullptr, this, Qt::Horizontal);
  QObject::connect(&qtActiveObjects::instance(), SIGNAL(activeModelChanged()),
    this->Internals->AssocModels, SLOT(clearEntityAssociations()));
  layout->addWidget(this->Internals->AssocModels->widget());

  this->Internals->FileItem = new qtFileItem(
    this->Internals->CurrentOp.lock()->specification()->findAs<smtk::attribute::FileSystemItem>(
      "filename", smtk::attribute::ACTIVE_CHILDREN),
    nullptr, this, Qt::Horizontal);
  layout->addWidget(this->Internals->FileItem->widget());

  QWidget* wtmp = new QWidget;
  this->Internals->setupUi(wtmp);
  layout->addWidget(wtmp);

  QObject::connect(
    this->Widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));

  // Signals and slots that run the operator
  QObject::connect(this->Internals->SaveBtn, SIGNAL(released()), this, SLOT(onSave()));
  QObject::connect(this->Internals->SaveAsBtn, SIGNAL(released()), this, SLOT(onSaveAs()));
  QObject::connect(this->Internals->SaveACopyBtn, SIGNAL(released()), this, SLOT(onSaveACopy()));

  // Respond to focus-in/out events on the save buttons:
  this->Internals->SaveBtn->installEventFilter(this);
  this->Internals->SaveBtn->setMouseTracking(true);
  this->Internals->SaveAsBtn->installEventFilter(this);
  this->Internals->SaveAsBtn->setMouseTracking(true);
  this->Internals->SaveACopyBtn->installEventFilter(this);
  this->Internals->SaveACopyBtn->setMouseTracking(true);

  QObject::connect(
    this->Internals->RenameModelsBtn, SIGNAL(stateChanged(int)), this, SLOT(refreshSummary()));
  QObject::connect(
    this->Internals->EmbedDataBtn, SIGNAL(stateChanged(int)), this, SLOT(refreshSummary()));

  // Ask the widget showing associations to fetch the
  // currently-selected models:
  this->Internals->AssocModels->onRequestEntityAssociation();
  this->updateSummary("unhovered");
}

bool smtkSaveModelView::eventFilter(QObject* obj, QEvent* evnt)
{
  if (obj == this->Internals->SaveBtn)
  {
    if (evnt->type() == QEvent::FocusIn || evnt->type() == QEvent::Enter)
    {
      this->updateSummary("save");
      //std::cout << "Save summary update\n";
    }
  }
  else if (obj == this->Internals->SaveAsBtn)
  {
    if (evnt->type() == QEvent::FocusIn || evnt->type() == QEvent::Enter)
    {
      this->updateSummary("save as");
      //std::cout << "Save as focus\n";
    }
  }
  else if (obj == this->Internals->SaveACopyBtn)
  {
    if (evnt->type() == QEvent::FocusIn || evnt->type() == QEvent::Enter)
    {
      this->updateSummary("save a copy");
      //std::cout << "Save a copy focus\n";
    }
  }

  if (obj == this->Internals->SaveBtn || obj == this->Internals->SaveAsBtn ||
    obj == this->Internals->SaveACopyBtn)
  {
    if (evnt->type() == QEvent::FocusOut || evnt->type() == QEvent::Leave)
    {
      //std::cout << "unhovered\n";
      this->updateSummary("unhovered");
    }
  }
  return this->smtk::extension::qtBaseView::eventFilter(obj, evnt);
}

void smtkSaveModelView::mouseMoveEvent(QMouseEvent* event)
{
  (void)event;
  //std::cout << "Urf\n";
}

void smtkSaveModelView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !op->specification())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkSaveModelView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentOp.lock())
  {
    return;
  }
  // Reset widgets here
}

void smtkSaveModelView::valueChanged(smtk::attribute::ItemPtr valItem)
{
  (void)valItem;
  std::cout << "Item " << valItem->name() << " type " << valItem->type() << " changed\n";
  this->updateSummary("unhovered");
}

bool smtkSaveModelView::canSave() const
{
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
}

bool smtkSaveModelView::onSave()
{
  std::cout << "Try saving\n";
  if (this->updateOperatorFromUI("save as", this->Internals->SaveActions))
  {
    this->requestOperation(this->Internals->CurrentOp.lock());
    return true;
  }
  return false;
}

bool smtkSaveModelView::onSaveAs()
{
  std::cout << "Try saving as\n";
  if (this->updateOperatorFromUI("save as", this->Internals->SaveAsActions))
  {
    this->requestOperation(this->Internals->CurrentOp.lock());
    return true;
  }
  return false;
}

bool smtkSaveModelView::onSaveACopy()
{
  std::cout << "Try saving a copy\n";
  if (this->updateOperatorFromUI("save a copy", this->Internals->SaveACopyActions))
  {
    this->requestOperation(this->Internals->CurrentOp.lock());
    return true;
  }
  return false;
}

void smtkSaveModelView::chooseFile(const std::string& mode)
{
  if (this->Internals->FileItem)
  {
    if (this->Internals->FileItem->onLaunchFileBrowser())
    { // User picked a file... try to perform the save
      this->attemptSave(mode);
    }
  }
}

void smtkSaveModelView::attemptSave(const std::string& mode)
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
      shouldSave = this->updateOperatorFromUI("save", this->Internals->SaveActions);
    }
    else if (mode == "save as")
    {
      shouldSave = this->updateOperatorFromUI("save as", this->Internals->SaveAsActions);
    }
    else if (mode == "save a copy")
    {
      shouldSave = this->updateOperatorFromUI("save a copy", this->Internals->SaveACopyActions);
    }
    else
    {
      shouldSave = false;
    }
    if (shouldSave)
    {
      this->requestOperation(this->Internals->CurrentOp.lock());
    }
    /*
    smtk::model::SessionRef sref =
      this->Internals->AssocModels->modelEntityItem()->value().owningSession();
    this->uiManager()->activeModelView()->requestOperation(
      "save smtk model", sref.entity(), true);
      */
  }
}

void smtkSaveModelView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void smtkSaveModelView::setEmbedData(bool doEmbed)
{
  this->Internals->EmbedDataBtn->setCheckState(doEmbed ? Qt::Checked : Qt::Unchecked);
}

void smtkSaveModelView::setRenameModels(bool doRename)
{
  this->Internals->RenameModelsBtn->setCheckState(doRename ? Qt::Checked : Qt::Unchecked);
}

void smtkSaveModelView::updateUI()
{
  this->qtBaseView::updateUI();
}

void smtkSaveModelView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

void smtkSaveModelView::requestModelEntityAssociation()
{
  std::cout << "requestModelEntityAssociation\n";
  this->updateAttributeData();
}

void smtkSaveModelView::setModeToPreview(const std::string& mode)
{
  this->updateSummary(mode);
}

void smtkSaveModelView::setModelToSave(const smtk::model::Model& model)
{
  smtk::model::EntityRefs assoc;
  assoc.insert(model);
  this->Internals->AssocModels->associateEntities(assoc, /*resetExisting*/ true);
  this->updateActions();
}

void smtkSaveModelView::updateSummary(const std::string& mode)
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
  smtkSaveActions* action = (this->Internals->SummaryMode == "save"
      ? &this->Internals->SaveActions
      : (this->Internals->SummaryMode == "save as"
            ? &this->Internals->SaveAsActions
            : (this->Internals->SummaryMode == "save a copy" ? &this->Internals->SaveACopyActions
                                                             : nullptr)));

  this->Internals->FileItem->widget()->setEnabled(mode != "save");

  if (action)
  {
    //this->Internals->SaveSummaryLabel->setText(action->m_saveErrors.str().c_str());
    std::string summary;
    std::ostringstream builder;
    builder << "<h2> Summary (" << this->Internals->SummaryMode << ")</h2><br>";

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
    /*
    std::map<smtk::mesh::CollectionPtr,smtk::model::StringData>::const_iterator mecit;
    for (mecit = action->m_meshChanges.begin(); mecit != action->m_meshChanges.end(); ++mecit)
    {
      if (mecit->second.find("name") != mecit->second.end())
      {
        builder << "<li>" << mecit->first->name() << " to " << mecit->second.find("name")->second[0] << "</li>\n";
        didRename = true;
      }
    }
    */
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

    this->Internals->SaveSummaryLabel->setText(summary.c_str());
  }

  QPushButton* btns[3] = {
    this->Internals->SaveBtn, this->Internals->SaveAsBtn, this->Internals->SaveACopyBtn,
  };
  QLabel* lbls[3] = {
    this->Internals->SaveStatus, this->Internals->SaveAsStatus, this->Internals->SaveACopyStatus,
  };
  smtkSaveActions* act[3] = {
    &this->Internals->SaveActions, &this->Internals->SaveAsActions,
    &this->Internals->SaveACopyActions,
  };
  QLabel* statusLabel;
  for (int ii = 0; ii < 3; ++ii)
  {
    focusBtn = btns[ii];
    action = act[ii];
    statusLabel = lbls[ii];
    if (!action->m_enabled)
    {
      QColor btnColor = this->uiManager()->invalidValueColor();
      QPalette pal = focusBtn->palette();
      pal.setColor(QPalette::Window, btnColor);
      statusLabel->setAutoFillBackground(true);
      statusLabel->setPalette(pal);
      statusLabel->update();
      statusLabel->setText(QString::fromUtf8(" \xe2\x9c\x97 ", 5));
      focusBtn->setEnabled(false);
    }
    else
    {
      focusBtn->setAutoFillBackground(false);
      QPalette pal = QApplication::palette();
      statusLabel->setPalette(pal);
      statusLabel->update();
      statusLabel->setText(QString::fromUtf8(" \xe2\x9c\x93 ", 5));
      focusBtn->setEnabled(true);
    }
  }
}

void smtkSaveModelView::updateActions()
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
  this->Internals->SaveACopyActions.reset();
  this->Internals->SaveACopyActions.m_enabled =
    smtk::io::SaveJSON::prepareToSave(models, "save a copy", filename,
      this->Internals->RenameModelsBtn->isChecked() ? "only default" : "none",
      this->Internals->EmbedDataBtn->isChecked(), this->Internals->SaveACopyActions);
}
