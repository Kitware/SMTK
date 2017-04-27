//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshReader.h"

#include "smtk/extension/vtk/reader/vtkCMBReaderHelperFunctions.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"

#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vtksys/SystemTools.hxx>

#define ABORT_FREQ 10000

vtkStandardNewMacro(vtkCMBMeshReader);

struct vtkCMBMeshReaderInternals
{
  ifstream* Stream;
  std::stringstream* Line;

  vtkCMBMeshReaderInternals()
    : Stream(0)
    , Line(0)
  {
  }
  ~vtkCMBMeshReaderInternals() { this->DeleteStreams(); }
  void DeleteStreams()
  {
    delete this->Stream;
    this->Stream = 0;
    delete this->Line;
    this->Line = 0;
  }
  void ReadLine()
  {
    if (!this->Line)
    {
      this->Line = new std::stringstream("", ios::out | ios::in);
    }
    std::string fileLine;
    vtksys::SystemTools::GetLineFromStream(*this->Stream, fileLine);
    this->Line->clear();
    this->Line->str(fileLine);
  }
};

vtkCMBMeshReader::vtkCMBMeshReader()
{
  this->FileName = NULL;
  this->CreateMeshElementIdArray = false;
  this->CreateMeshMaterialIdArray = true;
  this->CreateMeshNodeIdArray = false;
  this->RenameMaterialAsRegion = true;
  this->MeshDimension = MESH3D;
  this->SetNumberOfInputPorts(0);
  this->Internals = new vtkCMBMeshReaderInternals;
}

vtkCMBMeshReader::~vtkCMBMeshReader()
{
  delete[] this->FileName;
  delete this->Internals;
}

