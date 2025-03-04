//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCmbLayeredConeSource.h"

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>

#include "vtkPolyDataNormals.h"
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <vtkDelaunay2D.h>
#include <vtkPolygon.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <cassert>

vtkStandardNewMacro(vtkCmbLayeredConeSource);

vtkCmbLayeredConeSource::vtkCmbLayeredConeSource()
{
  this->SetNumberOfInputPorts(0);

  this->Height = 1.0;

  this->BaseCenter[0] = 0.0;
  this->BaseCenter[1] = 0.0;
  this->BaseCenter[2] = 0.0;

  this->Direction[0] = 0.0;
  this->Direction[1] = 0.0;
  this->Direction[2] = 1.0;
  this->GenerateNormals = 1;
  this->GenerateEnds = 1;
}

vtkCmbLayeredConeSource::~vtkCmbLayeredConeSource() = default;

void vtkCmbLayeredConeSource::SetNumberOfLayers(int layers)
{
  this->LayerRadii.resize(layers);
  this->Modified();
}

int vtkCmbLayeredConeSource::GetNumberOfLayers()
{
  return static_cast<int>(this->LayerRadii.size());
}

void vtkCmbLayeredConeSource::SetTopRadius(int layer, double r1, double r2)
{
  this->LayerRadii[layer].TopRadii[0] = r1;
  this->LayerRadii[layer].TopRadii[1] = r2;
  this->Modified();
}

void vtkCmbLayeredConeSource::SetBaseRadius(int layer, double r1, double r2)
{
  this->LayerRadii[layer].BaseRadii[0] = r1;
  this->LayerRadii[layer].BaseRadii[1] = r2;
  this->Modified();
}

void vtkCmbLayeredConeSource::SetTopRadius(int layer, double radius)
{
  this->LayerRadii[layer].TopRadii[0] = this->LayerRadii[layer].TopRadii[1] = radius;
  this->Modified();
}

double vtkCmbLayeredConeSource::GetTopRadius(int layer, int s)
{
  return this->LayerRadii[layer].TopRadii[s];
}

void vtkCmbLayeredConeSource::SetBaseRadius(int layer, double radius)
{
  this->LayerRadii[layer].BaseRadii[0] = this->LayerRadii[layer].BaseRadii[1] = radius;
  this->Modified();
}

void vtkCmbLayeredConeSource::SetResolution(int layer, int res)
{
  this->LayerRadii[layer].Resolution = res;
}

int vtkCmbLayeredConeSource::GetResolution(int layer)
{
  return this->LayerRadii[layer].Resolution;
}

double vtkCmbLayeredConeSource::GetBaseRadius(int layer, int s)
{
  return this->LayerRadii[layer].BaseRadii[s];
}

vtkSmartPointer<vtkPolyData> vtkCmbLayeredConeSource::CreateUnitLayer(int l)
{
  if (l < 0)
    return nullptr;
  if (l >= this->GetNumberOfLayers())
    return nullptr;

  int innerRes = 0;
  int outerRes = 0;
  double one[] = { 1, 1 };
  double* innerBottomR = nullptr;
  double* innerTopR = nullptr;
  double* outerBottomR = nullptr;
  double* outerTopR = nullptr;

  outerBottomR = this->LayerRadii[l].BaseRadii;
  outerTopR = this->LayerRadii[l].TopRadii;
  outerRes = this->LayerRadii[l].Resolution;

  if (l != 0)
  {
    innerBottomR = this->LayerRadii[l - 1].BaseRadii;
    innerTopR = this->LayerRadii[l - 1].TopRadii;
    innerRes = this->LayerRadii[l - 1].Resolution;
  }
  else if (l == 0 && *outerBottomR == *outerTopR && InnerPoints.empty())
  {
    outerBottomR = outerTopR = one;
  }

  vtkSmartPointer<vtkPolyData> tmpLayer =
    CreateLayer(1.0, innerBottomR, outerBottomR, innerTopR, outerTopR, innerRes, outerRes);
  if (this->GenerateNormals)
  {
    vtkNew<vtkPolyDataNormals> normals;
    normals->SetInputDataObject(tmpLayer);
    normals->ComputePointNormalsOn();
    normals->Update();
    return normals->GetOutput();
  }
  return tmpLayer;
}

