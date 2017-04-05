//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscoverRegions.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkMinimalStandardRandomSequence.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"
#include "vtkVariant.h"
#include "vtkVector.h"
#include "vtkVectorOperators.h"
#include "vtkXMLPolyDataWriter.h"

#include "vtkObjectFactory.h"

#include "smtk/extension/vtk/meshing/union_find.h"

#include <deque>
#include <map>
#include <set>
#include <sstream>
#include <vector>

typedef vtkVector2d vec2d;
typedef vtkVector3d vec3d;

// A edge's use of a vertex
struct VertexEdgeUse
{
  vtkIdType CellId; // index of edge in dataset
  bool Orientation; // true => face uses positive edge sense for its positive normal side. false => otherwise
  double Angle; // Angle of face (in plane with normal along edge) to first face use of edge.

  VertexEdgeUse(vtkIdType edgeId, bool orient, double angle)
    : CellId(edgeId), Orientation(orient), Angle(angle)
    {
    }
};

// Neighborhood of a vertex.
struct Vertexhood
{
  vec3d XAxis;
  vec3d BasePoint;
  std::vector<VertexEdgeUse> Bordants;
};

class Vertexhoods : public std::map<vtkIdType,Vertexhood>
{
public:
  enum
    {
    Dimension = 2
    };
};

ostream& operator << (ostream& os, Vertexhood& hood)
{
  os << " hood(" << "bpt" << hood.BasePoint <<  " x " << hood.XAxis << " Bordants [";
  for (std::vector<VertexEdgeUse>::iterator it = hood.Bordants.begin(); it != hood.Bordants.end(); ++it)
    {
    os << " cell " << it->CellId << " or " << it->Orientation << " ang " << it->Angle;
    }
  os << ")";
  return os;
};

// A face's use of an edge
struct EdgeFaceUse
{
  vtkIdType CellId; // index of face in dataset
  bool Orientation; // true => face uses positive edge sense for its positive normal side. false => otherwise
  double Angle; // Angle of face (in plane with normal along edge) to first face use of edge.
};

// Neighborhood of an edge.
struct Edgerhood
{
  vec3d Direction;
  vec3d BasePoint;
  // Axes defined by face normal of first entry in Faces.
  // They are used to keep faces ordered properly around edge:
  vec3d XAxis;
  vec3d YAxis;
  std::vector<EdgeFaceUse> Bordants;
};

typedef std::pair<vtkIdType,vtkIdType> EdgePair;

ostream& operator << (ostream& os, const EdgePair& ep)
{
  os << " edg(" << ep.first << "," << ep.second << ")";
  return os;
};

ostream& operator << (ostream& os, Edgerhood& hood)
{
  os << " hood(" << "bpt " << hood.BasePoint << " dir " << hood.Direction <<  " x " << hood.XAxis << " y " << hood.YAxis << " Bordants [";
  for (std::vector<EdgeFaceUse>::iterator it = hood.Bordants.begin(); it != hood.Bordants.end(); ++it)
    {
    os << " cell " << it->CellId << " or " << it->Orientation << " ang " << it->Angle;
    }
  os << ")";
  return os;
}

class Edgerhoods : public std::map<EdgePair,Edgerhood>
{
public:
  enum
    {
    Dimension = 3
    };
};

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

template<int dim>
bool EstimateNormal(vtkPoints* pts, vtkIdType npts, vtkIdType* conn, vec3d& norm);

template<>
bool EstimateNormal<2>(vtkPoints* pts, vtkIdType npts, vtkIdType* conn, vec3d& norm)
{
  if (npts < 2)
    {
    return false;
    }
  vec3d xA;
  vec3d xB;
  pts->GetPoint(conn[0], xA.GetData());
  pts->GetPoint(conn[1], xB.GetData());
  vec3d dir = xB - xA;
  vec3d z(0., 0., 1.);
  norm = dir.Cross(z);
  return norm.Normalize() < 1e-8 ? false : true;
}

template<>
bool EstimateNormal<3>(vtkPoints* pts, vtkIdType npts, vtkIdType* conn, vec3d& norm)
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
    vtkMath::Subtract(x[i % 3].GetData(), x[(i + 2) % 3].GetData(), y[(i - 1) % 2].GetData());
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

static bool SpliceFaceIntoHood(
  vtkIdType faceId, vtkIdType npts, vtkIdType* conn, vtkIdType vtkNotUsed(edge), bool sense,
  Edgerhoods::iterator hood, vtkPoints* pts, double vtkNotUsed(tol))
{
  vec3d norm;
  if (!EstimateNormal<3>(pts, npts, conn, norm))
    {
    return false;
    }

  EdgeFaceUse use;
  use.CellId = faceId;
  use.Orientation = sense;
  if (!sense)
    { // flip the normal if the sense is reversed.
    MultiplyScalar(norm, -1.);
    }
  if (hood->second.Bordants.empty())
    {
    use.Angle = 0.;
    hood->second.XAxis = norm;
    vtkMath::Cross(
      hood->second.Direction.GetData(),
      hood->second.XAxis.GetData(),
      hood->second.YAxis.GetData());
    hood->second.Bordants.push_back(use);
    return true;
    }

  double x = hood->second.XAxis.Dot(norm);
  double y = hood->second.YAxis.Dot(norm);
  use.Angle = atan2(y, x);
  if (use.Angle <= 0.)
    {
    use.Angle += 2. * vtkMath::Pi();
    }
  std::vector<EdgeFaceUse>::iterator it;
  for (it = hood->second.Bordants.begin(); it != hood->second.Bordants.end(); ++it)
    {
    if (it->Angle > use.Angle)
      {
      hood->second.Bordants.insert(it, use);
      return true;
      }
    }
  // insert at end.
  hood->second.Bordants.push_back(use);
  return true;
}

int SpliceEdgeIntoVertex(
  vtkIdType edgeId, vtkIdType& idA, vtkIdType& idB, bool sense, vtkPoints* pts,
  Vertexhoods& hoods, double tol)
{
  vec3d xA;
  vec3d xB;
  pts->GetPoint(idA, xA.GetData());
  pts->GetPoint(idB, xB.GetData());
  vec3d edgeDir = xB - xA;
  std::pair<Vertexhoods::iterator,bool> srch;
  std::pair<vtkIdType,Vertexhood> entry;
  entry.first = idA;
  srch.first = hoods.find(entry.first);
  if (srch.first == hoods.end())
    {
    entry.second.BasePoint = xA;
    entry.second.XAxis = edgeDir;
    if (entry.second.XAxis.Normalize() < tol)
      { // Zero-length vector... this is a degenerate edge.
      return -1;
      }
    srch = hoods.insert(entry);
    srch.first->second.Bordants.push_back(
      VertexEdgeUse(edgeId, sense, 0.));
    }
  else
    {
    vec3d yAxis = srch.first->second.XAxis.Cross(vec3d(0,0,1));
    double angle =
      atan2(
        yAxis.Dot(edgeDir),
        srch.first->second.XAxis.Dot(edgeDir));
    VertexEdgeUse use(edgeId, sense, angle);
    std::vector<VertexEdgeUse>::iterator it;
    for (it = srch.first->second.Bordants.begin(); it != srch.first->second.Bordants.end(); ++it)
      {
      if (it->Angle > use.Angle)
        {
        srch.first->second.Bordants.insert(it, use);
        return 0;
        }
      }
    // insert at end.
    srch.first->second.Bordants.push_back(use);
    }
  return 0;
}

int SpliceCellIntoHood(
  vtkIdType edgeId, vtkIdType* conn, vtkIdType i, vtkIdType npts, vtkPoints* pts,
  Vertexhoods& hoods, double tol)
{
  vtkIdType idA = conn[i];
  vtkIdType idB = conn[(i + 1) % npts];
  // Do not insert "point" edges into vertex neighborhoods as there is
  // no way to determine their adjacent regions.
  if (idA == idB)
    {
    return -1;
    }

  // Add the edge to both of its vertex neighborhoods:
  if (SpliceEdgeIntoVertex(edgeId, idA, idB, true, pts, hoods, tol) < 0)
    {
    return -2;
    }
  if (SpliceEdgeIntoVertex(edgeId, idB, idA, false, pts, hoods, tol) < 0)
    {
    return -2;
    }
  return 0;
}

