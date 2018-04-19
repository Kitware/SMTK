//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vxl/operators/smtkTerrainExtractionView.h"

#include "pqApplicationCore.h"
#include "pqFileDialog.h"

#include "smtk/extension/vxl/operators/ui_smtkTerrainExtractionParameters.h"
#include "smtk/extension/vxl/widgets/pqTerrainExtractionManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/view/View.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QValidator>
#include <QtWidgets/QWidget>

using namespace smtk::extension;

class smtkTerrainExtractionViewInternals : public Ui::TerrainExtractionParameters
{
public:
  smtkTerrainExtractionViewInternals() {}
  ~smtkTerrainExtractionViewInternals()
  {
    if (TerrainExtractionAtt)
    {
      delete TerrainExtractionAtt;
    }
  }

  qtAttribute* createAttUI(smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    if (att && att->numberOfItems() > 0)
    {
      qtAttribute* attInstance = new qtAttribute(att, pw, view);
      // attInstance->setUseSelectionManager(view->useSelectionManager());
      if (attInstance && attInstance->widget())
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("terrainExtractionEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
      }
      return attInstance;
    }
    return NULL;
  }

  QPointer<qtAttribute> TerrainExtractionAtt;
  smtk::weak_ptr<smtk::operation::Operation> terrainExtractionOp;
  smtk::weak_ptr<smtk::operation::Operation> addAux_GeomOp;
  QPointer<QWidget> terrainExtraction;
};

smtkTerrainExtractionView::smtkTerrainExtractionView(const smtk::extension::ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new smtkTerrainExtractionViewInternals;
  this->TerrainExtractionManager = new pqTerrainExtractionManager();

  // SIGNAL SLOTS that has nothing to do with UI
  QObject::connect(this->TerrainExtractionManager, SIGNAL(numPointsCalculationFinshed(long)), this,
    SLOT(onNumPointsCalculationFinshed(long)));
  // ResolutionEdit value is calculated by manager
  QObject::connect(this->TerrainExtractionManager,
    &pqTerrainExtractionManager::resolutionEditChanged, this,
    &smtkTerrainExtractionView::onResolutionEditChanged);

  // Let user choose desired result then load it in
  QObject::connect(this->TerrainExtractionManager, SIGNAL(showPickResultFileDialog(std::string&)),
    this, SLOT(onShowPickResultFileDialog(std::string&)));

  // Use add auxiliary geometry operator to load result into ModelBuilder
  QObject::connect(this->TerrainExtractionManager, SIGNAL(viewTerrainExtractionResults()), this,
    SLOT(onViewTerrainExtractionResults()));
}

smtkTerrainExtractionView::~smtkTerrainExtractionView()
{
  delete this->Internals;
  delete this->TerrainExtractionManager;
}

qtBaseView* smtkTerrainExtractionView::createViewWidget(const smtk::extension::ViewInfo& info)
{
  smtkTerrainExtractionView* view = new smtkTerrainExtractionView(info);
  view->buildUI();
  return view;
}

Ui::TerrainExtractionParameters* smtkTerrainExtractionView::terrainExtractionParameterUI()
{
  return this->Internals;
}

void smtkTerrainExtractionView::attributeModified()
{
  // Enable when user has picked a point cloud
  this->Internals->terrainExtraction->setEnabled(
    this->Internals->TerrainExtractionAtt->attribute()->isValid());

  if (this->Internals->TerrainExtractionAtt->attribute()->isValid())
  {
    // Pass in the aux_geom to manager
    smtk::attribute::AttributePtr spec = this->Internals->terrainExtractionOp.lock()->parameters();
    auto modelItem = spec->associations();
    smtk::model::AuxiliaryGeometry aux(modelItem->valueAs<smtk::model::Entity>());
    if (!aux.isValid())
    {
      qCritical() << "No AuxiliaryGeometry is associated with the operator.\n";
      return;
    }
    // set aux_geom and compute basic resolution
    this->TerrainExtractionManager->setAuxGeom(aux);
    // guess cache directory
    QFileInfo cacheDirInfo(this->Internals->CacheDirectoryLabel->text());

    if (!cacheDirInfo.isDir())
    {
      QString directory = QDir::tempPath();
      this->Internals->CacheDirectoryLabel->setText(directory);
      this->Internals->CacheDirectoryLabel->setToolTip(directory);

      QFileInfo extractFileInfo(directory + "/TerrainExtract.pts");
      this->Internals->autoSaveLabel->setText(extractFileInfo.absoluteFilePath());
      this->Internals->autoSaveLabel->setToolTip("Files of different resolution "
                                                 "would be generated");
    }

    // Cache add auxiliary geometry operator if not cached yet so when
    // terrain extraction has finished, we can load the result into Model Builder
    if (!this->Internals->addAux_GeomOp.lock())
    {
      // Add auxgom operator has been called before. Safe to skip check conditon
      std::string aux_GeomName("add auxiliary geometry");

      auto auxOp = this->uiManager()->operationManager()->create(aux_GeomName);
      this->Internals->addAux_GeomOp = auxOp;

      this->TerrainExtractionManager->setAuxGeomOperation(auxOp);
    }
  }
}

