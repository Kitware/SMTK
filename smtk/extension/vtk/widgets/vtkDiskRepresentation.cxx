//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/widgets/vtkDiskRepresentation.h"
#include "smtk/extension/vtk/source/vtkDisk.h"
#include "smtk/extension/vtk/source/vtkImplicitDisk.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkBox.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDiskSource.h"
#include "vtkDoubleArray.h"
#include "vtkGlyph3DMapper.h"
#include "vtkHardwarePicker.h"
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

vtkStandardNewMacro(vtkDiskRepresentation);

vtkDiskRepresentation::vtkDiskRepresentation()
{
  this->HandleSize = 7.5;
  this->Disk->SetResolution(128);

  // Set up the initial properties
  this->CreateDefaultProperties();

  for (int ii = 0; ii < NumberOfElements; ++ii)
  {
    this->Elements[ii].Actor->SetMapper(this->Elements[ii].Mapper);
    if (ii <= DiskFace)
    {
      this->Elements[ii].Actor->SetProperty(this->DiskProperty);
    }
    else if (ii <= DiskEdge)
    {
      this->Elements[ii].Actor->SetProperty(this->EdgeProperty);
    }
    else
    {
      this->Elements[ii].Actor->SetProperty(this->HandleProperty);
    }
  }

  // Set up the pipelines for the visual elements
  this->AxisTuber->SetInputConnection(this->Disk->GetOutputPort(vtkDisk::OutputPorts::DiskNormal));
  this->AxisTuber->SetNumberOfSides(12);
  this->Elements[DiskNormal].Mapper->SetInputConnection(this->AxisTuber->GetOutputPort());
  this->Elements[DiskNormal].Actor->SetMapper(this->Elements[DiskNormal].Mapper);

  this->Elements[DiskFace].Mapper->SetInputConnection(
    this->Disk->GetOutputPort(vtkDisk::OutputPorts::DiskFace));

  this->Elements[DiskEdge].Mapper->SetInputConnection(
    this->Disk->GetOutputPort(vtkDisk::OutputPorts::DiskEdge));

  // Create the endpoint geometry source
  this->Sphere->SetThetaResolution(16);
  this->Sphere->SetPhiResolution(8);

  this->CenterVertexMapper->SetSourceConnection(this->Sphere->GetOutputPort());
  this->CenterVertexMapper->SetInputConnection(
    this->Disk->GetOutputPort(vtkDisk::OutputPorts::CenterVertex));
  this->Elements[CenterVertex].Actor->SetMapper(this->CenterVertexMapper);

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
  for (int ii = DiskNormal; ii < NumberOfElements; ++ii)
  {
    this->Picker->AddPickList(this->Elements[ii].Actor);
  }
  this->Picker->PickFromListOn();

  this->FacePicker->SetTolerance(0.005);
  for (int ii = DiskFace; ii <= DiskFace; ++ii)
  {
    this->FacePicker->AddPickList(this->Elements[ii].Actor);
  }
  this->FacePicker->PickFromListOn();

  this->RepresentationState = vtkDiskRepresentation::Outside;
}

vtkDiskRepresentation::~vtkDiskRepresentation() = default;

bool vtkDiskRepresentation::SetCenter(double x, double y, double z)
{
  return this->SetCenter(vtkVector3d(x, y, z));
}

bool vtkDiskRepresentation::SetCenter(const vtkVector3d& pt)
{
  vtkVector3d p0(this->Disk->GetCenterPoint());

  vtkVector3d temp(pt); // Because vtkSetVectorMacro is not const correct.
  if (p0 == pt)
  {
    // If pt is already the existing value, do nothing.
    return false;
  }

  this->Disk->SetCenterPoint(temp.GetData());
  this->Modified();
  return true;
}

vtkVector3d vtkDiskRepresentation::GetCenter() const
{
  return vtkVector3d(this->Disk->GetCenterPoint());
}

double* vtkDiskRepresentation::GetCenterPoint()
{
  return this->Disk->GetCenterPoint();
}

bool vtkDiskRepresentation::SetNormal(double x, double y, double z)
{
  return this->SetNormal(vtkVector3d(x, y, z));
}