int SpliceCellIntoHood(
  vtkIdType faceId, vtkIdType* conn, vtkIdType i, vtkIdType npts, vtkPoints* pts,
  Edgerhoods& hoods, double tol)
{
  vtkIdType idA = conn[i];
  vtkIdType idB = conn[(i + 1) % npts];
  // Ignore topological degeneracies if we have enough vertices to continue:
  if (idA == idB)
    {
    return -1;
    }

  // Look up the edge by its canonical order and remember
  // its orientation (sense) relative to that order.
  std::pair<EdgePair,Edgerhood> entry;
  bool sense = idA < idB; // true -> edge and face are oriented consistently
  entry.first.first = sense ? idA : idB;
  entry.first.second = sense ? idB : idA;
  std::pair<Edgerhoods::iterator,bool> srch;
  srch.first = hoods.find(entry.first);
  // If we haven't seen the edge before, initialize a hood for it:
  if (srch.first == hoods.end())
    {
    srch = hoods.insert(entry);
    vec3d xA;
    vec3d xB;
    pts->GetPoint(entry.first.first, xA.GetData());
    pts->GetPoint(entry.first.second, xB.GetData());
    srch.first->second.BasePoint = xA;
    srch.first->second.Direction = Subtract(xB, xA);
    // If the edge is too short (i.e., pts[idA] == pts[idB]),
    // treat it as a single point as long as we have enough
    // other vertices. Of course, if the edge is also non-manifold
    // or bordant to 2 regions, we could be in trouble.
    if (srch.first->second.Direction.Normalize() < tol)
      {
      hoods.erase(srch.first);
      return -1;
      }
    }
  if (!SpliceFaceIntoHood(faceId, npts, conn, i, sense, srch.first, pts, tol))
    { // Error out as splicing only fails when the whole face is degenerate.
    return -2;
    }
  return 0;
}

template<typename H>
static void BuildNeighborhoods(vtkPolyData* pd, H& hoods)
{
  int dim = H::Dimension;
  hoods.clear();
  vtkCellArray* cells = (dim == 2 ? pd->GetLines() : pd->GetPolys());
  vtkPoints* pts = pd->GetPoints();
  if (!cells || !pts)
    {
    return;
    }

  // Compute tolerance for when distances are considered degenerate:
  double diam = pd->GetLength();
  double tol = diam * VTK_DBL_EPSILON * 2.;
  tol = (tol <= 0. ? VTK_DBL_EPSILON : tol);

  // Now traverse all (dim-1)-boundaries of all cells and insert the cell into
  // each (dim-1)-neighborhood (vertex neighborhoods for edge cells in 2-D,
  // edge neighborhoods for face cells in 3-D).
  vtkIdType npts;
  vtkIdType* conn;
  vtkIdType cellId =
    (dim == 2 ?
     pd->GetNumberOfVerts() :
     pd->GetNumberOfVerts() + pd->GetNumberOfLines());
  for (cells->InitTraversal(); cells->GetNextCell(npts, conn); ++cellId)
    {
    vtkIdType distinctVertsCriterion = npts;
    if (npts < dim)
      continue;
    // Iterate over the (dim-1)-boundaries of the cell.
    // Polygons (dim 3) are implicitly closed loops (the last edge
    // connecting the start and end vertices is implied).
    // Polylines (dim 2) may be open or closed, but are always explicit.
    vtkIdType loopBounds = (dim == 2 ? npts - 1 : npts);
    for (vtkIdType i = 0; i < loopBounds; ++i)
      {
      if (distinctVertsCriterion < dim)
        {
        break;
        }
      int status = SpliceCellIntoHood(cellId, conn, i, npts, pts, hoods, tol);
      switch (status)
        {
      case 0:
        // OK. Do nothing.
        break;
      case -1:
        // Degenerate coordinates. Skip.
        --distinctVertsCriterion;
        break;
      case -2:
        // Degenerate cell. Stop.
        distinctVertsCriterion = -1;
        break;
        }
      }
    if (distinctVertsCriterion < dim)
      {
      vtkGenericWarningMacro(
        << "Cell " << cellId << " has only "
        << distinctVertsCriterion << " distinct vertices");
      }
    }
}

namespace {

template<typename N>
struct RegionTracker
{
  // Each entry in Sets represents the region to one side of one triangle (or model face).
  UnionFind Sets;
  // Region to each side of each triangle (or model face when ModelFaceArrayName is set).
  vtkSmartPointer<vtkIdTypeArray> ModelRegions;
  // Region to each side of each *proper* model face (bounding 2 or less regions)
  vtkSmartPointer<vtkIdTypeArray> ReconciledModelRegions;
  // Map from model-facet ID to a tuple in ModelRegions
  std::map<vtkIdType,vtkIdType> ModelMap;
  // Renumbering of Sets IDs to sequential integers after merge along shared edges is complete.
  std::map<vtkIdType,vtkIdType> Collapse;
  // Faces in counterclockwise order around each edge.
  N CellNeighborhoods;
  // ID for each region assigned by points on input port 3.
  vtkNew<vtkIntArray> RegionIds;
  // ID of region that is exterior to all others
  vtkIdType ExteriorId;
  // Arrays to be associated with each identified region.
  vtkNew<vtkPointData> RegionAttributes;
  // A table describing containment relationships between connected components.
  vtkNew<vtkTable> ContainmentRelationships;
  // A column in ContainmentRelationships whose values are region IDs of contained shells
  vtkNew<vtkIdTypeArray> ContainedShellIds;
  // A column in ContainmentRelationships whose values are cells on contained shells
  vtkNew<vtkIdTypeArray> ContainedShellCells;
  // A column in ContainmentRelationships whose values are cells on contained shells
  vtkNew<vtkIntArray> ContainedShellSense;
  // A column in ContainmentRelationships whose values are region IDs of containers (or -1)
  vtkNew<vtkIdTypeArray> ContainerShellIds;
  // A column in ContainmentRelationships whose values are cells on containing shells (or -1)
  vtkNew<vtkIdTypeArray> ContainerShellCells;
  // A column in ContainmentRelationships whose values are cells on containing shells (or -1)
  vtkNew<vtkIntArray> ContainerShellSense;
  // An optional polydata pointer in which a point should be stored for each region
  vtkPolyData* RegionPoints;

  RegionTracker()
    {
    this->ExteriorId = -1;
    this->ContainedShellIds->SetName("ContainedShellIds");
    this->ContainedShellCells->SetName("ContainedShellCells");
    this->ContainedShellSense->SetName("ContainedShellSense");
    this->ContainerShellIds->SetName("ContainerShellIds");
    this->ContainerShellCells->SetName("ContainerShellCells");
    this->ContainerShellSense->SetName("ContainerShellSense");
    this->ContainmentRelationships->AddColumn(this->ContainedShellIds.GetPointer());
    this->ContainmentRelationships->AddColumn(this->ContainedShellCells.GetPointer());
    this->ContainmentRelationships->AddColumn(this->ContainedShellSense.GetPointer());
    this->ContainmentRelationships->AddColumn(this->ContainerShellIds.GetPointer());
    this->ContainmentRelationships->AddColumn(this->ContainerShellCells.GetPointer());
    this->ContainmentRelationships->AddColumn(this->ContainerShellSense.GetPointer());
    this->RegionPoints = NULL;
    }

  // Keep a record of which connected components are contained in which loops.
  void AddContainmentRelation(
    vtkIdType cellOnShell, bool cellSense,
    vtkIdType cellOnContainer, bool containerSense)
    {
    this->ContainedShellIds->InsertNextValue(-1);
    this->ContainedShellCells->InsertNextValue(cellOnShell);
    this->ContainedShellSense->InsertNextValue(cellSense);
    this->ContainerShellIds->InsertNextValue(-1);
    this->ContainerShellCells->InsertNextValue(cellOnContainer);
    this->ContainerShellSense->InsertNextValue(containerSense);
    }

