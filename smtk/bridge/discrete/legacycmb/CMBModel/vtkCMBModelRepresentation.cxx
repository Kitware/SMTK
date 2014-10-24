//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelRepresentation.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkDataObject.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkMPIMoveData.h"
#include "vtkObjectFactory.h"
#include "vtkQuadricClustering.h"
#include "vtkRenderer.h"
#include "vtkSelection.h"
#include "vtkSelectionConverter.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkTrivialProducer.h"
#include "vtkImageData.h"
#include "vtkImageTextureCrop.h"
#include "vtkWeakPointer.h"
#include "vtkPVArrowSource.h"
#include "vtkPVRenderView.h"
#include "vtkPVTrivialProducer.h"
#include "vtkPVCacheKeeper.h"

// Model headers
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelEntity.h"
#include "vtkCMBModelActor.h"
#include "vtkCMBModelMapper.h"
#include <set>

class vtkCMBModelRepresentation::vtkInternal
{
public:

  #define SetEntityPropertyMacro(name,type) \
    void Set##name (type _arg) \
    { \
    if(this->SelectedEntityIds.size()==0)\
      { \
      return; \
      } \
    for(std::set<vtkIdType>::iterator sit= \
      this->SelectedEntityIds.begin(); \
      sit!=this->SelectedEntityIds.end();sit++) \
      { \
      if(vtkProperty* entProp = this->ModelWrapper->GetEntityPropertyByEntityId(*sit)) \
        { \
        entProp->Set##name(_arg); \
        } \
      } \
    }

  #define SetEntityPropertyMacro3(name,type) \
    void Set##name (type _arg0, type _arg1, type _arg2) \
    { \
    if(this->SelectedEntityIds.size()==0)\
      { \
      return; \
      } \
    for(std::set<vtkIdType>::iterator sit= \
    this->SelectedEntityIds.begin(); \
    sit!=this->SelectedEntityIds.end();sit++) \
      { \
      if(vtkProperty* entProp = this->ModelWrapper->GetEntityPropertyByEntityId(*sit)) \
        { \
        entProp->Set##name(_arg0, _arg1, _arg2); \
        } \
      } \
    }

  SetEntityPropertyMacro(Representation, int);
  SetEntityPropertyMacro(EdgeVisibility, int);
  SetEntityPropertyMacro3(AmbientColor, double);
  //virtual void SetColor(double r, double g, double b);
  SetEntityPropertyMacro3(DiffuseColor, double);
  SetEntityPropertyMacro3(EdgeColor, double);
  SetEntityPropertyMacro(Interpolation, int);
  SetEntityPropertyMacro(LineWidth, double);
  SetEntityPropertyMacro(Opacity, double);
  SetEntityPropertyMacro(PointSize, double);
  SetEntityPropertyMacro3(SpecularColor, double);
  SetEntityPropertyMacro(SpecularPower, double);

  typedef std::set<vtkIdType> IDSetType;
  IDSetType SelectedEntityIds;
  vtkWeakPointer<vtkDiscreteModelWrapper> ModelWrapper;

};

vtkStandardNewMacro(vtkCMBModelRepresentation);
//----------------------------------------------------------------------------
vtkCMBModelRepresentation::vtkCMBModelRepresentation()
{
  this->Internal = new vtkInternal();

  this->SetNumberOfInputPorts(2); // 2nd input port for image for large texture

  this->ModelMapper = vtkCMBModelMapper::New();
  this->LODModelMapper = vtkCMBModelMapper::New();
  this->ModelActor = vtkCMBModelActor::New();

  this->ModelActor->SetMapper(this->ModelMapper);
  this->ModelActor->SetLODMapper(this->LODModelMapper);
  this->ModelActor->SetProperty(this->Property);

  vtkInformation* keys = vtkInformation::New();
  this->ModelActor->SetPropertyKeys(keys);
  keys->Delete();

  this->ModelVisibility = true;
  this->SetModelVisibility(false);
  this->LastSelectedEntityIds = vtkIdTypeArray::New();
  this->LastSelectedEntityIds->SetNumberOfComponents(1);
  this->LastSelectedEntityIds->SetNumberOfTuples(0);

  this->LODTextureCrop = vtkImageTextureCrop::New();
  this->LODTextureCrop->SetInputConnection( this->Decimator->GetOutputPort() );
  this->TextureCrop = vtkImageTextureCrop::New();
  this->TextureCrop->SetInputConnection( this->CacheKeeper->GetOutputPort() );
  this->LargeTexture = 0;

  this->ModelMapper->SetInterpolateScalarsBeforeMapping(0);
  this->LODModelMapper->SetInterpolateScalarsBeforeMapping(0);
}

