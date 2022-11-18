//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/io/vtkMedWriter.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMedHelper.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSideSetsToScalars.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtksys/SystemTools.hxx"

#include <unordered_map>

vtkStandardNewMacro(vtkMedWriter);

void vtkMedWriter::SetFileName(const std::string& filename)
{
  if (this->FileName == filename)
  {
    return;
  }
  this->FileName = filename;
  this->Modified();
}

const std::string& vtkMedWriter::GetFileName() const
{
  return this->FileName;
}

static void writeInt32Attribute(const hid_t id, const std::string& name, const int value)
{
  const hid_t dataSpaceId = H5Screate(H5S_SCALAR);
  const hid_t attributeId =
    H5Acreate(id, name.c_str(), H5T_NATIVE_INT, dataSpaceId, H5P_DEFAULT, H5P_DEFAULT);
  H5Awrite(attributeId, H5T_NATIVE_INT, &value);
  H5Aclose(attributeId);
  H5Sclose(dataSpaceId);
}

static void writeFloat64Attribute(const hid_t id, const std::string& name, const double value)
{
  const hid_t dataSpaceId = H5Screate(H5S_SCALAR);
  const hid_t attributeId =
    H5Acreate(id, name.c_str(), H5T_NATIVE_DOUBLE, dataSpaceId, H5P_DEFAULT, H5P_DEFAULT);
  H5Awrite(attributeId, H5T_NATIVE_DOUBLE, &value);
  H5Aclose(attributeId);
  H5Sclose(dataSpaceId);
}

static void writeStringAttribute(const hid_t id, const std::string& name, const std::string& value)
{
  const hid_t dataSpaceId = H5Screate(H5S_SCALAR);
  const hid_t atype = H5Tcopy(H5T_C_S1);
  H5Tset_size(atype, value.size() + 1);
  H5Tset_strpad(atype, H5T_STR_NULLTERM);
  const hid_t attributeId =
    H5Acreate2(id, name.c_str(), atype, dataSpaceId, H5P_DEFAULT, H5P_DEFAULT);
  H5Awrite(attributeId, atype, value.c_str());
  H5Aclose(attributeId);
  H5Sclose(dataSpaceId);
}

