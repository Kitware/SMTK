//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtExtractContoursView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/View.h"
#include "smtk/extension/paraview/widgets/pqGenerateContoursDialog.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/model/Operator.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"

#include "vtkClientServerStream.h"
#include "vtkPVDataInformation.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSession.h"
#include "vtkSMSourceProxy.h"

#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtExtractContoursViewInternals
{
public:
  qtExtractContoursViewInternals()
    {
    }
  ~qtExtractContoursViewInternals()
    {
    if(CurrentAtt)
      delete CurrentAtt;
    if(this->ContoursDialog)
      {
      this->ContoursDialog->close();
      delete this->ContoursDialog;
      }
    }
  qtAttribute* createAttUI(
    smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    if(att && att->numberOfItems()>0)
      {
      qtAttribute* attInstance = new qtAttribute(att, pw, view);
      if(attInstance && attInstance->widget())
        {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("polygonContourOpEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
        }
      return attInstance;
      }
    return NULL;
  }

  vtkSMProxy* createVTKContourOperator(
    vtkSMProxy *sourceProxy)
  {
    vtkSMProxy* smPolyEdgeOp = vtkSMProxyManager::GetProxyManager()->NewProxy(
      "polygon_operators", "PolygonContourOperator");
    if(!smPolyEdgeOp)
      return NULL;
    smPolyEdgeOp->UpdateVTKObjects();
    sourceProxy->UpdateVTKObjects();

    vtkClientServerStream stream;
    stream  << vtkClientServerStream::Invoke
            << VTKOBJECT(sourceProxy) << "GetOutputDataObject"
            << 0
            << vtkClientServerStream::End;
    smPolyEdgeOp->GetSession()->ExecuteStream(smPolyEdgeOp->GetLocation(), stream);

    stream  << vtkClientServerStream::Invoke
            << VTKOBJECT(smPolyEdgeOp) << "SetContourInput"
            << smPolyEdgeOp->GetSession()->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT).GetArgument(0,0)
            << vtkClientServerStream::End;
    smPolyEdgeOp->GetSession()->ExecuteStream(smPolyEdgeOp->GetLocation(), stream);
    return smPolyEdgeOp;
  }

  QPointer<pqGenerateContoursDialog> ContoursDialog;
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QVBoxLayout> EditorLayout;

  smtk::weak_ptr<smtk::model::Operator> CurrentOp;
  QPointer<pqPipelineSource> CurrentImage;
};

//----------------------------------------------------------------------------
qtBaseView *
qtExtractContoursView::createViewWidget(const ViewInfo &info)
{
  qtExtractContoursView *view = new qtExtractContoursView(info);
  view->buildUI();
  return view;
}

//----------------------------------------------------------------------------
qtExtractContoursView::
qtExtractContoursView(const ViewInfo &info) :
  qtBaseView(info)
{
  this->Internals = new qtExtractContoursViewInternals;
}

//----------------------------------------------------------------------------
qtExtractContoursView::~qtExtractContoursView()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtExtractContoursView::createWidget( )
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
    {
    return;
    }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());
  if(this->Widget)
    {
    if(parentlayout)
      {
      parentlayout->removeWidget(this->Widget);
      }
    delete this->Widget;
    }
  this->Widget = new QFrame(this->parentWidget());
  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  QPushButton* contourButton = new QPushButton(this->parentWidget());
  contourButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  contourButton->setMinimumHeight(32);
  contourButton->setObjectName("polygonStartContourButton");
  contourButton->setText("Launch Contour Preview");
  this->updateAttributeData();

  QObject::connect(contourButton, SIGNAL(clicked()),
    this, SLOT(startContourOperation()));
  layout->addWidget(contourButton);
}

//----------------------------------------------------------------------------
void qtExtractContoursView::updateAttributeData()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
    {
    return;
    }

  if(this->Internals->CurrentAtt)
    {
    delete this->Internals->CurrentAtt;
    }

  int i = view->details().findChild("AttributeTypes");
  if(i < 0)
    {
    return;
    }
  smtk::common::View::Component& comp = view->details().child(i);
  // for now, we only handle "edit edge" operator; later we could use a list
  // to show all operators (attributes), and a panel underneath to edit current
  // selected operator.
  std::string defName;
  for(std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
    {
    smtk::common::View::Component &attComp = comp.child(ci);
    if (attComp.name() != "Att")
      {
      continue;
      }
    std::string optype;
    if(attComp.attribute("Type", optype) &&
      (optype == "extract contours"))
      {
      defName = optype;
      break;
      }
    }
  if(defName.empty())
    {
    return;
    }

  smtk::model::OperatorPtr edgeOp = this->uiManager()->activeModelView()->
                       operatorsWidget()->existingOperator(defName);
  this->Internals->CurrentOp = edgeOp;
  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = edgeOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);

}
//----------------------------------------------------------------------------
void qtExtractContoursView::startContourOperation()
{
  this->operationSelected(this->Internals->CurrentOp.lock());
}

