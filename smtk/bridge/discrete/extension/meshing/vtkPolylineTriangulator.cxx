//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkPolylineTriangulator.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkVector.h"
#include "vtkXMLPolyDataWriter.h"

#include "vtkObjectFactory.h"
#include "smtk/bridge/discrete/extension/meshing/vtkCMBMeshServerLauncher.h"
#include "smtk/bridge/discrete/extension/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "smtk/bridge/discrete/extension/meshing/vtkCMBTriangleMesher.h"

#include <map>
#include <sstream>

typedef vtkVector2d vec2d;
typedef vtkVector3d vec3d;

namespace smtk {
  namespace bridge {
    namespace discrete {

struct FacetLoops
{
  vtkSmartPointer<vtkPoints> Points;
  vec3d BasePoint;
  vec3d Normal;
  vec3d Tangent;
  vec3d Binormal;
  // The following vectors are all the same length
  // (one entry per loop of the facet):
  std::vector<vtkIdType*> Conn;
  std::vector<vtkIdType> Length;
  std::vector<vtkIdType> PolygonId;
  std::vector<vtkIdType> OriginalId;
  std::vector<int> Container;
  std::vector<int> IsHole;
  std::vector<std::vector<vec2d> > Projected;
};

// Map from facet idx to loops that compose it.
typedef std::map<vtkIdType,FacetLoops> FacetSourceType;

/*
template<typename T, int Size>
ostream& operator << (ostream& os, const vtkTuple<T,Size>& vec)
{
  for (int i = 0; i < Size; ++i)
    {
    os << vec[i];
    if (i < Size - 1)
      {
      os << " ";
      }
    }

  return os;
}
*/

// -----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPolylineTriangulator);
vtkCxxSetObjectMacro(vtkPolylineTriangulator,Launcher,vtkCMBMeshServerLauncher);

vtkPolylineTriangulator::vtkPolylineTriangulator()
{
  this->ModelFaceArrayName = NULL;
  this->Launcher = NULL;
  // port 0: polylines describing facet contours
  // port 1: facet hole points
  this->SetNumberOfInputPorts(2);
}

vtkPolylineTriangulator::~vtkPolylineTriangulator()
{
  this->SetModelFaceArrayName(NULL);
  this->SetLauncher(NULL);
}

void vtkPolylineTriangulator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelFaceArrayName: " << this->ModelFaceArrayName << "\n";
}

template<typename T>
struct vtkPolylineTriangulatorFacetFromPolyArray
{
  vtkIdType operator () (vtkIdType poly) const
    {
    return this->Array->GetValue(poly);
    }
  T* Array;
};

struct vtkPolylineTriangulatorOneFacet
{
  vtkIdType operator () (vtkIdType) const
    {
    return 0;
    }
};

struct vtkPolylineTriangulatorOneFacetPerPoly
{
  vtkIdType operator () (vtkIdType poly) const
    {
    return poly;
    }
};

static bool EstimateNormal(
  vtkPoints* pts, vtkIdType npts, vtkIdType* conn, vec3d& norm)
{
  if (npts < 3)
    {
    return false;
    }

  vec3d x[3];
  vec3d y[2];
  pts->GetPoint(conn[0], x[0].GetData());
  pts->GetPoint(conn[1], x[1].GetData());
  vtkMath::Subtract(x[1].GetData(), x[0].GetData(), y[0].GetData());
  vtkMath::Normalize(y[0].GetData());
  for (vtkIdType i = 2; i < npts; ++ i)
    {
    pts->GetPoint(conn[i], x[i % 3].GetData());
    vtkMath::Subtract(
      x[i % 3].GetData(), x[(i + 2) % 3].GetData(),
      y[(i - 1) % 2].GetData());
    vtkMath::Normalize(y[(i - 1) % 2].GetData());
    vtkMath::Cross(y[0].GetData(), y[1].GetData(), norm.GetData());
    if (vtkMath::Normalize(norm.GetData()) > 1e-8)
      {
      return true;
      }
    }
  // No sequence of 3 points along loop had a well-defined normal.
  return false;
}

bool InitializeFacet(
  vtkPoints* pts, vtkIdType faceId, FacetLoops& facet, vtkIdType npts, vtkIdType* conn)
{
  (void)faceId;
  facet.Points = pts;
  if (!EstimateNormal(pts, npts, conn, facet.Normal))
    {
    return false;
    }
  pts->GetPoint(conn[0], facet.BasePoint.GetData());
  vtkMath::Perpendiculars(
    facet.Normal.GetData(),
    facet.Tangent.GetData(),
    facet.Binormal.GetData(),
    0.);
  //cout << "Facet " << faceId << " base " << facet.BasePoint << " x " << facet.Tangent << " y " << facet.Binormal << "\n";
  return true;
}

static vec2d ProjectPointToPlane(
  const vec3d& pt,
  const vec3d& basePt,
  const vec3d& normal, const vec3d& tangent, const vec3d& binormal)
{
  vec3d delta; // pt - basePt
  vtkMath::Subtract(pt.GetData(), basePt.GetData(), delta.GetData());
  vec3d outOfPlane(normal); // becomes vector from translated pt to
  vtkMath::MultiplyScalar(outOfPlane.GetData(), delta.Dot(normal));
  vec3d pip; // point in the plane
  vtkMath::Subtract(delta.GetData(), outOfPlane.GetData(), pip.GetData());
  vec2d projection;
  projection[0] = pip.Dot(tangent);
  projection[1] = pip.Dot(binormal);
  return projection;
}

void ProjectLoop(vtkIdType vtkNotUsed(modelFace), FacetLoops& loop)
{
  vec3d pt;
  int numPts = static_cast<int>(loop.Length.back());
  for (int i = 0; i < numPts; ++i)
    {
    loop.Points->GetPoint(loop.Conn.back()[i], pt.GetData());
    loop.Projected.back().push_back(
      ProjectPointToPlane(
        pt, loop.BasePoint, loop.Normal, loop.Tangent, loop.Binormal));
    }
}

void DumpLoop(vtkIdType facetId, FacetLoops& facet)
{
  cout
    << "Facet: " << facetId << " is:\n"
    << "  BasePoint: " << facet.BasePoint << "\n"
    << "  Normal: " << facet.Normal << "\n"
    << "  Tangent: " << facet.Tangent << "\n"
    << "  Binormal: " << facet.Binormal << "\n"
    << "  Loops:\n";
  for (vtkIdType i = 0; i < static_cast<int>(facet.Conn.size()); ++i)
    {
    cout
      << "    " << i << ":\n"
      << "       Conn:";
    for (vtkIdType j = 0; j < facet.Length[i]; ++j)
      {
      cout << " " << facet.Conn[i][j];
      }
    cout << "\n" << "       Container: " << facet.Container[i];
    cout << "\n" << "       PolygonId: " << facet.PolygonId[i];
    cout << "\n" << "       IsHole: " << (facet.IsHole[i] ? "T" : "F");
    cout << "\n" << "       Projected:\n";
    for (vtkIdType j = 0; j < facet.Length[i]; ++j)
      {
      cout << "          " << facet.Projected[i][j] << "\n";
      }
    }
  cout << "-----\n";
}

void AddLoopToFacet(
  vtkPoints* pts, FacetSourceType& facets,
  vtkIdType npts, vtkIdType* conn, vtkIdType fid, vtkIdType c)
{
  //cout << "Add loop " << c << " to " << fid << "\n";
  FacetSourceType::iterator it = facets.find(fid);
  if (it == facets.end())
    {
    std::pair<vtkIdType,FacetLoops> blank;
    blank.first = fid;
    facets.insert(blank);
    it = facets.find(fid);
    if (!InitializeFacet(pts, fid, it->second, npts, conn))
      {
      cerr << "Bad loop " << c << " (facet " << fid << ")\n";
      facets.erase(it);
      return;
      }
    }
  it->second.Conn.push_back(conn);
  it->second.Length.push_back(npts);
  it->second.Container.push_back(-1);
  it->second.PolygonId.push_back(-3); // -1 is reserved for holes, 1->\infty for polygons
  it->second.IsHole.push_back(false);
  it->second.OriginalId.push_back(c);
  std::vector<vec2d> blank;
  it->second.Projected.push_back(blank);
  ProjectLoop(fid, it->second);
}

template<typename T>
void GroupLoops(vtkPolyData* pdIn, FacetSourceType& facets, T& poly2facet)
{
  vtkPoints* pts = pdIn->GetPoints();
  vtkCellArray* polylines = pdIn->GetLines();
  vtkIdType c = pdIn->GetNumberOfVerts();
  vtkIdType npts;
  vtkIdType* conn;
  for (polylines->InitTraversal(); polylines->GetNextCell(npts, conn); ++c)
    {
    // Some polyfiles have line segments in them. We do not want to
    // triangulate those, so ignore "short" cells.
    // (Triangles have 4 points as initial vertex is repeated.)
    if (npts >= 4)
      {
      AddLoopToFacet(pts, facets, npts, conn, poly2facet(c), c);
      }
    }
}

// -----------------------------------------------------------------------------
#if 1
/*
  The function below was based on an original version retrieved from
  http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
  on June 18, 2013 and is Copyright (c) 1970-2003, Wm. Randolph Franklin:

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

    Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimers.
    Redistributions in binary form must reproduce the above copyright
      notice in the documentation and/or other materials provided with
      the distribution.
    The name of W. Randolph Franklin may not be used to endorse or promote
      products derived from this Software without specific prior
      written permission.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
int IsPointInPolygon(const vec2d& P, const std::vector<vec2d>& V)
{
  int n = static_cast<int>(V.size());
  int i, j, c = 0;
  for (i = 0, j = n-1; i < n; j = i++)
    {
    if ( ((V[i][1]>P[1]) != (V[j][1]>P[1])) &&
      (P[0] < (V[j][0]-V[i][0]) * (P[1]-V[i][1]) / (V[j][1]-V[i][1]) + V[i][0]) )
       c = !c;
    }
  return c;
}
#else
// -----------------------------------------------------------------------------
// The code below is adapted from http://geomalgorithms.com/a03-_inclusion.html
// as retrieved on June 18, 2013. The next two functions below (and only those)
// are copyright and licensed under the following terms:
//
// Copyright 2000 softSurfer, 2012 Dan Sunday
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.


// IsLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2  on the line
//            <0 for P2  right of the line
//    See: Algorithm 1 "Area of Triangles and Polygons"
inline int IsLeft(
  const vec2d& P0, const vec2d& P1, const vec2d& P2 )
{
  return ( (P1[0] - P0[0]) * (P2[1] - P0[1])
            - (P2[0] -  P0[0]) * (P1[1] - P0[1]) );
}

// IsPointInPolygon(): winding number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
//      Return:  wn = the winding number (=0 only when P is outside)
// NB: This only works for correctly-oriented polygons which wind
//     in a counter-clockwise direction. Clockwise windings are ignored.
int IsPointInPolygon(const vec2d& P, const std::vector<vec2d>& V)
{
  int n = static_cast<int>(V.size());
  int    wn = 0;    // the  winding number counter

  // loop through all edges of the polygon
  for (int i = 0; i < n; ++i)
    { // edge from V[i] to  V[i+1]
    if (V[i][1] <= P[1])
      { // start y <= P.y
      if (V[i+1][1] > P[1])
        { // an upward crossing
        if (IsLeft(V[i], V[i+1], P) > 0)
          {// P left of  edge
          ++wn; // have  a valid up intersect
          }
        }
      }
    else
      { // start y > P[1] (no test needed)
      if (V[i+1][1] <= P[1])
        { // a downward crossing
        if (IsLeft(V[i], V[i+1], P) < 0)
          { // P right of  edge
          --wn; // have  a valid down intersect
          }
        }
      }
    }
  return wn;
}
#endif
// -----------------------------------------------------------------------------

template<typename T>
T Add(const T& a, const T& b)
{
  T c;
  for (int i = 0; i < c.GetSize(); ++i)
    {
    c[i] = a[i] + b[i];
    }
  return c;
}

template<typename T>
T Subtract(const T& a, const T& b)
{
  T c;
  for (int i = 0; i < c.GetSize(); ++i)
    {
    c[i] = a[i] - b[i];
    }
  return c;
}

template<typename T>
void MultiplyScalar(T& a, double s)
{
  for (int i = 0; i < a.GetSize(); ++i)
    {
    a[i] *= s;
    }
}

template<typename T>
T MultiplyAdd(const T& a, const T& b, double s)
{
  T c;
  for (int i = 0; i < c.GetSize(); ++i)
    {
    c[i] = a[i] + s * b[i];
    }
  return c;
}

static vec3d LiftPoint(
  const vec3d& pt2d,
  const vec3d& basePoint,
  const vec3d& xAxis,
  const vec3d& yAxis)
{
  vec3d pt = basePoint;
  pt = MultiplyAdd(pt, xAxis, pt2d[0]);
  pt = MultiplyAdd(pt, yAxis, pt2d[1]);
  return pt;
}


static double PointLineSegmentSquaredDistance(
  const vec2d& pt, const vec2d& a, const vec2d& b)
{
  vec2d dir = Subtract(b, a);
  double len2 = dir.SquaredNorm();
  vec2d delta = Subtract(pt, a);
  if (len2 > 0.)
    { // a & b are not coincident.
    double t = delta.Dot(dir) / len2;
    if (t < 0.)
      {
      // Do nothing: delta.Dot(delta) is distance from pt to a.
      }
    else if (t > 1.)
      {
      delta = Subtract(pt, b);
      // Now delta.Dot(delta) is distance from pt to b.
      }
    else
      {
      // Take delta to be vector from pt to its projection.
      delta = Subtract(pt, MultiplyAdd(a, dir, t));
      }
    }
  return delta.Dot(delta);
}

static double PointLoopSquaredDistance(
  const vec2d& pt2d, const std::vector<vec2d>& loop, int& s)
{
  int numEdges = static_cast<int>(loop.size()) - 1;
  s = -1;
  double dBest = vtkMath::Inf();
  for (int i = 0; i < numEdges; ++i)
    {
    double d2 = PointLineSegmentSquaredDistance(pt2d, loop[i], loop[i+1]);
    if (d2 < dBest)
      {
      s = i;
      dBest = d2;
      }
    }
  return dBest;
}

// when sense == +1, find the closest loop that *contains* pt
// when sense == -1, find the closest loop for which pt is *exterior*.
// when sense ==  0, find the closest loop on either side.
//
// when except == -1, try *every* loop in loops.
// when except >=  0, try loops other than \a except.
// used to find a loop that contains a point *on* another loop
static int FindClosestLoop(
  FacetLoops& loops, const vec3d& pt,
  int& segment, double& distance2, int sense = +1, int except = -1)
{
  vec2d pt2d = ProjectPointToPlane(
    pt, loops.BasePoint, loops.Normal, loops.Tangent, loops.Binormal);
  //cout << "FindClosestLoop to pt: " << pt << " projected: " << pt2d << "\n";
  int numLoops = static_cast<int>(loops.Conn.size());
  int iBest = -1;
  segment = -1;
  distance2 = vtkMath::Inf();
  for (int i = 0; i < numLoops; ++i)
    {
    if (i == except)
      {
      continue;
      }
    int winding = (sense == 0 ? 0 : IsPointInPolygon(pt2d, loops.Projected[i]));
    //cout << "  loop " << i << " winding " << winding;
    if ((sense > 0 && winding > 0) || (sense <= 0 && winding == 0))
      {
      int s;
      double d2 = PointLoopSquaredDistance(pt2d, loops.Projected[i], s);
      //cout << "  d2 " << d2 << "\n";
      if (d2 < distance2)
        {
        distance2 = d2;
        segment = s;
        iBest = i;
        }
      }
    //cout << "\n";
    }
  return iBest;
}

static void AssignHoles(
  vtkPoints* holes, vtkIdTypeArray* holeFaceGroups, FacetSourceType& facets)
{
  vtkIdType* facetIds = holeFaceGroups->GetPointer(0);
  for (vtkIdType i = 0; i < holes->GetNumberOfPoints(); ++i)
    {
    vec3d pt;
    holes->GetPoint(i, pt.GetData());
    FacetLoops& facet(facets[facetIds[i]]);
    int segment;
    double distance2;
    int loop = FindClosestLoop(
      facet, pt, segment, distance2, /*loop contains pt*/ +1, /*except*/ -1);
    /*
    cout
      << "Hole " << i << " container " << loop
      << " segment " << segment << " d2 " << distance2 << "\n";
      */
    if (loop >= 0)
      {
      facet.IsHole[loop] = true;
      }
    }
}