  // Map region IDs in ContainmentRelationships table to their collapsed IDs
  void CollapseContainmentRelationships()
    {
    vtkIdType numRelations = this->ContainedShellIds->GetNumberOfTuples();
    vtkIdType* containedRegions = this->ContainedShellIds->GetPointer(0);
    vtkIdType* containerRegions = this->ContainerShellIds->GetPointer(0);
    vtkIdType* containedCellIds = this->ContainedShellCells->GetPointer(0);
    vtkIdType* containerCellIds = this->ContainerShellCells->GetPointer(0);
    int* containedSense = this->ContainedShellSense->GetPointer(0);
    int* containerSense = this->ContainerShellSense->GetPointer(0);
    for (vtkIdType i = 0; i < numRelations; ++i)
      {
      /*
      cout
        << "Contained " << containedCellIds[i] << "," << containedSense[i]
        << "," << this->Collapse[this->Sets.Find(this->ModelRegions->GetValue(3 * containedCellIds[i] + (containedSense[i] ? 2 : 1)))]
        << " container " << containerCellIds[i] << "," << containerSense[i]
        << "," << this->Collapse[this->Sets.Find(this->ModelRegions->GetValue(3 * containerCellIds[i] + (containerSense[i] ? 2 : 1)))]
        << "\n";
        */
      containedRegions[i] = this->Collapse[
        this->Sets.Find(
          this->ModelRegions->GetValue(
            3 * containedCellIds[i] + (containedSense[i] ? 2 : 1)))];
      containerRegions[i] = this->Collapse[
        this->Sets.Find(
          this->ModelRegions->GetValue(
            3 * containerCellIds[i] + (containerSense[i] ? 2 : 1)))];
      }
    }

  void DumpHoods()
    {
    typename N::iterator it;
    for (it = this->CellNeighborhoods.begin(); it != this->CellNeighborhoods.end(); ++it)
      {
      cout << it->first << ": " << it->second << "\n";
      }
    }

  void Dump()
    {
    vtkIdType entry[3];
    for (
      std::map<vtkIdType,vtkIdType>::iterator it = this->ModelMap.begin();
      it != this->ModelMap.end();
      ++it)
      {
      this->ModelRegions->GetTypedTuple(it->second, entry);
      vtkIdType r0 = this->Sets.Find(entry[1]);
      vtkIdType r1 = this->Sets.Find(entry[2]);
      cout
        << "  Face " << it->first << " facet " << it->second
        << " bounds regions " << r0 << "," << r1 << "\n";
      }
    }

  void DumpCollapsed()
    {
    vtkIdType entry[3];
    for (
      std::map<vtkIdType,vtkIdType>::iterator it = this->ModelMap.begin();
      it != this->ModelMap.end();
      ++it)
      {
      this->ModelRegions->GetTypedTuple(it->second, entry);
      vtkIdType r0 = this->Collapse[this->Sets.Find(entry[1])];
      vtkIdType r1 = this->Collapse[this->Sets.Find(entry[2])];
      cout
        << "  Face " << it->first << " facet " << it->second
        << " bounds regions " << r0 << "," << r1 << "\n";
      }
    }
};

} // namespace

vtkStandardNewMacro(vtkDiscoverRegions);

vtkDiscoverRegions::vtkDiscoverRegions()
{
  this->FaceGroupArrayName = NULL;
  this->ModelFaceArrayName = NULL;
  this->RegionGroupArrayName = NULL;
  this->SetFaceGroupArrayName("FaceGroups");
  this->ReportRegionsByModelFace = 0;
  this->GenerateRegionInteriorPoints = 0;
  // port 0: triangulated facets
  // port 1: volume hole points
  // port 2: region assignment points
  this->SetNumberOfInputPorts(3);
  // port 0: input marked with region information
  // port 1: table of containment relations between shells
  // port 2: points for discovered regions
  this->SetNumberOfOutputPorts(3);
}

vtkDiscoverRegions::~vtkDiscoverRegions()
{
  this->SetFaceGroupArrayName(NULL);
  this->SetModelFaceArrayName(NULL);
  this->SetRegionGroupArrayName(NULL);
}

void vtkDiscoverRegions::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ReportRegionsByModelFace: " << this->ReportRegionsByModelFace << "\n";
  os << indent << "GenerateRegionInteriorPoints: " << this->GenerateRegionInteriorPoints << "\n";
  os << indent << "FaceGroupArrayName: " << this->FaceGroupArrayName << "\n";
  os << indent << "ModelFaceArrayName: " << this->ModelFaceArrayName << "\n";
  os << indent << "RegionGroupArrayName: " << this->RegionGroupArrayName << "\n";
}

template<typename T>
struct vtkDiscoverRegionsFacetFromCellArray
{
  vtkIdType operator () (vtkIdType poly) const
    {
    return this->Array->GetValue(poly);
    }
  std::set<vtkIdType> FacetIds(vtkPolyData*) const
    {
    std::set<vtkIdType> uniques;
    for (vtkIdType i = 0; i <= this->Array->GetMaxId(); ++i)
      {
      uniques.insert(this->Array->GetValue(i));
      }
    return uniques;
    }
  vtkIdType CellForFacet(vtkIdType facet)
    {
    for (vtkIdType i = 0; i <= this->Array->GetMaxId(); ++i)
      {
      if (facet == this->Array->GetValue(i))
        {
        return i;
        }
      }
    return -1;
    }
  vtkIdTypeArray* GetFaceGroupArray()
    {
    return this->Array;
    }
  bool AreOversubscribedFacetsPossible(vtkIdTypeArray* other) const
    {
    return other != this->Array;
    }
  T* Array;
};

struct vtkDiscoverRegionsOneFacetPerCell
{
  vtkIdType operator () (vtkIdType poly) const
    {
    return poly;
    }
  std::set<vtkIdType> FacetIds(vtkPolyData* src) const
    {
    std::set<vtkIdType> uniques;
    for (vtkIdType i = 0; i < src->GetNumberOfCells(); ++i)
      {
      uniques.insert(i);
      }
    return uniques;
    }
  vtkIdType CellForFacet(vtkIdType facet)
    {
    return facet; // facet == cell
    }
  vtkIdTypeArray* GetFaceGroupArray()
    {
    return NULL;
    }
  bool AreOversubscribedFacetsPossible(vtkIdTypeArray*) const
    {
    return true;
    }
};

template<typename T, typename N>
void InitializeRegions(vtkPolyData* surface, RegionTracker<N>& regions, const T& cell2facet)
{
  std::set<vtkIdType> modelFacets = cell2facet.FacetIds(surface);
  regions.ModelRegions = vtkSmartPointer<vtkIdTypeArray>::New();
  regions.ModelRegions->SetName("ModelFaceRegionsMap");
  regions.ModelRegions->SetNumberOfComponents(3);
  regions.ModelRegions->SetComponentName(0, "ModelFace");
  regions.ModelRegions->SetComponentName(1, "BackfaceRegion");
  regions.ModelRegions->SetComponentName(2, "FrontfaceRegion");
  regions.ModelRegions->SetNumberOfTuples(modelFacets.size());
  vtkIdType i = 0;
  for (std::set<vtkIdType>::iterator it = modelFacets.begin(); it != modelFacets.end(); ++it, ++i)
    {
    vtkIdType tuple[3] = { *it, regions.Sets.NewSet(), regions.Sets.NewSet() };
    regions.ModelRegions->SetTypedTuple(i, tuple);
    regions.ModelMap[*it] = i;
    }
}

template<typename T, typename N>
void MergeRegions(vtkPolyData* pdIn, RegionTracker<N>& regions, const T& cell2facet)
{
  pdIn->BuildCells();
  BuildNeighborhoods<N>(pdIn, regions.CellNeighborhoods);
  // Iterate over each edge, E = {e0,e1} with attached faces {f_i = 0,n}
  typename N::iterator it;
  for (
    it = regions.CellNeighborhoods.begin();
    it != regions.CellNeighborhoods.end();
    ++it)
    {
    //cout << "Considering neighborhood of " << it->first << "\n";
    // Faces are orderedCCW along positive edge dir.
    vtkIdType numFaces = static_cast<vtkIdType>(it->second.Bordants.size());
    for (vtkIdType f = 0; f < numFaces; ++f)
      {
      vtkIdType cellIdA = it->second.Bordants[f].CellId;
      vtkIdType cellIdB = it->second.Bordants[(f + 1) % numFaces].CellId;

      vtkIdType modelRegionA = regions.ModelMap[cell2facet(cellIdA)];
      vtkIdType modelRegionB = regions.ModelMap[cell2facet(cellIdB)];

      vtkIdType materialA =
        regions.ModelRegions->GetValue(modelRegionA * 3 +
          (it->second.Bordants[f].Orientation ? 2 : 1));
      vtkIdType materialB =
        regions.ModelRegions->GetValue(modelRegionB * 3 +
          (it->second.Bordants[(f + 1) % numFaces].Orientation ? 1 : 2));
      /*
      cout << "  merge "
        << "cell " << cellIdA << " reg " << materialA << " with "
        << "cell " << cellIdB << " reg " << materialB << "\n";
        */

      regions.Sets.MergeSets(materialA, materialB);
      }
    }
  //regions.Dump();
}