vtkSmartPointer<vtkPolyData> vtkCmbLayeredConeSource::CreateBoundaryLayer(double thickness, int l)
{
  int innerRes = 0;
  int outerRes = 0;
  double* innerBottomR = nullptr;
  double* innerTopR = nullptr;
  innerBottomR = this->LayerRadii[l].BaseRadii;
  innerTopR = this->LayerRadii[l].TopRadii;
  outerRes = innerRes = this->LayerRadii[l].Resolution;

  double outerBottomR[] = { innerBottomR[0] + thickness, innerBottomR[1] + thickness };
  double outerTopR[] = { innerTopR[0] + thickness, innerTopR[1] + thickness };

  vtkSmartPointer<vtkPolyData> tmpLayer =
    CreateLayer(1.0, innerBottomR, outerBottomR, innerTopR, outerTopR, innerRes, outerRes, true);
  if (this->GenerateNormals)
  {
    vtkNew<vtkPolyDataNormals> normals;
    normals->SetInputDataObject(tmpLayer);
    normals->ComputePointNormalsOn();
    normals->Update();
    return normals->GetOutput();
  }
  return tmpLayer;
}

namespace
{
void Upsample(vtkPoints* points, double* pt0, double* pt1, int number)
{
  double tmpPt[] = { pt0[0], pt0[1], pt0[2] };
  /*int id =*/points->InsertNextPoint(tmpPt);
  double d[] = { pt1[0] - pt0[0], pt1[1] - pt0[1] };
  for (int i = 1; i < (number - 1); ++i)
  {
    double r = static_cast<double>(i) / (number - 1.0);
    tmpPt[0] = pt0[0] + r * d[0];
    tmpPt[1] = pt0[1] + r * d[1];
    /*id =*/points->InsertNextPoint(tmpPt);
  }
}

class GeneratePoints
{
public:
  GeneratePoints(int res)
  {
    bool rect = false;
    if (res == 4)
    {
      res = 8;
      rect = true;
    }
    multX.resize(res);
    multY.resize(res);
    if (rect)
    {
      multX[7] = -1;
      multY[7] = 0;
      multX[6] = -1;
      multY[6] = 1;
      multX[5] = 0;
      multY[5] = 1;
      multX[4] = 1;
      multY[4] = 1;
      multX[3] = 1;
      multY[3] = 0;
      multX[2] = 1;
      multY[2] = -1;
      multX[1] = 0;
      multY[1] = -1;
      multX[0] = -1;
      multY[0] = -1;
    }
    else
    {
      double angle = 2.0 * 3.141592654 / res;
      for (int j = 0; j < res; j++)
      {
        multX[j] = cos(j * angle);
        multY[j] = sin(j * angle);
      }
    }
  }
  int usedResolution() { return static_cast<int>(multX.size()); }
  void AddPoints(vtkPoints* points, double h, double* r, double shift)
  {
    double point[] = { 0, 0, h };
    int res = usedResolution();
    for (int j = 0; j < res; j++)
    {
      compPt(j, point, r, shift);
      points->InsertNextPoint(point);
    }
  }
  void AddPointsUpsampled(vtkPoints* points, double h, double* r, int maxRes, double shift)
  {
    double point[] = { 0, 0, h };
    double prevPoint[] = { 0, 0, h };
    int res = usedResolution();
    if (res == 0)
      return;
    int ptsPerSide = maxRes / res + 1;
    assert(maxRes % res == 0); //right now only handle res that are multiples of each other
    for (int j = 1; j < res; j++)
    {
      compPt(j - 1, prevPoint, r, shift);
      compPt(j, point, r, shift);
      Upsample(points, prevPoint, point, ptsPerSide);
    }
    compPt(res - 1, prevPoint, r, shift);
    compPt(0, point, r, shift);
    Upsample(points, prevPoint, point, ptsPerSide);
  }

protected:
  std::vector<double> multX;
  std::vector<double> multY;
  void compPt(int i, double* point, double* r, double shift)
  {
    point[0] = (r[0] + shift) * multX[i];
    point[1] = (r[1] + shift) * multY[i];
  }
};
}; // namespace