bool vtkDiskRepresentation::SetNormal(const vtkVector3d& nm)
{
  auto* norm = this->Disk->GetNormal();
  vtkVector3d dnorm(norm[0], norm[1], norm[2]);
  if (dnorm == nm)
  {
    return false;
  }
  this->Disk->SetNormal(nm.GetData());
  return true;
}

vtkVector3d vtkDiskRepresentation::GetNormal() const
{
  double* nm = this->Disk->GetNormal();
  return vtkVector3d(nm[0], nm[1], nm[2]);
}

double* vtkDiskRepresentation::GetNormalVector()
{
  return this->Disk->GetNormal();
}

bool vtkDiskRepresentation::SetRadius(double r)
{
  double prev = this->Disk->GetRadius();
  if (prev == r)
  {
    return false;
  }
  this->Disk->SetRadius(r);
  this->Modified();
  return true;
}

double vtkDiskRepresentation::GetRadius() const
{
  return this->Disk->GetRadius();
}

int vtkDiskRepresentation::ComputeInteractionState(int X, int Y, int /*modify*/)
{
  // See if anything has been selected
  vtkAssemblyPath* path = this->GetAssemblyPath(X, Y, 0., this->Picker);

  // The second picker may need to be called. This is done because the disk face
  // may obstruct things that can be picked; thus the disk face is the selection
  // of last resort. This allows users to rotate the normal vector even from behind
  // the disk
  if (path == nullptr)
  {
    this->FacePicker->Pick(X, Y, 0., this->Renderer);
    path = this->FacePicker->GetPath();
  }

  if (path == nullptr) // Nothing picked
  {
    this->SetRepresentationState(vtkDiskRepresentation::Outside);
    this->InteractionState = vtkDiskRepresentation::Outside;
    return this->InteractionState;
  }

  // Something picked, continue
  this->ValidPick = 1;

  // Depending on the interaction state (set by the widget) we modify
  // this state based on what is picked.
  if (this->InteractionState == vtkDiskRepresentation::Moving)
  {
    vtkProp* prop = path->GetFirstNode()->GetViewProp();
    if (prop == this->Elements[DiskNormal].Actor)
    {
      this->InteractionState = vtkDiskRepresentation::RotatingNormal;
      this->SetRepresentationState(vtkDiskRepresentation::RotatingNormal);
    }
    else if (prop == this->Elements[DiskEdge].Actor)
    {
      this->InteractionState = vtkDiskRepresentation::AdjustingRadius;
      this->SetRepresentationState(vtkDiskRepresentation::AdjustingRadius);
    }
    else if (prop == this->Elements[CenterVertex].Actor)
    {
      this->InteractionState = vtkDiskRepresentation::MovingCenterVertex;
      this->SetRepresentationState(vtkDiskRepresentation::MovingCenterVertex);
    }
    // Better to push the face along the normal than translate in view plane.
    // else if (prop == this->Elements[DiskFace].Actor)
    // {
    //   this->InteractionState = vtkDiskRepresentation::MovingWhole;
    //   this->SetRepresentationState(vtkDiskRepresentation::MovingWhole);
    // }
    else if (prop == this->Elements[DiskFace].Actor)
    {
      this->InteractionState = vtkDiskRepresentation::PushingDiskFace;
      this->SetRepresentationState(vtkDiskRepresentation::PushingDiskFace);
    }
    else
    {
      this->InteractionState = vtkDiskRepresentation::Outside;
      this->SetRepresentationState(vtkDiskRepresentation::Outside);
    }
  }
  else
  {
    this->InteractionState = vtkDiskRepresentation::Outside;
  }

  return this->InteractionState;
}