struct RayHitRecord
{
  vec3d S;
  double T;
  vec3d X;
  vtkIdType RegionInfo[3];
  vtkIdType CellId;
  bool Sense;

  bool operator < (const RayHitRecord& other) const
    { return this->T < other.T; }
};

typedef std::multiset<RayHitRecord> RayHitRecords;

template<typename N>
bool NearCorner(const vtkVector3d& param)
{
  int dim = N::Dimension;
  bool interior = true;
  for (int i = 0; i < dim; ++i)
    {
    // We are "well into" the interior if not near param[i] == 0 or param[i] == 1:
    interior &= (fabs(param[i]) > 1e-8 && fabs(1. - param[i]) > 1e-8);
    }
  return !interior;
};

template<typename T, typename N>
bool IntersectWithRay(
  vtkPolyData* pdIn, RegionTracker<N>& regions, T& cell2facet,
  vec3d& basePoint, const vec3d& direction,
  RayHitRecords& hits, vtkIdType except = -1)
{
  bool allHitsOk = true;
  int dim = N::Dimension;
  hits.clear();
  RayHitRecord hit;
  vtkPoints* pts = pdIn->GetPoints();
  double diam = pdIn->GetLength();
  vec3d p2 = MultiplyAdd(basePoint, direction, diam);
  vtkIdType begin = pdIn->GetNumberOfVerts() + (dim == 2 ? 0 : pdIn->GetNumberOfLines());
  vtkIdType end = begin + (dim == 2 ? pdIn->GetNumberOfLines() : pdIn->GetNumberOfPolys());
  for (hit.CellId = begin; hit.CellId < end; ++hit.CellId)
    {
    if (hit.CellId == except)
      { // Don't intersect with a face we're told to ignore.
      continue;
      }
    vtkCell* cell = pdIn->GetCell(hit.CellId);
    int dummySubId;
    if (
      cell->IntersectWithLine(
        basePoint.GetData(), p2.GetData(), 1e-8 * diam,
        hit.T, hit.X.GetData(), hit.S.GetData(), dummySubId))
      {
      vtkIdType npts;
      vtkIdType* conn;
      vec3d norm;
      pdIn->GetCellPoints(hit.CellId, npts, conn);
      EstimateNormal<N::Dimension>(pts, npts, conn, norm);
      vtkIdType modelFacet = regions.ModelMap[cell2facet(hit.CellId)];
      regions.ModelRegions->GetTypedTuple(modelFacet, hit.RegionInfo);
      double normDotDir;
      if ((normDotDir = norm.Dot(direction)) < 0.)
        {
        vtkIdType tmp = hit.RegionInfo[1];
        hit.RegionInfo[1] = hit.RegionInfo[2];
        hit.RegionInfo[2] = tmp;
        hit.Sense = false;
        }
      else
        {
        hit.Sense = true;
        }
      allHitsOk |= !(fabs(normDotDir) < 1e-8 || NearCorner<N>(hit.S));
      for (int j = 1; j < 3; ++j)
        {
        hit.RegionInfo[j] = regions.Sets.Find(hit.RegionInfo[j]);
        }
      hits.insert(hit);
      }
    }
  return true; //allHitsOk;
}

struct RegionHitCountRecord
{
  double FirstHit;
  bool DoesFirstHitExit; // is the first hit a region entry(F) or exit(T) event?
  int Count; // sum of signed entry(+1) + exit(-1) events

  RegionHitCountRecord()
    {
    this->FirstHit = vtkMath::Inf();
    this->DoesFirstHitExit = false;
    this->Count = 0;
    }
};

static bool ContainingShellFromHits(
  RayHitRecords& hits, vtkIdType& region, vtkIdType& hitCell, bool& hitSense, bool returnInwardShell)
{
  //cout << "--\n";
  std::deque<RayHitRecords::iterator> inwardHits;
  std::deque<RayHitRecords::iterator> outwardHits;
  for (RayHitRecords::iterator it = hits.begin(); it != hits.end(); ++it)
    {
    /*
    cout
      << "    * " << it->T << "  x" << it->X << " modfacet " << it->RegionInfo[0]
      << " regs: - " << it->RegionInfo[1] << " + " << it->RegionInfo[2] << "\n";
      */

    if (!outwardHits.empty() && outwardHits.front()->RegionInfo[2] == it->RegionInfo[1])
      {
      outwardHits.pop_front();
      }
    else
      {
      inwardHits.push_front(it);
      }
    if (!inwardHits.empty() && inwardHits.front()->RegionInfo[1] == it->RegionInfo[2])
      {
      inwardHits.pop_front();
      }
    else
      {
      outwardHits.push_front(it);
      }
    }
  if (returnInwardShell)
    {
    if (inwardHits.empty())
      { // Didn't hit anything but air.
      region = -1;
      hitCell = -1;
      //cout << "  REGION -1\n";
      return false;
      }
    region = inwardHits.back()->RegionInfo[1];
    hitCell = inwardHits.back()->CellId;
    hitSense = ! inwardHits.back()->Sense;
    //cout << "  REGION " << inwardHits.back()->RegionInfo[1] << "\n";
    }
  else
    {
    //if (outwardStack.empty())
    if (outwardHits.empty())
      { // Didn't hit anything but air.
      region = -1;
      hitCell = -1;
      //cout << "  REGION -1\n";
      return false;
      }
    region = outwardHits.back()->RegionInfo[2];
    hitCell = outwardHits.back()->CellId;
    hitSense = outwardHits.back()->Sense;
    //cout << "  REGION " << outwardHits.back()->RegionInfo[2] << "\n";
    }
  return true;
}

static bool ContainingShellFromHits(
  RayHitRecords& hits, vtkIdType& region, bool returnInwardShell = true)
{
  vtkIdType dummyCell;
  bool dummySense;
  return ContainingShellFromHits(hits, region, dummyCell, dummySense, returnInwardShell);
}

