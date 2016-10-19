//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include "smtk/extension/vtk/io/ImportVTKData.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"

#include "vtkAOSDataArrayTemplate.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLUnstructuredGridWriter.h"

#include "vtksys/SystemTools.hxx"

#include "moab/ReadUtilIface.hpp"

#include "smtk/mesh/moab/Interface.h"
#include "smtk/mesh/moab/CellTypeToType.h"

namespace smtk {
namespace extension {
namespace vtk {
namespace io {

namespace
{

smtk::mesh::CellType vtkToSMTKCell(int t)
  {
  smtk::mesh::CellType ctype = smtk::mesh::CellType_MAX;
  switch (t)
    {
    case VTK_VERTEX:
      ctype = smtk::mesh::Vertex;
      break;
    case VTK_LINE:
      ctype = smtk::mesh::Line;
      break;
    case VTK_TRIANGLE:
      ctype = smtk::mesh::Triangle;
      break;
    case VTK_QUAD:
      ctype = smtk::mesh::Quad;
      break;
    case VTK_POLYGON:
      ctype = smtk::mesh::Polygon;
      break;
    case VTK_TETRA:
      ctype = smtk::mesh::Tetrahedron;
      break;
    case VTK_PYRAMID:
      ctype = smtk::mesh::Pyramid;
      break;
    case VTK_WEDGE:
      ctype = smtk::mesh::Wedge;
      break;
    case VTK_HEXAHEDRON:
      ctype = smtk::mesh::Hexahedron;
      break;
    default:
      ctype = smtk::mesh::CellType_MAX;
      break;
    }
  return ctype;
  }

//----------------------------------------------------------------------------
template <typename VTKDataSetType>
smtk::mesh::HandleRange convertVTKDataSet(
  VTKDataSetType* dataset, smtk::mesh::BufferedCellAllocatorPtr& alloc)
{
  if (!alloc->reserveNumberOfCoordinates(dataset->GetPoints()->
                                          GetNumberOfPoints()))
    {
    return smtk::mesh::HandleRange();
    }

  //note this could become a performance bottleneck. If that occurs
  //we will need to move to a template dispatch solution to handle floats,
  //doubles, and vtk Mapped Arrays
  double point[3];
  for (vtkIdType i = 0; i< dataset->GetPoints()->GetNumberOfPoints(); ++i)
    {
    dataset->GetPoints()->GetPoint(i, point);
    alloc->setCoordinate(i, point);
    }

  vtkIdType npts, *pts;
  for (vtkIdType i = 0; i < dataset->GetNumberOfCells(); ++i)
    {
    dataset->GetCellPoints( i, npts, pts );
    alloc->addCell(vtkToSMTKCell(dataset->GetCellType(i)), pts, npts);
    }
  if (!alloc->flush())
    {
    return smtk::mesh::HandleRange();
    }

  return alloc->cells();
}

//----------------------------------------------------------------------------
template<typename TReader>
vtkDataSet* readXMLFile(const std::string& fileName)
{
  vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();
  reader->GetOutput()->Register(reader);
  return vtkDataSet::SafeDownCast(reader->GetOutput());
}

//----------------------------------------------------------------------------
bool convertDomain(vtkCellData* cellData,
                   const smtk::mesh::InterfacePtr& iface,
                   const smtk::mesh::HandleRange& cells,
                   const std::string& materialPropertyName)
{
  if(cellData == NULL)
    {
    //we have no information
    return false;
    }

  vtkDataArray* materialData = cellData->GetArray( materialPropertyName.c_str() );
  if(!materialData || materialData->GetNumberOfComponents() != 1)
    { //needs to be a scalar array
    return false;
    }

  if(materialData->GetNumberOfTuples() != static_cast<int>(cells.size()))
    { //we currently don't support applying material when
      //we only loaded in some of the cells
    return false;
    }

  std::map< int, smtk::mesh::HandleRange > meshes;
  //move each cell from the entire pool, into a range
  //that represents that material mesh. This is slowish.

  typedef smtk::mesh::HandleRange::const_iterator cit;
  vtkIdType index = 0;
  for(cit i=cells.begin(); i!=cells.end(); ++i, ++index)
    {
    const int currentMaterial = static_cast<int>(materialData->GetTuple1(index));
    meshes[currentMaterial].insert(*i);
    }

  typedef std::map< int, smtk::mesh::HandleRange >::const_iterator map_cit;
  for(map_cit i=meshes.begin(); i != meshes.end(); ++i)
    {
    smtk::mesh::Handle meshId;
    const bool created = iface->createMesh(i->second, meshId);
    if(created)
      {
      smtk::mesh::HandleRange meshHandles;
      meshHandles.insert(meshId);
      //assign a material id to the mesh
      iface->setDomain(meshHandles, smtk::mesh::Domain(i->first));
      }
    }

   return true;
}

}

//----------------------------------------------------------------------------
ImportVTKData::ImportVTKData()
{

}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
ImportVTKData::operator()(const std::string& filename,
                          smtk::mesh::ManagerPtr& manager,
                          std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr collection = manager->makeCollection();
  return this->operator()(filename, collection, materialPropertyName) ?
    collection : smtk::mesh::CollectionPtr();
}

//----------------------------------------------------------------------------
bool ImportVTKData::operator()(const std::string& filename,
                               smtk::mesh::CollectionPtr collection,
                               std::string materialPropertyName) const
{
  std::string extension =
    vtksys::SystemTools::GetFilenameLastExtension(filename.c_str());

  // Dispatch based on the file extension
  vtkDataSet* data;
  if (extension == ".vtu")
   {
   data = readXMLFile<vtkXMLUnstructuredGridReader> (filename);
   return this->operator()(vtkUnstructuredGrid::SafeDownCast(data),
                           collection,
                           materialPropertyName);
   }
  else if (extension == ".vtp")
   {
   data = readXMLFile<vtkXMLPolyDataReader> (filename);
   return this->operator()(vtkPolyData::SafeDownCast(data),
                           collection,
                           materialPropertyName);
   }

  return false;
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet ImportVTKData::operator()(
  vtkPolyData* polydata, smtk::mesh::CollectionPtr collection) const
{
  //make sure we have a valid poly data
  if(!polydata)
    {
    return smtk::mesh::MeshSet();
    }
  else if( polydata->GetNumberOfPoints() == 0 ||
           polydata->GetNumberOfCells() == 0)
    {
    //early terminate if the polydata is empty.
    return smtk::mesh::MeshSet();
    }

  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  if (polydata->NeedToBuildCells())
    {
    polydata->BuildCells();
    }
  smtk::mesh::HandleRange cells = convertVTKDataSet(polydata, alloc);

  return collection->createMesh(smtk::mesh::CellSet(collection, cells));
}

//----------------------------------------------------------------------------
bool ImportVTKData::operator()(vtkPolyData* polydata,
                               smtk::mesh::CollectionPtr collection,
                               std::string materialPropertyName) const
{
  //make sure we have valid data
  if(!polydata)
    {
    return false;
    }
  else if( polydata->GetNumberOfPoints() == 0 ||
           polydata->GetNumberOfCells() == 0)
    {
    //early terminate if the polydata is empty.
    return false;
    }

  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  if (polydata->NeedToBuildCells())
    {
    polydata->BuildCells();
    }
  smtk::mesh::HandleRange cells = convertVTKDataSet(polydata, alloc);

  if (materialPropertyName.empty())
    { //if we don't have a material we create a single mesh
    smtk::mesh::Handle vtkMeshHandle;
    return iface->createMesh(cells, vtkMeshHandle);
    }
  else
    { //make multiple meshes each one assigned a material value
    return convertDomain(polydata->GetCellData(),
                         iface,
                         cells,
                         materialPropertyName);
    }
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
ImportVTKData::operator()(vtkPolyData* polydata,
                          smtk::mesh::ManagerPtr& manager,
                          std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  return this->operator()(polydata,c,materialPropertyName) ? c :
    smtk::mesh::CollectionPtr();
}

//----------------------------------------------------------------------------
bool ImportVTKData::operator()(vtkUnstructuredGrid* ugrid,
                               smtk::mesh::CollectionPtr collection,
                               std::string materialPropertyName) const
{
  //make sure we have valid data
  if(!ugrid)
    {
    return false;
    }
  else if( ugrid->GetNumberOfPoints() == 0 ||
           ugrid->GetNumberOfCells() == 0)
    {
    //early terminate if the ugrid is empty.
    return false;
    }

  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  smtk::mesh::HandleRange cells = convertVTKDataSet(ugrid, alloc);

  if (materialPropertyName.empty())
    { //if we don't have a material we create a single mesh
    smtk::mesh::Handle vtkMeshHandle;
    return iface->createMesh(cells, vtkMeshHandle);
    }
  else
    { //make multiple meshes each one assigned a material value
    return convertDomain(ugrid->GetCellData(),
                         iface,
                         cells,
                         materialPropertyName);
    }
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
ImportVTKData::operator()(vtkUnstructuredGrid* ugrid,
                          smtk::mesh::ManagerPtr& manager,
                          std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  return this->operator()(ugrid,c,materialPropertyName) ? c :
    smtk::mesh::CollectionPtr();
}

}
}
}
}
