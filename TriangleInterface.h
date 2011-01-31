/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkCmbModelFaceMeshPrivate
// .SECTION Description
// Convert a vtkModelFace to a triangle input for meshing. Also
// restores the resulting mesh to a vtkPolyData



#ifndef __TriangleInterface_h
#define __TriangleInterface_h

#include <vtkPolyData.h>

class TriangleInterface
{
public:
  TriangleInterface( MeshInformation* );
  void setMaximumArea(const double &area){MaxArea=area;}
  void setMinimumAngle(const double &angle){MinAngle=angle;}
  void setOutputMesh(vtkPolyData *mesh);

  bool buildFaceMesh();
protected:
  void InitDataStructures();

  double MaxArea;
  double MinAngle;
  vtkPolyData *OutputMesh;
  MeshInformation *MeshInfo;
};
#endif