template<typename T, typename N>
void FindPointsInRegions(
  vtkPolyData* pdIn, RegionTracker<N>& regions, T& cell2facet,
  vtkMinimalStandardRandomSequence* rnd)
{
  std::set<vtkIdType> shells = regions.Sets.Roots();
  std::set<vtkIdType>::iterator shellIt;
  std::map<vtkIdType,vtkVector3d> pointsByRegion;

  vtkPoints* pts = pdIn->GetPoints();
  vtkIdType npts; // number of points in one cell (not pts)
  vtkIdType* conn; // cell connectivity

  // Iterate over the shells
  for (
    shellIt = shells.begin();
    shellIt != shells.end();
    ++shellIt)
    {
    if (pointsByRegion.find(*shellIt) != pointsByRegion.end())
      {
      continue; // Another ray already provided a point in the region.
      }
    //cout << "\nPoints for Shell " << *shellIt << "\n";

    // Note that shell IDs have a special relationship to ModelMap;
    // they are assigned to ModelMap in sequential, 0-based pairs.
    // So, given a shell ID, we can divide by two and know an offset
    // into ModelMap. The remainder indicates whether the negative
    // (1 remainder) or positive (0 remainder) orientation of facets
    // correspond to the shell ID. From this offset into ModelMap,
    // we can then find the Facet ID for a shell and from there obtain
    // a cell on the facet for the shell.
    bool sense = (*shellIt) % 2 ? true : false; // positive orientation?
    vtkIdType regionInfo[3];
    regions.ModelRegions->GetTypedTuple(*shellIt / 2, regionInfo);
    vtkIdType cellOnShell = cell2facet.CellForFacet(regionInfo[0]);

    // Get cell normal
    vec3d norm;
    pdIn->GetCellPoints(cellOnShell, npts, conn);
    EstimateNormal<N::Dimension>(pts, npts, conn, norm);

    // Flip normal if shell and cell sense are opposite
    if (!sense)
      {
      MultiplyScalar(norm, -1.);
      }

    // Now we have a cell on the shell and know the shell normal.
    // Shoot rays "near" the normal to find which shells contain us.
    // TODO:
    //  1. Ideally here we would loop over several different pcenter
    //     coordinates for numerical stability
    //  2. This assumes we are dealing with a cell whose average
    //     coordinate is in its interior
    vec3d basePt(0.);
    vec3d tmp;
    double invNpts = 1. / npts;
    for (int i = 0; i < npts; ++i)
      {
      //cout << " " << conn[i];
      pts->GetPoint(conn[i], tmp.GetData());
      basePt = MultiplyAdd(basePt, tmp, invNpts);
      }
    bool rayHitsAllOk = false; // Did the ray hit any cell vertices or run tangent near a cell?
    std::map<vtkIdType, double> containerCount;
    int numTries = 0;
    // Perturb the normal at least 5 times and repeat until
    // over half the container answers agree. Or we've tried 15
    // times, which is a local approximation to an infinite loop.
    while (!rayHitsAllOk && numTries < 15)
      {
      // Perturb the unit normal by at most a half-unit vector.
      // This prevents the normal from disappearing, or
      // pointing into the solid, or along a tangent to the solid.
      vec3d pnorm(0.);
      for (int i = 0; i < N::Dimension; ++i, rnd->Next())
        {
        pnorm[i] = rnd->GetValue();
        }
      double scale = pnorm.SquaredNorm();
      // Renormalize after adding.
      (pnorm = MultiplyAdd(norm, pnorm, scale > 0.25 ? 0.5 / sqrt(scale) : 1.)).Normalize();
      /*
      cout
        << "  Tries " << numTries << " Cell " << cellOnShell << " np " << npts << " ["
        << "] sense " << (sense ? "+" : "-") << " pt " << basePt << " dir " << pnorm
        << "\n";
        */
      RayHitRecords hits;
      rayHitsAllOk |= IntersectWithRay(pdIn, regions, cell2facet, basePt, pnorm, hits, -1);
      if (rayHitsAllOk)
        {
        RayHitRecords::iterator i0 = hits.begin();
        RayHitRecords::iterator i1 = i0;
        rayHitsAllOk = false;
        if (i0 != hits.end())
          {
          for (++i1; i1 != hits.end(); ++i0, ++i1)
            {
            vtkVector3d pt = 0.5 * (i0->X + i1->X);
            int region = FindRegionContainingPoint(pdIn, pt, regions, cell2facet, rnd);
            //cout << "  Region " << region << "  " << pt << "\n";
            if (region == *shellIt)
              { // We got the region we need
              rayHitsAllOk = true;
              }
            if (pointsByRegion.find(region) == pointsByRegion.end() && region == *shellIt)
              {
              pointsByRegion[region] = pt;
              }
            }
          }
        }
      ++numTries;
      }
    }
  vtkNew<vtkPoints> rpts;
  vtkNew<vtkIdTypeArray> regionId;
  regionId->SetName("Region");
  regionId->SetNumberOfTuples(pointsByRegion.size());
  rpts->SetNumberOfPoints(pointsByRegion.size());
  regions.RegionPoints->SetPoints(rpts.GetPointer());
  regions.RegionPoints->GetPointData()->SetScalars(regionId.GetPointer());
  //cout << "Points in regions: (" << shells.size() << " == " << pointsByRegion.size() << "?)\n";
  std::map<vtkIdType,vtkVector3d>::iterator pit;
  vtkIdType i = 0;
  for (pit = pointsByRegion.begin(); pit != pointsByRegion.end(); ++pit, ++i)
    {
    rpts->SetPoint(i, pit->second.GetData());
    regionId->SetValue(i, pit->first);
    //cout << "  " << pit->first << "   " << pit->second << "\n";
    }
}

template<typename T, typename N>
vtkIdType FindRegionContainingPoint(
  vtkPolyData* pdIn, vtkVector3d& pt,
  RegionTracker<N>& regions, T& cell2facet,
  vtkMinimalStandardRandomSequence* rnd)
{
  // Choose a random direction:
  vec3d r;
  do
    {
    for (int i = 0; i < N::Dimension; ++i, rnd->Next())
      {
      r[i] = rnd->GetValue();
      }
    }
  while (r.Normalize() < 1e-5);
  // Find shell intersections
  RayHitRecords hits;
  IntersectWithRay(pdIn, regions, cell2facet, pt, r, hits);
  vtkIdType region;
  if (!ContainingShellFromHits(hits, region) || region < 0)
    {
    region = regions.ExteriorId;
    }
  return region;
}

template<typename T, typename N>
vtkIdType DiscoverNestings(
  vtkPolyData* pdIn, RegionTracker<N>& regions, T& cell2facet)
{
  // For each *unique* entry in regions.Sets,
  //   Choose a random point, P, on a seed face, F
  //   For each co-face of F,
  //     Intersect a ray (from P normal to the co-face) with other faces not in the same region
  //     Choose the smallest t > 0 intersection (say with face I)
  //     If the intersection is close to an edge or vertex of I, choose a different face F and retry
  //     Resolve the region ID from the intersected face I (based on the orientation of I relative to P)
  //     Merge the regions for the co-face of F and the co-face of I that face each other.

  vtkPoints* pts = pdIn->GetPoints();
  vtkIdType npts; // number of points in one cell (not pts)
  vtkIdType* conn; // cell connectivity

  // Get a list of shells:
  std::set<vtkIdType> shells = regions.Sets.Roots();
  std::set<vtkIdType>::iterator shellIt;

  // A source of random vectors for repeated trials
  vtkNew<vtkMinimalStandardRandomSequence> rnd;

  // If requested by user, compute a point in the interior of each region:
  if (regions.RegionPoints)
    {
    FindPointsInRegions(pdIn, regions, cell2facet, rnd.GetPointer());
    }

  // Iterate over the shells
  for (
    shellIt = shells.begin();
    shellIt != shells.end();
    ++shellIt)
    {
    //cout << "\nNesting for Shell " << *shellIt << "\n";
    // Note that shell IDs have a special relationship to ModelMap;
    // they are assigned to ModelMap in sequential, 0-based pairs.
    // So, given a shell ID, we can divide by two and know an offset
    // into ModelMap. The remainder indicates whether the negative
    // (1 remainder) or positive (0 remainder) orientation of facets
    // correspond to the shell ID. From this offset into ModelMap,
    // we can then find the Facet ID for a shell and from there obtain
    // a cell on the facet for the shell.
    bool sense = (*shellIt) % 2 ? true : false; // positive orientation?
    vtkIdType regionInfo[3];
    regions.ModelRegions->GetTypedTuple(*shellIt / 2, regionInfo);
    vtkIdType cellOnShell = cell2facet.CellForFacet(regionInfo[0]);

    // Get cell normal
    vec3d norm;
    pdIn->GetCellPoints(cellOnShell, npts, conn);
    EstimateNormal<N::Dimension>(pts, npts, conn, norm);

    // Flip normal if shell and cell sense are opposite
    if (!sense)
      {
      MultiplyScalar(norm, -1.);
      }

    // Now we have a cell on the shell and know the shell normal.
    // Shoot rays "near" the normal to find which shells contain us.
    // TODO:
    //  1. Ideally here we would loop over several different pcenter
    //     coordinates for numerical stability
    //  2. This assumes we are dealing with a cell whose average
    //     coordinate is in its interior
    vec3d basePt(0.);
    vec3d tmp;
    double invNpts = 1. / npts;
    for (int i = 0; i < npts; ++i)
      {
      //cout << " " << conn[i];
      pts->GetPoint(conn[i], tmp.GetData());
      basePt = MultiplyAdd(basePt, tmp, invNpts);
      }
    std::map<vtkIdType, double> containerCount;
    vtkIdType container = -1; // The region ID of the innermost container hit by a test ray.
    vtkIdType containerCell = -1; // The ID of the cell hit on the container by a test ray.
    bool containerSense = false; // Whether the containerCell was hit on its positive (true) or negative (false) side.
    int numTries = 0;
    // Perturb the normal at least 5 times and repeat until
    // over half the container answers agree. Or we've tried 15
    // times, which is a local approximation to an infinite loop.
    while (
      numTries < 5 || (
        (!containerCount.empty() && containerCount[container] / numTries < 0.5) &&
        numTries < 15))
      {
      // Perturb the unit normal by at most a half-unit vector.
      // This prevents the normal from disappearing, or
      // pointing into the solid, or along a tangent to the solid.
      vec3d pnorm(0.);
      for (int i = 0; i < N::Dimension; ++i, rnd->Next())
        {
        pnorm[i] = rnd->GetValue();
        }
      double scale = pnorm.SquaredNorm();
      // Renormalize after adding.
      (pnorm = MultiplyAdd(norm, pnorm, scale > 0.25 ? 0.5 / sqrt(scale) : 1.)).Normalize();
      /*
      cout
        << "  Tries " << numTries << " Cell " << cellOnShell << " np " << npts << " ["
        << "] sense " << (sense ? "+" : "-") << " pt " << basePt << " dir " << pnorm
        << " container " << container
        << " ok " << (containerCount.empty() ? -1. : containerCount[container] / numTries)
        << "\n";
        */
      RayHitRecords hits;
      IntersectWithRay(pdIn, regions, cell2facet, basePt, pnorm, hits, cellOnShell);
      vtkIdType tmpCon;
      vtkIdType tmpCell;
      bool tmpSense;
      if (ContainingShellFromHits(hits, tmpCon, tmpCell, tmpSense, true))
        {
        containerCount[tmpCon] += 1.;
        if (containerCount[tmpCon] > containerCount[container])
          {
          container = tmpCon;
          containerCell = tmpCell;
          containerSense = tmpSense;
          }
        }

      ++numTries;
      }
    /*
    cout
      << "  Tries " << numTries << " Cell " << cellOnShell << " np " << npts << " ["
      << "] sense " << (sense ? "+" : "-") << " pt " << basePt << " dir " << norm
      << " container " << container << " ok " << (containerCount.empty() ? "F" : "T")
      << "\n";
      */
    vtkIdType actualShellId = regions.Sets.Find(*shellIt);
    if (!containerCount.empty() && container >= 0)
      {
      if (actualShellId == container)
        {
        //cout << "Shell " << container << " stabbed itself\n";
        }
      else
        {
        /*
        cout
          << "Shell " << *shellIt << " cell " << cellOnShell << " sense " << sense
          << " inside " << container << " cell " << containerCell << " sense " << containerSense << "\n";
          */
        regions.AddContainmentRelation(cellOnShell, sense, containerCell, containerSense);
        regions.Sets.MergeSets(*shellIt, container);
        }
      }
    else
      {
      //cout << "Shell " << *shellIt << " is exterior\n";
      if (regions.ExteriorId < 0)
        {
        regions.ExteriorId = *shellIt;
        }
      else
        {
        // This is not really a containment relationship, so do not
        // call regions.AddContainmentRelation(...); if you change
        // this, then the cell and sense for the exterior region
        // must be valid.
        regions.Sets.MergeSets(*shellIt, regions.ExteriorId);
        }
      }
    }
  return regions.ExteriorId;
}