int vtkCMBMeshReader::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  if (!this->FileName || (strlen(this->FileName) == 0))
  {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
  }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkUnstructuredGrid* output =
    vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  struct stat fs;
  if (stat(this->FileName, &fs) != 0)
  {
    vtkErrorMacro(<< "Unable to open file: " << this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
  }

  this->Internals->Stream = new ifstream(this->FileName, ios::in | ios::binary);
  if (this->Internals->Stream->fail())
  {
    vtkErrorMacro(<< "Unable to open file: " << this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    this->Internals->DeleteStreams();
    return 0;
  }

  vtkDebugMacro(<< "Executing CMB Mesh file reader");
  // This reader supports file formats with various file declarations
  // Force uppercase comparison to include files with bad capitalization
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(this->FileName);
  std::string card;
  this->Internals->ReadLine();
  *this->Internals->Line >> card;
  card = vtksys::SystemTools::UpperCase(card);

  if (card != "MESH" && // MESH is for backwards compatibility
    card != "MESH1D" && card != "MESH2D" && card != "MESH3D" && card != "WMS1DM" &&
    card != "WMS2DM" && card != "WMS3DM")
  {
    vtkErrorMacro("Failed reading file declaration card in the header.");
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    this->Internals->DeleteStreams();
    return 0;
  }
  if (card == "MESH1D" || card == "WMS1DM" || (card == "MESH" && ext == ".1dm"))
  {
    this->MeshDimension = this->MESH1D;
  }
  else if (card == "MESH2D" || card == "WMS2DM" || (card == "MESH" && ext == ".2dm"))
  {
    this->MeshDimension = this->MESH2D;
  }
  else // MESH3D, WMS3DM, or MESH with *.3dm
  {
    this->MeshDimension = this->MESH3D;
  }

  // Check for optional number of elements (cells) and nodes (points) cards
  vtkIdType numPts = -1, numCells = -1;
  while (this->Internals->Stream->peek() == '#')
  {
    this->Internals->ReadLine();
    *this->Internals->Line >> card;
    if (card == "#NELEM")
    {
      *this->Internals->Line >> numCells;
    }
    else if (card == "#NNODE")
    {
      *this->Internals->Line >> numPts;
    }
  }

  // If cell and point info not found then preview the file to get counts
  if (numCells == -1 || numPts == -1)
  {
    vtkDebugMacro(<< "Meta information not found; previewing file");
    // Save file position
    std::streampos filePos = this->Internals->Stream->tellg();
    if (!this->PreviewFile(numCells, numPts))
    {
      vtkErrorMacro("Error reading file. Element and node indices should be "
                    "one-based and contiguous (no gaps).");
      this->SetErrorCode(vtkErrorCode::FileFormatError);
      this->Internals->DeleteStreams();
      return 0;
    }
    // Reset the file position and actually read the file, you must reset the
    // flags before rewinding
    this->Internals->Stream->clear();
    this->Internals->Stream->seekg(filePos);
  }

  vtkDebugMacro(<< "Mesh information: " << numCells << " elements, " << numPts << " nodes");
  // Empty file or abort
  if ((numCells < 1 && numPts < 1) || this->GetAbortExecute())
  {
    this->Internals->DeleteStreams();
    return 1;
  }

  // Allocate and intialize data structures
  output->Allocate(numCells);

  vtkPoints* pts = vtkPoints::New();
  pts->SetDataTypeToDouble();
  pts->SetNumberOfPoints(numPts);
  output->SetPoints(pts);
  pts->Delete();
  vtkDoubleArray* dpts = vtkDoubleArray::SafeDownCast(pts->GetData());

  vtkIdTypeArray* nodeIdArray = NULL;
  if (this->CreateMeshNodeIdArray)
  {
    nodeIdArray = vtkIdTypeArray::New();
    nodeIdArray->SetName("Mesh Node ID");
    nodeIdArray->SetNumberOfValues(numPts);
    output->GetPointData()->AddArray(nodeIdArray);
    nodeIdArray->Delete();
  }

  vtkIntArray* cellMaterialArray = NULL;
  if (CreateMeshMaterialIdArray)
  {
    cellMaterialArray = vtkIntArray::New();
    cellMaterialArray->SetName(
      RenameMaterialAsRegion ? ReaderHelperFunctions::GetShellTagName() : "Mesh Material ID");
    cellMaterialArray->SetNumberOfValues(numCells);
    output->GetCellData()->AddArray(cellMaterialArray);
    cellMaterialArray->Delete();
  }

  vtkIdTypeArray* cellIdArray = NULL;
  if (CreateMeshElementIdArray)
  {
    cellIdArray = vtkIdTypeArray::New();
    cellIdArray->SetName("Mesh Cell ID");
    cellIdArray->SetNumberOfValues(numCells);
    output->GetCellData()->AddArray(cellIdArray);
    cellIdArray->Delete();
  }

  vtkDebugMacro(<< "Reading and creating unstructured mesh");
  vtkIdType line = 0;
  this->Internals->ReadLine();
  while (!this->Internals->Stream->eof())
  {
    *this->Internals->Line >> card;
    // The order is an attempt to optimize the number of comparisons
    if (card == "ND" || card == "GN")
    {
      this->ReadNode(dpts, nodeIdArray);
    }
    else if (card == "E3T" || card == "GE3")
    {
      this->ReadCell(VTK_TRIANGLE, 3, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E4T" || (card == "GE4" && this->MeshDimension == MESH3D))
    {
      this->ReadCell(VTK_TETRA, 4, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E4Q" || (card == "GE4" && this->MeshDimension == MESH2D))
    {
      this->ReadCell(VTK_QUAD, 4, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E6W" || card == "GE6")
    {
      this->ReadCell(VTK_WEDGE, 6, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E8H" || card == "GE8")
    {
      this->ReadCell(VTK_HEXAHEDRON, 8, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E2L" || card == "GE2")
    {
      this->ReadCell(VTK_LINE, 2, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E5P")
    {
      this->ReadCell(VTK_PYRAMID, 5, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E6T")
    {
      this->ReadCell(VTK_QUADRATIC_TRIANGLE, 6, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E8Q")
    {
      this->ReadCell(VTK_QUADRATIC_QUAD, 8, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E9Q")
    {
      this->ReadCell(VTK_BIQUADRATIC_QUAD, 9, output, cellMaterialArray, cellIdArray);
    }
    else if (card == "E3L")
    {
      this->ReadCell(VTK_QUADRATIC_EDGE, 3, output, cellMaterialArray, cellIdArray);
    }
    this->Internals->ReadLine();
    // Check Abort
    if (!(line % ABORT_FREQ) && this->GetAbortExecute())
    {
      break;
    }
    ++line;
  }
  output->Squeeze();
  this->Internals->DeleteStreams();
  vtkDebugMacro(<< "Executed CMB Mesh file reader");
  return 1;
}

int vtkCMBMeshReader::CanReadFile(const char* fname)
{
  struct stat fs;
  if (!fname || strlen(fname) == 0 || stat(fname, &fs) != 0)
  {
    return 0;
  }

  std::string card;
  ifstream file(fname, ios::in | ios::binary);
  if (!file.fail())
  {
    // Get file declaration card and capitalize for comparison
    file >> card;
    card = vtksys::SystemTools::UpperCase(card);
  }

  return card == "MESH" || card == "MESH2D" || card == "MESH3D" || card == "WMS1DM" ||
    card == "WMS2DM" || card == "WMS3DM";
}

int vtkCMBMeshReader::PreviewFile(vtkIdType& ncells, vtkIdType& npts)
{
  std::string card = "";
  vtkIdType id = -1, maxPt = -1, maxCell = -1, line = 0;

  ncells = npts = 0;
  this->Internals->ReadLine();
  while (!this->Internals->Stream->eof())
  {
    // Read card from file line
    *this->Internals->Line >> card;
    // The order is an attempt to optimize the number of comparisons
    if (card == "ND" || card == "GN")
    {
      *this->Internals->Line >> id;
      maxPt = (id > maxPt ? id : maxPt);
      ++npts;
    }
    else if (card == "E3T" || card == "GE3" || card == "E4T" || card == "GE4" || card == "E4Q" ||
      card == "E6W" || card == "GE6" || card == "E5P" || card == "E8H" || card == "GE8" ||
      card == "E6T" || card == "E8Q" || card == "E9Q" || card == "E2L" || card == "GE2" ||
      card == "E3L")
    {
      *this->Internals->Line >> id;
      maxCell = (id > maxCell ? id : maxCell);
      ++ncells;
    }
    // Read next file line
    this->Internals->ReadLine();
    // Check Abort
    if (!(line % ABORT_FREQ) && this->GetAbortExecute())
    {
      return 1;
    }
    ++line;
  }

  if (npts != maxPt || ncells != maxCell)
  {
    return 0;
  }
  return 1;
}

void vtkCMBMeshReader::ReadCell(int cellType, int numPts, vtkUnstructuredGrid* output,
  vtkIntArray* cellMaterialArray, vtkIdTypeArray* cellIdArray)
{
  vtkIdType i, cellFileId = -1, cellId = -1;
  int material = 0;
  vtkIdType ptIds[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };

  *this->Internals->Line >> cellFileId;
  for (i = 0; i < numPts; ++i)
  {
    *this->Internals->Line >> ptIds[i];
    // VTK indices are zero-based and file format's indices are one-based so
    // convert.
    --ptIds[i];
  }
  *this->Internals->Line >> material;
  // The point ordering of VTK and some of the file format's cell types are
  // different. Re-order to match VTK's if necessary and then add cell.
  switch (cellType)
  {
    case VTK_QUADRATIC_TRIANGLE:
    {
      vtkIdType vtkIds[6];
      unsigned int indices[6] = { 0, 3, 1, 4, 2, 5 };
      for (i = 0; i < 6; ++i)
      {
        vtkIds[indices[i]] = ptIds[i];
      }
      cellId = output->InsertNextCell(cellType, numPts, vtkIds);
      break;
    }
    case VTK_QUADRATIC_QUAD:
    case VTK_BIQUADRATIC_QUAD:
    {
      vtkIdType vtkIds[9];
      unsigned int indices[9] = { 0, 4, 1, 5, 2, 6, 3, 7, 8 };
      for (i = 0; i < numPts; ++i)
      {
        vtkIds[indices[i]] = ptIds[i];
      }
      cellId = output->InsertNextCell(cellType, numPts, vtkIds);
      break;
    }
    default:
      cellId = output->InsertNextCell(cellType, numPts, ptIds);
      break;
  }
  if (cellMaterialArray)
  {
    cellMaterialArray->SetValue(cellId, material);
  }
  if (cellIdArray)
  {
    cellIdArray->SetValue(cellId, cellId);
  }
}

void vtkCMBMeshReader::ReadNode(vtkDoubleArray* dpts, vtkIdTypeArray* nodeIdArray)
{
  vtkIdType nodeFileId = -1, nodeId = -1;
  double coords[3] = { 0.0, 0.0, 0.0 };
  *this->Internals->Line >> nodeFileId >> coords[0] >> coords[1] >> coords[2];
  nodeId = nodeFileId - 1;
  dpts->SetTuple(nodeId, coords);
  if (nodeIdArray)
  {
    nodeIdArray->SetValue(nodeId, nodeId);
  }
}

void vtkCMBMeshReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << endl;
  os << indent << "MeshDimension: " << this->MeshDimension << "D" << endl;
  os << indent << "MeshElementIDArray: " << (this->CreateMeshElementIdArray ? "On" : "Off") << endl;
  os << indent << "MeshNodeIDArray: " << (this->CreateMeshNodeIdArray ? "On" : "Off") << endl;
  os << indent << "MeshMaterialIDArray: " << (this->CreateMeshMaterialIdArray ? "On" : "Off")
     << endl;
  os << indent << "RenameMaterialAsRegion: " << (this->RenameMaterialAsRegion ? "On" : "Off")
     << endl;
}

int vtkCMBMeshReader::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  if (!this->FileName)
  {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
  }

  return 1;
}
