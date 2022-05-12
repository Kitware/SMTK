//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/io/vtkMedReader.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMedHelper.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPointData.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtksys/SystemTools.hxx"

vtkStandardNewMacro(vtkMedReader);

// Reads all the integers in a FAM node
static bool readFamilies(
  const hid_t fileId,
  HdfNode* famNode,
  vtkSmartPointer<vtkIntArray> const& families,
  vtkSmartPointer<vtkObject> const& self)
{
  // Load in the tag dataset
  const hid_t dataset = H5Dopen(fileId, famNode->path.c_str(), H5P_DEFAULT);

  // Read the number of points/cells attribute NBR, FAM->NBR
  const hid_t attributeId = H5Aopen_name(dataset, "NBR");
  int numVals = -1;
  if (H5Aread(attributeId, H5T_NATIVE_INT, &numVals) != 0)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read FAMs, Failed to read NBR attribute");
    return false;
  }
  H5Aclose(attributeId);

  // Check the datatype
  const hid_t datatype = H5Dget_type(dataset);
  const hid_t type = H5Tget_class(datatype);
  if (type != H5T_INTEGER)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read FAMs, not H5T_INTEGER");
    return false;
  }

  // Read directly into the VTK data array
  families->SetNumberOfValues(numVals);
  int* tagsPtr = static_cast<int*>(families->GetVoidPointer(0));
  H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, tagsPtr);
  H5Dclose(dataset);
  return true;
}

static bool readNums(
  const hid_t fileId,
  HdfNode* numNode,
  vtkSmartPointer<vtkIntArray> const& numArray,
  vtkSmartPointer<vtkObject> const& self)
{
  // Load in the tag dataset
  const hid_t dataset = H5Dopen(fileId, numNode->path.c_str(), H5P_DEFAULT);

  // Read the number of points/cells attribute NBR, NUM->NBR
  const hid_t attributeId = H5Aopen_name(dataset, "NBR");
  int numVals = -1;
  if (H5Aread(attributeId, H5T_NATIVE_INT, &numVals) != 0)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read NUMs, Failed to read NBR attribute");
    return false;
  }
  H5Aclose(attributeId);

  // Check the datatype
  const hid_t datatype = H5Dget_type(dataset);
  const hid_t type = H5Tget_class(datatype);
  if (type != H5T_INTEGER)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read NUMs, not H5T_INTEGER");
    return false;
  }

  // Read directly into the VTK data array
  numArray->SetNumberOfValues(numVals);
  int* numsPtr = static_cast<int*>(numArray->GetVoidPointer(0));
  H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, numsPtr);
  H5Dclose(dataset);
  return true;
}

// Reads all the vertices in a COO node
static bool readPoints(
  const hid_t fileId,
  HdfNode* cooNode,
  vtkSmartPointer<vtkPoints> const& points,
  vtkSmartPointer<vtkObject> const& self)
{
  // Open the points dataset
  const hid_t dataset = H5Dopen(fileId, cooNode->path.c_str(), H5P_DEFAULT);

  // Read the number of points attribute NBR, NOE->COO->NBR
  const hid_t attributeId = H5Aopen_name(dataset, "NBR");
  int numberOfPoints = -1;
  if (H5Aread(attributeId, H5T_NATIVE_INT, &numberOfPoints) != 0)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read points, error reading NBR attribute");
    return false;
  }
  H5Aclose(attributeId);

  // Check the datatype
  const hid_t datatype = H5Dget_type(dataset);
  const hid_t type = H5Tget_class(datatype);
  if (type != H5T_FLOAT)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read points, not H5T_FLOAT");
    return false;
  }

  // Med vertices are not strided like VTKs, so we copy
  // (not x1, y1, z1, x2, y2, z2, ... but x1, x2, ..., y1, y2, ..., z1, z2, ...)
  auto* pts = new double[numberOfPoints * 3];
  H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pts);
  H5Dclose(dataset);
  points->SetNumberOfPoints(numberOfPoints);
  for (vtkIdType i = 0; i < numberOfPoints; i++)
  {
    points->SetPoint(i, pts[i], pts[i + numberOfPoints], pts[i + 2 * numberOfPoints]);
  }
  delete[] pts;
  return true;
}