static void DiscoverNestings(FacetLoops& facet)
{
  int numLoops = static_cast<int>(facet.Projected.size());
  for (int i = 0; i < numLoops; ++i)
    {
    int segment;
    double distance2;
    vec3d pt;
    facet.Points->GetPoint(facet.Conn[i][0], pt.GetData());
    int loop = FindClosestLoop(
      facet, pt, segment, distance2, /*loop contains pt*/ +1, /*except*/ i);
    if (loop >= 0)
      {
      facet.Container[i] = loop;
      }
    }
}

static void DiscoverNestings(FacetSourceType& facets)
{
  for (FacetSourceType::iterator it = facets.begin(); it != facets.end(); ++it)
    {
    DiscoverNestings(it->second);
    }
}

static void RecursivelyAssignPolygonId(
  FacetLoops& facet, vtkIdType loopNum, vtkIdType modelFaceId, vtkIdType& nextPolygon, bool& first)
{
  int numLoops = static_cast<int>(facet.Conn.size());
  if (loopNum < 0 || loopNum > numLoops)
    {
    return;
    }
  if (facet.PolygonId[loopNum] != -3)
    { // already visited this loop:
    return;
    }
  if (facet.Container[loopNum] < 0)
    { // a top level loop:
    facet.PolygonId[loopNum] = first ? modelFaceId + 1 : nextPolygon++;
    first = false;
    return;
    }
  // assign parents, then self:
  RecursivelyAssignPolygonId(facet, facet.Container[loopNum], modelFaceId, nextPolygon, first);
  facet.PolygonId[loopNum] = facet.IsHole[loopNum] ? -1 : ++nextPolygon;
}