//----------------------------------------------------------------------------
void qtExtractContoursView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if(!op || !op->specification())
    {
    return;
    }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

//----------------------------------------------------------------------------
void qtExtractContoursView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if( !op || !this->Widget || !this->Internals->CurrentAtt )
    return;
  if(this->Internals->ContoursDialog)
    {
    this->Internals->ContoursDialog->close();
    }
}

//----------------------------------------------------------------------------
void qtExtractContoursView::acceptContours(pqPipelineSource* contourSource)
{
  if(!contourSource || !this->Internals->CurrentAtt ||
     !this->Widget || !this->Internals->CurrentOp.lock())
    {
    return;
    }

  smtk::attribute::AttributePtr spec = this->Internals->CurrentOp.lock()->specification();
  if(spec->type() != "extract contours")
    return;
  smtk::attribute::IntItem::Ptr opProxyIdItem = spec->findInt("HelperGlobalID");
  if(!opProxyIdItem)
    return;
  vtkSMProxy* smPolyEdgeOp = this->Internals->createVTKContourOperator(contourSource->getProxy());
  if(!smPolyEdgeOp)
    return;
  if(this->Internals->CurrentImage)
    {
    double bounds[6];
    this->Internals->CurrentImage->getOutputPort(0)->getDataInformation()->GetBounds(bounds);
    vtkSMPropertyHelper(smPolyEdgeOp, "ImageBounds").Set(bounds, 6);
    smPolyEdgeOp->UpdateVTKObjects();
    }
  // Now set the GlobalId of smPolyEdgeOp proxy to the edge op, and later
  // on the GlobalId will be used to find the proxy
    // for Create and Edit operation, we need arc source
  opProxyIdItem->setValue(smPolyEdgeOp->GetGlobalID());
  this->requestOperation(this->Internals->CurrentOp.lock());
}

pqPipelineSource* internal_createImageSource(const std::string& imageurl)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqServer* server = core->getActiveServer();
  builder->blockSignals(true);

  QStringList files;
  files << imageurl.c_str();

  pqPipelineSource* source;
  QFileInfo finfo(imageurl.c_str());
  if (finfo.completeSuffix().toLower() == "tif" ||
      finfo.completeSuffix().toLower() == "tiff" ||
      finfo.completeSuffix().toLower() == "dem")
    {
    source =  builder->createReader("sources", "GDALRasterReader", files, server);
    source =  builder->createFilter("props", "ImageSpacingFlip", source);
    }
  else
    {
    source =  builder->createReader("sources", "XMLImageDataReader", files, server);
    }
  builder->blockSignals(false);

  return source;
}

//----------------------------------------------------------------------------
void qtExtractContoursView::operationSelected(const smtk::model::OperatorPtr& op)
{
  if(!this->Internals->CurrentAtt || !this->Widget || op->name() != "extract contours")
    return;

  if(this->Internals->ContoursDialog)
    {
    this->Internals->ContoursDialog->close();
    delete this->Internals->ContoursDialog;
    }

  smtk::attribute::AttributePtr spec = op->specification();
  smtk::attribute::ModelEntityItem::Ptr modelItem = spec->associations();
  smtk::model::AuxiliaryGeometry aux(modelItem->value(0));
  if (!aux.isValid())
    {
    qCritical() << "No AuxiliaryGeometry is associated with the operator.\n";
    return;
    }
  std::string imagefile = aux.url();
  if (imagefile.empty())
    {
    qCritical() << "No image file is associated with AuxiliaryGeometry, use \"add auxiliary geometry\" operator first.\n";
    return;
    }
  if(pqPipelineSource* source = internal_createImageSource(imagefile))
    {
    this->Internals->CurrentImage = source;
    QFileInfo fInfo(imagefile.c_str());
    QString lastExt = fInfo.suffix().toLower();
    bool scalarColoring = (lastExt == "vti" || lastExt== "dem");
    if(aux.hasIntegerProperty("UseScalarColoring"))
      {
      const smtk::model::IntegerList& uprop(aux.integerProperty("UseScalarColoring"));
      scalarColoring = !uprop.empty() ? (uprop[0] != 0) : scalarColoring;
      }
    this->Internals->ContoursDialog = new pqGenerateContoursDialog(
      source, scalarColoring, this->parentWidget());
    QObject::connect(this->Internals->ContoursDialog,
          SIGNAL(contoursAccepted(pqPipelineSource*)),
          this, SLOT(acceptContours(pqPipelineSource*)));
    this->Internals->ContoursDialog->setModal(true);
    this->Internals->ContoursDialog->exec();
    }
}

//----------------------------------------------------------------------------
void qtExtractContoursView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

//----------------------------------------------------------------------------
void qtExtractContoursView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}