// Reads all the cells in a NOD node
static bool readCells(
  const hid_t fileId,
  HdfNode* nodNode,
  const std::string& cellType,
  vtkSmartPointer<vtkCellArray> const& cells,
  vtkSmartPointer<vtkObject> const& self)
{
  // Open the cells
  const hid_t dataset = H5Dopen(fileId, nodNode->path.c_str(), H5P_DEFAULT);

  // Read the number of cells attribute, MAI->CellTypeGroup->NOD->NBR
  const hid_t nbrId = H5Aopen_name(dataset, "NBR");
  int numberOfCells = -1;
  if (H5Aread(nbrId, H5T_NATIVE_INT, &numberOfCells) != 0)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read cells, error reading NBR attribute");
    return false;
  }
  H5Aclose(nbrId);

  // Check the datatype
  const hid_t datatype = H5Dget_type(dataset);
  const hid_t type = H5Tget_class(datatype);
  if (type != H5T_INTEGER)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read cells, not H5T_INTEGER");
    return false;
  }

  // Check cell type is supported
  if (vertexCount.find(cellType) == vertexCount.end())
  {
    vtkWarningWithObjectMacro(
      self, << "Failed to read cells, \"" << cellType << "\" cell type not supported.");
    return false;
  }
  const vtkIdType cellVertexCount = vertexCount[cellType];
  int* indices = new int[numberOfCells * cellVertexCount];
  H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, indices);
  H5Dclose(dataset);

  // Copy into VTKs cell datastruct
  // Like the vertices the cells are not strided either
  size_t iter = 0;
  cells->AllocateEstimate(numberOfCells, cellVertexCount);
  for (vtkIdType i = 0; i < numberOfCells; i++)
  {
    iter = i;
    cells->InsertNextCell(cellVertexCount);
    for (vtkIdType j = 0; j < cellVertexCount; j++)
    {
      cells->InsertCellPoint(indices[iter] - 1);
      iter += numberOfCells;
    }
  }
  delete[] indices;
  return true;
}

