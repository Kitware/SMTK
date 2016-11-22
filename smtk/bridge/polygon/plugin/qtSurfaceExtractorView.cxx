//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtSurfaceExtractorView.h"
#include "smtk/bridge/polygon/qt/imageFeatureExtractorWidget.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/common/View.h"
#include "smtk/model/Operator.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"

#include "vtkClientServerStream.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMSession.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <QPushButton>
#include <QDebug>
#include <QFileInfo>

using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtSurfaceExtractorViewInternals
{
public:
  qtSurfaceExtractorViewInternals()
    {
    }
  ~qtSurfaceExtractorViewInternals()
    {
    }
  QPointer<qtAttribute> CurrentAtt;
  smtk::weak_ptr<smtk::model::Operator> CurrentOp;
  imageFeatureExtractorWidget * ExtractorWidget;
};

//----------------------------------------------------------------------------
qtBaseView *
qtSurfaceExtractorView::createViewWidget(const ViewInfo &info)
{
  qtSurfaceExtractorView *view = new qtSurfaceExtractorView(info);
  view->buildUI();
  return view;
}

//----------------------------------------------------------------------------
qtSurfaceExtractorView::
qtSurfaceExtractorView(const ViewInfo &info) :
  qtBaseView(info)
{
  this->Internals = new qtSurfaceExtractorViewInternals;
  this->Internals->ExtractorWidget = NULL;// new imageFeatureExtractorWidget();
}

//----------------------------------------------------------------------------
qtSurfaceExtractorView::~qtSurfaceExtractorView()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtSurfaceExtractorView::createWidget( )
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
  contourButton->setObjectName("surfaceExtractorView");
  contourButton->setText("Launch Extract Contour");
  this->updateAttributeData();

  QObject::connect(contourButton, SIGNAL(clicked()),
    this, SLOT(startContourOperation()));
  layout->addWidget(contourButton);
}

inline qtAttribute*
internal_createAttUI(smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
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

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::updateAttributeData()
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
      (optype == "extract surface contours"))
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
  this->Internals->CurrentAtt = internal_createAttUI(att, this->Widget, this);
}

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::startContourOperation()
{
  this->operationSelected(this->Internals->CurrentOp.lock());
}

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if(!op || !op->specification())
    {
    return;
    }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  /*if( !op || !this->Widget || !this->Internals->CurrentAtt )
    return;
  if(this->Internals->ContoursDialog)
    {
    this->Internals->ContoursDialog->close();
    }*/
}

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::acceptContours(vtkSmartPointer<vtkPolyData> contourSource)
{
  if(!contourSource || !this->Internals->CurrentAtt ||
     !this->Widget  || !this->Internals->CurrentOp.lock())
    {
    return;
    }

  smtk::attribute::AttributePtr spec = this->Internals->CurrentOp.lock()->specification();
  if(spec->type() != "extract surface contours")
    return;
  smtk::attribute::IntItem::Ptr opProxyIdItem = spec->findInt("HelperGlobalID");
  if(!opProxyIdItem)
    return;
  /*vtkSMProxy* smPolyEdgeOp = internal_createVTKContourOperator(contourSource->getProxy());
  if(!smPolyEdgeOp)
    return;
  // Now set the GlobalId of smPolyEdgeOp proxy to the edge op, and later
  // on the GlobalId will be used to find the proxy
    // for Create and Edit operation, we need arc source
  opProxyIdItem->setValue(smPolyEdgeOp->GetGlobalID());
  this->requestOperation(this->Internals->CurrentOp.lock());*/
}

/*
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
 */

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::operationSelected(const smtk::model::OperatorPtr& op)
{
  if(!this->Internals->CurrentAtt || !this->Widget || op->name() != "extract surface contours")
    return;

  if(this->Internals->ExtractorWidget)
  {
    delete this->Internals->ExtractorWidget;
  }
  this->Internals->ExtractorWidget = new imageFeatureExtractorWidget();

  smtk::attribute::AttributePtr spec = op->specification();
  smtk::attribute::ModelEntityItem::Ptr modelItem = spec->associations();
  smtk::model::AuxiliaryGeometry aux(modelItem->value(0));
  if (!aux.isValid())
    {
    qCritical() << "No AuxiliaryGeometry is associated with the operator.\n";
    return;
    }
  std::string imagefile = aux.url();

  this->Internals->ExtractorWidget->setImage(imagefile);

  if(this->Internals->ExtractorWidget->exec())
  {
    vtkSmartPointer<vtkPolyData> pd = this->Internals->ExtractorWidget->getPolydata();
    smtk::model::OperatorResult edgeResult;
    //smtk::attribute::AttributePtr spec = this->m_smtkOp.lock()->specification();
    smtk::attribute::IntItem::Ptr offsetsItem =
            spec->findAs<smtk::attribute::IntItem>( "offsets", smtk::attribute::ALL_CHILDREN);
    smtk::attribute::DoubleItem::Ptr pointsItem =
            spec->findAs<smtk::attribute::DoubleItem>("points", smtk::attribute::ALL_CHILDREN);
    smtk::attribute::IntItem::Ptr numCoords =
            spec->findAs<smtk::attribute::IntItem>( "coordinates", smtk::attribute::ALL_CHILDREN);
    numCoords->setValue(3); // number of elements in coordinates

    double p[3];
    int numPoints = 0;
    vtkIdType *pts,npts;
    vtkCellArray* lines = pd->GetLines();
    lines->InitTraversal();
    std::vector<int> offsets;
    while(lines->GetNextCell(npts,pts))
    {
      // for each line we are creating an edge, so set the "offsets" into the
      // points list
      offsets.push_back(numPoints);
      // add points for current line cell
      pointsItem->setNumberOfValues((numPoints + npts) * 3);
      for (vtkIdType j=0; j < npts; ++j)
      {
        pd->GetPoint(pts[j],p);
        int idx = 3 * (numPoints+j);
        for (int i = 0; i < 3; ++i)
        {
          pointsItem->setValue( idx + i, p[i]);
        }
      }
      numPoints += npts;
    }
    
    offsetsItem->setValues(offsets.begin(), offsets.end());
    //edgeResult = this->m_smtkOp.lock()->operate();
  }

}

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

//----------------------------------------------------------------------------
void qtSurfaceExtractorView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}