// This is called instead of TriangulateFacet for the case where
// \a facet has exactly one loop of 3 distinct vertices (4 vertices
// including the repeated vertex at the front+back of facet.Conn)
static void TriangulateATriangle(
  vtkPolyData* pdIn,
  vtkIdType modelFaceId, vtkIdType& vtkNotUsed(nextPolygon), FacetLoops& facet,
  vtkPolyData* pdOut, vtkIdTypeArray* pedigreeIds)
{
  if (facet.Length.size() != 1 || facet.Length[0] != 4)
    {
    return;
    }
  // Add one triangle to the output mesh along with
  // a pedigree ID indicating the generating facet.
  vtkCellArray* dstPolys = pdOut->GetPolys();
  vtkCellData* srcAttr = pdIn->GetCellData();
  vtkCellData* dstAttr = pdOut->GetCellData();
  vtkIdType* conn = facet.Conn[0];
  vtkIdType npts = facet.Length[0] - 1;
  vtkIdType dstCellId = dstPolys->InsertNextCell(npts, conn);
  dstAttr->CopyData(srcAttr, facet.OriginalId[0], dstCellId);
  pedigreeIds->InsertNextValue(modelFaceId);
}

static void TriangulateFacet(
  vtkPolyData* pdIn,
  vtkIdType modelFaceId, vtkIdType& nextPolygon, FacetLoops& facet,
  vtkPolyData* pdOut, vtkIdTypeArray* pedigreeIds, vtkCMBMeshServerLauncher* lau)
{
  vtkNew<vtkPolyData> fpoly;
  //vtkNew<vtkCellArray> verts;
  vtkNew<vtkCellArray> lines;
  vtkNew<vtkPoints> projPts;
  vtkNew<vtkCMBPrepareForTriangleMesher> prepr;
  fpoly->SetPoints(projPts.GetPointer());
  //fpoly->SetVerts(verts.GetPointer());
  fpoly->SetLines(lines.GetPointer());
  prepr->SetPolyData(fpoly.GetPointer());

  // For adding projected points to projPts, avoiding duplicates:
  std::map<vtkIdType,vtkIdType> fwdMap;
  std::map<vtkIdType,vtkIdType> bckMap;

  // Prepare to translate the facet loops into a new polydata.
  // Because we only store the container of each loop (and not the
  // list of loops contained by a given loop), we must make an
  // additional pass through the loops to ensure that we use the
  // same consistent PolygonId for each edge neighbor we list.
  vtkIdType srcCellId = -1;
  int numLoops = static_cast<int>(facet.Conn.size());
  vtkIdType numArcs = 0;
  for (int i = 0; i < numLoops; ++i)
    {
    numArcs += facet.Length[i] - 1; // Don't count repeated vertex in Conn[i]
    }
  //prepr->SetNumberOfArcs(numArcs);
  //cout << numArcs << " arcs in " << numLoops << " loops\n";
  //prepr->SetNumberOfLoops(numLoops);
  //prepr->SetNumberOfCells(numArcs + pdIn->GetNumberOfPoints());
  prepr->InitializeNewMapInfo();
  for (int i = 0; i < numLoops; ++i)
    {
    bool useModelFaceId = false;
    RecursivelyAssignPolygonId(facet, i, modelFaceId, nextPolygon, useModelFaceId);
    // I. Assign a facet ID to each loop as required.
    // Multiple top-level loops may exist and only one will be assigned
    // the current facet's ID. Other top-level loops get new IDs.
    // Any loop that is not a top-level loop and not a hole will also
    // get a new ID.
    if (facet.Container[i] < 0)
      {
      srcCellId = facet.OriginalId[i];
      prepr->AddLoop(facet.PolygonId[i], -1);
      //cout << "Loop " << i << " polygons " << facet.PolygonId[i] << ", -1  **\n";
      }
    else
      {
      prepr->AddLoop(facet.PolygonId[i], facet.PolygonId[facet.Container[i]]);
      //cout << "Loop " << i << " polygons " << (facet.IsHole[i] ? -1 : facet.PolygonId[i]) << ", " << facet.PolygonId[facet.Container[i]] << "\n";
      }


    // II. Add projected facet vertices to the new polydata.
    // Update maps used to translate vertices to/from the new
    // polydata and the input polydata.
    int numEdges = facet.Length[i] - 1; // last pt == first pt
    for (int e = 0; e < numEdges; ++e)
      {
      vtkIdType pidx = facet.Conn[i][e];
      if (fwdMap.find(pidx) == fwdMap.end())
        {
        vtkIdType nextNewPt =
          projPts->InsertNextPoint(
            facet.Projected[i][e][0], facet.Projected[i][e][1], 0.);
        //verts->InsertNextCell(1, &nextNewPt);
        //vtkIdType nodeId = prepr->AddNode(nextNewPt);
        fwdMap[pidx] = nextNewPt;
        bckMap[nextNewPt] = pidx;
        //vec3d xx;
        //pdIn->GetPoint(pidx, xx.GetData());
        //cout << "  map " << pidx << xx << " to " << nextNewPt << facet.Projected[i][e] /* << " as " << nodeId */ << "\n";
        }
      }
    }

  // Now create edge cells for all the edges in all the loops of the facet.
  vtkIdType arcId = 0;
  for (int i = 0; i < numLoops; ++i)
    {
    int numEdges = facet.Length[i] - 1; // last pt == first pt

    for (int e = 0; e < numEdges; ++e, ++arcId)
      {
      vtkIdType arcStart = lines->GetInsertLocation(-1);
      vtkIdType conn[2] = { fwdMap[facet.Conn[i][e]], fwdMap[facet.Conn[i][e + 1]] };
      /*vtkIdType cell = */lines->InsertNextCell(2, &conn[0]);
      vtkIdType arcEnd = lines->GetInsertLocation(-1);
      prepr->AddArc(arcStart, arcEnd - arcStart, arcId, i, facet.Container[i], conn[0], conn[1]);
      }
    }
  // Add arrays to fpoly:
  prepr->FinalizeNewMapInfo();

#if 0
  vtkNew<vtkXMLPolyDataWriter> wri;
  wri->SetInputDataObject(fpoly.GetPointer());
  std::ostringstream fname;
  fname << "FacetOutline-" << modelFaceId << ".vtp";
  wri->SetFileName(fname.str().c_str());
  wri->SetDataModeToAscii();
  wri->Write();
#endif // 1

  vtkNew<vtkCMBTriangleMesher> msh;
  msh->SetLauncher(lau);
  msh->SetInputDataObject(fpoly.GetPointer());
  msh->SetMaxAreaMode(vtkCMBTriangleMesher::NoMaxArea);
  msh->SetPreserveEdgesAndNodes(true);
  msh->SetPreserveBoundaries(true);
  msh->VerboseOutputOn();
  msh->Update();

#if 0
  vtkNew<vtkXMLPolyDataWriter> mwr;
  mwr->SetInputConnection(msh->GetOutputPort());
  std::ostringstream mfname;
  mfname << "Facet-" << modelFaceId << ".vtp";
  mwr->SetFileName(mfname.str().c_str());
  mwr->SetDataModeToAscii();
  mwr->Write();
#endif // 0

  // Now add the triangles to the output mesh along with
  // a pedigree ID indicating the generating facet
  vtkCellArray* srcPolys = msh->GetOutput()->GetPolys();
  vtkPoints* srcPts = msh->GetOutput()->GetPoints();
  vtkCellArray* dstPolys = pdOut->GetPolys();
  vtkPoints* dstPts = pdOut->GetPoints();
  vtkCellData* srcAttr = pdIn->GetCellData();
  vtkCellData* dstAttr = pdOut->GetCellData();
  vtkDataArray* globIds = pdOut->GetPointData()->GetGlobalIds();

  // Create a point locator for finding original vertex
  // from list of projected points.
  double diam = msh->GetOutput()->GetLength(); // "diameter" of dataset
  double maxDist = 1e-9 * diam; // search radius for coincident points
  vtkNew<vtkPointLocator> loc;
  loc->SetDataSet(fpoly.GetPointer());
  bool locatorBuilt = false;

  vtkIdType* conn;
  vtkIdType npts;
  for (srcPolys->InitTraversal(); srcPolys->GetNextCell(npts, conn); )
    {
    if (npts < 3 || (npts == 3 && (conn[0] == conn[1] || conn[1] == conn[2] || conn[2] == conn[0])))
      continue;
    for (vtkIdType i = 0; i < npts; ++i)
      {
#if 0
      // WARNING WARNING!!!  Triangle is reordering the input point IDs but there
      //                     is no mapping reported back!!!
      std::map<vtkIdType,vtkIdType>::iterator pit =
        bckMap.find(conn[i]);
      if (pit == bckMap.end())
#endif
      vtkIdType searchPt;
        {
        // TODO: We REALLY want to avoid this as neighboring model faces
        //       will no longer have proper connectivity for region discovery
        //       once we use a newly-created vertex on the triangulated face.
        //       This is fine if it's interior to the polygon but not OK if
        //       it is on the boundary.
        vec3d pt2;
        srcPts->GetPoint(conn[i], pt2.GetData());
        if (!locatorBuilt)
          {
          loc->BuildLocator();
          locatorBuilt = true;
          }
        double dist2;
        searchPt = loc->FindClosestPointWithinRadius(maxDist, pt2.GetData(), dist2);
        if (searchPt < 0)
          { // Insert new vertex
          vec3d pt = LiftPoint(pt2, facet.BasePoint, facet.Tangent, facet.Binormal);
          vtkIdType newDstPt = dstPts->InsertNextPoint(pt.GetData());
          cout
            << "WARNING: Point " << conn[i]
            << " (" << pt2[0] << ", " << pt2[1] << ") lifted to " << newDstPt
            << " (" << pt[0] << ", " << pt[1] << ", " << pt[2] << ")"
            << " on model face " << modelFaceId
            << ". Region computation may be incorrect.\n";
          if (globIds)
            {
            globIds->InsertNextTuple1(-1);
            }
          bckMap[conn[i]] = newDstPt;
          //pit = bckMap.find(conn[i]);
          searchPt = newDstPt;
          }
        else
          {
          searchPt = bckMap[searchPt];
          /*
          cout
            << "NOTE: Point " << conn[i]
            << " (" << pt2[0] << ", " << pt2[1] << ") maps to " << searchPt
            << " on model face " << modelFaceId << " to within sqrt(" << dist2 << ")\n";
            */
          }
        }
#if 0
      else
        {
        searchPt = pit->second;
        }
#endif
      /*
      vec3d pin;
      vec3d pout;
      pdIn->GetPoint(searchPt, pout.GetData());
      srcPts->GetPoint(conn[i], pin.GetData());
      pin = LiftPoint(pin, facet.BasePoint, facet.Tangent, facet.Binormal);
      cout << "      ";
      cout.width(5); cout << modelFaceId;
      cout.width(5); cout << i;
      cout.width(5); cout << conn[i];
      cout << "  " << pin << " -> ";
      cout.width(5); cout << searchPt;
      cout << pout << " ?\n";
      */
      conn[i] = searchPt; // pit->second;
      }
    if (npts == 3 && (conn[0] == conn[1] || conn[1] == conn[2] || conn[2] == conn[0]))
      continue;
    vtkIdType dstCellId = dstPolys->InsertNextCell(npts, conn);
    dstAttr->CopyData(srcAttr, srcCellId, dstCellId);
    pedigreeIds->InsertNextValue(modelFaceId);
    }

#if 0
  vtkNew<vtkXMLPolyDataWriter> fwr;
  fwr->SetInputDataObject(pdOut);
  std::ostringstream ffname;
  ffname << "UpdatedFacet-" << modelFaceId << ".vtp";
  fwr->SetFileName(ffname.str().c_str());
  fwr->SetDataModeToAscii();
  fwr->Write();
#endif // 0
}

