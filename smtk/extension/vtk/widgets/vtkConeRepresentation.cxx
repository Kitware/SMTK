//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/widgets/vtkConeRepresentation.h"
#include "smtk/extension/vtk/source/vtkConeFrustum.h"
#include "smtk/extension/vtk/source/vtkImplicitConeFrustum.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkBox.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkConeSource.h"
#include "vtkDoubleArray.h"
#include "vtkGlyph3DMapper.h"
#include "vtkImageData.h"
#include "vtkInteractorObserver.h"
#include "vtkLineSource.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOutlineFilter.h"
#include "vtkPickingManager.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTubeFilter.h"
#include "vtkVersionMacros.h"
#include "vtkWindow.h"

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 5, 0)
#include "vtkVectorOperators.h"
#endif

#include <algorithm>
#include <cfloat> //for FLT_EPSILON

vtkStandardNewMacro(vtkConeRepresentation);

vtkConeRepresentation::vtkConeRepresentation()
{
  this->HandleSize = 7.5;
  this->Cone->SetResolution(128);

  // Set up the initial properties
  this->CreateDefaultProperties();

  for (int ii = 0; ii < NumberOfElements; ++ii)
  {
    this->Elements[ii].Actor->SetMapper(this->Elements[ii].Mapper);
    if (ii <= TopFace)
    {
      this->Elements[ii].Actor->SetProperty(this->ConeProperty);
    }
    else if (ii <= TopCurve)
    {
      this->Elements[ii].Actor->SetProperty(this->EdgeProperty);
    }
    else
    {
      this->Elements[ii].Actor->SetProperty(this->HandleProperty);
    }
  }

  // Set up the pipelines for the visual elements
  this->AxisTuber->SetInputConnection(this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::Axis));
  this->AxisTuber->SetNumberOfSides(12);
  this->Elements[ConeAxis].Mapper->SetInputConnection(this->AxisTuber->GetOutputPort());
  this->Elements[ConeAxis].Actor->SetMapper(this->Elements[ConeAxis].Mapper);

  this->Elements[ConeFace].Mapper->SetInputConnection(
    this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::SideFace));

  this->Elements[BottomFace].Mapper->SetInputConnection(
    this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::BottomFace));

  this->Elements[TopFace].Mapper->SetInputConnection(
    this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::TopFace));

  this->Elements[BottomCurve].Mapper->SetInputConnection(
    this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::BottomEdge));

  this->Elements[TopCurve].Mapper->SetInputConnection(
    this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::TopEdge));

  // Create the endpoint geometry source
  this->Sphere->SetThetaResolution(16);
  this->Sphere->SetPhiResolution(8);

  this->BottomHandleMapper->SetSourceConnection(this->Sphere->GetOutputPort());
  this->BottomHandleMapper->SetInputConnection(
    this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::BottomVertex));
  this->Elements[BottomHandle].Actor->SetMapper(this->BottomHandleMapper);

  this->TopHandleMapper->SetSourceConnection(this->Sphere->GetOutputPort());
  this->TopHandleMapper->SetInputConnection(
    this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::TopVertex));
  this->Elements[TopHandle].Actor->SetMapper(this->TopHandleMapper);

  // Define the point coordinates
  double bounds[6];
  bounds[0] = -0.5;
  bounds[1] = 0.5;
  bounds[2] = -0.5;
  bounds[3] = 0.5;
  bounds[4] = -0.5;
  bounds[5] = 0.5;

  // Initial creation of the widget, serves to initialize it
  this->PlaceWidget(bounds);

  //Manage the picking stuff
  this->Picker->SetTolerance(0.005);
  for (int ii = ConeAxis; ii < NumberOfElements; ++ii)
  {
    this->Picker->AddPickList(this->Elements[ii].Actor);
  }
  this->Picker->PickFromListOn();

  this->CylPicker->SetTolerance(0.005);
  for (int ii = ConeFace; ii <= TopFace; ++ii)
  {
    this->CylPicker->AddPickList(this->Elements[ii].Actor);
  }
  this->CylPicker->PickFromListOn();

  this->RepresentationState = vtkConeRepresentation::Outside;
}

vtkConeRepresentation::~vtkConeRepresentation() = default;

bool vtkConeRepresentation::SetEndpoint(bool isBottom, double x, double y, double z)
{
  return this->SetEndpoint(isBottom, vtkVector3d(x, y, z));
}

