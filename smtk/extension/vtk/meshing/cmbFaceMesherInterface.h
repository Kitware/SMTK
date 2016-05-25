//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME cmbFaceMesherInterface
// .SECTION Description
// Wraps the Triangle library with an easy to use class



#ifndef __smtk_vtk_cmbFaceMesherInterface_h
#define __smtk_vtk_cmbFaceMesherInterface_h

#include "smtk/extension/vtk/meshing/Exports.h" // For export macro
#include <string> //for std string
#include "vtkABI.h"

class vtkPolyData;

namespace smtk {
  namespace vtk {

class vtkCMBMeshServerLauncher;

class VTKSMTKMESHINGEXT_EXPORT cmbFaceMesherInterface
{
public:
  cmbFaceMesherInterface();
  cmbFaceMesherInterface(const int &numPoints,const int &numSegments,
                         const int &numHoles=0, const int& numRegions=0,
                         const bool &preserveEdgesAndNodes=false);
  ~cmbFaceMesherInterface();

  void setUseMinAngle(const bool &useMin){MinAngleOn=useMin;}
  void setMinAngle(const double &angle){MinAngle=angle;}

  void setUseMaxArea(const bool &useMin){MaxAreaOn=useMin;}
  void setMaxArea(const double &inArea){MaxArea=inArea;}

  void setPreserveBoundaries(const bool& preserve){PreserveBoundaries=preserve;}

  void setVerboseOutput(const bool& verbose){VerboseOutput=verbose;}

  void setOutputMesh(vtkPolyData *mesh);

  bool setPoint(const int index, const double &x, const double &y, const int& nodeId=-1);

  bool setSegment(const int index, const int &pId1, const int &pId2, const int &arcId=0);

  bool setHole(const int index, const double &x, const double &y);

  bool setRegion(const int index, const double &x, const double &y, const double &attribute, const double& max_area);

  //returns the area of the bounds
  double area( ) const;

  //returns the bounds in the order of:
  //xmin,ymin,xmax,ymax
  void bounds(double bounds[4]) const;

  //if preserveEdges is set to true VTK_LINES
  //will be set in the output mesh and ElementIds
  //will be set for each edge
  //Uses the given server connection instead of creating it own.
  bool buildFaceMesh(vtkCMBMeshServerLauncher* activeServer,
                     const long &faceId, const double &zValue=0);

  //if preserveEdges is set to true VTK_LINES
  //will be set in the output mesh and ElementIds
  //will be set for each edge
  bool buildFaceMesh(const long &faceId, const double &zValue=0);
protected:
  void InitDataStructures();

  bool PackData(std::string& rawData);
  bool unPackData(const char* rawData, std::size_t rawDataSize,
                  const long &faceId, const double &zValue);

  vtkPolyData *OutputMesh;

private:
  bool MinAngleOn;
  bool MaxAreaOn;
  double MaxArea;
  double MinAngle;
  bool PreserveBoundaries;
  bool PreserveEdgesAndNodes;
  bool VerboseOutput;

  const int NumberOfPoints;
  const int NumberOfSegments;
  const int NumberOfHoles;
  const int NumberOfRegions;
  int NumberOfNodes; //Set implicitly by adding points with ids
  //BTX
  struct TriangleInput;
  TriangleInput *Ti;
  //ETX
};

  } // namespace vtk
} // namespace smtk

#endif