static void TriangulateFacets(
  vtkPolyData* pdIn, FacetSourceType& facets,
  vtkIdTypeArray* mapArray, vtkPolyData* pdOut,
  vtkCMBMeshServerLauncher* lau)
{
  // Copy input points to output
  pdOut->SetPoints(pdIn->GetPoints());
  // For now, do not copy vertices and edges to output
  //pdOut->SetVerts(pdIn->GetVerts());
  //pdOut->SetLines(pdIn->GetLines());

  // NB: Must deep copy if triangle mesher adds any points
  pdOut->GetPointData()->ShallowCopy(pdIn->GetPointData());
  // ... but create new polygons to hold triangles.
  vtkNew<vtkCellArray> outPolys;
  pdOut->SetPolys(outPolys.GetPointer());

  pdOut->GetCellData()->CopyAllOn();
  pdOut->GetCellData()->CopyAllocate(pdIn->GetCellData());

  // Add in facet IDs for existing geometry or create it if
  // none exists.
  vtkNew<vtkIdTypeArray> facetIds;
  facetIds->SetName(mapArray ? mapArray->GetName() : "FacetIndex");

  // Now triangulate each facet and add these triangles to the output

  // nextPolygon holds an ID that may be used to label hypothetical polygons
  // which neighbor loops. Normally the loop ID + 1 (= cell ID + 1 since each
  // cell holds a loop) is used but sometimes a loop must be split into
  // disconnnected components. This variable keeps polygon IDs from being
  // reused.
  //vtkIdType nextPolygon = facets.rbegin()->first + 2;
  for (FacetSourceType::iterator it = facets.begin(); it != facets.end(); ++it)
    {
    vtkIdType nextPolygon = 1;
    if (it->second.Length.size() == 1 && it->second.Length[0] == 4)
      {
      // Don't call a mesher to triangulate a facet containing a single triangle:
      TriangulateATriangle(
        pdIn, it->first, nextPolygon, it->second, pdOut, facetIds.GetPointer());
      }
    else
      {
      // Call a mesher to triangulate the facet:
      TriangulateFacet(
        pdIn, it->first, nextPolygon, it->second,
        pdOut, facetIds.GetPointer(), lau);
      }
    }

  pdOut->GetCellData()->SetPedigreeIds(facetIds.GetPointer());
}