struct ModelFacetSplitRec
{
  vtkIdType OriginalFacet;
  vtkIdType Regions[2];

  ModelFacetSplitRec(vtkIdType orig, vtkIdType r0, vtkIdType r1)
    {
    this->OriginalFacet = orig;
    this->Regions[0] = r0 < r1 ? r0 : r1;
    this->Regions[1] = r0 < r1 ? r1 : r0;
    }

  ModelFacetSplitRec(const ModelFacetSplitRec& other)
    {
    this->OriginalFacet = other.OriginalFacet;
    for (int i = 0; i < 2; ++i)
      {
      this->Regions[i] = other.Regions[i];
      }
    }

  bool operator < (const ModelFacetSplitRec& other) const
    {
    return
      (this->OriginalFacet < other.OriginalFacet ? true :
       (this->OriginalFacet > other.OriginalFacet ? false :
        (this->Regions[0] < other.Regions[0] ? true :
         (this->Regions[0] > other.Regions[0] ? false :
          (this->Regions[1] < other.Regions[1] ? true :
           (this->Regions[1] > other.Regions[1] ? false :
            false))))));
    }
};

class ModelFacetSplitMap : public std::map<ModelFacetSplitRec,vtkIdType>
{
public:
  ModelFacetSplitMap(vtkIdType numOriginalModelFacets)
    {
    this->NextModelFacet = numOriginalModelFacets;
    }
  vtkIdType SplitFaceByRegion(vtkIdType orig, vtkIdType r0, vtkIdType r1)
    {
    std::pair<ModelFacetSplitRec,vtkIdType>
      rec(ModelFacetSplitRec(orig, r0, r1), this->NextModelFacet);
    iterator it = this->find(rec.first);
    if (it == this->end())
      { // Insert a new model facet
      if (this->Originals.find(orig) == this->Originals.end())
        {
        rec.second = orig;
        this->Originals.insert(orig);
        }
      else
        {
        ++this->NextModelFacet;
        }
      it = this->insert(rec).first;
      }
    return it->second;
    }
  vtkIdType NextModelFacet;
  std::set<vtkIdType> Originals;
};

template<typename T, typename N>
void ReconcileModelFacets(
  vtkPolyData* pdIn, const char* modelFaceArrayName,
  RegionTracker<N>& regions, T& cell2facet)
{
  vtkIdTypeArray* modelFaceArray =
    vtkIdTypeArray::SafeDownCast(
      pdIn->GetCellData()->GetArray(modelFaceArrayName));
  if (!modelFaceArray)
    {
    vtkGenericWarningMacro("No array \"" << modelFaceArrayName << "\" found.");
    return;
    }
  vtkIdType numModelFaces = modelFaceArray->GetValueRange(0)[1];
  if (modelFaceArray->GetValueRange(0)[0] == 0)
    {
    ++numModelFaces;
    }
  //vtkIdType nextModelFace = numModelFaces + 1;
  //std::set<ModelFacetSplitRec> modelFaceSplitter;
  ModelFacetSplitMap modelFaceSplitter(numModelFaces);
  vtkIdType numCellRegions = regions.ModelRegions->GetNumberOfTuples();
  for (vtkIdType i = 0; i < numCellRegions; ++i)
    {
    vtkIdType regionIds[3];
    regions.ModelRegions->GetTypedTuple(i, regionIds);
    vtkIdType currentModelFace = cell2facet(regionIds[0]);
    vtkIdType revisedModelFace = modelFaceSplitter.SplitFaceByRegion(
      currentModelFace,
      regions.Sets.Find(regionIds[1]),
      regions.Sets.Find(regionIds[2]));
    if (revisedModelFace != currentModelFace)
      {
      regionIds[0] = revisedModelFace;
      regions.ModelRegions->SetTypedTuple(i, regionIds);
      modelFaceArray->SetValue(i, revisedModelFace);
      }
    }
  /*
  cout
    << numModelFaces << " faces, now " << modelFaceSplitter.NextModelFacet
    << ", " << pdIn->GetNumberOfCells() << " triangles\n";
    */
  vtkNew<vtkIdTypeArray> reconciledFaceRegionMap;
  reconciledFaceRegionMap->SetName("ModelFaceRegionsMap");
  reconciledFaceRegionMap->SetNumberOfComponents(3);
  reconciledFaceRegionMap->SetComponentName(0, "ModelFace");
  reconciledFaceRegionMap->SetComponentName(1, "BackfaceRegion");
  reconciledFaceRegionMap->SetComponentName(2, "FrontfaceRegion");
  reconciledFaceRegionMap->SetNumberOfTuples(modelFaceSplitter.NextModelFacet);
  ModelFacetSplitMap::iterator mit = modelFaceSplitter.begin();
  vtkIdType i = 0;
  for (mit = modelFaceSplitter.begin(); mit != modelFaceSplitter.end(); ++i, ++mit)
    {
    vtkIdType regionIds[3] = { mit->second, mit->first.Regions[0], mit->first.Regions[1] };
    reconciledFaceRegionMap->SetTypedTuple(i, regionIds);
    }
  regions.ReconciledModelRegions = reconciledFaceRegionMap.GetPointer();
}

template<typename T, typename N>
void AssignHoles(
  vtkPolyData* pdIn, vtkPoints* holePoints,
  RegionTracker<N>& regions, T& cell2facet,
  vtkIntArray* isAHole)
{
  //cout << "Assigning holes\n";
  // For each hole point H, choose a random direction D and fire
  // a ray R=(H,D), recording intersections with each shell.
  // Find the first properly-oriented (inward-pointing) shell with
  // an odd number of hits and assign that shell to be a hole.
  //
  // NB: For safety from numerical artifacts, it would be best
  // to repeat the intersection several times for each hole point
  // and vote on the best containing shell.
  vtkNew<vtkMinimalStandardRandomSequence> rnd;
  vtkIdType numHolePts = holePoints->GetNumberOfPoints();
  for (vtkIdType i = 0; i < numHolePts; ++i)
    {
    // Get the hole point:
    vec3d h;
    holePoints->GetPoint(i, h.GetData());
    vtkIdType region = FindRegionContainingPoint(
      pdIn, h, regions, cell2facet, rnd.GetPointer());
    vtkIdType collapsedId = (region >= 0 ? regions.Collapse[region] : -1);
    if (collapsedId >= 0)
      {
      isAHole->SetValue(collapsedId, 1);
      }
    }
}