// Reads all the points, cells, pointData, cellData from a mesh node
static bool readMesh(
  const hid_t fileId,
  HdfNode* meshNode,
  vtkMedPointData& medPoints,
  std::list<vtkMedCellData>& medCells,
  vtkSmartPointer<vtkObject> const& self)
{
  medCells.clear();

  // Under each mesh node should exist timepoints, assume only usage of the default
  HdfNode* defaultTimePtNode = meshNode->findChild("-0000000000000000001-0000000000000000001");
  if (defaultTimePtNode == nullptr)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read mesh, there exists no default timepoint");
    return false;
  }

  // Under each timepoint should exist MAI (cells) and NOE (points)
  HdfNode* maiNode = defaultTimePtNode->findChild("MAI");
  HdfNode* noeNode = defaultTimePtNode->findChild("NOE");
  if (maiNode == nullptr || noeNode == nullptr)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read mesh, missing either MAI or NOE groups");
    return false;
  }

  {
    // Under NOE (points) there should exist COO (vertices), FAM (scalars), & NUM (index/unused)
    HdfNode* noeCooNode = noeNode->findChild("COO");
    HdfNode* noeFamNode = noeNode->findChild("FAM");
    HdfNode* noeNumNode = noeNode->findChild("NUM");
    if (noeCooNode == nullptr || noeFamNode == nullptr || noeNumNode == nullptr)
    {
      vtkWarningWithObjectMacro(
        self, << "Failed to read mesh, missing either COO, FAM, or NUM groups");
      return false;
    }

    // Read the points
    medPoints.points = vtkSmartPointer<vtkPoints>::New();
    medPoints.pointData = vtkSmartPointer<vtkPointData>::New();

    if (!readPoints(fileId, noeCooNode, medPoints.points, self))
    {
      vtkWarningWithObjectMacro(self, << "Failed to read mesh, points could not be read");
      return false;
    }
    // Read the point nums/global ids
    vtkSmartPointer<vtkIntArray> ptNums = vtkSmartPointer<vtkIntArray>::New();
    ptNums->SetName("NUM");
    if (!readNums(fileId, noeNumNode, ptNums, self))
    {
      vtkWarningWithObjectMacro(self, << "Failed to read mesh, pointData could not be read");
      return false;
    }
    medPoints.pointData->SetGlobalIds(ptNums);
    // Read the point scalars
    vtkSmartPointer<vtkIntArray> pointFamilies = vtkSmartPointer<vtkIntArray>::New();
    pointFamilies->SetName("FAM");
    if (!readFamilies(fileId, noeFamNode, pointFamilies, self))
    {
      vtkWarningWithObjectMacro(self, << "Failed to read mesh, pointData could not be read");
      return false;
    };
    medPoints.pointData->AddArray(pointFamilies);
  }

  // Under MAI (cells) there should exist a set of hdf groups for each cell
  for (auto* maiNodeChild : maiNode->children)
  {
    vtkMedCellData medCell;
    medCell.cells = vtkSmartPointer<vtkCellArray>::New();
    medCell.cellData = vtkSmartPointer<vtkCellData>::New();
    medCell.cellType = maiNodeChild->name;

    HdfNode* maiNodNode = maiNodeChild->findChild("NOD");
    HdfNode* maiFamNode = maiNodeChild->findChild("FAM");
    HdfNode* maiNumNode = maiNodeChild->findChild("NUM");
    if (maiNodNode == nullptr || maiFamNode == nullptr || maiNumNode == nullptr)
    {
      vtkWarningWithObjectMacro(
        self, << "Failed to read mesh, missing either NOD, FAM, or NUM groups");
      return false;
    }

    // Read the cells
    if (!readCells(fileId, maiNodNode, medCell.cellType, medCell.cells, self))
    {
      vtkWarningWithObjectMacro(self, << "Failed to read mesh, cells could not be read");
      return false;
    }
    // Read the nums/global ids
    vtkSmartPointer<vtkIntArray> cellNums = vtkSmartPointer<vtkIntArray>::New();
    cellNums->SetName("NUM");
    if (!readNums(fileId, maiNumNode, cellNums, self))
    {
      vtkWarningWithObjectMacro(self, << "Failed to read mesh, cellData could not be read");
      return false;
    }
    medCell.cellData->SetGlobalIds(cellNums);
    // Read the cell scalars
    vtkSmartPointer<vtkIntArray> cellFamilies = vtkSmartPointer<vtkIntArray>::New();
    cellFamilies->SetName("FAM");
    if (!readFamilies(fileId, maiFamNode, cellFamilies, self))
    {
      vtkWarningWithObjectMacro(self, << "Failed to read mesh, cellData could not be read");
      return false;
    }
    medCell.cellData->AddArray(cellFamilies);
    medCells.push_back(medCell);
  }
  return true;
}

// Reads the strings from a NOM node
static bool readTags(
  const hid_t fileId,
  HdfNode* nomNode,
  vtkSmartPointer<vtkStringArray> const& tags,
  vtkSmartPointer<vtkObject> const& self)
{
  const hid_t dataset = H5Dopen(fileId, nomNode->path.c_str(), H5P_DEFAULT);
  const hid_t fileType = H5Dget_type(dataset);
  const hid_t arrayNdims = H5Tget_array_ndims(fileType);
  if (arrayNdims != 1)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read tags, incorrect string array dimensions");
    return false;
  }
  hsize_t arrayDims;
  H5Tget_array_dims(fileType, &arrayDims);
  // This is a nd dataset of 1d arrays of 80 8 bit integers
  if (arrayDims != 80)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read tags, string not length 80");
    return false;
  }
  const hid_t dataspace = H5Dget_space(dataset);
  const hid_t nDims = H5Sget_simple_extent_ndims(dataspace);
  if (nDims != 1)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read tags, incorrect string array dimensions");
    return false;
  }
  hsize_t dim;
  H5Sget_simple_extent_dims(dataspace, &dim, nullptr);

  // Finally allocate and read the data
  int* nomData = new int[arrayDims * dim];
  const hid_t memType = H5Tarray_create(H5T_NATIVE_INT, 1, &arrayDims);
  H5Dread(dataset, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT, nomData);
  H5Dclose(dataset);
  H5Tclose(memType);
  for (hsize_t j = 0; j < dim; j++)
  {
    std::string str;
    for (size_t k = 0; k < arrayDims; k++)
    {
      char c = static_cast<char>(nomData[k + j * arrayDims]);
      if (c == 0)
      {
        break;
      }
      str += c;
    }
    tags->InsertNextValue(str.c_str());
  }
  delete[] nomData;

  return true;
}