int vtkPolylineTriangulator::FillInputPortInformation(
  int port, vtkInformation* info)
{
  int status = this->Superclass::FillInputPortInformation(port, info);
  if (status && port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

int vtkPolylineTriangulator::RequestData(
    vtkInformation* vtkNotUsed(req),
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  vtkPolyData* pdIn = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData* pdOut = vtkPolyData::GetData(outputVector, 0);

  vtkCMBMeshServerLauncher* lau;
  if (!this->Launcher)
    {
    // Do not call Delete as we want to own the reference:
    this->Launcher = vtkCMBMeshServerLauncher::New();
    }
  lau = this->Launcher;

  // Copy field data from input to output.
  // This is important when the input is a vtkPolyFileReader
  // with the FacetMarksAsCellData set to false as boundary
  // marks will be stored in field data.
  pdOut->GetFieldData()->ShallowCopy(pdIn->GetFieldData());

  FacetSourceType facets;
  vtkIdTypeArray* mapArray =
    this->ModelFaceArrayName ?
      vtkIdTypeArray::SafeDownCast(
        pdIn->GetCellData()->GetArray(this->ModelFaceArrayName)) : NULL;

  // Group polylines into per-facet records
  if (mapArray)
    {
    vtkPolylineTriangulatorFacetFromPolyArray<vtkIdTypeArray> poly2facet;
    poly2facet.Array = mapArray;
    GroupLoops(pdIn, facets, poly2facet);
    }
  else if (this->ModelFaceArrayName && this->ModelFaceArrayName[0])
    { // non-NULL name, but not valid array: one facet
    vtkPolylineTriangulatorOneFacet poly2facet;
    GroupLoops(pdIn, facets, poly2facet);
    }
  else
    { // NULL name: one facet per poly
    vtkPolylineTriangulatorOneFacetPerPoly poly2facet;
    GroupLoops(pdIn, facets, poly2facet);
    }

  // Split hole points out according to the facet they reference
  vtkPolyData* holesIn = this->GetPolyDataInput(1);
  vtkPoints* holePoints = holesIn ? holesIn->GetPoints() : NULL;
  vtkIdTypeArray* holeFaceGroups = holesIn ?
    vtkIdTypeArray::SafeDownCast(
      holesIn->GetPointData()->GetPedigreeIds()) : NULL;
  if (holePoints && holePoints->GetNumberOfPoints())
    {
    if (!holeFaceGroups)
      {
      vtkErrorMacro("Holes exist but do not have vtkIdType pedigree IDs");
      return 0;
      }
    AssignHoles(holePoints, holeFaceGroups, facets);
    }

  DiscoverNestings(facets);
  TriangulateFacets(pdIn, facets, mapArray, pdOut, lau);

  return 1;
}
    } // namespace discrete
  } // namespace bridge
} // namespace smtk