void vtkCmbLayeredConeSource::TriangulateEnd(
  const int innerRes,
  const int outerRes,
  bool forceDelaunay,
  vtkCellArray* cells,
  vtkPoints* fullPoints)
{
  vtkIdType pts[32];
  const int offset = outerRes + innerRes;
  const bool has_hole = innerRes != 0;
  if (!forceDelaunay && has_hole && outerRes == innerRes && outerRes < 32)
  {
    for (int i = 0; i < innerRes; ++i)
    {
      pts[0] = i;
      pts[3] = (i + 1) % innerRes;
      pts[2] = (i + 1) % innerRes + innerRes;
      pts[1] = i + innerRes;
      cells->InsertNextCell(4, pts);
    }
    for (int i = 0; i < innerRes; ++i)
    {
      pts[0] = i + offset;
      pts[3] = (i + 1) % innerRes + offset;
      pts[2] = (i + 1) % innerRes + innerRes + offset;
      pts[1] = i + innerRes + offset;
      cells->InsertNextCell(4, pts);
    }
  }
  else if (!forceDelaunay && !has_hole && outerRes < 32)
  {
    for (int i = 0; i < outerRes; ++i)
    {
      pts[outerRes - 1 - i] = i;
    }
    cells->InsertNextCell(outerRes, pts);
    for (int i = 0; i < outerRes; ++i)
    {
      pts[outerRes - 1 - i] = i + offset;
    }
    cells->InsertNextCell(outerRes, pts);
  }
  else
  {
    vtkPoints* points = vtkPoints::New();
    // Create a cell array to store the polygon in
    vtkSmartPointer<vtkCellArray> aCellArray = vtkSmartPointer<vtkCellArray>::New();

    // Define a polygonal hole with a clockwise polygon
    vtkSmartPointer<vtkPolygon> aPolygon = vtkSmartPointer<vtkPolygon>::New();
    for (int i = 0; i < outerRes; ++i)
    {
      double* tmppt = fullPoints->GetPoint(i);
      points->InsertNextPoint(tmppt[0], tmppt[1], 0);
    }
    for (int i = outerRes; i < offset; ++i)
    {
      double* tmppt = fullPoints->GetPoint(i);
      points->InsertNextPoint(tmppt[0], tmppt[1], 0);
      aPolygon->GetPointIds()->InsertNextId(offset - (i - outerRes) - 1);
    }

    vtkSmartPointer<vtkPolyData> aPolyData = vtkSmartPointer<vtkPolyData>::New();
    aPolyData->SetPoints(points);

    aCellArray->InsertNextCell(aPolygon);
    vtkSmartPointer<vtkPolyData> boundary = vtkSmartPointer<vtkPolyData>::New();
    boundary->SetPoints(aPolyData->GetPoints());
    boundary->SetPolys(aCellArray);
    vtkSmartPointer<vtkDelaunay2D> delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
    delaunay->SetTolerance(0.0);

    delaunay->SetInputData(aPolyData);
    delaunay->SetSourceData(boundary);

    delaunay->Update();

    vtkPolyData* pd = delaunay->GetOutput();

    vtkCellArray* tri = pd->GetPolys();
    tri->InitTraversal();

    vtkIdType npts;
    const vtkIdType* tmppts = nullptr;
    while (tri->GetNextCell(npts, tmppts))
    {
      assert(npts == 3);
      pts[0] = tmppts[0];
      pts[1] = tmppts[2];
      pts[2] = tmppts[1];
      cells->InsertNextCell(3, pts);
    }

    tri->InitTraversal();
    while (tri->GetNextCell(npts, tmppts))
    {
      for (vtkIdType j = 0; j < npts; ++j)
        pts[j] = tmppts[j] + offset;
      cells->InsertNextCell(npts, pts);
    }
    points->Delete();
  }
}

