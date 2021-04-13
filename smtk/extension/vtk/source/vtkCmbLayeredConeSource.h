//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_vtk_vtkCmbLayeredConeSource_h
#define __smtk_vtk_vtkCmbLayeredConeSource_h

#include <vector>

#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h"
#include <vtkMultiBlockDataSetAlgorithm.h>
#include <vtkSmartPointer.h>

class vtkPolyData;
class vtkPoints;
class vtkCellArray;

// Copied from rgg project
class VTKSMTKSOURCEEXT_EXPORT vtkCmbLayeredConeSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkCmbLayeredConeSource* New();
  vtkTypeMacro(vtkCmbLayeredConeSource, vtkMultiBlockDataSetAlgorithm);

  void SetNumberOfLayers(int layers);
  int GetNumberOfLayers();

  void SetTopRadius(int layer, double radius);
  void SetTopRadius(int layer, double r1, double r2);
  double GetTopRadius(int layer, int s = 0);

  void SetBaseRadius(int layer, double radius);
  void SetBaseRadius(int layer, double r1, double r2);
  double GetBaseRadius(int layer, int s = 0);

  void addInnerPoint(double x, double y);

  void SetResolution(int layer, int res);
  int GetResolution(int layer);

  vtkSetMacro(Height, double);
  vtkGetMacro(Height, double);

  vtkSetVector3Macro(BaseCenter, double);
  vtkGetVectorMacro(BaseCenter, double, 3);

  vtkSetVector3Macro(Direction, double);
  //vtkGetVectorMacro(Direction,double,3);

  // Determines whether surface normals should be generated
  // On by default
  vtkSetMacro(GenerateNormals, int);
  vtkGetMacro(GenerateNormals, int);
  vtkBooleanMacro(GenerateNormals, int);

  //vtkSetMacro(GenerateEnds,int);
  //vtkGetMacro(GenerateEnds,int);
  //vtkBooleanMacro(GenerateEnds,int);

  vtkSmartPointer<vtkPolyData> CreateUnitLayer(int l);
  vtkSmartPointer<vtkPolyData> CreateBoundaryLayer(double thickness, int l);

protected:
  vtkCmbLayeredConeSource();
  ~vtkCmbLayeredConeSource();

  vtkSmartPointer<vtkPolyData> CreateLayer(
    double h,
    double* innerBottomR,
    double* outerBottomR,
    double* innerTopR,
    double* outerTopR,
    int innerRes,
    int outerRes,
    bool lines = false);

  double Height;
  struct radii
  {
    int Resolution;
    double BaseRadii[2];
    double TopRadii[2];
  };
  std::vector<radii> LayerRadii;
  std::vector<std::vector<double>> InnerPoints;
  double BaseCenter[3];
  double Direction[3];
  int Resolution;
  int GenerateNormals;
  int GenerateEnds;

private:
  vtkCmbLayeredConeSource(const vtkCmbLayeredConeSource&);
  void operator=(const vtkCmbLayeredConeSource&);

  void TriangulateEnd(
    const int innerRes,
    const int outerRes,
    bool forceDelaunay,
    vtkCellArray* cells,
    vtkPoints* fullPoints);
};

#endif