// Reads all the tags from a mesh tag node
static bool readMeshTags(
  const hid_t fileId,
  HdfNode* meshTagNode,
  std::unordered_map<int, vtkSmartPointer<vtkStringArray>>& meshTags,
  vtkSmartPointer<vtkObject> self)
{
  auto readTagsFunc = [&](HdfNode* node) {
    for (auto* childNode : node->children)
    {
      HdfNode* groNode = childNode->findChild("GRO");
      HdfNode* nomNode = groNode->findChild("NOM");
      if (groNode == nullptr || nomNode == nullptr)
      {
        vtkWarningWithObjectMacro(
          self, << "Failed to read mesh tags, missing either GRO or NOM node");
        return false;
      }

      // Read the tag
      int tag = 0;
      const hid_t childNodeId = H5Gopen(fileId, childNode->path.c_str(), H5P_DEFAULT);
      const hid_t numId = H5Aopen_name(childNodeId, "NUM");
      if (H5Aread(numId, H5T_NATIVE_INT, &tag) != 0)
      {
        vtkWarningWithObjectMacro(self, << "Failed to read mesh tags, error reading NUM attribute");
        return false;
      }
      H5Aclose(numId);
      H5Gclose(childNodeId);

      // Read the tag strings
      vtkSmartPointer<vtkStringArray> tags = vtkSmartPointer<vtkStringArray>::New();
      if (!readTags(fileId, nomNode, tags, self))
      {
        vtkWarningWithObjectMacro(self, << "Failed to read mesh tags, tags could not be read");
        return false;
      }

      meshTags[tag] = tags;
    }
    return true;
  };

  HdfNode* elemeNode = meshTagNode->findChild("ELEME");
  // HdfNode* familleZeroNode = meshTagNode->findChild("FAMILLE_ZERO");
  HdfNode* noeudNode = meshTagNode->findChild("NOEUD");

  if (elemeNode && !readTagsFunc(elemeNode))
  {
    return false;
  }
  if (noeudNode && !readTagsFunc(noeudNode))
  {
    return false;
  }
  return true;
}

static bool isValidVersion(
  const hid_t fileId,
  HdfNode* infosNode,
  int minValidMajor,
  int minValidMinor,
  int minValidRelease,
  vtkSmartPointer<vtkObject> const& self)
{
  // Read the attribute on the INFOS_GENERALES
  int major = 0;
  const hid_t infosNodeId = H5Gopen(fileId, infosNode->path.c_str(), H5P_DEFAULT);
  const hid_t majId = H5Aopen_name(infosNodeId, "MAJ");
  if (H5Aread(majId, H5T_NATIVE_INT, &major) != 0)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read version");
    return false;
  }
  H5Aclose(majId);
  int minor = 0;
  const hid_t minId = H5Aopen_name(infosNodeId, "MIN");
  if (H5Aread(minId, H5T_NATIVE_INT, &minor) != 0)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read version");
    return false;
  }
  H5Aclose(minId);
  int release = 0;
  const hid_t relId = H5Aopen_name(infosNodeId, "REL");
  if (H5Aread(relId, H5T_NATIVE_INT, &release) != 0)
  {
    vtkWarningWithObjectMacro(self, << "Failed to read version");
    return false;
  }
  H5Aclose(relId);
  H5Gclose(infosNodeId);

  // Check the version is greater than the minimum required
  if (major > minValidMajor)
  {
    return true;
  }
  if (major == minValidMajor)
  {
    if (minor > minValidMinor)
    {
      return true;
    }
    if (minor == minValidMinor)
    {
      if (release >= minValidRelease)
      {
        return true;
      }
    }
  }
  return false;
}