// Writes COO, FAM, & NUM to the specified group
static bool writePoints(
  const hid_t fileId,
  const std::string& groupPath,
  const vtkMedPointData& medPts,
  vtkSmartPointer<vtkObject> const& self)
{
  vtkPoints* points = medPts.points;
  if (medPts.pointData->GetArray("FAM") == nullptr)
  {
    vtkWarningWithObjectMacro(self, << "Failed to write points, vtkPointData missing FAMs");
    return false;
  }
  vtkIntArray* pointFams = vtkIntArray::SafeDownCast(medPts.pointData->GetArray("FAM"));
  vtkIntArray* pointNums = vtkIntArray::SafeDownCast(medPts.pointData->GetGlobalIds());
  if (pointFams == nullptr || pointNums == nullptr)
  {
    vtkWarningWithObjectMacro(
      self, << "Failed to write points, vtkPointData missing globalids or FAMs");
    return false;
  }

  const vtkIdType numPts = points->GetNumberOfPoints();

  // Write the COO data
  {
    const hsize_t cooDataSpaceDims[1] = { static_cast<hsize_t>(numPts * 3) };
    const hid_t cooSpaceId = H5Screate_simple(1, cooDataSpaceDims, nullptr);
    const hid_t cooDataId = H5Dcreate(
      fileId,
      (groupPath + "/COO").c_str(),
      H5T_NATIVE_DOUBLE,
      cooSpaceId,
      H5P_DEFAULT,
      H5P_DEFAULT,
      H5P_DEFAULT);
    auto* vertexBuffer = new double[numPts * 3];
    for (vtkIdType i = 0; i < numPts; i++)
    {
      double pt[3];
      points->GetPoint(i, pt);

      vertexBuffer[i] = pt[0];
      vertexBuffer[i + numPts] = pt[1];
      vertexBuffer[i + numPts * 2] = pt[2];
    }
    H5Dwrite(cooDataId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, vertexBuffer);
    writeInt32Attribute(cooDataId, "NBR", numPts);
    writeInt32Attribute(cooDataId, "CGT", 1);

    H5Dclose(cooDataId);
    H5Sclose(cooSpaceId);
    delete[] vertexBuffer;
  }

  // Write the NUM data
  {
    const hsize_t numDataSpaceDims[1] = { static_cast<hsize_t>(numPts) };
    const hid_t numSpaceId = H5Screate_simple(1, numDataSpaceDims, nullptr);
    const hid_t numDataId = H5Dcreate(
      fileId,
      (groupPath + "/NUM").c_str(),
      H5T_NATIVE_INT,
      numSpaceId,
      H5P_DEFAULT,
      H5P_DEFAULT,
      H5P_DEFAULT);
    H5Dwrite(numDataId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pointNums->GetPointer(0));
    writeInt32Attribute(numDataId, "NBR", numPts);
    writeInt32Attribute(numDataId, "CGT", 1);

    H5Dclose(numDataId);
    H5Sclose(numSpaceId);
  }

  // Write the FAM data
  {
    const hsize_t famDataSpaceDims[1] = { static_cast<hsize_t>(numPts) };
    const hid_t famSpaceId = H5Screate_simple(1, famDataSpaceDims, nullptr);
    const hid_t famDataId = H5Dcreate(
      fileId,
      (groupPath + "/FAM").c_str(),
      H5T_NATIVE_INT,
      famSpaceId,
      H5P_DEFAULT,
      H5P_DEFAULT,
      H5P_DEFAULT);
    H5Dwrite(famDataId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pointFams->GetPointer(0));
    writeInt32Attribute(famDataId, "NBR", numPts);
    writeInt32Attribute(famDataId, "CGT", 1);

    H5Dclose(famDataId);
    H5Sclose(famSpaceId);
  }
  return true;
}

