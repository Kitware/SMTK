//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkRegionsToLoops.h"

#include "vtkAppendPolyData.h"
#include "vtkAssignAttribute.h"
#include "vtkArrayCalculator.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkIdTypeArray.h"
#include "vtkStripper.h"
#include "vtkTable.h"
#include "vtkThresholdPoints.h"
#include "vtkXMLPolyDataWriter.h"

#include "smtk/bridge/discrete/extension/meshing/vtkCMBMeshServerLauncher.h"
#include "smtk/bridge/discrete/extension/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "smtk/bridge/discrete/extension/meshing/vtkCMBTriangleMesher.h"
#include "smtk/bridge/discrete/extension/meshing/vtkPolylineTriangulator.h"
#include "smtk/bridge/discrete/extension/meshing/union_find.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace smtk {
  namespace bridge {
    namespace discrete {


vtkStandardNewMacro(vtkRegionsToLoops);

vtkRegionsToLoops::vtkRegionsToLoops()
{
  // port 0: polydata loops containing regions to triangulate
  // port 1: table of containment relationships
  // port 2: polydata points of holes inside each region
  this->SetNumberOfInputPorts(3);
}

vtkRegionsToLoops::~vtkRegionsToLoops()
{
}

void vtkRegionsToLoops::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkRegionsToLoops::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port < 1)
    {
    return this->Superclass::FillInputPortInformation(port, info);
    }
  else  if (port == 1)
    {
    // Accept a table describing the containment relationships of
    // among regions as the second input.
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  else if (port == 2)
    {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

namespace {

class Snippets
{
public:
  typedef std::set<vtkIdType> EdgeSetType;
  typedef std::map<vtkIdType,EdgeSetType> EdgeUsesType;
  typedef std::map<vtkIdType,vtkIdType> IdMapType;

  UnionFind Sets;
  EdgeUsesType Uses; // Map from vertex IDs to the edges that use them.
  IdMapType EdgeToSet; // Given an edge, provide the set it belongs to.
  IdMapType SetToEdge; // Given a set, provide one edge belonging to it.
  IdMapType Pedigrees;
  vtkNew<vtkCellArray> CellsOut;
  IdMapType Containers; // maps set IDs to other set IDs which contain them.

  void AddCoedge(
    vtkIdType cellId, vtkIdType* conn, vtkIdType npts,
    bool sense, vtkIdType pedigreeId)
    {
    if (npts < 2)
      {
      return;
      }
    // NB: head and tail really don't need to pay attention to sense:
    vtkIdType head = sense ? conn[0] : conn[npts - 1];
    vtkIdType tail = sense ? conn[npts - 1] : conn[0];

    // Create a new set to represent the new cell
    vtkIdType snippet = this->Sets.NewSet();
    this->CellsOut->InsertNextCell(npts, conn); // returned cell ID should match snippet ID.
    this->SetToEdge[snippet] = cellId;
    this->EdgeToSet[cellId] = snippet;

    EdgeUsesType::iterator headIt = this->Uses.find(head);
    if (headIt == this->Uses.end())
      {
      EdgeSetType blank;
      std::pair<vtkIdType,EdgeSetType> entry(head,blank);
      headIt = this->Uses.insert(entry).first;
      }
    else
      {
      this->Sets.MergeSets(snippet, this->EdgeToSet[*headIt->second.begin()]);
      }
    headIt->second.insert(cellId);

    EdgeUsesType::iterator tailIt = this->Uses.find(tail);
    if (tailIt == this->Uses.end())
      {
      EdgeSetType blank;
      std::pair<vtkIdType,EdgeSetType> entry(tail,blank);
      tailIt = this->Uses.insert(entry).first;
      }
    else
      {
      this->Sets.MergeSets(snippet, this->EdgeToSet[*tailIt->second.begin()]);
      }
    tailIt->second.insert(cellId);

    this->Pedigrees[cellId] = pedigreeId;
    }

  void AddContainerInfo(vtkIdType containedCell, vtkIdType containerCell)
    {
    this->Containers[this->Sets.Find(this->EdgeToSet[containedCell])] =
      this->Sets.Find(this->EdgeToSet[containerCell]);
    }

  void Generate(
    vtkIdType region, vtkPolyData* pdIn, vtkPolyData* holesIn,
    vtkPolyData* pdOut, vtkIdTypeArray* modelFace, vtkIdTypeArray* pedigreeIds,
    vtkCMBMeshServerLauncher* lau)
    {
    std::set<vtkIdType> roots = this->Sets.Roots();

    vtkNew<vtkIdTypeArray> loopModelFace;
    loopModelFace->SetName("modelfaceids");

    vtkNew<vtkPolyData> pdLines;
    pdLines->SetPoints(pdIn->GetPoints());
    pdLines->GetPointData()->ShallowCopy(pdIn->GetPointData());
    pdLines->GetCellData()->AddArray(loopModelFace.GetPointer());
    //pdLines->GetCellData()->CopyAllocate(pdIn->GetCellData());
    pdLines->SetLines(this->CellsOut.GetPointer());
    vtkNew<vtkPolylineTriangulator> tri;
    tri->SetLauncher(lau);

    this->CellsOut->InitTraversal();
    vtkIdType npts;
    vtkIdType* conn;
    //vtkIdType offset = 0;
    IdMapType polyToPedigree;
    //vtkIdType polygon = 0;
    for (vtkIdType i = 0; i < this->Sets.Size(); ++i)
      {
      this->CellsOut->GetNextCell(npts, conn);
      loopModelFace->InsertNextValue(region);
      }

    vtkNew<vtkStripper> str;
    str->SetInputDataObject(pdLines.GetPointer());
    str->Update();

    vtkNew<vtkArrayCalculator> cal;
    cal->SetInputConnection(str->GetOutputPort());
    cal->SetAttributeModeToUseCellData();
    cal->SetResultArrayName("modelfaceids");
    cal->SetResultArrayType(VTK_ID_TYPE);
      {
      std::ostringstream fn;
      fn << region;
      cal->SetFunction(fn.str().c_str());
      }
    cal->Update();

#if 0
    vtkNew<vtkXMLPolyDataWriter> mwr;
    mwr->SetInputConnection(cal->GetOutputPort());
      {
      std::ostringstream mfname;
      mfname << "Region-" << region << ".vtp";
      mwr->SetFileName(mfname.str().c_str());
      }
    mwr->SetDataModeToAscii();
    mwr->Write();
#endif // 0

    vtkNew<vtkThresholdPoints> th1;
    vtkNew<vtkThresholdPoints> th2;
    vtkNew<vtkAppendPolyData> app;
    th1->ThresholdByLower(region - 0.5);
    th2->ThresholdByUpper(region + 0.5);
    th1->SetInputArrayToProcess(/*idx*/0, /*port*/0, /*connection*/0,
      vtkDataObject::FIELD_ASSOCIATION_POINTS, "Region");
    th2->SetInputArrayToProcess(/*idx*/0, /*port*/0, /*connection*/0,
      vtkDataObject::FIELD_ASSOCIATION_POINTS, "Region");

    th1->SetInputDataObject(holesIn);
    th2->SetInputDataObject(holesIn);
    app->AddInputConnection(th1->GetOutputPort());
    app->AddInputConnection(th2->GetOutputPort());
    app->Update();

    vtkNew<vtkArrayCalculator> ped;
    ped->SetInputConnection(app->GetOutputPort());
    ped->SetAttributeModeToUsePointData();
    ped->SetResultArrayName("modelfaceids");
    ped->SetResultArrayType(VTK_ID_TYPE);
      {
      std::ostringstream fn;
      fn << region;
      ped->SetFunction(fn.str().c_str());
      }
    ped->Update();
    vtkNew<vtkAssignAttribute> att;
    att->Assign("modelfaceids", vtkDataSetAttributes::PEDIGREEIDS, vtkAssignAttribute::POINT_DATA);
    att->SetInputConnection(ped->GetOutputPort());
    att->Update();

#if 0
    //vtkNew<vtkXMLPolyDataWriter> mwr;
    mwr->SetInputConnection(app->GetOutputPort());
      {
      std::ostringstream mfname;
      mfname << "FacetHoles-" << region << ".vtp";
      mwr->SetFileName(mfname.str().c_str());
      }
    mwr->SetDataModeToAscii();
    mwr->Write();
#endif // 0
    tri->SetInputConnection(cal->GetOutputPort());
    tri->SetInputDataObject(1, att->GetOutput());
    tri->SetModelFaceArrayName("modelfaceids");
    tri->Update();

#if 0
    //vtkNew<vtkXMLPolyDataWriter> mwr;
    mwr->SetInputConnection(tri->GetOutputPort());
      {
      std::ostringstream mfname;
      mfname << "Facet-" << region << ".vtp";
      mwr->SetFileName(mfname.str().c_str());
      }
    mwr->SetDataModeToAscii();
    mwr->Write();
#endif // 0

    // Now add the triangles to the output mesh along with
    // a pedigree ID indicating the generating facet
    vtkCellArray* srcPolys = tri->GetOutput()->GetPolys();
    vtkCellArray* dstPolys = pdOut->GetPolys();
    vtkCellData* srcAttr = pdIn->GetCellData();
    vtkCellData* dstAttr = pdOut->GetCellData();
    vtkIdTypeArray* mface = vtkIdTypeArray::SafeDownCast(
      tri->GetOutput()->GetCellData()->GetArray("modelfaceids"));

    // Take the lowest positive pedigree ID of any edge in our loop.
    // Or -1 if the pedigree IDs of *any* of its edges are -1.
    vtkIdType pedigree = -2;
    for (
      IdMapType::iterator mit = this->Pedigrees.begin();
      mit != this->Pedigrees.end();
      ++mit)
      {
      if (pedigree < -1 || pedigree > mit->second)
        {
        pedigree = mit->second;
        }
      }
    pedigree = (pedigree == -2 ? -1 : pedigree);
    //cout << "   ped " << pedigree << " modelFace " << region << " mface " << mface << "\n";
    srcPolys->InitTraversal();
    for (vtkIdType i = 0; srcPolys->GetNextCell(npts, conn); ++i)
      {
      vtkIdType dstCell = dstPolys->InsertNextCell(npts, conn);
      if (mface)
        {
        //cout << "      i " << i << " mf " << mface->GetValue(i) << "\n";
        //dstAttr->CopyData(srcAttr, mface->GetValue(i), dstCell);
        dstAttr->CopyData(srcAttr, this->SetToEdge[region], dstCell);
        }
      modelFace->InsertNextValue(region);
      pedigreeIds->InsertNextValue(pedigree);
      }
    }

  void Report()
    {
    std::set<vtkIdType> roots = this->Sets.Roots();
    std::set<vtkIdType>::iterator rootIt;
    cout << "  " << roots.size() << " Connected Components:";
    for (rootIt = roots.begin(); rootIt != roots.end(); ++rootIt)
      {
      cout << " " << this->SetToEdge[*rootIt];
      }
    cout << "\n";

    EdgeUsesType::iterator useIt;
    cout << "  Uses:\n";
    for (useIt = this->Uses.begin(); useIt != this->Uses.end(); ++useIt)
      {
      cout << "    " << useIt->first << " ->";
      EdgeSetType::iterator setIt;
      for (setIt = useIt->second.begin(); setIt != useIt->second.end(); ++setIt)
        {
        cout << " " << *setIt << " (" << this->SetToEdge[this->Sets.Find(this->EdgeToSet[*setIt])] << ")";
        }
      cout << "\n";
      }
    }

  Snippets()
    {
    }
  Snippets(const Snippets& other)
    : Sets(other.Sets), Uses(other.Uses), EdgeToSet(other.EdgeToSet), SetToEdge(other.SetToEdge),
    Pedigrees(other.Pedigrees), Containers(other.Containers)
    {
    this->CellsOut->DeepCopy(other.CellsOut.GetPointer());
    }
  Snippets& operator = (const Snippets& other)
    {
    this->Sets = other.Sets;
    this->Uses = other.Uses;
    this->EdgeToSet = other.EdgeToSet;
    this->SetToEdge = other.SetToEdge;
    this->Pedigrees = other.Pedigrees;
    this->Containers = other.Containers;
    this->CellsOut->DeepCopy(other.CellsOut.GetPointer());
    return *this;
    }
};

class SnippetCollection : public std::map<vtkIdType,Snippets>
{
public:
  void AddContainerInfo(vtkTable* tabIn)
    {
    if (!tabIn)
      {
      return;
      }
    vtkIdTypeArray* containedShellsCol =
      vtkIdTypeArray::SafeDownCast(
        tabIn->GetColumnByName("ContainedShellIds"));
    vtkIdTypeArray* containedCellsCol =
      vtkIdTypeArray::SafeDownCast(
        tabIn->GetColumnByName("ContainedShellCells"));
    vtkIdTypeArray* containerCellsCol =
      vtkIdTypeArray::SafeDownCast(
        tabIn->GetColumnByName("ContainerShellCells"));
    if (!containedShellsCol || !containedCellsCol || !containerCellsCol)
      {
      vtkGenericWarningMacro("Expected columns not present.");
      return;
      }
    vtkIdType* containedShells = containedShellsCol->GetPointer(0);
    vtkIdType* containedCells = containedCellsCol->GetPointer(0);
    vtkIdType* containerCells = containerCellsCol->GetPointer(0);
    vtkIdType nrows = tabIn->GetNumberOfRows();
    for (vtkIdType i = 0; i < nrows; ++i)
      {
      iterator it = this->find(containedShells[i]);
      if (it == this->end())
        {
        std::pair<vtkIdType,Snippets> blank;
        blank.first = containedShells[i];
        it = this->insert(blank).first;
        }
      it->second.AddContainerInfo(containedCells[i], containerCells[i]);
      }
    }
  void AddCoedgeToRegion(
    vtkIdType cellId, vtkIdType* conn, vtkIdType npts, bool sense,
    vtkIdType pedigreeId, vtkIdType region)
    {
    iterator it = this->find(region);
    if (it == this->end())
      {
      std::pair<vtkIdType,Snippets> blank;
      blank.first = region;
      it = this->insert(blank).first;
      }
    it->second.AddCoedge(cellId, conn, npts, sense, pedigreeId);
    //cout << " Region " << region << " now\n";
    //it->second.Report();
    }
};

} // namespace

int vtkRegionsToLoops::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputInfo,
  vtkInformationVector* outputInfo)
{
  if (!inputInfo || !outputInfo)
    {
    return 0;
    }

  vtkPolyData* pdIn = vtkPolyData::GetData(inputInfo[0], 0);
  vtkTable* tabIn = vtkTable::GetData(inputInfo[1], 0);
  vtkPolyData* holesIn = vtkPolyData::GetData(inputInfo[2], 0);
  vtkPolyData* pdOut = vtkPolyData::GetData(outputInfo, 0);
  //tabIn->Dump(20);

  if (!pdIn || !pdOut)
    {
    return 0;
    }

  vtkCellData* ocd = pdOut->GetCellData();
  vtkCellData* icd = pdIn->GetCellData();
  vtkIdTypeArray* cellRegions = vtkIdTypeArray::SafeDownCast(icd->GetArray("Region"));
  if (!cellRegions)
    {
    vtkErrorMacro("Could not find \"Regions\" array or it was not an ID-type array.");
    return 0;
    }
  vtkIdTypeArray* pedigreeIds = vtkIdTypeArray::SafeDownCast(icd->GetPedigreeIds()); //Array("vtkPedigreeIds"));
  ocd->CopyAllocate(icd);

  vtkNew<vtkPoints> opts;
  opts->ShallowCopy(pdIn->GetPoints());
  pdOut->GetPointData()->ShallowCopy(pdIn->GetPointData());
  pdOut->SetPoints(opts.GetPointer());

  // Now loop through edges of input data, adding coedges to each region's
  // list of connected components.
  // We build up an array of snippets for each region and maps to the head
  // and tail of each free snippet.
  vtkIdType* conn;
  vtkIdType npts;
  vtkIdType numCells = pdIn->GetNumberOfLines();
  vtkCellArray* cells = pdIn->GetLines();
  vtkIdType cellStart = pdIn->GetNumberOfVerts();
  vtkIdType cellEnd = cellStart + numCells;
  cells->InitTraversal();
  SnippetCollection snippetsByRegion;
  for (vtkIdType i = cellStart; i < cellEnd; ++i)
    {
    vtkIdType pedigree;
    if (pedigreeIds)
      {
      pedigreeIds->GetTupleValue(i, &pedigree);
      }
    else
      {
      pedigree = -1;
      }
    cells->GetNextCell(npts, conn);
    vtkIdType regions[2];
    cellRegions->GetTupleValue(i, regions);
    //cout << "** Considering edge " << conn[0] << " " << conn[1] << "  r " << regions[0] << " " << regions[1] << " **\n";
    // Only add manifold edges
    // TODO: Handle non-manifold loops.
    if (regions[0] != regions[1])
      {
      snippetsByRegion.AddCoedgeToRegion(i, conn, npts, true, pedigree, regions[1]);
      snippetsByRegion.AddCoedgeToRegion(i, conn, npts, false, pedigree, regions[0]);
      }
    }
  snippetsByRegion.AddContainerInfo(tabIn);
  //cout << "\n\n";

  vtkNew<vtkCMBMeshServerLauncher> lau;
  // Now triangulate each region
  SnippetCollection::iterator snipIt;
  vtkNew<vtkIdTypeArray> modelFaces;
  vtkNew<vtkIdTypeArray> pedigrees;
  vtkNew<vtkCellArray> cellsOut;
  modelFaces->SetName("modelfaceids");
  pedigrees->SetName("_vtkPedigreeIds");
  pdOut->SetPolys(cellsOut.GetPointer());
  pdOut->GetCellData()->CopyAllocate(pdIn->GetCellData());
  for (snipIt = snippetsByRegion.begin(); snipIt != snippetsByRegion.end(); ++snipIt)
    {
    //cout << "Region " << snipIt->first << "\n";
    //snipIt->second.Report();
    if (snipIt->first == -1)
      {
      continue; // do not try to triangulate exterior.
      }
    snipIt->second.Generate(
      snipIt->first, pdIn, holesIn, pdOut,
      modelFaces.GetPointer(), pedigrees.GetPointer(), lau.GetPointer());
    }
  pdOut->GetCellData()->AddArray(modelFaces.GetPointer());
  pdOut->GetCellData()->SetPedigreeIds(pedigrees.GetPointer());
  return 1;
}
    } // namespace discrete
  } // namespace bridge
} // namespace smtk
