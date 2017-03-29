//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelRepresentation
// .SECTION Description
// vtkCMBModelRepresentation is a representation that uses the
// vtkCMBModelActor and vtkCMBModelMapper
// for rendering models.

#ifndef __vtkCMBModelRepresentation_h
#define __vtkCMBModelRepresentation_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkGeometryRepresentationWithFaces.h"

class vtkCMBModelMapper;
class vtkIdTypeArray;
class vtkDiscreteModelWrapper;
class vtkImageTextureCrop;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelRepresentation : public vtkGeometryRepresentationWithFaces
{
public:
  static vtkCMBModelRepresentation* New();
  vtkTypeMacro(vtkCMBModelRepresentation, vtkGeometryRepresentationWithFaces);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // vtkAlgorithm::ProcessRequest() equivalent for rendering passes. This is
  // typically called by the vtkView to request meta-data from the
  // representations or ask them to perform certain tasks e.g.
  // PrepareForRendering.
  virtual int ProcessViewRequest(vtkInformationRequestKey* request_type,
    vtkInformation* inInfo, vtkInformation* outInfo);

  // Description:
  // Remove the texture input to the model.
  void RemoveLargeTextureInput();

  // Description:
  // Toggle the visibility of the original mesh.
  // If this->GetVisibility() is false, then this has no effect.
  void SetModelVisibility(bool visible);

  // Description:
  // Get/Set the visibility for this representation. When the visibility of
  // representation of false, all view passes are ignored.
  virtual void SetVisibility(bool);

  //**************************************************************************
  // Forwarded to vtkCMBModelMapper
  double* GetBounds();

  // Description:
  // Convert the selection to a type appropriate for sharing with other
  // representations through vtkAnnotationLink, possibly using the view.
  // For the superclass, we just return the same selection.
  // This class will convert the selection to proper cmbmodel selection
  // based on the selection mode (Faces/Edges, domainset, bcs, region, etc)
  // If the selection cannot be applied to this representation, return NULL.
  virtual vtkSelection* ConvertSelection(vtkView* view, vtkSelection* selection);

  // Description:
  // Get the list of selected model entities.
  vtkGetObjectMacro(LastSelectedEntityIds, vtkIdTypeArray);

  // Description:
  // Set the model wrapper of this representation. It is essentially the same as
  // the representation's input. We set it here so that we can modify some model's
  // data property for rendering.
  void SetCMBModel(vtkDiscreteModelWrapper* model);

  // Description:
  // Add the SelectedEntityId.
  void AddSelectedEntityId(vtkIdType SelectedEntityId);
  void RemoveAllSelectedEntityIds();

  //***************************************************************************
  // Forwarded to selected model entities.
  virtual void SetShowEdgePoints(bool);
  virtual void SetRepresentation(int rep);
  virtual void SetAmbientColor(double r, double g, double b);
  virtual void SetColor(double r, double g, double b);
  virtual void SetDiffuseColor(double r, double g, double b);
  virtual void SetEdgeColor(double r, double g, double b);
  virtual void SetInterpolation(int val);
  virtual void SetLineWidth(double val);
  virtual void SetOpacity(double val);
  virtual void SetPointSize(double val);
  virtual void SetSpecularColor(double r, double g, double b);
  virtual void SetSpecularPower(double val);

protected:
  vtkCMBModelRepresentation();
  ~vtkCMBModelRepresentation();

  // Description:
  // Fill input port information.
  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // Description:
  // Overriding to connect in the vtkImageTextureCrop filter
  virtual int RequestData(vtkInformation*,
    vtkInformationVector**, vtkInformationVector*);

  // Description:
  // Adds the representation to the view.  This is called from
  // vtkView::AddRepresentation().  Subclasses should override this method.
  // Returns true if the addition succeeds.
  virtual bool AddToView(vtkView* view);

  // Description:
  // Removes the representation to the view.  This is called from
  // vtkView::RemoveRepresentation().  Subclasses should override this method.
  // Returns true if the removal succeeds.
  virtual bool RemoveFromView(vtkView* view);

  // Description:
  // Used in ConvertSelection to locate the prop used for actual rendering.
  virtual vtkPVLODActor* GetRenderedProp()
    { return this->ModelActor; }

  // Description:
  // Passes on parameters to vtkProperty and vtkMapper
  virtual void UpdateColoringParameters();

  vtkCMBModelMapper* ModelMapper;
  vtkCMBModelMapper* LODModelMapper;
  vtkPVLODActor* ModelActor;

  bool ModelVisibility;
  vtkIdTypeArray* LastSelectedEntityIds;

  vtkImageTextureCrop *LODTextureCrop;
  vtkImageTextureCrop *TextureCrop;
  vtkTexture *LargeTexture;

private:
  vtkCMBModelRepresentation(const vtkCMBModelRepresentation&); // Not implemented
  void operator=(const vtkCMBModelRepresentation&); // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  void RemoveAllSelectedEntityIdsInternal();

};

#endif