bool vtkConeRepresentation::SetEndpoint(bool isBottom, const vtkVector3d& pt)
{
  vtkVector3d p0(this->Cone->GetBottomPoint());
  vtkVector3d p1(this->Cone->GetTopPoint());

  vtkVector3d temp(pt); // Because vtkSetVectorMacro is not const correct.
  if (isBottom)
  {
    if (p0 == pt)
    {
      // If pt is already the existing value, do nothing.
      return false;
    }
    else if (p1 == pt)
    {
      // We cannot allow coincident endpoints, but we can
      // swap endpoints (and radii), which allows the
      // ParaView widget to initialize the representation
      // even when the new bottom/top point is coincident
      // with the old top/bottom point.
      this->SwapTopAndBottom();
      return true;
    }
    this->Cone->SetBottomPoint(temp.GetData());
  }
  else
  {
    if (p1 == pt)
    {
      // If pt is already the existing value, do nothing.
      return false;
    }
    else if (p0 == pt)
    {
      // We cannot allow coincident endpoints, but we can
      // swap endpoints (and radii), which allows the
      // ParaView widget to initialize the representation
      // even when the new bottom/top point is coincident
      // with the old top/bottom point.
      this->SwapTopAndBottom();
      return true;
    }
    this->Cone->SetTopPoint(temp.GetData());
  }
  this->Modified();
  return true;
}

vtkVector3d vtkConeRepresentation::GetEndpoint(bool isBottom) const
{
  return vtkVector3d(isBottom ? this->Cone->GetBottomPoint() : this->Cone->GetTopPoint());
}

double* vtkConeRepresentation::GetBottomPoint()
{
  return this->Cone->GetBottomPoint();
}

double* vtkConeRepresentation::GetTopPoint()
{
  return this->Cone->GetTopPoint();
}

bool vtkConeRepresentation::SetRadius(bool isBottom, double r)
{
  double prev = isBottom ? this->Cone->GetBottomRadius() : this->Cone->GetTopRadius();
  if (prev == r)
  {
    return false;
  }
  if (this->Cylindrical)
  {
    this->Cone->SetBottomRadius(r);
    this->Cone->SetTopRadius(r);
  }
  else if (isBottom)
  {
    this->Cone->SetBottomRadius(r);
  }
  else
  {
    this->Cone->SetTopRadius(r);
  }
  this->Modified();
  return true;
}

double vtkConeRepresentation::GetRadius(bool isBottom) const
{
  return isBottom ? this->Cone->GetBottomRadius() : this->Cone->GetTopRadius();
}

bool vtkConeRepresentation::SetCylindrical(vtkTypeBool isCylindrical)
{
  if (isCylindrical == this->Cylindrical)
  {
    return false;
  }
  this->Cylindrical = isCylindrical;
  if (this->Cylindrical)
  {
    double radius = 0.5 * (this->GetRadius(false) + this->GetRadius(true));
    this->SetRadius(false, radius);
    this->SetRadius(true, radius);
  }
  this->Modified();
  return true;
}

int vtkConeRepresentation::ComputeInteractionState(int X, int Y, int /*modify*/)
{
  // See if anything has been selected
  vtkAssemblyPath* path = this->GetAssemblyPath(X, Y, 0., this->Picker);

  // The second picker may need to be called. This is done because the cylinder
  // wraps around things that can be picked; thus the cylinder is the selection
  // of last resort.
  if (path == nullptr)
  {
    this->CylPicker->Pick(X, Y, 0., this->Renderer);
    path = this->CylPicker->GetPath();
  }

  if (path == nullptr) // Nothing picked
  {
    this->SetRepresentationState(vtkConeRepresentation::Outside);
    this->InteractionState = vtkConeRepresentation::Outside;
    return this->InteractionState;
  }

  // Something picked, continue
  this->ValidPick = 1;

  // Depending on the interaction state (set by the widget) we modify
  // this state based on what is picked.
  if (this->InteractionState == vtkConeRepresentation::Moving)
  {
    vtkProp* prop = path->GetFirstNode()->GetViewProp();
    if (prop == this->Elements[ConeAxis].Actor)
    {
      this->InteractionState = vtkConeRepresentation::RotatingAxis;
      this->SetRepresentationState(vtkConeRepresentation::RotatingAxis);
    }
    else if (prop == this->Elements[BottomCurve].Actor)
    {
      this->InteractionState = vtkConeRepresentation::AdjustingBottomRadius;
      this->SetRepresentationState(vtkConeRepresentation::AdjustingBottomRadius);
    }
    else if (prop == Elements[TopCurve].Actor)
    {
      this->InteractionState = vtkConeRepresentation::AdjustingTopRadius;
      this->SetRepresentationState(vtkConeRepresentation::AdjustingTopRadius);
    }
    else if (prop == this->Elements[BottomHandle].Actor)
    {
      this->InteractionState = vtkConeRepresentation::MovingBottomHandle;
      this->SetRepresentationState(vtkConeRepresentation::MovingBottomHandle);
    }
    else if (prop == this->Elements[TopHandle].Actor)
    {
      this->InteractionState = vtkConeRepresentation::MovingTopHandle;
      this->SetRepresentationState(vtkConeRepresentation::MovingTopHandle);
    }
    else if (prop == this->Elements[ConeFace].Actor)
    {
      this->InteractionState = vtkConeRepresentation::MovingWhole;
      this->SetRepresentationState(vtkConeRepresentation::MovingWhole);
    }
    else if (prop == this->Elements[BottomFace].Actor)
    {
      this->InteractionState = vtkConeRepresentation::PushingBottomFace;
      this->SetRepresentationState(vtkConeRepresentation::PushingBottomFace);
    }
    else if (prop == this->Elements[TopFace].Actor)
    {
      this->InteractionState = vtkConeRepresentation::PushingTopFace;
      this->SetRepresentationState(vtkConeRepresentation::PushingTopFace);
    }
    else
    {
      this->InteractionState = vtkConeRepresentation::Outside;
      this->SetRepresentationState(vtkConeRepresentation::Outside);
    }
  }
  // We may add a condition to allow the camera to work IO scaling
  else if (this->InteractionState != vtkConeRepresentation::Scaling)
  {
    this->InteractionState = vtkConeRepresentation::Outside;
  }

  return this->InteractionState;
}