static void setGroupBlock(
  vtkSmartPointer<vtkMultiBlockDataSet> const& output,
  const vtkMedPointData& medPoints,
  std::list<vtkMedCellData>& medCells,
  std::unordered_map<int, vtkSmartPointer<vtkStringArray>>& meshTags,
  vtkSmartPointer<vtkObject> const& self)
{
  // Establish three small maps before performing the split
  // 1.) group name -> block index (establish a block for every group)
  // 2.) block index -> group name (inverse of above)
  // 3.) **fam id -> N block indices** (every fam has a set of blocks/groups)
  std::unordered_map<std::string, size_t> groupNameToBlockIndex;
  std::unordered_map<size_t, std::string> blockIndexToGroupName;
  std::unordered_map<size_t, std::vector<size_t>> famIntToBlockIndices;
  for (auto& i : meshTags)
  {
    vtkSmartPointer<vtkStringArray> stringArr = i.second;
    for (vtkIdType j = 0; j < stringArr->GetNumberOfValues(); j++)
    {
      const std::string& val = stringArr->GetValue(j);
      if (!groupNameToBlockIndex.count(val))
      {
        size_t blockIndex = groupNameToBlockIndex.size();
        groupNameToBlockIndex[val] = blockIndex;
        blockIndexToGroupName[blockIndex] = val;
      }
      famIntToBlockIndices[i.first].push_back(groupNameToBlockIndex[val]);
    }
  }

  // Setup each's block/groups cell array, we will build them all at once
  const size_t numBlocks = groupNameToBlockIndex.size();
  std::vector<vtkMedCellData> blockedCells(numBlocks);
  for (auto& blockedCell : blockedCells)
  {
    blockedCell.cells = vtkSmartPointer<vtkCellArray>::New();
    blockedCell.cells->InitTraversal();
    blockedCell.cellData = vtkSmartPointer<vtkCellData>::New();
    vtkSmartPointer<vtkIntArray> globalIds = vtkSmartPointer<vtkIntArray>::New();
    globalIds->SetName("NUM");
    blockedCell.cellData->SetGlobalIds(globalIds);
  }

  {
    // Split the geometry into the cell arrays by face families (loop over all cells, copying into appropriate group)
    for (auto& medCell : medCells)
    {
      vtkSmartPointer<vtkCellArray> cells = medCell.cells;
      vtkSmartPointer<vtkCellData> cellData = medCell.cellData;
      const std::string& cellType = medCell.cellType;
      vtkSmartPointer<vtkIntArray> cellFams = vtkIntArray::SafeDownCast(cellData->GetArray("FAM"));
      vtkSmartPointer<vtkIntArray> cellNums = vtkIntArray::SafeDownCast(cellData->GetGlobalIds());

      cells->InitTraversal();
      vtkNew<vtkIdList> cellIds;
      for (vtkIdType j = 0; j < cells->GetNumberOfCells(); j++)
      {
        cells->GetNextCell(cellIds);
        const int fam = cellFams->GetValue(j);
        const int globalId = cellNums->GetValue(j);
        if (fam == 0) // Default family
        {
          continue;
        }

        // Copy this cell to all the blocks
        const std::vector<size_t>& blockIndices = famIntToBlockIndices[fam];
        for (auto const blockIndex : blockIndices)
        {
          blockedCells[blockIndex].cells->InsertNextCell(cellIds);
          vtkIntArray::SafeDownCast(blockedCells[blockIndex].cellData->GetGlobalIds())
            ->InsertNextValue(globalId); // Check this
          blockedCells[blockIndex].cellType = cellType;
        }
      }
    }
  }
  {
    // Split the geometry by point families (points are given cells in the vtkCellArray)
    vtkSmartPointer<vtkPoints> points = medPoints.points;
    vtkSmartPointer<vtkPointData> pointData = medPoints.pointData;
    vtkSmartPointer<vtkIntArray> pointFams = vtkIntArray::SafeDownCast(pointData->GetArray("FAM"));
    vtkSmartPointer<vtkIntArray> pointNums = vtkIntArray::SafeDownCast(pointData->GetGlobalIds());
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
    {
      double pt[3];
      points->GetPoint(i, pt);
      const int fam = pointFams->GetValue(i);
      const int globalId = pointNums->GetValue(i);
      if (fam == 0) // Default family
      {
        continue;
      }

      // Create point reference in the block
      vtkSmartPointer<vtkStringArray> stringArr = meshTags[fam];
      const std::vector<size_t>& blockIndices = famIntToBlockIndices[fam];
      for (auto const blockIndex : blockIndices)
      {
        vtkNew<vtkIdList> cellIds;
        cellIds->InsertId(0, i);
        blockedCells[blockIndex].cells->InsertNextCell(cellIds);
        blockedCells[blockIndex].cellType = "P01";
        // Here, point global ids turn into cell global ids, these overlap with the other cells
        vtkIntArray::SafeDownCast(blockedCells[blockIndex].cellData->GetGlobalIds())
          ->InsertNextValue(globalId);
      }
    }
  }
  for (auto& blockedCell : blockedCells)
  {
    vtkIntArray::SafeDownCast(blockedCell.cellData->GetGlobalIds())->Squeeze();
  }

  // Form the output
  output->SetNumberOfBlocks(static_cast<unsigned int>(numBlocks));
  for (int i = 0; i < static_cast<int>(numBlocks); i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> mesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
    const std::string medCellType = blockedCells[i].cellType;
    if (medToVtkCellType.find(medCellType) == medToVtkCellType.end())
    {
      output->SetBlock(i, nullptr);
      vtkWarningWithObjectMacro(self, << "Med cell type unsupported");
      continue;
    }
    mesh->SetCells(medToVtkCellType[medCellType], blockedCells[i].cells);
    mesh->GetCellData()->SetGlobalIds(blockedCells[i].cellData->GetGlobalIds());
    mesh->SetPoints(medPoints.points);
    // copy globalIds, too.
    mesh->GetPointData()->ShallowCopy(medPoints.pointData);
    output->SetBlock(i, mesh);
    output->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), blockIndexToGroupName[i]);
  }
}