//----------------------------------------------------------------------------
vtkCMBModelRepresentation::~vtkCMBModelRepresentation()
{
  this->LODTextureCrop->Delete();
  this->TextureCrop->Delete();
  if (this->LargeTexture)
    {
    this->LargeTexture->Delete();
    }

  this->GeometryFilter->RemoveAllInputs();
  this->LastSelectedEntityIds->Delete();
  this->ModelMapper->Delete();
  this->LODModelMapper->Delete();
  this->ModelActor->Delete();
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::RemoveLargeTextureInput()
{
  this->SetInputConnection(1, 0);
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkCMBModelRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->AddActor(this->ModelActor);
    this->TextureCrop->SetRenderer( rview->GetRenderer() );
    }
  return this->Superclass::AddToView(view);
}

//----------------------------------------------------------------------------
bool vtkCMBModelRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->RemoveActor(this->ModelActor);
    }

  return this->Superclass::RemoveFromView(view);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
  this->ModelActor->SetVisibility(val?  1 : 0);
  this->Actor->SetVisibility((val && this->ModelVisibility)? 1 : 0);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetModelVisibility(bool val)
{
  this->ModelVisibility = val;
  this->Actor->SetVisibility((val && this->ModelVisibility)? 1 : 0);
}
//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetCMBModel(vtkDiscreteModelWrapper* model)
{
  this->ModelMapper->SetCMBModel(model);
  this->LODModelMapper->SetCMBModel(model);
  this->Internal->ModelWrapper = model;
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkCMBModelRepresentation::FillInputPortInformation(int port,
  vtkInformation *info)
{
  if (port == 0)
    {
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDiscreteModelWrapper");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    return this->Superclass::FillInputPortInformation(port, info);
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::UpdateColoringParameters()
{
  this->Superclass::UpdateColoringParameters();

  if (this->ModelActor->GetVisibility())
    {
    // Copy paramters from this->Mapper
    this->ModelMapper->SetLookupTable(this->Mapper->GetLookupTable());
    this->ModelMapper->SetColorMode(this->Mapper->GetColorMode());
//    this->ModelMapper->SetInterpolateScalarsBeforeMapping(
//      this->Mapper->GetInterpolateScalarsBeforeMapping());
    this->ModelMapper->SetStatic(this->Mapper->GetStatic());
    this->ModelMapper->SetScalarVisibility(this->Mapper->GetScalarVisibility());
    this->ModelMapper->SelectColorArray(this->Mapper->GetArrayName());
    this->ModelMapper->SetScalarMode(this->Mapper->GetScalarMode());

    // Copy paramters from this->LODMapper
    this->LODModelMapper->SetLookupTable(this->LODMapper->GetLookupTable());
    this->LODModelMapper->SetColorMode(this->LODMapper->GetColorMode());
//    this->LODModelMapper->SetInterpolateScalarsBeforeMapping(
//      this->LODMapper->GetInterpolateScalarsBeforeMapping());
    this->LODModelMapper->SetStatic(this->LODMapper->GetStatic());
    this->LODModelMapper->SetScalarVisibility(this->LODMapper->GetScalarVisibility());
    this->LODModelMapper->SelectColorArray(this->LODMapper->GetArrayName());
    this->LODModelMapper->SetScalarMode(this->LODMapper->GetScalarMode());

    // Copy parameters from this->Actor
    this->ModelActor->SetOrientation(this->Actor->GetOrientation());
    this->ModelActor->SetOrigin(this->Actor->GetOrigin());
    this->ModelActor->SetPickable(this->Actor->GetPickable());
    this->ModelActor->SetPosition(this->Actor->GetPosition());
    this->ModelActor->SetScale(this->Actor->GetScale());
    //this->ModelActor->SetTexture(this->Actor->GetTexture());
    }
  //this->ModelMapper->UpdateColorProperties();
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkSelection* vtkCMBModelRepresentation::ConvertSelection(
  vtkView* _view, vtkSelection* selection)
{
  this->RemoveAllSelectedEntityIdsInternal();

  vtkSelection* output = this->Superclass::ConvertSelection(_view, selection);
  vtkDiscreteModelWrapper *inputModel = vtkDiscreteModelWrapper::SafeDownCast(
    this->GetInputDataObject(0, 0));
  if(inputModel && output)
    {
    this->LastSelectedEntityIds->Initialize();
    this->LastSelectedEntityIds->SetNumberOfComponents(1);
    this->LastSelectedEntityIds->SetNumberOfTuples(0);
    unsigned int numSelNodes = selection->GetNumberOfNodes();
    // locate any selection nodes which belong to this representation.
    for (unsigned int cc=0; cc < numSelNodes; cc++)
      {
      vtkSelectionNode* node = selection->GetNode(cc);
      vtkProp* prop = NULL;
      if (node->GetProperties()->Has(vtkSelectionNode::PROP()))
        {
        prop = vtkProp::SafeDownCast(node->GetProperties()->Get(
          vtkSelectionNode::PROP()));
        }

      if (prop == this->GetRenderedProp())
        {
        vtkInformation* nodeProperties = node->GetProperties();
        if (nodeProperties->Has(vtkSelectionNode::COMPOSITE_INDEX()))
          {
          unsigned int cur_index = static_cast<unsigned int>(
            nodeProperties->Get(vtkSelectionNode::COMPOSITE_INDEX()));
          // need to skip the root index for selection in composite data
          cur_index--;
          vtkIdType entId;
          if (inputModel->GetEntityIdByChildIndex(cur_index, entId))
            {
            this->LastSelectedEntityIds->InsertNextValue(entId);
            }
          }
        }
      }
    }
  return output;
}

//----------------------------------------------------------------------------
int vtkCMBModelRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (request_type == vtkPVView::REQUEST_RENDER())
    {
    bool lod = this->SuppressLOD? false :
      (inInfo->Has(vtkPVRenderView::USE_LOD()) == 1);

    this->ModelMapper->SetInputConnection(0,
      vtkPVRenderView::GetPieceProducer(inInfo, this));
    this->LODModelMapper->SetInputConnection(0,
        vtkPVRenderView::GetPieceProducerLOD(inInfo, this));

    this->ModelActor->SetEnableLOD(lod? 1 : 0);
    this->ModelActor->GetMapper()->Update();
/*
    // update data bounds
    double bounds[6];
    this->ModelMapper->GetBounds(bounds);
    vtkBoundingBox mbox(bounds);
    vtkBoundingBox bbox(this->DataBounds);
    if(bbox != mbox)
      {
      mbox.GetBounds(this->DataBounds);
      }
*/
    }
//  return this->Superclass::ProcessViewRequest( request_type, inInfo, outInfo);
  if(!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
    {
    return 0;
    }

  if (this->LargeTexture)
    {
    vtkImageData *textureInput = this->LargeTexture->GetInput();
    if(request_type == vtkPVView::REQUEST_UPDATE())
      {
      vtkPVRenderView::SetPiece(inInfo, this,
        this->TextureCrop->GetOutputDataObject(0));
      this->TextureCrop->Modified();
      this->TextureCrop->Update();
      textureInput->ShallowCopy( this->TextureCrop->GetOutputDataObject(1) );
      textureInput->Modified();
      this->LargeTexture->Modified();
      }
    else if(request_type == vtkPVView::REQUEST_UPDATE_LOD())
      {
      vtkPVRenderView::SetPieceLOD(inInfo, this,
        this->LODTextureCrop->GetOutputDataObject(0));
      this->LODTextureCrop->Update(); // should only do somethign the first time
      textureInput->ShallowCopy( this->LODTextureCrop->GetOutputDataObject(1) );
      textureInput->Modified();
      this->LargeTexture->Modified();
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBModelRepresentation::RequestData(vtkInformation* request,
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{

  // Pass caching information to the cache keeper.
  this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if (inputVector[0]->GetNumberOfInformationObjects()==1)
    {
    this->GeometryFilter->SetInputConnection(
      this->GetInternalOutputPort());
    this->CacheKeeper->Update();

    if (inputVector[1]->GetNumberOfInformationObjects()==1)
      {
      this->TextureCrop->SetImageData( vtkDataSet::SafeDownCast( this->GetInput(1) ));
      this->LODTextureCrop->SetImageData( vtkDataSet::SafeDownCast( this->GetInput(1) ));
      if (!this->LargeTexture)
        {
        this->LargeTexture = vtkTexture::New();
        vtkImageData *textureInput = vtkImageData::New();
        this->LargeTexture->SetInputData( textureInput );
        textureInput->FastDelete();
        }
      this->ModelActor->SetTexture( this->LargeTexture );
//      this->ModelActor->GetProperty()->SetTexture(0, this->LargeTexture );
//      this->Actor->SetTexture( this->LargeTexture );

      }
    else
      {
      if (this->LargeTexture)
        {
        this->LargeTexture->Delete();
        this->LargeTexture = 0;
        this->ModelActor->SetTexture( 0 );
//        this->ModelActor->GetProperty()->RemoveAllTextures();
//        this->Actor->SetTexture( 0 );
        }
      }
    }

  // Determine data bounds.
  vtkCompositeDataSet* cd = vtkCompositeDataSet::SafeDownCast(
    this->CacheKeeper->GetOutputDataObject(0));
  if (cd)
    {
    vtkBoundingBox bbox;
    vtkCompositeDataIterator* iter = cd->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
      {
      vtkDataSet* ds = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (ds)
        {
        bbox.AddBounds(ds->GetBounds());
        }
      }
    iter->Delete();
    if (bbox.IsValid())
      {
      bbox.GetBounds(this->DataBounds);
      }
    }

  return this->vtkPVDataRepresentation::RequestData(request, inputVector, outputVector);
}


//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::AddSelectedEntityId(vtkIdType SelectedEntityId)
{
  this->Internal->SelectedEntityIds.insert(SelectedEntityId);
  this->LastSelectedEntityIds->InsertNextValue(SelectedEntityId);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::RemoveAllSelectedEntityIds()
{
  this->RemoveAllSelectedEntityIdsInternal();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::RemoveAllSelectedEntityIdsInternal()
{
  if(this->Internal->SelectedEntityIds.size()>0)
    {
    this->Internal->SelectedEntityIds.clear();
    }
  this->LastSelectedEntityIds->Initialize();
  this->LastSelectedEntityIds->SetNumberOfComponents(1);
  this->LastSelectedEntityIds->SetNumberOfTuples(0);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetShowEdgePoints(bool bVal)
{
  this->ModelMapper->SetShowEdgePoints(bVal);
}
//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetRepresentation(int rep)
{
  this->Superclass::SetRepresentation(rep);
/*
  if(rep == SURFACE_WITH_EDGES)
    {
    this->Internal->SetRepresentation(SURFACE);
    this->Internal->SetEdgeVisibility(1);
    }
  else
    {
    this->Internal->SetRepresentation(rep);
    this->Internal->SetEdgeVisibility(0);
    }
*/
}
//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetColor(double r, double g, double b)
{
  // The Color Should NOT be passed to the individual entity, because
  // the entity color is determined by color mode (domain set, region, etc)
  this->Superclass::SetColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetLineWidth(double val)
{
  this->Superclass::SetLineWidth(val);
//  this->Internal->SetLineWidth(val);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetOpacity(double val)
{
  this->Superclass::SetOpacity(val);
//  this->Internal->SetOpacity(val);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetPointSize(double val)
{
  this->Superclass::SetPointSize(val);
//  this->Internal->SetPointSize(val);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetAmbientColor(double r, double g, double b)
{
  this->Superclass::SetAmbientColor(r, g, b);
//  this->Internal->SetAmbientColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetDiffuseColor(double r, double g, double b)
{
  this->Superclass::SetDiffuseColor(r, g, b);
//  this->Internal->SetDiffuseColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetEdgeColor(double r, double g, double b)
{
  this->Superclass::SetEdgeColor(r, g, b);
//  this->Internal->SetDiffuseColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetInterpolation(int val)
{
  this->Superclass::SetInterpolation(val);
//  this->Internal->SetInterpolation(val);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetSpecularColor(double r, double g, double b)
{
  this->Superclass::SetSpecularColor(r, g, b);
//  this->Internal->SetSpecularColor(r, g, b);
}

//----------------------------------------------------------------------------
void vtkCMBModelRepresentation::SetSpecularPower(double val)
{
  this->Superclass::SetSpecularPower(val);
//  this->Internal->SetSpecularPower(val);
}

//**************************************************************************
// Forwarded to vtkCMBModelMapper
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
double* vtkCMBModelRepresentation::GetBounds()
{
  double bounds[6];
  this->ModelMapper->GetBounds(bounds);
  vtkBoundingBox mbox(bounds);
  vtkBoundingBox bbox(this->DataBounds);
  if(bbox != mbox)
    {
    mbox.GetBounds(this->DataBounds);
    }
  return this->DataBounds;
}