void vtkConeRepresentation::SetRepresentationState(int state)
{
  if (this->RepresentationState == state)
  {
    return;
  }

  // Clamp the state
  state =
    (state < vtkConeRepresentation::Outside
       ? vtkConeRepresentation::Outside
       : (state > vtkConeRepresentation::Scaling ? vtkConeRepresentation::Scaling : state));

  this->RepresentationState = state;
  this->Modified();

#if 0
  // For debugging, it is handy to see state changes:
  std::cout
    << "   State "
    << vtkConeRepresentation::InteractionStateToString(this->RepresentationState)
    << "\n";
#endif

  this->HighlightElement(NumberOfElements, 0); // Turn everything off
  if (state == vtkConeRepresentation::RotatingAxis)
  {
    this->HighlightAxis(1);
  }
  else if (state == vtkConeRepresentation::PushingBottomFace)
  {
    this->HighlightCap(true, 1);
  }
  else if (state == vtkConeRepresentation::PushingTopFace)
  {
    this->HighlightCap(false, 1);
  }
  else if (state == vtkConeRepresentation::AdjustingBottomRadius)
  {
    this->HighlightCurve(true, 1);
  }
  else if (state == vtkConeRepresentation::AdjustingTopRadius)
  {
    this->HighlightCurve(false, 1);
  }
  else if (state == vtkConeRepresentation::MovingWhole)
  {
    this->HighlightCone(1);
    this->HighlightCap(true, 1);
    this->HighlightCap(false, 1);
  }
  else if (state == vtkConeRepresentation::Scaling && this->ScaleEnabled)
  {
    this->HighlightAxis(1);
    this->HighlightCone(1);
    // this->HighlightOutline(1);
  }
  else if (state == vtkConeRepresentation::TranslatingCenter)
  {
    this->HighlightAxis(1);
  }
  else
  {
    this->HighlightAxis(0);
    this->HighlightCone(0);
    // this->HighlightOutline(0);
  }
}

