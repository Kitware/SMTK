
//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtkToMesh/VTKDataConverter.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/WriteMesh.h"

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
class Filter : public smtk::mesh::CellForEach
{
public:
  Filter() :
    smtk::mesh::CellForEach(true) // needs coordinates
  { }

  smtk::mesh::HandleRange validPoints;
};

//----------------------------------------------------------------------------
class YFilter : public Filter
{
  double yvalue;
public:

  YFilter( double value ) :
    Filter(),
    yvalue( value )
    {
    }

  //--------------------------------------------------------------------------
  void forCell(const smtk::mesh::Handle&,
               smtk::mesh::CellType,
               int numPts)
  {
    const std::vector<double>& coords = this->coordinates();
    const smtk::mesh::Handle* const ptIds = this->pointIds();
    for( int i=0; i < numPts; ++i)
      {
      const double currValue = coords[(i*3)+1];
      //add in a small tolerance

      if(currValue >= (this->yvalue-0.002) &&
         currValue <= (this->yvalue+0.002))
        {
          this->validPoints.insert(ptIds[i]);
        }
      }
  }
};

//----------------------------------------------------------------------------
class OuterEdgeFilter : public Filter
{
  double origin[3];
  double rmin;
public:
  OuterEdgeFilter( const double o[3], double r ) :
    Filter(),
    rmin(r)
    {
      for (int i=0;i<3;i++)
      {
        this->origin[i] = o[i];
      }
    }

  //--------------------------------------------------------------------------
  void forCell(const smtk::mesh::Handle&,
               smtk::mesh::CellType,
               int numPts)
  {
    const std::vector<double>& coords = this->coordinates();
    const smtk::mesh::Handle* const ptIds = this->pointIds();

    if (numPts < 3)
    {
      return;
    }

    double v0[3], v1[3], v2[3], normal[3];
    double len[3] = {0.,0,0.};

    for ( int i=0; i < 3; ++i)
    {
      v0[i] = coords[i] - this->origin[i];
      len[0] += v0[i]*v0[i];
      v1[i] = coords[3+i] - coords[i];
      len[1] += v1[i]*v1[i];
      v2[i] = coords[6+i] - coords[i];
      len[2] += v2[i]*v2[i];
    }

    for ( int i=0; i < 3; ++i)
    {
      len[i] = sqrt(len[i]);
    }

    for ( int i=0; i < 3; ++i)
    {
      v0[i] /= len[0];
      v1[i] /= len[1];
      v2[i] /= len[2];
    }

    int i1,i2;
    for ( int i=0; i < 3; ++i)
    {
      i1 = (i+1)%3;
      i2 = (i+2)%3;
      normal[i] = v1[i1]*v2[i2] - v1[i2]*v2[i1];
    }

    if (fabs(normal[0]) < 1.e-6 ||
        fabs(normal[1]) < 1.e-6 ||
        fabs(normal[2]) < 1.e-6)
    {
      return;
    }

    double dot = 0;
    for ( int i=0; i < 3; ++i)
    {
      dot += v0[i]*normal[i];
    }

    if (dot < .5)
    {
      return;
    }

    for ( int i=0; i < numPts; ++i)
    {
      double r = sqrt(coords[3*i]*coords[3*i] + coords[3*i+2]*coords[3*i+2]);
      if (r > rmin)
      {
        this->validPoints.insert(ptIds[i]);
      }
    }
  }
};

//----------------------------------------------------------------------------
void labelShellWithMaterial(const smtk::mesh::CollectionPtr& c,
                            const smtk::mesh::MeshSet& shell)
{
  //for each material we iterate the meshsets
  typedef std::vector< smtk::mesh::Domain > DomainVecType;
  typedef DomainVecType::const_iterator dom_cit;

  DomainVecType domains = c->domains();

  for(dom_cit dom = domains.begin(); dom != domains.end(); ++dom)
    {
    smtk::mesh::MeshSet domainMeshes = c->meshes(*dom);

    //find all cells in the shell that share a point in common
    //with domain volume
    smtk::mesh::CellSet contactCells = smtk::mesh::point_intersect(domainMeshes.cells(),
                                                                   shell.cells(),
                                                                   smtk::mesh::FullyContained);
    if(!contactCells.is_empty())
      {
      //create a new meshset that represents the section of exterior shell
      //that has this domain
      smtk::mesh::MeshSet contactD = c->createMesh( contactCells );
      c->setDomainOnMeshes( contactD, *dom );
      }
    }
}

  static int nextDirId = 0;