void vtkDiskRepresentation::SetRepresentationState(int state)
{
  if (this->RepresentationState == state)
  {
    return;
  }

  // Clamp the state
  state =
    (state < vtkDiskRepresentation::Outside
       ? vtkDiskRepresentation::Outside
       : (state > vtkDiskRepresentation::RotatingNormal ? vtkDiskRepresentation::RotatingNormal
                                                        : state));

  this->RepresentationState = state;
  this->Modified();

#if 0
  // For debugging, it is handy to see state changes:
  std::cout
    << "   State "
    << vtkDiskRepresentation::InteractionStateToString(this->RepresentationState)
    << "\n";
#endif

  this->HighlightElement(NumberOfElements, 0); // Turn everything off
  if (state == vtkDiskRepresentation::RotatingNormal)
  {
    this->HighlightAxis(1);
  }
  else if (state == vtkDiskRepresentation::PushingDiskFace)
  {
    this->HighlightDisk(1);
  }
  else if (state == vtkDiskRepresentation::AdjustingRadius)
  {
    this->HighlightCurve(1);
  }
  else if (state == vtkDiskRepresentation::MovingWhole)
  {
    this->HighlightCurve(1);
    this->HighlightDisk(1);
    this->HighlightAxis(1);
    this->HighlightHandle(1);
  }
  else
  {
    this->HighlightAxis(0);
    this->HighlightDisk(0);
    // this->HighlightOutline(0);
  }
}