template<typename T, typename N>
void AssignRegionIDsHolesAndAttributes(
  vtkPolyData* pdIn, vtkPoints* holePtsIn, vtkPolyData* regionPtsIn,
  const std::string& vtkNotUsed(regionIdAttributeName),
  RegionTracker<N>& regions, T& cell2facet)
{
  // I. Collapse remaining union-find sets to a sequential integer numbering.
  //
  // Any pre-existing map entries will not be overwritten; we use this
  // to label the exterior shell as -1.
  //
  // Do not call regions.Sets.Find() on the new values or
  // bad things will happen quickly.
  regions.Collapse[regions.Sets.Find(regions.ExteriorId)] = -1;
  regions.Sets.CollapseIds(regions.Collapse, 0);
  regions.CollapseContainmentRelationships();

  // II. Create arrays mapping collapsed region IDs to holes and
  //     region groups.

  // Create an array to hold indexes into the user-specified groups:
  vtkNew<vtkIdTypeArray> regionGroupMembership;
  regionGroupMembership->SetName("RegionGroupMembership");
  // Don't include the exterior region:
  regionGroupMembership->SetNumberOfTuples(regions.Collapse.size() - 1);
  // Initialize regions to an invalid index (-1):
  regionGroupMembership->FillComponent(0, -1);

  // Create a matching array that specifies "hole membership"
  vtkNew<vtkIntArray> holeRegions;
  holeRegions->SetName("IsRegionAHole");
  holeRegions->SetNumberOfComponents(1);
  holeRegions->SetNumberOfTuples(regions.Collapse.size() - 1);
  holeRegions->FillComponent(0, 0);

  // III. Find which regions are marked as holes.
  if (holePtsIn && holePtsIn->GetNumberOfPoints())
    {
    AssignHoles(
      pdIn, holePtsIn, regions, cell2facet,
      holeRegions.GetPointer());
    }

  // IV. Now find which regions are in which region groups specified
  //     by the input data.
  vtkNew<vtkMinimalStandardRandomSequence> rnd;
  // Copy the region-group attribute data to our output field data:
  if (regionPtsIn)
    {
    vtkPoints* regionPts = regionPtsIn->GetPoints();
    vtkIdType numRegionPts = regionPts ? regionPts->GetNumberOfPoints() : 0;
    regions.RegionAttributes->DeepCopy(regionPtsIn->GetPointData());

    for (vtkIdType i = 0; i < numRegionPts; ++i)
      {
      // Get the region point:
      vec3d regionPt;
      regionPts->GetPoint(i, regionPt.GetData());
      // Find which region contains the point:
      vtkIdType region = FindRegionContainingPoint(
        pdIn, regionPt, regions, cell2facet, rnd.GetPointer());
      // Find the collapsed ID of the region
      vtkIdType collapsedId = (region >= 0 ? regions.Collapse[region] : -1);
      if (collapsedId >= 0)
        {
        regionGroupMembership->SetValue(collapsedId, i);
        }
      }
    }
  regions.RegionAttributes->AddArray(regionGroupMembership.GetPointer());
  regions.RegionAttributes->AddArray(holeRegions.GetPointer());

  // V. Overwrite ModelRegions with collapsed IDs.
  vtkIdType numModelFacets = regions.ModelRegions->GetNumberOfTuples();
  //cout << "TriangleRegions: (" << numModelFacets << ")\n";
  for (vtkIdType i = 0; i < numModelFacets; ++i)
    {
    vtkIdType facetRegions[3];
    regions.ModelRegions->GetTypedTuple(i, facetRegions);
    for (int j = 1; j < 3; ++j)
      {
      facetRegions[j] = regions.Collapse[regions.Sets.Find(facetRegions[j])];
      }
    // Flip model-faces that have the "outside" on their backface:
    // we'll reverse the cell connectivity later in PrepareOutput.
    if (facetRegions[1] < facetRegions[2])
      {
      vtkIdType tmp = facetRegions[1];
      facetRegions[1] = facetRegions[2];
      facetRegions[2] = tmp;
      }
    regions.ModelRegions->SetTypedTuple(i, facetRegions);
    //cout << i << ": " << facetRegions[0] << " " << facetRegions[1] << " " << facetRegions[2] << "\n";
    }

  // VI. Overwrite output region points with collapsed IDs.
  if (regions.RegionPoints)
    {
    vtkIdTypeArray* regionIds = vtkIdTypeArray::SafeDownCast(
      regions.RegionPoints->GetPointData()->GetArray("Region"));
    vtkIdType* reg = regionIds->GetPointer(0);
    for (vtkIdType i = 0; i < regionIds->GetNumberOfTuples(); ++i)
      {
      reg[i] = reg[i] < 0 ? reg[i] : regions.Collapse[reg[i]];
      }
    }
}

template<typename T, typename N>
void PrepareOutput(
  vtkPolyData* pdIn, RegionTracker<N>& regions, T& cell2facet,
  const char* faceGroupArrayName, vtkPolyData* pdOut, vtkTable* tabOut)
{
  pdOut->ShallowCopy(pdIn);
  vtkNew<vtkIdTypeArray> cellRegions;
  cellRegions->SetName(VTK_CELL_REGION_IDS);
  cellRegions->SetNumberOfComponents(2);
  cellRegions->SetNumberOfTuples(pdIn->GetNumberOfCells());
  vtkIdType cellIdOffset = pdIn->GetNumberOfVerts();
  cellRegions->FillComponent(0, -1);
  cellRegions->FillComponent(1, -1);
  vtkIdType polyRegions[3];
  vtkCellArray* cells =
    (N::Dimension == 2 ?
     pdIn->GetLines() : pdIn->GetPolys());
  cells->InitTraversal();
  vtkIdType npts;
  vtkIdType* conn;
  vtkIdType connOffset = 0;
  // Prepare a per-cell region array and flip cells
  // that have the "exterior" region on their backface.
  for (vtkIdType i = 0; cells->GetNextCell(npts, conn); ++i)
    {
    vtkIdType curFaceId = i + cellIdOffset;
    // Get the shell information for both co-facets of this face:
    vtkIdType modelMapEntry = regions.ModelMap[cell2facet(curFaceId)];
    regions.ModelRegions->GetTypedTuple(modelMapEntry, polyRegions);
    //polyRegions[1] = regions.Sets.Find(polyRegions[1]);
    //polyRegions[2] = regions.Sets.Find(polyRegions[2]);
    //regions.ModelRegions->SetTypedTuple(modelMapEntry, polyRegions);

    // Reverse the cell if its normal points inward and it is an outer shell
    // Note that this invalidates regions.EdgeNeighborhoods[...].Sense.
    if (polyRegions[1] < polyRegions[2])
      {
      cells->ReverseCell(connOffset);
      vtkIdType tmp = polyRegions[1];
      polyRegions[1] = polyRegions[2];
      polyRegions[2] = tmp;
      }

    cellRegions->SetTypedTuple(curFaceId, &polyRegions[1]);
    connOffset = cells->GetTraversalLocation();
    }
  pdOut->GetCellData()->AddArray(cellRegions.GetPointer());

  // Copy the region attribute stuff to the field data.
  pdOut->GetFieldData()->ShallowCopy(regions.RegionAttributes.GetPointer());

  // Copy the model-facet boundary markers from the input if they exist:
  vtkAbstractArray* facetBoundaryMarkers =
    pdIn->GetFieldData()->GetAbstractArray(faceGroupArrayName);
  if (facetBoundaryMarkers)
    {
    pdOut->GetFieldData()->AddArray(facetBoundaryMarkers);
    }

  // If we reconciled model facets, collapse and substitute the new array
  if (
    regions.ReconciledModelRegions &&
    regions.ReconciledModelRegions->GetNumberOfTuples() > 0)
    {
    regions.ModelRegions = regions.ReconciledModelRegions;

    vtkIdType numModelFacets = regions.ModelRegions->GetNumberOfTuples();
    //cout << "ModelFaceRegions: (" << numModelFacets << ")\n";
    for (vtkIdType i = 0; i < numModelFacets; ++i)
      {
      vtkIdType facetRegions[3];
      regions.ModelRegions->GetTypedTuple(i, facetRegions);
      for (int j = 1; j < 3; ++j)
        {
        facetRegions[j] = regions.Collapse[regions.Sets.Find(facetRegions[j])];
        }
      // We reversed cells above, now reverse region entries so outside is always along positive normal
      if (facetRegions[1] == -1 && facetRegions[2] != -1)
        {
        vtkIdType tmp = facetRegions[1];
        facetRegions[1] = facetRegions[2];
        facetRegions[2] = tmp;
        }
      regions.ModelRegions->SetTypedTuple(i, facetRegions);
      //cout << i << ": " << facetRegions[0] << " " << facetRegions[1] << " " << facetRegions[2] << "\n";
      }
    }
  // Add the model facet -> region ID map to the field data.
  pdOut->GetFieldData()->AddArray(regions.ModelRegions.GetPointer());

  // Annotate region components
  regions.ModelRegions->SetComponentName(0, "ModelFacet");
  regions.ModelRegions->SetComponentName(1, "BackfaceRegion");
  regions.ModelRegions->SetComponentName(2, "FrontfaceRegion");
  for (int i = 0; i < 2; ++i)
    {
    cellRegions->SetComponentName(i,
      regions.ModelRegions->GetComponentName(i + 1));
    }

  tabOut->ShallowCopy(regions.ContainmentRelationships.GetPointer());
}

