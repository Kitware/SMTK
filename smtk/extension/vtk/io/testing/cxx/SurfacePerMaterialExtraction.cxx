

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/io/ImportVTKData.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/WriteMesh.h"
#include "smtk/io/mesh/MeshIO.h"

#include "vtkNew.h"
#include "vtkParametricBoy.h"
#include "vtkParametricFunctionSource.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkTetra.h"
#include "vtkUnstructuredGrid.h"
#include "vtkWedge.h"
#include "vtksys/SystemTools.hxx"

#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLPolyDataReader.h"

namespace
{

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
class ShellPerMesh : public smtk::mesh::MeshForEach
{
  int currentMaterialValue;
public:
  //--------------------------------------------------------------------------
  ShellPerMesh():
    smtk::mesh::MeshForEach(),
    currentMaterialValue(0)
    {
    }

  //--------------------------------------------------------------------------
  void forMesh(smtk::mesh::MeshSet& mesh)
  {
  smtk::mesh::CellSet existingShellCells = mesh.cells( smtk::mesh::Dims2 );
  smtk::mesh::MeshSet shell = mesh;
  if(existingShellCells.is_empty())
    {
    shell = mesh.extractShell();
    }
  else
    {
    shell = this->m_collection->createMesh( existingShellCells );
    }

  smtk::mesh::Domain domain(currentMaterialValue++);
  this->m_collection->setDomainOnMeshes( shell, domain );
  }
};

//----------------------------------------------------------------------------
void createShellPerMaterial(const smtk::mesh::CollectionPtr& c)
{
  //for each material we iterate the meshsets
  typedef std::vector< smtk::mesh::Domain > DomainVecType;
  typedef DomainVecType::const_iterator dom_cit;

  DomainVecType domains = c->domains();

  for(dom_cit dom = domains.begin(); dom != domains.end(); ++dom)
    {
    smtk::mesh::MeshSet domainMeshes = c->meshes(*dom);
    smtk::mesh::MeshSet domainShell = domainMeshes.extractShell();
    c->setDomainOnMeshes( domainShell, *dom );

    c->removeMeshes( domainMeshes );
    }
}
//----------------------------------------------------------------------------
void breakMaterialsByCellType(const smtk::mesh::CollectionPtr& c)
{
  //for each material we iterate the meshsets
  typedef std::vector< smtk::mesh::Domain > DomainVecType;
  typedef DomainVecType::const_iterator dom_cit;

  DomainVecType domains = c->domains();

  int domainsMade = 0;
  for(dom_cit dom = domains.begin(); dom != domains.end(); ++dom)
    {
    smtk::mesh::MeshSet domainMeshes = c->meshes(*dom);

    //determine if we have mixed cell types for a given
    //material
    int numCellTypes = 0;
    for(int i = smtk::mesh::Line; i != smtk::mesh::CellType_MAX; ++i)
      {
      smtk::mesh::CellType ct = static_cast<smtk::mesh::CellType>(i);
      smtk::mesh::CellSet cells = domainMeshes.cells(ct);
      numCellTypes += cells.is_empty() ? 0 : 1;
      }

    if(numCellTypes > 1)
      {
      //Iterate over all the different cell types,
      //creating a single mesh for each cell type
      for(int i = smtk::mesh::Line; i != smtk::mesh::CellType_MAX; ++i)
        {
        smtk::mesh::CellType ct = static_cast<smtk::mesh::CellType>(i);
        smtk::mesh::CellSet cells = domainMeshes.cells(ct);
        if(!cells.is_empty())
          {
          smtk::mesh::MeshSet ms = c->createMesh(cells);
          const int v = (dom->value() * 100) + i;
          c->setDomainOnMeshes( ms, smtk::mesh::Domain(v) );
          domainsMade++;
          }
        }
      //now remove the original material mesh
      c->removeMeshes(domainMeshes);
      }
    }
}

//----------------------------------------------------------------------------
template<typename vtkDataSetType >
smtk::mesh::CollectionPtr convert(vtkDataSetType* input,
                                  smtk::mesh::ManagerPtr manager,
                                  std::string material)
{
  smtk::extension::vtk::io::ImportVTKData imprt;

  //we convert the vtk data into a single mesh.
  smtk::mesh::CollectionPtr collection = imprt( input, manager, material );

  if(!collection)
    {
    std::cerr << "unable to import the collection properly" << std::endl;
    }

  return collection;
}

//----------------------------------------------------------------------------
void extractSurfaces(smtk::mesh::CollectionPtr c, std::string outputFile)
  {
  std::cout << "Info on input: " << std::endl;
  std::cout << " volume mesh count: " << c->meshes( smtk::mesh::Dims3 ).size() << std::endl;
  std::cout << " surface mesh count: " << c->meshes( smtk::mesh::Dims2 ).size() << std::endl;
  std::cout << " material count: " << c->domains().size() << std::endl;
  std::cout << " neumann count: " << c->dirichlets().size() << std::endl;
  std::cout << " dirichlet count: " << c->neumanns().size() << std::endl;

  if(c->domains().size() == 0)
    { //if no domains make a fake domain per volumetric mesh
    ShellPerMesh spm;
    smtk::mesh::MeshSet volMeshes = c->meshes( smtk::mesh::Dims3 );
    smtk::mesh::for_each( volMeshes, spm );
    c->removeMeshes( volMeshes  );
    }
  else
    {
    //create a shell per material
    createShellPerMaterial( c );
    }

  //take all meshes that have a material, and break them into a mesh per
  //cell type. This is required since a block in exodus must be of
  //a single cell type
  breakMaterialsByCellType( c );

  std::cout << "number of domains in output: " << c->domains().size() << std::endl;

  smtk::io::writeMesh(outputFile, c, smtk::io::mesh::Subset::EntireCollection);
}

}

//MultiScaleConverter
//Another executalbe that is SurfacePerMaterial
int main(int argc, char* argv[])
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();

  std::string inputFileName(argc > 1 ? argv[1] : "mesh3D.vtu");
  std::string outputFileName(argc > 2 ? argv[2] : "mesh3D.exo");
  std::string materialName(argc > 3 ? argv[3] : std::string());

  std::string extension =
    vtksys::SystemTools::GetFilenameLastExtension(inputFileName.c_str());

  // Dispatch based on the file extension
  vtkDataSet* data;
  smtk::mesh::CollectionPtr c;
  if (extension == ".vtu")
   {
   data = readXMLFile<vtkXMLUnstructuredGridReader> (inputFileName);
   c = convert(vtkUnstructuredGrid::SafeDownCast(data), manager, materialName);
   }
  else if (extension == ".vtp")
   {
   data = readXMLFile<vtkXMLPolyDataReader> (inputFileName);
   c = convert(vtkPolyData::SafeDownCast(data), manager, materialName);
   }
  else if (extension == ".h5m" || extension == ".exo")
   {
   c = smtk::io::importMesh(inputFileName, manager);
   }

  if(!c)
    {
    std::cerr << "failed to load the requested data" << std::endl;
    return 1;
    }
  extractSurfaces(c, outputFileName);
  return 0;
}