// Writes FAM, NOD, NUM to the specified group
static bool writeCells(
  const hid_t fileId,
  const std::string& groupPath,
  const vtkMedCellData& medCells,
  vtkSmartPointer<vtkObject> const& self)
{
  vtkCellArray* cells = medCells.cells;
  if (medCells.cellData->GetArray("FAM") == nullptr)
  {
    vtkWarningWithObjectMacro(self, << "Failed to write points, vtkCellData missing FAMs");
    return false;
  }
  vtkIntArray* cellFams = vtkIntArray::SafeDownCast(medCells.cellData->GetArray("FAM"));
  vtkIntArray* cellNums = vtkIntArray::SafeDownCast(medCells.cellData->GetGlobalIds());
  const std::string cellType = medCells.cellType;

  if (cellNums == nullptr || cellFams == nullptr)
  {
    vtkWarningWithObjectMacro(
      self, << "Failed to write cells, vtkCellData missing globalids or scalars");
    return false;
  }

  // Create cell group
  const std::string cellGroupPath = groupPath + '/' + cellType;
  const vtkIdType numCells = cells->GetNumberOfCells();
  const hid_t cellGroupId =
    H5Gcreate(fileId, cellGroupPath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  writeInt32Attribute(cellGroupId, "CGS", 1);
  writeInt32Attribute(cellGroupId, "CGT", 1);
  if (cellType == "TE4")
  {
    writeInt32Attribute(cellGroupId, "GEO", 304);
  }
  else
  {
    writeInt32Attribute(cellGroupId, "GEO", 203);
  }
  writeStringAttribute(cellGroupId, "PFL", "MED_NO_PROFILE_INTERNAL");

  // Write the NOD data
  {
    const vtkIdType numPtsPerCell = vertexCount[medCells.cellType];
    const hsize_t nodDataSpaceDims[1] = { static_cast<hsize_t>(numCells * numPtsPerCell) };
    const hid_t nodSpaceId = H5Screate_simple(1, nodDataSpaceDims, nullptr);
    const hid_t nodDataId = H5Dcreate(
      fileId,
      (cellGroupPath + "/NOD").c_str(),
      H5T_NATIVE_INT,
      nodSpaceId,
      H5P_DEFAULT,
      H5P_DEFAULT,
      H5P_DEFAULT);
    auto* indexBuffer = new int[numCells * numPtsPerCell];
    cells->InitTraversal();
    vtkNew<vtkIdList> ids;
    for (vtkIdType i = 0; i < numCells; i++)
    {
      cells->GetNextCell(ids);
      for (vtkIdType j = 0; j < numPtsPerCell; j++)
      {
        indexBuffer[i + numCells * j] = static_cast<int>(ids->GetId(j)) + 1;
      }
    }
    H5Dwrite(nodDataId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, indexBuffer);
    writeInt32Attribute(nodDataId, "NBR", numCells);
    writeInt32Attribute(nodDataId, "CGT", 1);

    H5Dclose(nodDataId);
    delete[] indexBuffer;
  }

  // Write the NUM data
  {
    const hsize_t numDataSpaceDims[1] = { static_cast<hsize_t>(numCells) };
    const hid_t numSpaceId = H5Screate_simple(1, numDataSpaceDims, nullptr);
    const hid_t numDataId = H5Dcreate(
      fileId,
      (cellGroupPath + "/NUM").c_str(),
      H5T_NATIVE_INT,
      numSpaceId,
      H5P_DEFAULT,
      H5P_DEFAULT,
      H5P_DEFAULT);
    H5Dwrite(numDataId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, cellNums->GetPointer(0));
    writeInt32Attribute(numDataId, "NBR", numCells);
    writeInt32Attribute(numDataId, "CGT", 1);

    H5Dclose(numDataId);
    H5Sclose(numSpaceId);
  }

  // Write the FAM data
  {
    const hsize_t famDataSpaceDims[1] = { static_cast<hsize_t>(numCells) };
    const hid_t famSpaceId = H5Screate_simple(1, famDataSpaceDims, nullptr);
    const hid_t famDataId = H5Dcreate(
      fileId,
      (cellGroupPath + "/FAM").c_str(),
      H5T_NATIVE_INT,
      famSpaceId,
      H5P_DEFAULT,
      H5P_DEFAULT,
      H5P_DEFAULT);
    // Negate cell family ids (because of spec)
    int* famBuffer = new int[cellFams->GetNumberOfValues()];
    std::transform(
      cellFams->Begin(),
      cellFams->Begin() + cellFams->GetNumberOfValues(),
      famBuffer,
      std::negate<int>() /* NOLINT(modernize-use-transparent-functors) */);
    H5Dwrite(famDataId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, famBuffer);
    delete[] famBuffer;
    writeInt32Attribute(famDataId, "NBR", numCells);
    writeInt32Attribute(famDataId, "CGT", 1);

    H5Dclose(famDataId);
    H5Sclose(famSpaceId);
  }

  H5Gclose(cellGroupId);
  return true;
}

static bool writeTags(
  const hid_t fileId,
  const std::string& elemePath,
  const std::unordered_map<int, std::string>& familyNames,
  std::unordered_map<int, std::vector<std::string>>& familyGroupNames,
  bool isCells) // is cells or points?
{
  // For every family
  for (auto const& i : familyNames)
  {
    const int famId = i.first;
    if (isCells && famId > 0) // If cells, skip if positive family id
    {
      continue;
    }
    if (!isCells && famId < 0) // If points, skip if negative family id
    {
      continue;
    }
    const std::vector<std::string>& groupNames = familyGroupNames[famId];

    // Create a group for the family
    //std::string tagGroupName = elemePath + "/FAM_" + std::to_string(famId) + "_" + i.second;
    std::string tagGroupName = elemePath + "/FAM_" + std::to_string(famId);
    const hid_t tagGroupId =
      H5Gcreate(fileId, tagGroupName.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    writeInt32Attribute(tagGroupId, "NUM", famId);

    {
      // Create a GRO subgroup
      const std::string groGroupName = tagGroupName + "/GRO";
      const hid_t groGroupId =
        H5Gcreate(fileId, groGroupName.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      writeInt32Attribute(groGroupId, "NBR", static_cast<int>(groupNames.size()));

      const hsize_t arrayDim = 80;
      const hid_t memType = H5Tarray_create(H5T_NATIVE_INT, 1, &arrayDim);
      const hid_t fileType = H5Tarray_create(H5T_STD_I8LE, 1, &arrayDim);

      const hsize_t groDataSpaceDims[1] = { static_cast<hsize_t>(groupNames.size()) };
      const hid_t groSpaceId = H5Screate_simple(1, groDataSpaceDims, nullptr);
      const hid_t groDataId = H5Dcreate(
        fileId,
        (groGroupName + "/NOM").c_str(),
        fileType,
        groSpaceId,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT);

      int* buffer = new int[arrayDim * groDataSpaceDims[0]];
      for (int j = 0; j < static_cast<int>(groDataSpaceDims[0]); j++)
      {
        const std::string& str = groupNames[j];
        for (int k = 0; k < static_cast<int>(arrayDim); k++)
        {
          if (k < static_cast<int>(str.length()))
          {
            buffer[j * arrayDim + k] = static_cast<int>(static_cast<unsigned char>(str[k]));
          }
          else
          {
            buffer[j * arrayDim + k] = 0;
          }
        }
      }
      H5Dwrite(groDataId, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

      H5Dclose(groDataId);
      H5Sclose(groSpaceId);
      H5Tclose(fileType);
      H5Tclose(memType);
      H5Gclose(groGroupId);
      delete[] buffer;
    }

    H5Gclose(tagGroupId);
  }
  return true;
}

static bool writeFamilies(
  const hid_t fileId,
  const std::string& fasPath,
  const std::string& meshName,
  vtkSmartPointer<vtkMultiBlockDataSet> const& groupBlock,
  const std::unordered_map<int, std::set<vtkIdType>>& familyIdToCellGroupSet,
  const std::unordered_map<int, std::set<vtkIdType>>& familyIdToVertexGroupSet,
  vtkSmartPointer<vtkObject> const& self)
{
  // Get all the group names
  std::vector<std::string> groupNames;
  groupNames.reserve(groupBlock->GetNumberOfBlocks());
  for (unsigned int i = 0; i < groupBlock->GetNumberOfBlocks(); i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> input =
      vtkUnstructuredGrid::SafeDownCast(groupBlock->GetBlock(i));
    const std::string name = groupBlock->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
    groupNames.push_back(name);
  }

  // Get all the families
  std::unordered_map<int, std::string> familyNames;
  std::unordered_map<int, std::vector<std::string>> familyGroupNames;
  {
    // For every vertex family
    for (auto const& i : familyIdToVertexGroupSet)
    {
      std::string famName = groupNames[*i.second.begin()];
      familyGroupNames[i.first].push_back(groupNames[*i.second.begin()]);
      for (auto j = std::next(i.second.begin()); j != i.second.end(); j++)
      {
        famName += "_" + groupNames[*j];
        familyGroupNames[i.first].push_back(groupNames[*j]);
      }
      familyNames[i.first] = famName;
    }
    // For every cell family
    // Negate cell family ids (because of spec)
    for (auto const& i : familyIdToCellGroupSet)
    {
      std::string famName = groupNames[*i.second.begin()];
      familyGroupNames[-i.first].push_back(groupNames[*i.second.begin()]);
      for (auto j = std::next(i.second.begin()); j != i.second.end(); j++)
      {
        famName += "_" + groupNames[*j];
        familyGroupNames[-i.first].push_back(groupNames[*j]);
      }
      familyNames[-i.first] = famName;
    }
  }

  const std::string meshFamPath = fasPath + "/" + meshName;
  const hid_t meshFamGroupId =
    H5Gcreate(fileId, meshFamPath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  {
    // Create a group creation property list
    const hid_t gcpl = H5Pcreate(H5P_GROUP_CREATE);
    H5Pset_link_creation_order(gcpl, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
    // Write ELEME families
    const std::string elemePath = meshFamPath + "/ELEME";
    const hid_t elemeGroupId = H5Gcreate(fileId, elemePath.c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);

    if (!writeTags(fileId, elemePath, familyNames, familyGroupNames, true))
    {
      vtkWarningWithObjectMacro(
        self, << "Failed to write families, could not write ELEME/cell families");
      return false;
    }

    H5Gclose(elemeGroupId);
    H5Pclose(gcpl);
  }

  {
    // Create a group creation property list
    const hid_t gcpl = H5Pcreate(H5P_GROUP_CREATE);
    H5Pset_link_creation_order(gcpl, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
    // Write ELEME families
    const std::string noeudPath = meshFamPath + "/NOEUD";
    const hid_t noeudGroupId = H5Gcreate(fileId, noeudPath.c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);

    if (!writeTags(fileId, noeudPath, familyNames, familyGroupNames, false))
    {
      vtkWarningWithObjectMacro(
        self, << "Failed to write families, could not write NOEUD/node families");
      return false;
    }

    H5Gclose(noeudGroupId);
    H5Pclose(gcpl);
  }

  {
    // Create a group creation property list
    const hid_t gcpl = H5Pcreate(H5P_GROUP_CREATE);
    H5Pset_link_creation_order(gcpl, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);

    // Create zero/default group
    const std::string familleZeroPath = meshFamPath + "/FAMILLE_ZERO";
    const hid_t zeroGroupId =
      H5Gcreate(fileId, familleZeroPath.c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);
    writeInt32Attribute(zeroGroupId, "NUM", 0);
    H5Gclose(zeroGroupId);
    H5Pclose(gcpl);
  }

  H5Gclose(meshFamGroupId);
  return true;
}

static bool writeMesh(
  const hid_t fileId,
  const std::string& ensMaaPath,
  const std::string& meshName,
  const vtkMedPointData& medPoints,
  const std::list<vtkMedCellData>& medCells,
  vtkSmartPointer<vtkObject> const& self)
{
  // Create a group under ENS_MAA for the mesh
  // These attributes were copied from example data, it's not clear which
  // are hard required.
  const std::string meshPath = ensMaaPath + "/" + meshName;
  const hid_t meshGroupId =
    H5Gcreate(fileId, meshPath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  writeStringAttribute(meshGroupId, "DES", "");
  writeInt32Attribute(meshGroupId, "DIM", 3);
  writeInt32Attribute(meshGroupId, "ESP", 3);
  writeStringAttribute(meshGroupId, "NOM", "");
  writeInt32Attribute(meshGroupId, "REP", 0);
  writeInt32Attribute(meshGroupId, "SRT", 0);
  writeInt32Attribute(meshGroupId, "TYP", 0);
  writeStringAttribute(meshGroupId, "UNI", "");
  writeStringAttribute(meshGroupId, "UNT", "");

  writeInt32Attribute(meshGroupId, "NXI", -1);
  writeInt32Attribute(meshGroupId, "NXT", -1);

  // Create group for the default timestep
  std::string timestepPath = meshPath + "/-0000000000000000001-0000000000000000001";
  const hid_t dataGroupId =
    H5Gcreate(fileId, timestepPath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  writeInt32Attribute(dataGroupId, "CGT", 1);
  writeInt32Attribute(dataGroupId, "NDT", -1);
  writeInt32Attribute(dataGroupId, "NOR", -1);
  writeFloat64Attribute(dataGroupId, "PDT", -1.0);

  writeInt32Attribute(dataGroupId, "NXI", -1);
  writeInt32Attribute(dataGroupId, "NXT", -1);
  writeInt32Attribute(dataGroupId, "PVI", -1);
  writeInt32Attribute(dataGroupId, "PVT", -1);

  // Write MAI (cell data), inputs must share vertices
  {
    const std::string maiPath = timestepPath + "/MAI";
    const hid_t maiGroupId =
      H5Gcreate(fileId, maiPath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    writeInt32Attribute(maiGroupId, "CGT", 1);
    for (auto const& i : medCells)
    {
      if (!writeCells(fileId, maiPath, i, self))
      {
        vtkWarningWithObjectMacro(self, << "Failed to write mesh, cells could not be written");
        return false;
      }
    }
    H5Gclose(maiGroupId);
  }

  // Write NOE (point data)
  {
    const std::string noePath = timestepPath + "/NOE";
    const hid_t noeGroupId =
      H5Gcreate(fileId, noePath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    writeInt32Attribute(noeGroupId, "CGS", 1);
    writeInt32Attribute(noeGroupId, "CGT", 1);
    writeStringAttribute(noeGroupId, "PFL", "MED_NO_PROFILE_INTERNAL");
    if (!writePoints(fileId, noePath, medPoints, self))
    {
      vtkWarningWithObjectMacro(self, << "Failed to write mesh, points could not be written");
      return false;
    }
    H5Gclose(noeGroupId);
  }

  H5Gclose(dataGroupId);
  H5Gclose(meshGroupId);
  return true;
}

static void writeVersion(const hid_t fileId, const int major, const int minor, const int release)
{
  // Write INFOS_GENERALES (versioning info)
  const hid_t infosGroupId =
    H5Gcreate(fileId, "./INFOS_GENERALES", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  writeInt32Attribute(infosGroupId, "MAJ", major);
  writeInt32Attribute(infosGroupId, "MIN", minor);
  writeInt32Attribute(infosGroupId, "REL", release);
  H5Gclose(infosGroupId);
}

//----------------------------------------------------------------------------
int vtkMedWriter::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** inputVec,
  vtkInformationVector* /*outputVec*/)
{
  if (FileName.empty())
  {
    vtkWarningMacro(<< "Failed to write file, filename not specified");
    return 1;
  }

  const int numInputs = inputVec[0]->GetNumberOfInformationObjects();
  if (numInputs == 0)
  {
    vtkWarningMacro(<< "Failed to write file, no inputs supplied to vtkMedWriter");
    return 1;
  }

  // Create an HDF file
  const hid_t fileId = H5Fcreate(FileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  // Create the ensemble/ENS_MAA group
  const std::string ensMaaPath = "./ENS_MAA";
  const hid_t ensMaaGroupId =
    H5Gcreate(fileId, ensMaaPath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  // Create the family/FAS group
  const std::string fasPath = "./FAS";
  const hid_t fasGroupId =
    H5Gcreate(fileId, fasPath.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  writeVersion(fileId, 3, 2, 0);

  for (int i = 0; i < numInputs; i++)
  {
    // Get the input
    vtkInformation* inInfo = inputVec[0]->GetInformationObject(i);
    vtkMultiBlockDataSet* inputBlockSet =
      vtkMultiBlockDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

    // Verify the input
    if (inputBlockSet->GetNumberOfBlocks() != 2)
    {
      vtkWarningMacro(<< "Failed to write file, invalid input vtkMultiBlockDataSet.");
      return 1;
    }

    vtkMultiBlockDataSet* inputMasterBlock =
      vtkMultiBlockDataSet::SafeDownCast(inputBlockSet->GetBlock(0));
    if (inputMasterBlock == nullptr)
    {
      vtkWarningMacro(
        << "Failed to write file, invalid input, block 0 of input not a vtkMultiBlockDataSet");
      return 1;
    }
    vtkMultiBlockDataSet* inputGroupBlock =
      vtkMultiBlockDataSet::SafeDownCast(inputBlockSet->GetBlock(1));
    if (inputGroupBlock == nullptr)
    {
      vtkWarningMacro(
        << "Failed to write file, invalid input, block 1 of input not a vtkMultiBlockDataSet");
      return 1;
    }

    // Get the first master block to acquire the points which should be shared among all inputs
    if (inputMasterBlock->GetNumberOfBlocks() == 0)
    {
      vtkWarningMacro(<< "Failed to write file, Invalid input, no blocks in master block");
      return 1;
    }
    vtkSmartPointer<vtkUnstructuredGrid> masterBlockInput0 =
      vtkUnstructuredGrid::SafeDownCast(inputMasterBlock->GetBlock(0));
    if (masterBlockInput0 == nullptr)
    {
      vtkWarningMacro(<< "Failed to write file, invalid input, block not vtkUnstructuredGrid.");
      return 1;
    }

    // If the name is present use it, otherwise use the one given by the user
    std::string meshName = "UNNAMED_MESH";
    if (inputBlockSet->GetInformation()->Has(vtkMultiBlockDataSet::NAME()))
    {
      meshName = inputBlockSet->GetInformation()->Get(vtkMultiBlockDataSet::NAME());
      if (meshName.empty())
      {
        meshName = "UNNAMED_MESH";
      }
    }

    // Extract datasets and verify the contents
    std::list<vtkMedCellData> medCells;
    vtkMedPointData medPts;
    medPts.points = masterBlockInput0->GetPoints();
    medPts.pointData = masterBlockInput0->GetPointData();
    for (unsigned int j = 0; j < inputMasterBlock->GetNumberOfBlocks(); j++)
    {
      vtkSmartPointer<vtkUnstructuredGrid> input =
        vtkUnstructuredGrid::SafeDownCast(inputMasterBlock->GetBlock(j));
      if (input == nullptr)
      {
        vtkWarningMacro(<< "Failed to write file, invalid input, block not vtkUnstructuredGrid.");
        return 1;
      }
      if (input->GetPoints() != medPts.points)
      {
        vtkWarningMacro(<< "Failed to write file, invalid input, blocks do not share points");
        return 1;
      }
      vtkMedCellData medCell;
      medCell.cells = input->GetCells();
      medCell.cellData = input->GetCellData();
      if (input->GetNumberOfCells() == 0)
      {
        vtkWarningMacro(
          << "Failed to write file, invalid input, vtkUnstructuredGrid contains no cells");
        return 1;
      }
      medCell.cellType = vtkToMedCellType[input->GetCell(0)->GetCellType()];
      medCells.push_back(medCell);
    }

    // Using this filter we can produce the families from the groups
    vtkNew<vtkSideSetsToScalars> sideSetsFilter;
    sideSetsFilter->SetInputData(0, inputMasterBlock);
    sideSetsFilter->SetInputData(1, inputGroupBlock);
    sideSetsFilter->Update();

    // Write the mesh
    if (!writeMesh(fileId, ensMaaPath, meshName, medPts, medCells, this))
    {
      vtkWarningMacro(<< "Failed to write file, mesh could not be written");
      return 1;
    }
    // Write FAS (field data)
    if (!writeFamilies(
          fileId,
          fasPath,
          meshName,
          inputGroupBlock,
          sideSetsFilter->getFamilyIdToCellGroupSetMap(),
          sideSetsFilter->getFamilyIdToVertexGroupSetMap(),
          this))
    {
      vtkWarningMacro(<< "Failed to write file, FAS/families could not be written");
      return 1;
    }
  }

  H5Gclose(ensMaaGroupId);
  H5Gclose(fasGroupId);

  H5Fclose(fileId);

  return 1;
}

//----------------------------------------------------------------------------
int vtkMedWriter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    return 1;
  }
  return 0;
}

//----------------------------------------------------------------------------
void vtkMedWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "FileName: " << this->FileName << "\n";
}