vtkSmartPointer<vtkPolyData> vtkCmbLayeredConeSource::CreateLayer(
  double h,
  double* innerBottomR,
  double* outerBottomR,
  double* innerTopR,
  double* outerTopR,
  int innerRes,
  int outerRes,
  bool lines)
{
  if (outerTopR == nullptr || outerBottomR == nullptr)
    return nullptr;
  if (outerRes == 0)
    return nullptr;
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->Allocate();

  vtkPoints* points = vtkPoints::New();
  vtkCellArray* cells = vtkCellArray::New();
  vtkCellArray* lineCells = vtkCellArray::New();

  points->SetDataTypeToDouble(); //used later during transformation

  vtkIdType pts[5];
  bool forceDelaunay = false;

  if (innerRes == 0 && InnerPoints.empty())
  {
    GeneratePoints gp(outerRes);
    outerRes = gp.usedResolution();
    points->Allocate(outerRes * 2);
    gp.AddPoints(points, 0, outerBottomR, 0);
    gp.AddPoints(points, h, outerTopR, 0);
  }
  else if (innerRes == 0 && !InnerPoints.empty())
  {
    GeneratePoints gp(outerRes);
    outerRes = gp.usedResolution();
    innerRes = static_cast<int>(InnerPoints.size());
    points->Allocate((outerRes + innerRes) * 2);
    gp.AddPoints(points, 0, outerBottomR, 0);
    for (size_t i = 0; i < InnerPoints.size(); ++i)
    {
      points->InsertNextPoint(InnerPoints[i][0], InnerPoints[i][1], 0);
    }
    gp.AddPoints(points, h, outerTopR, 0);
    for (size_t i = 0; i < InnerPoints.size(); ++i)
    {
      points->InsertNextPoint(InnerPoints[i][0], InnerPoints[i][1], h);
    }
    forceDelaunay = true;
  }
  else if (innerRes == outerRes)
  {
    GeneratePoints gp(outerRes);
    innerRes = outerRes = gp.usedResolution();
    points->Allocate(outerRes * 4);
    gp.AddPoints(points, 0, outerBottomR, 0);
    gp.AddPoints(points, 0, innerBottomR, 0.0005);
    gp.AddPoints(points, h, outerTopR, 0);
    gp.AddPoints(points, h, innerTopR, 0.0005);
  }
  else if (outerRes < innerRes)
  {
    double mult = 2.0; //std::floor(innerRes/outerRes);
    GeneratePoints gpO(outerRes);
    outerRes = gpO.usedResolution() * mult;
    GeneratePoints gpI(innerRes);
    innerRes = gpI.usedResolution();
    points->Allocate(((outerRes * mult) + innerRes) * 2);
    gpO.AddPointsUpsampled(points, 0, outerBottomR, outerRes, 0);
    gpI.AddPoints(points, 0, innerBottomR, 0.0005);
    gpO.AddPointsUpsampled(points, h, outerTopR, outerRes, 0);
    gpI.AddPoints(points, h, innerTopR, 0.0005);
  }
  else
  {
    assert(false && "currently, this should not happen");
    /*
    double mult = std::floor(double(outerRes)/innerRes);
    GeneratePoints gpO(outerRes);
    outerRes = gpO.usedResolution();
    GeneratePoints gpI(innerRes);
    innerRes = gpI.usedResolution()*mult;
    points->Allocate((outerRes+(innerRes*mult))*2);
    gpO.AddPoints(points, 0, outerBottomR, 0);
    gpI.AddPointsUpsampled(points, 0, innerBottomR, innerRes, 0.0005);
    gpO.AddPoints(points, h, outerTopR, 0);
    gpI.AddPointsUpsampled(points, h, innerTopR, innerRes,  0.0005);
     */
  }

  //Add bottom calls
  if (GenerateEnds && !lines)
  {
    TriangulateEnd(innerRes, outerRes, forceDelaunay, cells, points);
  }
  /*else if(lines)
  {
    int offset = outerRes+innerRes;
    for(int i = 0; i < outerRes; ++i)
    {
      pts[0] = i;
      pts[1] = (i+1) % outerRes;
      lineCells->InsertNextCell(2, pts);
      pts[0] = (i+1) % outerRes + offset;
      pts[1] = i + offset;
      lineCells->InsertNextCell(2, pts);
    }
  }*/
  //Outer Wall;
  if (!lines)
  {
    int offset = outerRes + innerRes;
    for (int i = 0; i < outerRes; ++i)
    {
      pts[0] = i;
      pts[1] = (i + 1) % outerRes;
      pts[2] = (i + 1) % outerRes + offset;
      pts[3] = i + offset;
      cells->InsertNextCell(4, pts);
    }
  }
  else
  {
    int offset = outerRes + innerRes;
    for (int i = 0; i < outerRes; ++i)
    {
      pts[0] = i;
      pts[1] = (i + 1) % outerRes;
      pts[2] = (i + 1) % outerRes + offset;
      pts[3] = i + offset;
      lineCells->InsertNextCell(2, pts);
      //lineCells->InsertNextCell(2, pts+1);
      lineCells->InsertNextCell(2, pts + 2);
      pts[0] = i + offset;
      pts[0] = i;
      //lineCells->InsertNextCell(2, pts);
    }
    /*int offset = outerRes+innerRes;
    for(int i = 0; i < outerRes; ++i)
    {
      pts[0] = i;
      pts[1] = (i+1) % outerRes;
      pts[2] = (i+1) % outerRes + offset;
      pts[3] = i + offset;
      cells->InsertNextCell(4, pts);
    }*/
  }
  //Inner Wall
  if (!lines)
  {
    int offset = outerRes + innerRes;
    for (int i = 0; i < innerRes; ++i)
    {
      pts[0] = i + outerRes;
      pts[3] = (i + 1) % innerRes + outerRes;
      pts[2] = (i + 1) % innerRes + outerRes + offset;
      pts[1] = i + outerRes + offset;
      cells->InsertNextCell(4, pts);
    }
  }

  polyData->SetPoints(points);
  polyData->SetPolys(cells);
  if (lines)
  {
    polyData->SetLines(lineCells);
  }
  else
  {
  }

  points->Delete();
  cells->Delete();
  lineCells->Delete();

  return polyData;
}

void vtkCmbLayeredConeSource::addInnerPoint(double x, double y)
{
  std::vector<double> pt(2);
  pt[0] = x;
  pt[1] = y;
  InnerPoints.push_back(pt);
}
