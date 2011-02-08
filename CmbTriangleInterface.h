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
// .NAME CmbTriangleInterface
// .SECTION Description
// Wraps the Triangle library with an easy to use class



#ifndef __CmbTriangleInterface_h
#define __CmbTriangleInterface_h

#include <vtkstd/string> //for std string
class vtkPolyData;

class CmbTriangleInterface
{
public:
  CmbTriangleInterface(const int &numPoints,const int &numSegments, const int &numHoles);
  ~CmbTriangleInterface();

  void setUseMinAngle(const bool &useMin){MinAngleOn=useMin;}
  void setMinAngle(const double &angle){MinAngle=angle;}

  void setUseMaxArea(const bool &useMin){MaxAreaOn=useMin;}
  void setMaxArea(const double &area){MaxArea=area;}

  void setOutputMesh(vtkPolyData *mesh);

  bool setPoint(const int index, const double &x, const double &y);

  bool setSegement(const int index, const int &pId1, const int &pId2);

  bool setHole(const int index, const double &x, const double &y);

  //returns the area of the bounds
  double area( ) const;

  //returns the bounds in the order of:
  //xmin,ymin,xmax,ymax
  void bounds(double bounds[4]) const;

  bool buildFaceMesh(const long &faceId);
protected:
  void InitDataStructures();

  std::string BuildTriangleArguments() const;

  vtkPolyData *OutputMesh;

private:
  bool MinAngleOn;
  bool MaxAreaOn;
  double MaxArea;
  double MinAngle;

  const int NumberOfPoints;
  const int NumberOfSegments;
  const int NumberOfHoles;
  //BTX
  struct TriangleIO;
  TriangleIO *TIO;
  //EXT
};
#endif
