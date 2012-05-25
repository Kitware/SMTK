//=============================================================================
//   This file is part of VTKEdge. See vtkedge.org for more information.
//
//   Copyright (c) 2010 Kitware, Inc.
//
//   VTKEdge may be used under the terms of the BSD License
//   Please see the file Copyright.txt in the root directory of
//   VTKEdge for further information.
//
//   Alternatively, you may see: 
//
//   http://www.vtkedge.org/vtkedge/project/license.html
//
//
//   For custom extensions, consulting services, or training for
//   this or any other Kitware supported open source project, please
//   contact Kitware at sales@kitware.com.
//
//
//=============================================================================
#include "vtkRenderingSerializationHelper.h"

#include "vtkCamera.h"
#include "vtkSerializationHelperMap.h"
#include "vtkSerializer.h"
#include "vtkXMLElement.h"
#include <vtkObjectFactory.h>

vtkCxxRevisionMacro(vtkRenderingSerializationHelper, "1774");
vtkStandardNewMacro(vtkRenderingSerializationHelper);

//-----------------------------------------------------------------------------
vtkRenderingSerializationHelper::vtkRenderingSerializationHelper()
{
}

//-----------------------------------------------------------------------------
int vtkRenderingSerializationHelper::Serialize(vtkObject *object,
                                                  vtkSerializer *serializer)
{
  if (vtkCamera::SafeDownCast(object))
    {
    this->SerializeCamera(vtkCamera::SafeDownCast(object), serializer);
    return 1;
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vtkRenderingSerializationHelper::RegisterWithHelperMap()
{
  vtkSerializationHelperMap::RegisterHelperForClass("vtkCamera", this);
  vtkSerializationHelperMap::RegisterHelperForClass("vtkOpenGLCamera", this);
  vtkSerializationHelperMap::RegisterHelperForClass("vtkMesaCamera", this);
}

//-----------------------------------------------------------------------------
void vtkRenderingSerializationHelper::UnRegisterWithHelperMap()
{
  vtkSerializationHelperMap::UnRegisterHelperForClass("vtkCamera", this);
  vtkSerializationHelperMap::UnRegisterHelperForClass("vtkOpenGLCamera", this);
  vtkSerializationHelperMap::UnRegisterHelperForClass("vtkMesaCamera", this);
}

//-----------------------------------------------------------------------------
void vtkRenderingSerializationHelper::SerializeCamera(vtkCamera *camera,
                                                         vtkSerializer *serializer)
{
  if (serializer->IsWriting())
    {
    // vectors of length 3
    unsigned int length = 3;
    double *position = camera->GetPosition();
    serializer->Serialize("Position", position, length);
    double *focalPt = camera->GetFocalPoint();
    serializer->Serialize("FocalPoint", focalPt, length);
    double *viewUp = camera->GetViewUp();
    serializer->Serialize("ViewUp", viewUp, length);

    // vectors of length 2
    length = 2;
    double *clippingRange = camera->GetClippingRange();
    serializer->Serialize("ClippingRange", clippingRange, length);
    double *windowCenter = camera->GetWindowCenter();
    serializer->Serialize("WindowCenter", windowCenter, length);

    // scalars
    double viewAngle = camera->GetViewAngle();
    serializer->Serialize("ViewAngle", viewAngle);
    int parallelProjection = camera->GetParallelProjection();
    serializer->Serialize("ParallelProjection", parallelProjection);
    double parallelScale = camera->GetParallelScale();
    serializer->Serialize("ParallelScale", parallelScale);
    int useHorizontalViewAngle = camera->GetUseHorizontalViewAngle();
    serializer->Serialize("UseHorizontalViewAngle", useHorizontalViewAngle);
    }
  else
    {
    unsigned int length = 0;

    // vectors of length 3
    double *position = 0, *focalPt = 0, *viewUp = 0;
    serializer->Serialize("Position", position, length);
    if (length > 0)
      {
      camera->SetPosition(position);
      delete [] position;
      }
    serializer->Serialize("FocalPoint", focalPt, length);
    if (length > 0)
      {
      camera->SetFocalPoint(focalPt);
      delete [] focalPt;
      }
    serializer->Serialize("ViewUp", viewUp, length);
    if (length > 0)
      {
      camera->SetViewUp(viewUp);
      delete [] viewUp;
      }

    // vectors of length 2
    double *clippingRange = 0, *windowCenter = 0;
    serializer->Serialize("ClippingRange", clippingRange, length);
    if (length > 0)
      {
      camera->SetClippingRange(clippingRange);
      delete [] clippingRange;
      }
    serializer->Serialize("WindowCenter", windowCenter, length);
    if (length > 0)
      {
      camera->SetWindowCenter(windowCenter[0], windowCenter[1]);
      delete [] windowCenter;
      }

    // scalars
    double viewAngle = camera->GetViewAngle(); // default value
    serializer->Serialize("ViewAngle", viewAngle);
    camera->SetViewAngle(viewAngle);

    int parallelProjection = camera->GetParallelProjection();
    serializer->Serialize("ParallelProjection", parallelProjection);
    camera->SetParallelProjection(parallelProjection);

    double parallelScale = camera->GetParallelScale();
    serializer->Serialize("ParallelScale", parallelScale);
    camera->SetParallelScale(parallelScale);

    int useHorizontalViewAngle = camera->GetUseHorizontalViewAngle();
    serializer->Serialize("UseHorizontalViewAngle", useHorizontalViewAngle);
    camera->SetUseHorizontalViewAngle(useHorizontalViewAngle);
    }
}


//-----------------------------------------------------------------------------
const char *vtkRenderingSerializationHelper::GetSerializationType(vtkObject *object)
{
  if (vtkCamera::SafeDownCast(object))
    {
    return "vtkCamera";
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkRenderingSerializationHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Supported ClassTypes:\n";
  os << indent.GetNextIndent() << "vtkCamera\n";
  os << indent.GetNextIndent() << "vtkOpenGLCamera\n";
  os << indent.GetNextIndent() << "vtkMesaCamera\n";
}