void smtkTerrainExtractionView::onViewTerrainExtractionResults()
{
  this->requestOperation(this->Internals->addAux_GeomOp.lock());
  this->requestOperation(this->Internals->terrainExtractionOp.lock());
}

void smtkTerrainExtractionView::onShowPickResultFileDialog(std::string& filename)
{
  QString filters = "LIDAR ASCII (*.pts);; LIDAR binary (*.bin.pts);; VTK PolyData (*.vtp);;";
  QString baseFileName = this->Internals->autoSaveLabel->text();
  QFileInfo baseFileInfo(baseFileName);
  pqFileDialog file_dialog(pqApplicationCore::instance()->getActiveServer(), this->parentWidget(),
    tr("Base Filename for Extraction Output:"), baseFileInfo.absolutePath(), filters);
  file_dialog.setFileMode(pqFileDialog::ExistingFile);
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.setObjectName("FileOpenDialog");

  bool ret = file_dialog.exec() == QDialog::Accepted;
  if (ret)
  {
    QFileInfo extractFileInfo(file_dialog.getSelectedFiles()[0]);
    filename = extractFileInfo.absoluteFilePath().toStdString();
  }
}

void smtkTerrainExtractionView::onNumPointsCalculationFinshed(long numPoints)
{
  this->Internals->scaleNumPointsLabel->setText(
    "will generate ~" + QString::number(numPoints) + " points.");
}

void smtkTerrainExtractionView::createWidget()
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
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  this->updateAttributeData();

  this->Internals->terrainExtraction = new QWidget;
  this->Internals->setupUi(this->Internals->terrainExtraction);
  // ui must have a default layout other wise it would not work
  layout->addWidget(this->Internals->terrainExtraction);
  this->Internals->terrainExtraction->setEnabled(false);

  /// Signals and slots
  // User changes resolutionEdit
  QObject::connect(this->Internals->resolutionEdit, SIGNAL(textChanged(QString)),
    this->TerrainExtractionManager, SLOT(onResolutionScaleChange(QString)));

  QObject::connect(this->Internals->detailedResolutionButton, SIGNAL(clicked(bool)),
    this->TerrainExtractionManager, SLOT(ComputeDetailedResolution()));

  QObject::connect(this->Internals->autoSaveFileButton, SIGNAL(clicked(bool)), this,
    SLOT(onAutoSaveExtractFileName()));

  // For now just hide cachedGroup since it's not clear
  this->Internals->cacheGroup->setVisible(false);
  QObject::connect(this->Internals->cacheDirectoryButton, SIGNAL(clicked(bool)), this,
    SLOT(onSelectCacheDirectory()));
  QObject::connect(this->Internals->saveRefinedCheckBox, SIGNAL(toggled(bool)),
    this->TerrainExtractionManager, SLOT(onSaveRefineResultsChange(bool)));

  // Mask size controls
  QDoubleValidator* maskValidator = new QDoubleValidator(0.0, 1.0, 8, this->Internals->MaskSize);
  maskValidator->setNotation(QDoubleValidator::StandardNotation);
  this->Internals->MaskSize->setValidator(maskValidator);
  QObject::connect(this->Internals->MaskSize, SIGNAL(textChanged(QString)), this,
    SLOT(onMaskSizeTextChanged(QString)));
  QObject::connect(this->Internals->processFullExtraction, SIGNAL(clicked()), this,
    SLOT(onProcessFullExtraction()));

  // Show help when the info button is clicked.
  QObject::connect(this->Internals->InfoButton, SIGNAL(released()), this, SLOT(onInfo()));

  // Check if the user has already specified a point cloud
  this->attributeModified();
}

void smtkTerrainExtractionView::updateAttributeData()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
  {
    return;
  }

  if (this->Internals->TerrainExtractionAtt)
  {
    delete this->Internals->TerrainExtractionAtt;
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
      if (optype == "terrain extraction")
      {
        defName = optype;
        break;
      }
    }
  }
  if (defName.empty())
  {
    std::cout << "No match terrain extraction!" << std::endl;
    return;
  }

  // FIXME: This used to fetch a pre-existing operation, which assumed there was only one.
  smtk::operation::OperationPtr terrainExtractionOp =
    this->uiManager()->operationManager()->create(defName);
  this->Internals->terrainExtractionOp = terrainExtractionOp;

  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = terrainExtractionOp->parameters();
  this->Internals->TerrainExtractionAtt = this->Internals->createAttUI(att, this->Widget, this);
  if (this->Internals->TerrainExtractionAtt)
  {
    QObject::connect(
      this->Internals->TerrainExtractionAtt, SIGNAL(modified()), this, SLOT(attributeModified()));
  }
}