void vtkConeRepresentation::StartWidgetInteraction(double e[2])
{
  this->StartEventPosition[0] = e[0];
  this->StartEventPosition[1] = e[1];
  this->StartEventPosition[2] = 0.0;

  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

void vtkConeRepresentation::WidgetInteraction(double e[2])
{
  // Do different things depending on state
  // Calculations everybody does
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z, vpn[3];

  vtkCamera* camera = this->Renderer->GetActiveCamera();
  if (!camera)
  {
    return;
  }

  // Compute the two points defining the motion vector
  double pos[3];
  this->Picker->GetPickPosition(pos);
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer, pos[0], pos[1], pos[2], focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(
    this->Renderer, this->LastEventPosition[0], this->LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if (this->InteractionState == vtkConeRepresentation::AdjustingBottomRadius)
  {
    this->AdjustBottomRadius(e[0], e[1], prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::MovingBottomHandle)
  {
    this->TranslateHandle(true, prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::MovingTopHandle)
  {
    this->TranslateHandle(false, prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::MovingWhole)
  {
    this->TranslateCenter(prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::TranslatingCenter)
  {
    this->TranslateCenterOnAxis(prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::AdjustingTopRadius)
  {
    this->AdjustTopRadius(e[0], e[1], prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::PushingBottomFace)
  {
    this->PushCap(true, prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::PushingTopFace)
  {
    this->PushCap(false, prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkConeRepresentation::Scaling && this->ScaleEnabled)
  {
    this->Scale(prevPickPoint, pickPoint, e[0], e[1]);
  }
  else if (this->InteractionState == vtkConeRepresentation::RotatingAxis)
  {
    camera->GetViewPlaneNormal(vpn);
    this->Rotate(e[0], e[1], prevPickPoint, pickPoint, vpn);
  }

  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

void vtkConeRepresentation::EndWidgetInteraction(double /*newEventPos*/[2])
{
  this->SetRepresentationState(vtkConeRepresentation::Outside);
}

double* vtkConeRepresentation::GetBounds()
{
  this->BuildRepresentation();
  this->BoundingBox->SetBounds(this->Elements[ConeFace].Actor->GetBounds());
  for (int ii = BottomFace; ii < NumberOfElements; ++ii)
  {
    this->BoundingBox->AddBounds(this->Elements[ii].Actor->GetBounds());
  }

  return this->BoundingBox->GetBounds();
}

void vtkConeRepresentation::GetActors(vtkPropCollection* pc)
{
  for (int ii = 0; ii < NumberOfElements; ++ii)
  {
    this->Elements[ii].Actor->GetActors(pc);
  }
}

void vtkConeRepresentation::ReleaseGraphicsResources(vtkWindow* w)
{
  for (int ii = 0; ii < NumberOfElements; ++ii)
  {
    this->Elements[ii].Actor->ReleaseGraphicsResources(w);
  }
}

int vtkConeRepresentation::RenderOpaqueGeometry(vtkViewport* v)
{
  int count = 0;
  this->BuildRepresentation();
  for (int ii = ConeAxis; ii < NumberOfElements; ++ii)
  {
    count += this->Elements[ii].Actor->RenderOpaqueGeometry(v);
  }

  if (this->DrawCone)
  {
    for (int ii = ConeFace; ii < ConeAxis; ++ii)
    {
      count += this->Elements[ii].Actor->RenderOpaqueGeometry(v);
    }
  }

  return count;
}

int vtkConeRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* v)
{
  int count = 0;
  this->BuildRepresentation();
  for (int ii = ConeAxis; ii < NumberOfElements; ++ii)
  {
    count += this->Elements[ii].Actor->RenderTranslucentPolygonalGeometry(v);
  }

  if (this->DrawCone)
  {
    for (int ii = ConeFace; ii < ConeAxis; ++ii)
    {
      count += this->Elements[ii].Actor->RenderTranslucentPolygonalGeometry(v);
    }
  }

  return count;
}

vtkTypeBool vtkConeRepresentation::HasTranslucentPolygonalGeometry()
{
  int result = 0;
  for (int ii = ConeAxis; ii < NumberOfElements; ++ii)
  {
    result |= this->Elements[ii].Actor->HasTranslucentPolygonalGeometry();
  }

  if (this->DrawCone)
  {
    for (int ii = ConeFace; ii < ConeAxis; ++ii)
    {
      result |= this->Elements[ii].Actor->HasTranslucentPolygonalGeometry();
    }
  }

  return result;
}

void vtkConeRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Resolution: " << this->Resolution << "\n";

  if (this->HandleProperty)
  {
    os << indent << "Handle Property: " << this->HandleProperty << "\n";
  }
  else
  {
    os << indent << "Handle Property: (none)\n";
  }
  if (this->SelectedHandleProperty)
  {
    os << indent << "Selected Handle Property: " << this->SelectedHandleProperty << "\n";
  }
  else
  {
    os << indent << "Selected Handle Property: (none)\n";
  }

  if (this->ConeProperty)
  {
    os << indent << "Cone Property: " << this->ConeProperty << "\n";
  }
  else
  {
    os << indent << "Cone Property: (none)\n";
  }
  if (this->SelectedConeProperty)
  {
    os << indent << "Selected Cone Property: " << this->SelectedConeProperty << "\n";
  }
  else
  {
    os << indent << "Selected Cone Property: (none)\n";
  }

  if (this->EdgeProperty)
  {
    os << indent << "Edge Property: " << this->EdgeProperty << "\n";
  }
  else
  {
    os << indent << "Edge Property: (none)\n";
  }

  os << indent << "Along X Axis: " << (this->AlongXAxis ? "On" : "Off") << "\n";
  os << indent << "Along Y Axis: " << (this->AlongYAxis ? "On" : "Off") << "\n";
  os << indent << "ALong Z Axis: " << (this->AlongZAxis ? "On" : "Off") << "\n";

  os << indent << "Tubing: " << (this->Tubing ? "On" : "Off") << "\n";
  os << indent << "Scale Enabled: " << (this->ScaleEnabled ? "On" : "Off") << "\n";
  os << indent << "Draw Cone: " << (this->DrawCone ? "On" : "Off") << "\n";
  os << indent << "Bump Distance: " << this->BumpDistance << "\n";

  os << indent << "Representation State: "
     << vtkConeRepresentation::InteractionStateToString(this->RepresentationState) << "\n";

  // this->InteractionState is printed in superclass
  // this is commented to avoid PrintSelf errors
}

void vtkConeRepresentation::HighlightElement(ElementType elem, int highlight)
{
  switch (elem)
  {
    case ConeFace:
      this->HighlightCone(highlight);
      break;
    case BottomFace:
      this->HighlightCap(true, highlight);
      break;
    case TopFace:
      this->HighlightCap(false, highlight);
      break;
    case ConeAxis:
      this->HighlightAxis(highlight);
      break;
    case BottomCurve:
      this->HighlightCurve(true, highlight);
      break;
    case TopCurve:
      this->HighlightCurve(false, highlight);
      break;
    case BottomHandle:
      this->HighlightHandle(true, highlight);
      break;
    case TopHandle:
      this->HighlightHandle(false, highlight);
      break;
    case NumberOfElements:
      // Set everything to the given highlight state.
      this->HighlightAxis(highlight);
      this->HighlightCone(highlight);
      this->HighlightCap(true, highlight);
      this->HighlightCap(false, highlight);
      this->HighlightCurve(true, highlight);
      this->HighlightCurve(false, highlight);
      this->HighlightHandle(true, highlight);
      this->HighlightHandle(false, highlight);
      break;
  }
}

void vtkConeRepresentation::HighlightCone(int highlight)
{
  if (highlight)
  {
    this->Elements[ConeFace].Actor->SetProperty(this->SelectedConeProperty);
  }
  else
  {
    this->Elements[ConeFace].Actor->SetProperty(this->ConeProperty);
  }
}

void vtkConeRepresentation::HighlightAxis(int highlight)
{
  if (highlight)
  {
    this->Elements[ConeAxis].Actor->SetProperty(this->SelectedEdgeProperty);
  }
  else
  {
    this->Elements[ConeAxis].Actor->SetProperty(this->EdgeProperty);
  }
}

void vtkConeRepresentation::HighlightCap(bool isBottom, int highlight)
{
  ElementType elem = isBottom ? BottomFace : TopFace;
  if (highlight)
  {
    this->Elements[elem].Actor->SetProperty(this->SelectedConeProperty);
  }
  else
  {
    this->Elements[elem].Actor->SetProperty(this->ConeProperty);
  }
}

void vtkConeRepresentation::HighlightCurve(bool isBottom, int highlight)
{
  ElementType elem = isBottom ? BottomCurve : TopCurve;
  if (highlight)
  {
    this->Elements[elem].Actor->SetProperty(this->SelectedEdgeProperty);
  }
  else
  {
    this->Elements[elem].Actor->SetProperty(this->EdgeProperty);
  }
}

void vtkConeRepresentation::HighlightHandle(bool isBottom, int highlight)
{
  ElementType elem = isBottom ? BottomHandle : TopHandle;
  if (highlight)
  {
    this->Elements[elem].Actor->SetProperty(this->SelectedHandleProperty);
  }
  else
  {
    this->Elements[elem].Actor->SetProperty(this->HandleProperty);
  }
}

void vtkConeRepresentation::Rotate(double X, double Y, double* p1, double* p2, double* vpn)
{
  double v[3];    //vector of motion
  double axis[3]; //axis of rotation
  double theta;   //rotation angle

  // mouse motion vector in world space
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Cone->GetBottomPoint());
  vtkVector3d cp1(this->Cone->GetTopPoint());
  vtkVector3d center = 0.5 * (cp0 + cp1);

  // Create axis of rotation and angle of rotation
  vtkMath::Cross(vpn, v, axis);
  if (vtkMath::Normalize(axis) == 0.0)
  {
    return;
  }
  int* size = this->Renderer->GetSize();
  double l2 = (X - this->LastEventPosition[0]) * (X - this->LastEventPosition[0]) +
    (Y - this->LastEventPosition[1]) * (Y - this->LastEventPosition[1]);
  theta = 360.0 * sqrt(l2 / (size[0] * size[0] + size[1] * size[1]));

  // Manipulate the transform to reflect the rotation
  this->Transform->Identity();
  this->Transform->Translate(center[0], center[1], center[2]);
  this->Transform->RotateWXYZ(theta, axis);
  this->Transform->Translate(-center[0], -center[1], -center[2]);

  this->Transform->TransformPoint(cp0.GetData(), cp0.GetData());
  this->Transform->TransformPoint(cp1.GetData(), cp1.GetData());

  this->Cone->SetBottomPoint(cp0.GetData());
  this->Cone->SetTopPoint(cp1.GetData());
}

void vtkConeRepresentation::PushCap(bool isBottom, double* p1, double* p2)
{
  //Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Cone->GetBottomPoint());
  vtkVector3d cp1(this->Cone->GetTopPoint());
  double cr0 = this->Cone->GetBottomRadius();
  double cr1 = this->Cone->GetTopRadius();
  vtkVector3d axis = cp1 - cp0;
  double length = axis.Norm();
  axis.Normalize();
  double deltaR = cr1 - cr0;
  double bump = v.Dot(axis);

  if (fabs(bump - length) < this->Tolerance)
  {
    // Ignore accidentally troublesome bumps that
    // would result in coincident endpoints.
    return;
  }

  if (isBottom)
  {
    double radius = cr0 + deltaR * bump / length;
    if (cr1 == 0.0 && radius < this->Tolerance)
    {
      // Ignore bumps that would make the cone too small near an apex.
      return;
    }
    else if (radius < 0.)
    {
      radius = 0.0;
      bump = -length * cr0 / deltaR;
    }
    this->Cone->SetBottomPoint((cp0 + bump * axis).GetData());
    this->Cone->SetBottomRadius(radius);
  }
  else
  {
    double radius = cr1 + deltaR * bump / length;
    if (cr0 == 0.0 && radius < this->Tolerance)
    {
      // Ignore bumps that would make the cone too small near an apex.
      return;
    }
    else if (radius < 0.)
    {
      radius = 0.0;
      bump = -length * cr1 / deltaR;
    }
    this->Cone->SetTopPoint((cp1 + bump * axis).GetData());
    this->Cone->SetTopRadius(radius);
  }
}

// Loop through all points and translate them
void vtkConeRepresentation::AdjustBottomRadius(double /*X*/, double Y, double* p1, double* p2)
{
  if (Y == this->LastEventPosition[1])
  {
    return;
  }

  double dr;
  double radius = this->Cone->GetBottomRadius();
  double v[3]; //vector of motion
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];
  double l = sqrt(vtkMath::Dot(v, v));

  dr = l / 4;
  if (Y < this->LastEventPosition[1])
  {
    dr *= -1.0;
  }

  bool apexTop = (this->Cone->GetTopRadius() <= this->Tolerance);
  double nextRadius = radius + dr;

  if ((apexTop || this->Cylindrical) && nextRadius < this->Tolerance)
  {
    nextRadius = this->Tolerance;
  }
  else if (nextRadius < 0.)
  {
    nextRadius = 0.0;
  }
  this->Cone->SetBottomRadius(nextRadius);
  if (this->Cylindrical)
  {
    this->Cone->SetTopRadius(nextRadius);
  }
  this->BuildRepresentation();
}

void vtkConeRepresentation::AdjustTopRadius(double /*X*/, double Y, double* p1, double* p2)
{
  if (Y == this->LastEventPosition[1])
  {
    return;
  }

  double dr;
  double radius = this->Cone->GetTopRadius();
  double v[3]; //vector of motion
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];
  double l = sqrt(vtkMath::Dot(v, v));

  dr = l / 4;
  if (Y < this->LastEventPosition[1])
  {
    dr *= -1.0;
  }

  bool apexBtm = (this->Cone->GetBottomRadius() <= this->Tolerance);
  double nextRadius = radius + dr;

  if ((apexBtm || this->Cylindrical) && nextRadius < this->Tolerance)
  {
    nextRadius = this->Tolerance;
  }
  else if (nextRadius < 0.)
  {
    nextRadius = 0.0;
  }
  this->Cone->SetTopRadius(nextRadius);
  if (this->Cylindrical)
  {
    this->Cone->SetBottomRadius(nextRadius);
  }
  this->BuildRepresentation();
}

// Loop through all points and translate them
void vtkConeRepresentation::TranslateCenter(double* p1, double* p2)
{
  //Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Cone->GetBottomPoint());
  vtkVector3d cp1(this->Cone->GetTopPoint());

  this->Cone->SetBottomPoint((cp0 + v).GetData());
  this->Cone->SetTopPoint((cp1 + v).GetData());

  this->BuildRepresentation();
}

// Translate the center on the axis
void vtkConeRepresentation::TranslateCenterOnAxis(double* p1, double* p2)
{
  // Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Cone->GetBottomPoint());
  vtkVector3d cp1(this->Cone->GetTopPoint());

  this->Cone->SetBottomPoint((cp0 + v).GetData());
  this->Cone->SetTopPoint((cp1 + v).GetData());

  this->BuildRepresentation();
}

// Loop through all points and translate them
void vtkConeRepresentation::TranslateHandle(bool isBottomHandle, double* p1, double* p2)
{
  //Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d handle(isBottomHandle ? this->Cone->GetBottomPoint() : this->Cone->GetTopPoint());

  if (isBottomHandle)
  {
    this->Cone->SetBottomPoint((handle + v).GetData());
  }
  else
  {
    this->Cone->SetTopPoint((handle + v).GetData());
  }

  this->BuildRepresentation();
}

void vtkConeRepresentation::Scale(double* p1, double* p2, double /*X*/, double Y)
{
  //Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Cone->GetBottomPoint());
  vtkVector3d cp1(this->Cone->GetTopPoint());
  vtkVector3d axis = cp1 - cp0;
  vtkVector3d origin = 0.5 * (cp1 + cp0);

  // Compute the scale factor
  double sf = v.Norm() / axis.Norm();
  sf = 1.0 + (Y > this->LastEventPosition[1] ? sf : -sf);

  this->Cone->SetBottomPoint((origin + (cp0 - origin) * sf).GetData());
  this->Cone->SetTopPoint((origin + (cp1 - origin) * sf).GetData());
  this->Cone->SetBottomRadius(this->Cone->GetBottomRadius() * sf);
  this->Cone->SetTopRadius(this->Cone->GetTopRadius() * sf);

  this->BuildRepresentation();
}

void vtkConeRepresentation::SizeHandles()
{
  double radius =
    this->vtkWidgetRepresentation::SizeHandlesInPixels(1.5, this->Sphere->GetCenter());

  this->Sphere->SetRadius(radius);
  this->AxisTuber->SetRadius(0.25 * radius);
}

void vtkConeRepresentation::CreateDefaultProperties()
{
  this->HandleProperty->SetColor(1., 1., 1.);

  this->EdgeProperty->SetColor(1., 1., 1.);
  this->EdgeProperty->SetLineWidth(3);

  this->ConeProperty->SetColor(1., 1., 1.);
  this->ConeProperty->SetOpacity(0.5);

  this->SelectedHandleProperty->SetColor(0.0, 1.0, 0.);
  this->SelectedHandleProperty->SetAmbient(1.0);

  this->SelectedEdgeProperty->SetColor(0., 1.0, 0.0);
  this->SelectedEdgeProperty->SetLineWidth(3);

  this->SelectedConeProperty->SetColor(0., 1., 0.);
  this->SelectedConeProperty->SetOpacity(0.5);
}

void vtkConeRepresentation::PlaceWidget(double bds[6])
{
  vtkVector3d lo(bds[0], bds[2], bds[4]);
  vtkVector3d hi(bds[1], bds[3], bds[5]);
  vtkVector3d md = 0.5 * (lo + hi);

  this->InitialLength = (hi - lo).Norm();

  if (this->AlongYAxis)
  {
    this->Cone->SetBottomPoint(md[0], lo[1], md[2]);
    this->Cone->SetTopPoint(md[0], hi[1], md[2]);
    double radius = hi[2] - md[2] > hi[0] - md[0] ? hi[0] - md[0] : hi[2] - md[2];
    this->Cone->SetBottomRadius(radius);
    this->Cone->SetTopRadius(radius);
  }
  else if (this->AlongZAxis)
  {
    this->Cone->SetBottomPoint(md[0], md[1], lo[2]);
    this->Cone->SetTopPoint(md[0], md[1], hi[2]);
    double radius = hi[0] - md[0] > hi[1] - md[1] ? hi[1] - md[1] : hi[0] - md[0];
    this->Cone->SetBottomRadius(radius);
    this->Cone->SetTopRadius(radius);
  }
  else //default or x-normal
  {
    this->Cone->SetBottomPoint(lo[0], md[1], md[2]);
    this->Cone->SetTopPoint(hi[0], md[1], md[2]);
    double radius = hi[2] - md[2] > hi[1] - md[1] ? hi[1] - md[1] : hi[2] - md[2];
    this->Cone->SetBottomRadius(radius);
    this->Cone->SetTopRadius(radius);
  }

  this->ValidPick = 1; // since we have positioned the widget successfully
  this->BuildRepresentation();
}

void vtkConeRepresentation::SetDrawCone(vtkTypeBool drawCyl)
{
  if (drawCyl == this->DrawCone)
  {
    return;
  }

  this->Modified();
  this->DrawCone = drawCyl;
  this->BuildRepresentation();
}

void vtkConeRepresentation::SetAlongXAxis(vtkTypeBool var)
{
  if (this->AlongXAxis != var)
  {
    this->AlongXAxis = var;
    this->Modified();
  }
  if (var)
  {
    this->AlongYAxisOff();
    this->AlongZAxisOff();
  }
}

void vtkConeRepresentation::SetAlongYAxis(vtkTypeBool var)
{
  if (this->AlongYAxis != var)
  {
    this->AlongYAxis = var;
    this->Modified();
  }
  if (var)
  {
    this->AlongXAxisOff();
    this->AlongZAxisOff();
  }
}

void vtkConeRepresentation::SetAlongZAxis(vtkTypeBool var)
{
  if (this->AlongZAxis != var)
  {
    this->AlongZAxis = var;
    this->Modified();
  }
  if (var)
  {
    this->AlongXAxisOff();
    this->AlongYAxisOff();
  }
}

void vtkConeRepresentation::GetCone(vtkImplicitConeFrustum* cone)
{
  if (cone == nullptr)
  {
    return;
  }

  cone->SetBottomPoint(this->GetBottomEndpoint());
  cone->SetTopPoint(this->GetTopEndpoint());
  cone->SetBottomRadius(this->Cone->GetBottomRadius());
  cone->SetTopRadius(this->Cone->GetTopRadius());
}

void vtkConeRepresentation::UpdatePlacement()
{
  this->BuildRepresentation();
}

void vtkConeRepresentation::BumpCone(int dir, double factor)
{
  // Compute the distance
  double d = this->InitialLength * this->BumpDistance * factor;

  // Push the cylinder
  this->PushCone((dir > 0 ? d : -d));
}

void vtkConeRepresentation::PushCone(double d)
{
  vtkCamera* camera = this->Renderer->GetActiveCamera();
  if (!camera)
  {
    return;
  }
  vtkVector3d vpn;
  vtkVector3d p0(this->Cone->GetBottomPoint());
  vtkVector3d p1(this->Cone->GetTopPoint());
  camera->GetViewPlaneNormal(vpn.GetData());

  p0 = p0 + d * vpn;
  p1 = p1 + d * vpn;

  this->Cone->SetBottomPoint(p0.GetData());
  this->Cone->SetTopPoint(p1.GetData());
  this->BuildRepresentation();
}

std::string vtkConeRepresentation::InteractionStateToString(int state)
{
  switch (state)
  {
    case Outside:
      return "Outside";
      break;
    case Moving:
      return "Moving";
      break;
    case PushingBottomFace:
      return "PushingBottomFace";
      break;
    case PushingTopFace:
      return "PushingTopFace";
      break;
    case AdjustingBottomRadius:
      return "AdjustingBottomRadius";
      break;
    case AdjustingTopRadius:
      return "AdjustingTopRadius";
      break;
    case MovingBottomHandle:
      return "MovingBottomHandle";
      break;
    case MovingTopHandle:
      return "MovingTopHandle";
      break;
    case MovingWhole:
      return "MovingWhole";
      break;
    case RotatingAxis:
      return "RotatingAxis";
      break;
    case Scaling:
      return "Scaling";
      break;
    case TranslatingCenter:
      return "TranslatingCenter";
      break;
    default:
      break;
  }
  return "Invalid";
}

void vtkConeRepresentation::BuildRepresentation()
{
  if (!this->Renderer || !this->Renderer->GetRenderWindow())
  {
    return;
  }

  vtkInformation* info = this->GetPropertyKeys();
  for (int ii = 0; ii < NumberOfElements; ++ii)
  {
    this->Elements[ii].Actor->SetPropertyKeys(info);
  }

  if (
    this->GetMTime() > this->BuildTime || this->Cone->GetMTime() > this->BuildTime ||
    this->Renderer->GetRenderWindow()->GetMTime() > this->BuildTime)
  {
    vtkVector3d p0(this->Cone->GetBottomPoint());
    vtkVector3d p1(this->Cone->GetTopPoint());

    // Control the look of the edges
    if (this->Tubing)
    {
      this->Elements[ConeAxis].Mapper->SetInputConnection(this->AxisTuber->GetOutputPort());
    }
    else
    {
      this->Elements[ConeAxis].Mapper->SetInputConnection(
        this->Cone->GetOutputPort(vtkConeFrustum::OutputPorts::Axis));
    }

    this->SizeHandles();
    this->BuildTime.Modified();
  }
}

void vtkConeRepresentation::RegisterPickers()
{
  vtkPickingManager* pm = this->GetPickingManager();
  if (!pm)
  {
    return;
  }
  pm->AddPicker(this->Picker, this);
}

void vtkConeRepresentation::SwapTopAndBottom()
{
  vtkVector3d p0(this->Cone->GetBottomPoint());
  vtkVector3d p1(this->Cone->GetTopPoint());
  double r0 = this->Cone->GetBottomRadius();
  double r1 = this->Cone->GetTopRadius();
  this->Cone->SetTopPoint(p0.GetData());
  this->Cone->SetBottomPoint(p1.GetData());
  this->Cone->SetTopRadius(r0);
  this->Cone->SetBottomRadius(r1);
  this->Modified();
}