static void setMasterBlock(
  vtkSmartPointer<vtkMultiBlockDataSet> const& masterBlock,
  const vtkMedPointData& medPoints,
  const std::list<vtkMedCellData>& medCells)
{
  masterBlock->SetNumberOfBlocks(static_cast<unsigned int>(medCells.size()));
  int j = 0;
  for (auto const& medCell : medCells)
  {
    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();

    // Set the points, fams, and global ids
    grid->SetPoints(medPoints.points);
    vtkDataArray* ptFams = medPoints.pointData->GetArray("FAM");
    if (ptFams != nullptr)
    {
      grid->GetPointData()->AddArray(ptFams);
    }
    vtkDataArray* ptNums = medPoints.pointData->GetGlobalIds();
    if (ptNums != nullptr)
    {
      grid->GetPointData()->SetGlobalIds(ptNums);
    }

    // Set the cells
    grid->SetCells(medToVtkCellType[medCell.cellType], medCell.cells);
    vtkDataArray* cellFams = medCell.cellData->GetArray("FAM");
    if (cellFams != nullptr)
    {
      grid->GetCellData()->AddArray(cellFams);
    }
    vtkDataArray* cellNums = medCell.cellData->GetGlobalIds();
    if (cellNums != nullptr)
    {
      grid->GetCellData()->SetGlobalIds(cellNums);
    }

    masterBlock->SetBlock(j++, grid);
  }
}