void vtkDiskRepresentation::StartWidgetInteraction(double e[2])
{
  this->StartEventPosition[0] = e[0];
  this->StartEventPosition[1] = e[1];
  this->StartEventPosition[2] = 0.0;

  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

void vtkDiskRepresentation::WidgetInteraction(double e[2])
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
  if (this->InteractionState == vtkDiskRepresentation::AdjustingRadius)
  {
    this->AdjustRadius(e[0], e[1], prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkDiskRepresentation::MovingCenterVertex)
  {
    this->TranslateCenterInPlane(prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkDiskRepresentation::MovingWhole)
  {
    this->TranslateCenter(prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkDiskRepresentation::PushingDiskFace)
  {
    this->PushFace(prevPickPoint, pickPoint);
  }
  else if (this->InteractionState == vtkDiskRepresentation::RotatingNormal)
  {
    camera->GetViewPlaneNormal(vpn);
    this->Rotate(e[0], e[1], prevPickPoint, pickPoint, vpn);
  }

  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

void vtkDiskRepresentation::EndWidgetInteraction(double /*newEventPos*/[2])
{
  this->SetRepresentationState(vtkDiskRepresentation::Outside);
}

double* vtkDiskRepresentation::GetBounds()
{
  this->BuildRepresentation();
  this->BoundingBox->SetBounds(this->Elements[DiskFace].Actor->GetBounds());
  for (int ii = DiskFace; ii < NumberOfElements; ++ii)
  {
    this->BoundingBox->AddBounds(this->Elements[ii].Actor->GetBounds());
  }

  return this->BoundingBox->GetBounds();
}

void vtkDiskRepresentation::GetActors(vtkPropCollection* pc)
{
  for (int ii = 0; ii < NumberOfElements; ++ii)
  {
    this->Elements[ii].Actor->GetActors(pc);
  }
}

void vtkDiskRepresentation::ReleaseGraphicsResources(vtkWindow* w)
{
  for (int ii = 0; ii < NumberOfElements; ++ii)
  {
    this->Elements[ii].Actor->ReleaseGraphicsResources(w);
  }
}

int vtkDiskRepresentation::RenderOpaqueGeometry(vtkViewport* v)
{
  int count = 0;
  this->BuildRepresentation();
  for (int ii = DiskNormal; ii < NumberOfElements; ++ii)
  {
    count += this->Elements[ii].Actor->RenderOpaqueGeometry(v);
  }

  if (this->DrawDisk)
  {
    for (int ii = DiskFace; ii < DiskNormal; ++ii)
    {
      count += this->Elements[ii].Actor->RenderOpaqueGeometry(v);
    }
  }

  return count;
}

int vtkDiskRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* v)
{
  int count = 0;
  this->BuildRepresentation();
  for (int ii = DiskNormal; ii < NumberOfElements; ++ii)
  {
    count += this->Elements[ii].Actor->RenderTranslucentPolygonalGeometry(v);
  }

  if (this->DrawDisk)
  {
    for (int ii = DiskFace; ii < DiskNormal; ++ii)
    {
      count += this->Elements[ii].Actor->RenderTranslucentPolygonalGeometry(v);
    }
  }

  return count;
}

vtkTypeBool vtkDiskRepresentation::HasTranslucentPolygonalGeometry()
{
  int result = 0;
  for (int ii = DiskNormal; ii < NumberOfElements; ++ii)
  {
    result |= this->Elements[ii].Actor->HasTranslucentPolygonalGeometry();
  }

  if (this->DrawDisk)
  {
    for (int ii = DiskFace; ii < DiskNormal; ++ii)
    {
      result |= this->Elements[ii].Actor->HasTranslucentPolygonalGeometry();
    }
  }

  return result;
}

void vtkDiskRepresentation::PrintSelf(ostream& os, vtkIndent indent)
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

  if (this->DiskProperty)
  {
    os << indent << "Disk Property: " << this->DiskProperty << "\n";
  }
  else
  {
    os << indent << "Disk Property: (none)\n";
  }
  if (this->SelectedDiskProperty)
  {
    os << indent << "Selected Disk Property: " << this->SelectedDiskProperty << "\n";
  }
  else
  {
    os << indent << "Selected Disk Property: (none)\n";
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
  os << indent << "Draw Disk: " << (this->DrawDisk ? "On" : "Off") << "\n";
  os << indent << "Bump Distance: " << this->BumpDistance << "\n";

  os << indent << "Representation State: "
     << vtkDiskRepresentation::InteractionStateToString(this->RepresentationState) << "\n";

  // this->InteractionState is printed in superclass
  // this is commented to avoid PrintSelf errors
}

void vtkDiskRepresentation::HighlightElement(ElementType elem, int highlight)
{
  switch (elem)
  {
    case DiskFace:
      this->HighlightDisk(highlight);
      break;
    case DiskNormal:
      this->HighlightAxis(highlight);
      break;
    case DiskEdge:
      this->HighlightCurve(highlight);
      break;
    case CenterVertex:
      this->HighlightHandle(highlight);
      break;
    case NumberOfElements:
      // Set everything to the given highlight state.
      this->HighlightAxis(highlight);
      this->HighlightDisk(highlight);
      this->HighlightCurve(highlight);
      this->HighlightHandle(highlight);
      break;
  }
}

void vtkDiskRepresentation::HighlightDisk(int highlight)
{
  if (highlight)
  {
    this->Elements[DiskFace].Actor->SetProperty(this->SelectedDiskProperty);
  }
  else
  {
    this->Elements[DiskFace].Actor->SetProperty(this->DiskProperty);
  }
}

void vtkDiskRepresentation::HighlightAxis(int highlight)
{
  if (highlight)
  {
    this->Elements[DiskNormal].Actor->SetProperty(this->SelectedEdgeProperty);
  }
  else
  {
    this->Elements[DiskNormal].Actor->SetProperty(this->EdgeProperty);
  }
}

void vtkDiskRepresentation::HighlightCurve(int highlight)
{
  ElementType elem = DiskEdge;
  if (highlight)
  {
    this->Elements[elem].Actor->SetProperty(this->SelectedEdgeProperty);
  }
  else
  {
    this->Elements[elem].Actor->SetProperty(this->EdgeProperty);
  }
}

void vtkDiskRepresentation::HighlightHandle(int highlight)
{
  ElementType elem = CenterVertex;
  if (highlight)
  {
    this->Elements[elem].Actor->SetProperty(this->SelectedHandleProperty);
  }
  else
  {
    this->Elements[elem].Actor->SetProperty(this->HandleProperty);
  }
}

void vtkDiskRepresentation::Rotate(double X, double Y, double* p1, double* p2, double* vpn)
{
  double v[3];    //vector of motion
  double axis[3]; //axis of rotation
  double theta;   //rotation angle

  // mouse motion vector in world space
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Disk->GetCenterPoint());
  vtkVector3d cp1(this->Disk->GetNormal());
  vtkVector3d center = cp0;

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

  // cp0 is the center of rotation, it will not move.
  // this->Transform->TransformPoint(cp0.GetData(), cp0.GetData());
  // this->Transform->TransformPoint(cp1.GetData(), cp1.GetData());
  this->Transform->TransformVector(cp1.GetData(), cp1.GetData());

  // this->Disk->SetCenterPoint(cp0.GetData());
  // this->Disk->SetTopPoint(cp1.GetData());
  this->Disk->SetNormal(cp1.GetData());
}

void vtkDiskRepresentation::PushFace(double* p1, double* p2)
{
  //Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Disk->GetCenterPoint());
  vtkVector3d cp1(this->Disk->GetNormal());
  vtkVector3d axis = cp1;
  axis.Normalize();
  double bump = v.Dot(axis);

  this->Disk->SetCenterPoint((cp0 + bump * axis).GetData());
}

// Loop through all points and translate them
void vtkDiskRepresentation::AdjustRadius(double /*X*/, double Y, double* p1, double* p2)
{
  if (Y == this->LastEventPosition[1])
  {
    return;
  }

  double dr;
  double radius = this->Disk->GetRadius();
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

  double nextRadius = radius + dr;

  if (nextRadius < 1e-30)
  {
    nextRadius = 1e-30;
  }
  else if (nextRadius < 0.)
  {
    nextRadius = -nextRadius;
  }
  this->Disk->SetRadius(nextRadius);
  this->BuildRepresentation();
}

// Loop through all points and translate them
void vtkDiskRepresentation::TranslateCenter(double* p1, double* p2)
{
  //Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp0(this->Disk->GetCenterPoint());

  this->Disk->SetCenterPoint((cp0 + v).GetData());
  this->BuildRepresentation();
}

// Translate the center point within the plane of the disk.
void vtkDiskRepresentation::TranslateCenterInPlane(double* p1, double* p2)
{
  // Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d cp(this->Disk->GetCenterPoint());
  vtkVector3d norm(this->Disk->GetNormal());

  v = v - v.Dot(norm) * norm;

  this->Disk->SetCenterPoint((cp + v).GetData());
  this->BuildRepresentation();
}

// Loop through all points and translate them
void vtkDiskRepresentation::TranslateHandle(double* p1, double* p2)
{
  //Get the motion vector
  vtkVector3d v;
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  vtkVector3d handle(this->Disk->GetCenterPoint());

  this->Disk->SetCenterPoint((handle + v).GetData());
  this->BuildRepresentation();
}

void vtkDiskRepresentation::SizeHandles()
{
  double radius =
    this->vtkWidgetRepresentation::SizeHandlesInPixels(1.5, this->Sphere->GetCenter());

  this->Sphere->SetRadius(radius);
  this->AxisTuber->SetRadius(0.25 * radius);
}

void vtkDiskRepresentation::CreateDefaultProperties()
{
  this->HandleProperty->SetColor(1., 1., 1.);

  this->EdgeProperty->SetColor(1., 1., 1.);
  this->EdgeProperty->SetLineWidth(3);

  this->DiskProperty->SetColor(1., 1., 1.);
  this->DiskProperty->SetOpacity(0.5);

  this->SelectedHandleProperty->SetColor(0.0, 1.0, 0.);
  this->SelectedHandleProperty->SetAmbient(1.0);

  this->SelectedEdgeProperty->SetColor(0., 1.0, 0.0);
  this->SelectedEdgeProperty->SetLineWidth(3);

  this->SelectedDiskProperty->SetColor(0., 1., 0.);
  this->SelectedDiskProperty->SetOpacity(0.5);
}

void vtkDiskRepresentation::PlaceWidget(double bds[6])
{
  vtkVector3d lo(bds[0], bds[2], bds[4]);
  vtkVector3d hi(bds[1], bds[3], bds[5]);
  vtkVector3d md = 0.5 * (lo + hi);

  this->InitialLength = (hi - lo).Norm();

  if (this->AlongYAxis)
  {
    this->Disk->SetCenterPoint(md[0], lo[1], md[2]);
    double radius = hi[2] - md[2] > hi[0] - md[0] ? hi[0] - md[0] : hi[2] - md[2];
    this->Disk->SetRadius(radius);
  }
  else if (this->AlongZAxis)
  {
    this->Disk->SetCenterPoint(md[0], md[1], lo[2]);
    double radius = hi[0] - md[0] > hi[1] - md[1] ? hi[1] - md[1] : hi[0] - md[0];
    this->Disk->SetRadius(radius);
  }
  else //default or x-normal
  {
    this->Disk->SetCenterPoint(lo[0], md[1], md[2]);
    double radius = hi[2] - md[2] > hi[1] - md[1] ? hi[1] - md[1] : hi[2] - md[2];
    this->Disk->SetRadius(radius);
  }

  this->ValidPick = 1; // since we have positioned the widget successfully
  this->BuildRepresentation();
}

void vtkDiskRepresentation::SetDrawDisk(vtkTypeBool drawCyl)
{
  if (drawCyl == this->DrawDisk)
  {
    return;
  }

  this->Modified();
  this->DrawDisk = drawCyl;
  this->BuildRepresentation();
}

void vtkDiskRepresentation::SetAlongXAxis(vtkTypeBool var)
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

void vtkDiskRepresentation::SetAlongYAxis(vtkTypeBool var)
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

void vtkDiskRepresentation::SetAlongZAxis(vtkTypeBool var)
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

void vtkDiskRepresentation::GetDisk(vtkImplicitDisk* disk)
{
  if (disk == nullptr)
  {
    return;
  }

  disk->SetCenterPoint(this->GetCenter());
  disk->SetRadius(this->Disk->GetRadius());
  disk->SetNormal(this->Disk->GetNormal());
}

void vtkDiskRepresentation::UpdatePlacement()
{
  this->BuildRepresentation();
}

void vtkDiskRepresentation::BumpDisk(int dir, double factor)
{
  // Compute the distance
  double d = this->InitialLength * this->BumpDistance * factor;

  // Push the cylinder
  this->PushDisk((dir > 0 ? d : -d));
}

void vtkDiskRepresentation::PushDisk(double d)
{
  vtkCamera* camera = this->Renderer->GetActiveCamera();
  if (!camera)
  {
    return;
  }
  vtkVector3d vpn;
  vtkVector3d p0(this->Disk->GetCenterPoint());
  vtkVector3d p1(this->Disk->GetNormal());
  camera->GetViewPlaneNormal(vpn.GetData());

  p0 = p0 + d * vpn;

  this->Disk->SetCenterPoint(p0.GetData());
  this->BuildRepresentation();
}

bool vtkDiskRepresentation::PickNormal(int X, int Y, bool snapToMeshPoint)
{
  this->HardwarePicker->SetSnapToMeshPoint(snapToMeshPoint);
  vtkAssemblyPath* path = this->GetAssemblyPath(X, Y, 0., this->HardwarePicker);
  if (path == nullptr) // actors of renderer were not touched
  {
    if (this->PickCameraFocalInfo)
    {
      double normal[3];
      this->HardwarePicker->GetPickNormal(normal);
      this->SetNormal(normal);
      this->BuildRepresentation();
    }
    return this->PickCameraFocalInfo;
  }
  else // actors of renderer were touched
  {
    double normal[3];
    this->HardwarePicker->GetPickNormal(normal);
    if (!std::isnan(normal[0]) || !std::isnan(normal[1]) || !std::isnan(normal[2]))
    {
      this->SetNormal(normal);
      this->BuildRepresentation();
      return true;
    }
    else
    {
      return false;
    }
  }
}

std::string vtkDiskRepresentation::InteractionStateToString(int state)
{
  switch (state)
  {
    case Outside:
      return "Outside";
      break;
    case Moving:
      return "Moving";
      break;
    case PushingDiskFace:
      return "PushingDiskFace";
      break;
    case AdjustingRadius:
      return "AdjustingRadius";
      break;
    case MovingCenterVertex:
      return "MovingCenterVertex";
      break;
    case MovingWhole:
      return "MovingWhole";
      break;
    case RotatingNormal:
      return "RotatingNormal";
      break;
    default:
      break;
  }
  return "Invalid";
}

void vtkDiskRepresentation::BuildRepresentation()
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
    this->GetMTime() > this->BuildTime || this->Disk->GetMTime() > this->BuildTime ||
    this->Renderer->GetRenderWindow()->GetMTime() > this->BuildTime)
  {
    // Control the look of the edges
    if (this->Tubing)
    {
      this->Elements[DiskNormal].Mapper->SetInputConnection(this->AxisTuber->GetOutputPort());
    }
    else
    {
      this->Elements[DiskNormal].Mapper->SetInputConnection(
        this->Disk->GetOutputPort(vtkDisk::OutputPorts::DiskNormal));
    }

    this->SizeHandles();
    this->BuildTime.Modified();
  }
}

void vtkDiskRepresentation::RegisterPickers()
{
  vtkPickingManager* pm = this->GetPickingManager();
  if (!pm)
  {
    return;
  }
  pm->AddPicker(this->Picker, this);
}