template<typename T, typename N>
int DiscoverRegions(
  vtkPolyData* pdIn, vtkPolyData* holesIn, vtkPolyData* regionPtsIn,
  const char* regionGroupArrayName, const char* faceGroupArrayName,
  const char* modelFaceArrayName, vtkPolyData* pdOut, vtkTable* tabOut,
  T& cell2facet, RegionTracker<N>& regions)
{
  vtkIdTypeArray* mapArray = NULL;

  InitializeRegions(pdIn, regions, cell2facet);
  if (pdIn->GetNumberOfCells())
    {
    // Merge regions with adjacent (dim-2)-boundaries.
    MergeRegions(pdIn, regions, cell2facet);

    // Find which regions (if any) are contained in other regions
    // Merge them as appropriate.
    DiscoverNestings(pdIn, regions, cell2facet);

    // It is possible that some model faces are "oversubscribed"
    // (i.e., they are bordant to more than 2 regions). This can
    // only occur if model faces were not initially simplices
    // but were provided to us after tessellation -- defined by
    // way of a cell-data scalar specifying the original model
    // face.
    // This is corrected by splitting (d-1)-simplices in the
    // tessellation of model faces into groups that share the
    // same regions.
    //
    // This will alter the array specified by modelFaceArrayName
    // by changing existing model face assignments as required.
    // It will also change entries in regions.ModelRegions.
    mapArray =
      modelFaceArrayName ?
      vtkIdTypeArray::SafeDownCast(
        pdIn->GetCellData()->GetArray(modelFaceArrayName)) :
      NULL;
    if (mapArray)
      {
      vtkDiscoverRegionsFacetFromCellArray<vtkIdTypeArray> reconciler;
      reconciler.Array = mapArray;
      ReconcileModelFacets(pdIn, modelFaceArrayName, regions, reconciler);
      }
    }

  // See if we have enough information to mark regions
  // as belonging to domain sets.
  vtkPoints* regionPts = regionPtsIn ? regionPtsIn->GetPoints() : NULL;
  if (
    !regionGroupArrayName || !regionGroupArrayName[0] ||
    !regionPts || !regionPts->GetNumberOfPoints())
    {
    regionPtsIn = NULL;
    }

  // Points identifying regions as holes.
  vtkPoints* holePointsIn = holesIn ? holesIn->GetPoints() : NULL;

  // Map region IDs into a 0-based integer sequence.
  AssignRegionIDsHolesAndAttributes(
    pdIn, holePointsIn, regionPtsIn, regionGroupArrayName ? regionGroupArrayName : "", regions, cell2facet);

  // Copy region IDs to output dataset
  PrepareOutput(pdIn, regions, cell2facet, faceGroupArrayName, pdOut, tabOut);
  return 1;
}

int vtkDiscoverRegions::FillInputPortInformation(
  int port,
  vtkInformation* info)
{
  if (!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  if (port > 0)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

int vtkDiscoverRegions::FillOutputPortInformation(
  int port,
  vtkInformation* info)
{
  if (port == 1)
    {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    }
  else if (!this->Superclass::FillOutputPortInformation(port, info))
    {
    return 0;
    }
  return 1;
}

int vtkDiscoverRegions::RequestData(
    vtkInformation* vtkNotUsed(req),
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo)
{
  // get the input and output
  vtkPolyData* pdIn = vtkPolyData::GetData(inInfo[0], 0);
  vtkPolyData* holesIn = vtkPolyData::GetData(inInfo[1], 0);
  vtkPolyData* regionPtsIn = vtkPolyData::GetData(inInfo[2], 0);
  vtkPolyData* pdOut = vtkPolyData::GetData(outInfo, 0);
  vtkTable* tabOut = vtkTable::GetData(outInfo, 1);
  vtkPolyData* regionPtsOut =
    this->GenerateRegionInteriorPoints ?
    vtkPolyData::GetData(outInfo, 2) : NULL;

  /*
  vtkPolyData* pdIn = this->GetPolyDataInput(0);
  vtkPolyData* holesIn = this->GetPolyDataInput(1);
  vtkPolyData* regionPtsIn = this->GetPolyDataInput(2);
  vtkPolyData* pdOut = this->GetOutput();
  */

  vtkIdTypeArray* mapArray =
    this->ModelFaceArrayName && !this->ReportRegionsByModelFace ?
      vtkIdTypeArray::SafeDownCast(
        pdIn->GetCellData()->GetArray(this->ModelFaceArrayName)) :
      NULL;

  int dimension = -1;
  if (!pdIn->GetNumberOfPolys() && !pdIn->GetNumberOfStrips())
    {
    if (pdIn->GetNumberOfLines() > 0)
      {
      dimension = 2;
      }
    }
  else
    {
    dimension = 3;
    }

  int status;
  switch (dimension)
    {
  case 2:
      {
      RegionTracker<Vertexhoods> regionTracker;
      regionTracker.RegionPoints = regionPtsOut;
      if (mapArray)
        {
        vtkDiscoverRegionsFacetFromCellArray<vtkIdTypeArray> cell2facet;
        cell2facet.Array = mapArray;
        status = DiscoverRegions(
          pdIn, holesIn, regionPtsIn,
          this->RegionGroupArrayName,
          this->FaceGroupArrayName,
          this->ModelFaceArrayName,
          pdOut, tabOut, cell2facet, regionTracker);
        }
      else
        { // NULL or invalid name: one facet per poly
        vtkDiscoverRegionsOneFacetPerCell cell2facet;
        status = DiscoverRegions(
          pdIn, holesIn, regionPtsIn,
          this->RegionGroupArrayName,
          this->FaceGroupArrayName,
          this->ModelFaceArrayName,
          pdOut, tabOut, cell2facet, regionTracker);
        }
      }
    break;
  case 3:
      {
      RegionTracker<Edgerhoods> regionTracker;
      regionTracker.RegionPoints = regionPtsOut;
      if (mapArray)
        {
        vtkDiscoverRegionsFacetFromCellArray<vtkIdTypeArray> cell2facet;
        cell2facet.Array = mapArray;
        status = DiscoverRegions(
          pdIn, holesIn, regionPtsIn,
          this->RegionGroupArrayName,
          this->FaceGroupArrayName,
          this->ModelFaceArrayName,
          pdOut, tabOut, cell2facet, regionTracker);
        }
      else
        { // NULL or invalid name: one facet per poly
        vtkDiscoverRegionsOneFacetPerCell cell2facet;
        status = DiscoverRegions(
          pdIn, holesIn, regionPtsIn,
          this->RegionGroupArrayName,
          this->FaceGroupArrayName,
          this->ModelFaceArrayName,
          pdOut, tabOut, cell2facet, regionTracker);
        }
      }
    break;
  default:
    //vtkErrorMacro("Unable to determine problem dimension.");
    return 1;
    }

  return 1;
}