//----------------------------------------------------------------------------
int vtkMedReader::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** /*inputVec*/,
  vtkInformationVector* outputVec)
{
  // Make sure the file exists
  if (!vtksys::SystemTools::FileExists(FileName))
  {
    vtkWarningMacro(
      << ("Failed to read file. File " + std::string(FileName) + " does not exist").c_str());
    return 0;
  }

  // Open up the HDF file and root group
  const hid_t fileId = H5Fopen(FileName, H5F_ACC_RDONLY, H5P_DEFAULT);
  const hid_t rootGroupId = H5Gopen(fileId, ".", H5P_DEFAULT);

  // Build our own tree from the HDF file for easier movement
  // There should be no cicular dependencies in med files
  HdfNode* rootNode = rootBuildTree(rootGroupId);

  // There should be one INFOS_GENERALES
  HdfNode* infosNode = rootNode->findChild("INFOS_GENERALES");
  // Version >= 3.2.0
  if (infosNode == nullptr || !isValidVersion(fileId, infosNode, 3, 2, 0, this))
  {
    vtkWarningMacro(<< "Invalid med file version, need at least 3.2.0");
    Cleanup(fileId, rootNode);
    return 0;
  }

  // There should be one ENS_MAA (mesh ensemble) and FAS
  HdfNode* ensMaaNode = rootNode->findChild("ENS_MAA");
  HdfNode* fasNode = rootNode->findChild("FAS");
  if (ensMaaNode == nullptr || fasNode == nullptr)
  {
    vtkWarningMacro(<< "Failed to read file, missing either mesh ensemble or tags");
    Cleanup(fileId, rootNode);
    return 0;
  }

  // Get the output of this filter
  vtkInformation* outInfo = outputVec->GetInformationObject(0);
  vtkMultiBlockDataSet* output =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // One block for every mesh in the file
  output->SetNumberOfBlocks(static_cast<unsigned int>(ensMaaNode->children.size()));
  // For every mesh
  int i = 0;
  for (auto const& iter : ensMaaNode->children)
  {
    // Read the mesh points and cells
    HdfNode* meshNode = iter;
    vtkMedPointData medPoints;
    std::list<vtkMedCellData> medCells;
    if (!meshNode || !readMesh(fileId, meshNode, medPoints, medCells, this))
    {
      vtkWarningMacro("Failed to read file, mesh could not be read");
      Cleanup(fileId, rootNode);
      return 0;
    }
    output->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), meshNode->name);

    // There should be tag info under the same name in fasNode (suprisingly no id's for meshes in MED files)
    HdfNode* meshTagsNode = fasNode->findChild(meshNode->name);

    std::unordered_map<int, vtkSmartPointer<vtkStringArray>> meshTags;
    if (meshTagsNode && !readMeshTags(fileId, meshTagsNode, meshTags, this))
    {
      vtkWarningMacro("Failed to read file, mesh tags could not be read");
      Cleanup(fileId, rootNode);
      return 0;
    }

    vtkNew<vtkMultiBlockDataSet> meshOutput;
    output->SetBlock(i, meshOutput);
    meshOutput->SetNumberOfBlocks(2);
    vtkNew<vtkMultiBlockDataSet> masterBlock;
    vtkNew<vtkMultiBlockDataSet> groupBlock;
    meshOutput->SetBlock(0, masterBlock);
    meshOutput->SetBlock(1, groupBlock);

    // Provide all the data per cell type in block 1
    setMasterBlock(masterBlock, medPoints, medCells);

    // Provide all the groups per group in block 2
    setGroupBlock(groupBlock, medPoints, medCells, meshTags, this);
    i++;
  }

  // Cleanup
  Cleanup(fileId, rootNode);
  return 1;
}

//----------------------------------------------------------------------------
void vtkMedReader::Cleanup(hid_t fileId, HdfNode* rootNode)
{
  H5Gclose(rootNode->locId);
  H5Fclose(fileId);
  deleteTree(rootNode);
}

//----------------------------------------------------------------------------
int vtkMedReader::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    return 1;
  }
  return 0;
}

//----------------------------------------------------------------------------
void vtkMedReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "FileName: " << this->FileName << "\n";
}