//----------------------------------------------------------------------------
bool labelIntersection(const smtk::mesh::CollectionPtr& c,
                       const smtk::mesh::MeshSet& shell,
                       Filter& filter)
{
  //need to removing the verts cells from the query for now
  //todo: filter needs to support vert cells
  smtk::mesh::CellSet shellCells = shell.cells( );

  //extract the top cells
  smtk::mesh::for_each(shellCells ,filter);
  smtk::mesh::CellSet filteredCells = smtk::mesh::CellSet(c, filter.validPoints);

  //for each material we iterate the meshsets
  typedef std::vector< smtk::mesh::Domain > DomainVecType;
  typedef DomainVecType::const_iterator dom_cit;

  DomainVecType domains = c->domains();

  //intersect the material and verts to find the verts of a given
  //material that passed the filter.
  //This verts than become a dirichlet set
  for(dom_cit dom = domains.begin(); dom != domains.end(); ++dom)
    {
    smtk::mesh::MeshSet domainMeshes = c->meshes(*dom);

    //find all cells on the top of shell that share a vert in common
    //with material volume
    smtk::mesh::CellSet domainCells = domainMeshes.cells( );
    smtk::mesh::CellSet contactCells = smtk::mesh::point_intersect(domainCells,
                                                                   filteredCells,
                                                                   smtk::mesh::FullyContained);
    if(!contactCells.is_empty())
      {
      smtk::mesh::MeshSet contactD = c->createMesh( contactCells );
      c->setDirichletOnMeshes(contactD, smtk::mesh::Dirichlet( nextDirId ) );
      nextDirId++;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
void breakMaterialsByCellType(const smtk::mesh::CollectionPtr& c)
{
  //for each material we iterate the meshsets
  typedef std::vector< smtk::mesh::Domain > DomainVecType;
  typedef DomainVecType::const_iterator mat_cit;

  DomainVecType domains = c->domains();

  int domainsMade = 0;
  for(mat_cit dom = domains.begin(); dom != domains.end(); ++dom)
    {
    smtk::mesh::MeshSet domainMeshes = c->meshes(*dom);

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

//----------------------------------------------------------------------------
template<typename vtkDataSetType >
smtk::mesh::CollectionPtr convert(vtkDataSetType* input,
                                  smtk::mesh::ManagerPtr manager,
                                  std::string material)
{
  smtk::extension::vtkToMesh::VTKDataConverter convert(manager);

  //we convert the vtk data into a single mesh.
  smtk::mesh::CollectionPtr collection = convert( input, material );

  if(!collection)
    {
    std::cerr << "unable to import the collection properly" << std::endl;
    }

  return collection;
}

//----------------------------------------------------------------------------
void extractMaterials(smtk::mesh::CollectionPtr c, std::string outputFile, double* bounds)
  {
  //extract the exterior-shell for all meshes.
  smtk::mesh::MeshSet shell = c->meshes().extractShell();

  //break the shell based on the materials
  // labelShellWithMaterial( c, shell );

  //find the top and bottom of the shell and apply dirichlet properties
  //to each section
  if(bounds != NULL)
  {
    const double ymin = bounds[2];
    {
      YFilter filter(ymin);
      labelIntersection(c, shell, filter);
    }

    const double ymax = bounds[3];
    {
      YFilter filter(ymax);
      labelIntersection(c, shell, filter);
    }

    const double center[3] = {0.,(ymax+ymin)*.5,0.};
    const double rmin = (bounds[1]-bounds[0])*.75;
    {
      OuterEdgeFilter filter(center,rmin);
      labelIntersection(c, shell, filter);
    }
  }

  //take all meshes that have a material, and break them into a mesh per
  //cell type. This is required since a block in exodus must be of
  //a single cell type
  breakMaterialsByCellType( c );

  std::cout << "number of domains: " << c->domains().size() << std::endl;
  std::cout << "number of dirichlets: " <<  c->dirichlets().size() << std::endl;

  smtk::io::WriteMesh::entireCollection(outputFile, c);
}

}

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
  double* bounds = NULL;
  smtk::mesh::CollectionPtr c;
  if (extension == ".vtu")
   {
   data = readXMLFile<vtkXMLUnstructuredGridReader> (inputFileName);
   c = convert(vtkUnstructuredGrid::SafeDownCast(data), manager, materialName);
   bounds = data->GetBounds();
   }
  else if (extension == ".vtp")
   {
   data = readXMLFile<vtkXMLPolyDataReader> (inputFileName);
   c = convert(vtkPolyData::SafeDownCast(data), manager, materialName);
   bounds = data->GetBounds();
   }
  else if (extension == ".h5m" || extension == ".exo")
   {
   c = smtk::io::ImportMesh::entireFile(inputFileName, manager);
   }

  if(!c)
    {
    std::cerr << "failed to load the requested data" << std::endl;
    return 1;
    }
  extractMaterials(c, outputFileName, bounds);
  return 0;
}