void smtkTerrainExtractionView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void smtkTerrainExtractionView::cancelOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !this->Widget || !this->Internals->TerrainExtractionAtt)
  {
    return;
  }
  // Reset widgets here
}

bool smtkTerrainExtractionView::onAutoSaveExtractFileName()
{
  QString filters = "LIDAR ASCII (*.pts);; LIDAR binary (*.bin.pts);; VTK PolyData (*.vtp);;";
  QString baseFileName = this->Internals->autoSaveLabel->text();
  QFileInfo baseFileInfo(baseFileName);
  pqFileDialog file_dialog(pqApplicationCore::instance()->getActiveServer(), this->parentWidget(),
    tr("Base Filename for Extraction Output:"), baseFileInfo.canonicalPath(), filters);
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.setObjectName("FileSaveDialog");

  bool ret = file_dialog.exec() == QDialog::Accepted;
  if (ret)
  {
    //use the file info so that sperators between auto save & cache stay consitent
    QFileInfo extractFileInfo(file_dialog.getSelectedFiles()[0]);
    this->Internals->autoSaveLabel->setText(extractFileInfo.absoluteFilePath());
    this->Internals->autoSaveLabel->setToolTip(extractFileInfo.absoluteFilePath());
  }
  return ret;
}

void smtkTerrainExtractionView::onMaskSizeTextChanged(QString text)
{
  if (text.size() == 0 || text == ".")
  {
    //we want to exempt empty strings from the below changes so that people
    //can delete the current text.
    //we want to also exempt a string starting with the decimal dot.
    return;
  }
  QLineEdit* masksize = this->Internals->MaskSize;
  const QDoubleValidator* validator = qobject_cast<const QDoubleValidator*>(masksize->validator());
  if (validator)
  {
    int pos = 0; //needed just as paramter for the double validator
    QValidator::State state = validator->validate(text, pos);
    if (state != QValidator::Acceptable)
    {
      //convert this to the closest value
      double value = text.toDouble();
      value = (value < validator->bottom()) ? validator->bottom() : validator->top();
      masksize->setText(QString::number(value));
    }
  }
}

bool smtkTerrainExtractionView::onSelectCacheDirectory()
{
  QString directory = this->Internals->CacheDirectoryLabel->text();
  QFileInfo dirInfo(directory);

  pqFileDialog file_dialog(pqApplicationCore::instance()->getActiveServer(), this->parentWidget(),
    tr("Cache Directory:"), dirInfo.absoluteFilePath());
  file_dialog.setObjectName("Cache Directory Dialog");
  file_dialog.setFileMode(pqFileDialog::Directory);

  bool ret = file_dialog.exec() == QDialog::Accepted;
  if (ret)
  {
    QFileInfo cacheDirInfo(file_dialog.getSelectedFiles()[0]);
    QString afp = cacheDirInfo.absoluteFilePath();
    QLabel* cacheLbl = this->Internals->CacheDirectoryLabel;

    //if the text is longer than the viewable area, right align the text
    Qt::Alignment align = (afp.size() >= 35) ? Qt::AlignRight : Qt::AlignLeft;
    cacheLbl->setAlignment(align);

    this->Internals->CacheDirectoryLabel->setText(afp);
    this->Internals->CacheDirectoryLabel->setToolTip(afp);
  }
  return ret;
}

void smtkTerrainExtractionView::onProcessFullExtraction()
{
  double scale = this->Internals->resolutionEdit->text().toDouble();
  double maskSize = this->Internals->MaskSize->text().toDouble();
  QFileInfo cacheFileInfo(this->Internals->CacheDirectoryLabel->text());
  QFileInfo autoSaveInfo(this->Internals->autoSaveLabel->text());
  bool computeColor = this->Internals->computeColor->isChecked();
  bool viewOutput = this->Internals->ViewOnCompletionCheckBox->isChecked();
  bool pickCustomResult = this->Internals->terrainExtractionOp.lock()
                            ->parameters()
                            ->findVoid("pick custom result")
                            ->isEnabled();
  this->TerrainExtractionManager->onProcesssFullData(
    scale, maskSize, cacheFileInfo, autoSaveInfo, computeColor, viewOutput, pickCustomResult);
}

void smtkTerrainExtractionView::valueChanged(smtk::attribute::ItemPtr /*valItem*/)
{
  // Do nothing since the operation is done in the custom view
  //this->requestOperation(this->Internals->terrainExtractionOp.lock());
}

void smtkTerrainExtractionView::onResolutionEditChanged(QString scaleString)
{
  this->Internals->resolutionEdit->setText(scaleString);
}

void smtkTerrainExtractionView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}

void smtkTerrainExtractionView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->getObject());
}
