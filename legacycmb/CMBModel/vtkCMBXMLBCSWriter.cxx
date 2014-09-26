/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkCMBXMLBCSWriter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelUserName.h"
#include "vtkErrorCode.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#
#include "vtkStringArray.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"

#include <iomanip>
#include <sstream>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCMBXMLBCSWriter);
vtkCxxSetObjectMacro(vtkCMBXMLBCSWriter, ModelWrapper, vtkDiscreteModelWrapper);

//----------------------------------------------------------------------------
vtkCMBXMLBCSWriter::vtkCMBXMLBCSWriter()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(0);
  this->ModelWrapper = NULL;
  this->Stream = NULL;
  this->WritingToFile = true;
}

//----------------------------------------------------------------------------
vtkCMBXMLBCSWriter::~vtkCMBXMLBCSWriter()
{
  this->SetModelWrapper(0);
}

//----------------------------------------------------------------------------
const char* vtkCMBXMLBCSWriter::GetDefaultFileExtension()
{
  return "bcs";
}

//----------------------------------------------------------------------------
int vtkCMBXMLBCSWriter::GetDataSetMajorVersion()
{
  return 2;
}

//----------------------------------------------------------------------------
int vtkCMBXMLBCSWriter::GetDataSetMinorVersion()
{
  return 0;
}

//----------------------------------------------------------------------------
void vtkCMBXMLBCSWriter::SetFileName(const char* fileName)
{
  this->Superclass::SetFileName(fileName);
  this->WritingToFile = true;
}

//----------------------------------------------------------------------------
void vtkCMBXMLBCSWriter::SetStream(std::ostream& stream)
{
  //we have to pass a fake file name so that vtkXMLWriter thinks we have
  //a file name we are saving too, yes this is fairly hackish. But this
  //is the only way while we wait for vtk 6.2 which xml writer support
  //streams natively.
  this->Superclass::SetFileName("fake file name");
  this->Stream = &stream;
  this->WritingToFile = false;
}

//----------------------------------------------------------------------------
bool vtkCMBXMLBCSWriter::IsWritingToStream() const
{
  return !this->WritingToFile;
}

//----------------------------------------------------------------------------
bool vtkCMBXMLBCSWriter::IsWrittingToFile() const
{
  return this->WritingToFile;
}

//----------------------------------------------------------------------------
int vtkCMBXMLBCSWriter::WriteData()
{
  vtkDiscreteModel* Model = this->ModelWrapper->GetModel();
  const DiscreteMesh& mesh = Model->GetMesh();
  vtkDiscreteModel::ClassificationType& meshClassif =
                                        Model->GetMeshClassification();

  vtkIndent indent;
  vtkIndent indent2 = indent.GetNextIndent();
  vtkIndent indent3 = indent2.GetNextIndent();
  vtkIndent indent4 = indent3.GetNextIndent();

  vtkSmartPointer<vtkXMLDataElement> bcsfile =
    vtkSmartPointer<vtkXMLDataElement>::New();
  bcsfile->SetName("BCSFile");
  std::stringstream version;
  version << this->GetDataSetMajorVersion() << "." << this->GetDataSetMinorVersion();
  bcsfile->SetAttribute("Version", version.str().c_str());
  // current objects are Points, MeshFacets, ModelFaces, Materials,
  // ModelRegions, and BoundaryConditionSets
  bcsfile->SetIntAttribute("NumberOfObjects", 6);

  //
  // write out the point data
  //

  vtkSmartPointer<vtkXMLDataElement> points =
    vtkSmartPointer<vtkXMLDataElement>::New();
  points->SetName("Points");
  points->SetAttribute("Description",
                       "Implicitly ordered array of point locations (x,y,z tuples) starting at 0 and numbered consecutively. Optionally may contain information for mapping from point Ids in this grid to another grid (e.g. mapping from point Ids in the surface mesh to point Ids in the volumetric mesh that the surface mesh was extracted from.");
  points->SetIntAttribute("NumberOfObjects", 1);
  bcsfile->AddNestedElement(points);
  vtkSmartPointer<vtkXMLDataElement> locations =
    vtkSmartPointer<vtkXMLDataElement>::New();
  locations->SetName("Data");
  locations->SetIntAttribute("NumberOfValues", mesh.GetNumberOfPoints()*3);
  locations->SetAttribute("type", "Float64");
  locations->SetAttribute("Description", "The point location tuples.");
  locations->SetIntAttribute("NumberOfObjects", mesh.GetNumberOfPoints());
  vtkIdType i;
  std::stringstream data;
  data << "\n";
  char str[1024];
  double loc[3];
  for(i=0; i<mesh.GetNumberOfPoints();i++)
    {
    mesh.GetPoint(i,loc);

    // make sure we write sufficient precision
    sprintf (str, "%.16lg %.16lg %.16lg\n", loc[0], loc[1], loc[2]);
    data << indent4 << str;
    //cout << i << " is the point " << data.str().c_str() << " " << data.gcount() << endl;
    }
  data << indent3;
  locations->AddCharacterData(data.str().c_str(), data.str().length());
  points->AddNestedElement(locations);

  //
  // write out the cell/mesh facets data
  //

  vtkSmartPointer<vtkPolyData> MasterPoly = mesh.ShallowCopyFaceData();
  vtkCellArray* cells = MasterPoly->GetPolys();
  cells->InitTraversal();
  vtkIdType numCells = mesh.GetNumberOfCells();

  vtkSmartPointer<vtkXMLDataElement> meshFacets =
    vtkSmartPointer<vtkXMLDataElement>::New();
  meshFacets->SetName("MeshFacets");
  meshFacets->SetIntAttribute("NumberOfObjects", 1);
  meshFacets->SetAttribute("Description",
                             "Implicitly ordered cell data starting from 0 and numbered consecutively with each line being an instance of cell data.  The first number is the number of points of the cell, the second is the unique persistent Id of the model face the cell belongs to and the subsequent numbers are the point ids of the cell.");
  bcsfile->AddNestedElement(meshFacets);
  vtkSmartPointer<vtkXMLDataElement> connectivity =
    vtkSmartPointer<vtkXMLDataElement>::New();
  connectivity->SetName("Data");

  meshFacets->AddNestedElement(connectivity);

  vtkIdType npts;
  vtkIdType* pts;
  i = 0;
  std::stringstream conn;
  conn << "\n";
  int NumberOfConnectivityValues = 0;

  while(cells->GetNextCell(npts, pts))
    {
    vtkDiscreteModelGeometricEntity* entity = meshClassif.GetEntity(i);
    const vtkIdType CellClassification =
                        entity->GetThisModelEntity()->GetUniquePersistentId();
    conn << indent4 << npts << " " << CellClassification;
    for(int j=0;j<npts;j++)
      {
      conn << " " << pts[j];
      }
    conn << "\n";
    NumberOfConnectivityValues += 2+npts;
    i++;
    }
  conn << indent3;
  connectivity->AddCharacterData(conn.str().c_str(), conn.str().length());
  connectivity->SetIntAttribute("NumberOfValues", NumberOfConnectivityValues);
  connectivity->SetIntAttribute("NumberOfObjects", numCells);
  connectivity->SetAttribute("type", "Int64");

  //
  // write out the domain set data
  //
  vtkSmartPointer<vtkXMLDataElement> materials =
    vtkSmartPointer<vtkXMLDataElement>::New();
  materials->SetName("DomainSets");
  materials->SetIntAttribute("NumberOfObjects", Model->GetNumberOfModelEntities(vtkModelMaterialType));
  materials->SetAttribute("Description",
                          "Implicitly ordered domain sets starting at 0 and numbered consecutively along with a Name of the domain set that can be set and a unique persistent Id of the domain set.");
  bcsfile->AddNestedElement(materials);
  std::map<int, int> materialIdToOutputId;
  vtkModelItemIterator* Materials = Model->NewIterator(vtkModelMaterialType);
  for(Materials->Begin();!Materials->IsAtEnd();Materials->Next())
    {
    vtkModelMaterial* Material = vtkModelMaterial::SafeDownCast(Materials->GetCurrentItem());
    vtkIdType MaterialId = Material->GetUniquePersistentId();
    const char* MaterialUserName = vtkModelUserName::GetUserName(Material);
    vtkSmartPointer<vtkXMLDataElement> material =
      vtkSmartPointer<vtkXMLDataElement>::New();
    material->SetName("DomainSet");
    material->SetIntAttribute("UniquePersistentId", MaterialId);
    material->SetAttribute("Name", MaterialUserName);
    materials->AddNestedElement(material);
    vtkIdType Count = materialIdToOutputId.size();
    materialIdToOutputId[Material->GetUniquePersistentId()] = Count;
    }
  Materials->Delete();

  //
  // write out the region data (previously referred to as a shell)
  //
  vtkSmartPointer<vtkXMLDataElement> regions =
    vtkSmartPointer<vtkXMLDataElement>::New();
  regions->SetName("ModelRegions");
  regions->SetIntAttribute("NumberOfObjects", Model->GetNumberOfModelEntities(vtkModelRegionType));
  regions->SetAttribute("Description",
                       "Implicitly ordered model regions starting at 0 and numbered consecutively along with a Name of the model region that can be set in CMB and the domain set Id the model region is associated with.  The region also has a unique persistent Id.");
  bcsfile->AddNestedElement(regions);
  vtkModelItemIterator* Regions = Model->NewIterator(vtkModelRegionType);
  std::map<vtkIdType,vtkIdType> regionIdToOutputId;
  for(Regions->Begin();!Regions->IsAtEnd();Regions->Next())
    {
    vtkDiscreteModelRegion* Region = vtkDiscreteModelRegion::SafeDownCast(Regions->GetCurrentItem());
    vtkIdType RegionId = vtkModelEntity::SafeDownCast(
      Regions->GetCurrentItem())->GetUniquePersistentId();
    const char* RegionUserName  = vtkModelUserName::GetUserName(Region);
    vtkModelMaterial* Material = Region->GetMaterial();
    vtkSmartPointer<vtkXMLDataElement> region =
      vtkSmartPointer<vtkXMLDataElement>::New();
    region->SetName("ModelRegion");
    region->SetAttribute("Name", RegionUserName);
    region->SetIntAttribute("DomainSetId",
                           materialIdToOutputId[Material->GetUniquePersistentId()]);
    region->SetIntAttribute("UniquePersistentId", Region->GetUniquePersistentId());
    regions->AddNestedElement(region);
    vtkIdType count = regionIdToOutputId.size();
    regionIdToOutputId[RegionId] = count;
    }
  Regions->Delete();

  //
  // write out the model face data
  //
  vtkSmartPointer<vtkXMLDataElement> modelFaces =
    vtkSmartPointer<vtkXMLDataElement>::New();
  modelFaces->SetName("ModelFaces");
  modelFaces->SetIntAttribute("NumberOfObjects", Model->GetNumberOfAssociations(vtkModelFaceType));
  modelFaces->SetAttribute("Description",
                           "A model face is an aggregation of mesh facets that is used to specify information collectively for the mesh facets.  The model face has two associated model regions and User specified Name. ModelRegionId1 corresponds to the side of the model face that the mesh facets' normals point towards (using a counterclockwise ordering). A -1 indicates that there is no associated model region for the indicated side of the model face. The face also has a unique persistent Id.");
  bcsfile->AddNestedElement(modelFaces);
  int FaceCounter = 0;
  vtkModelItemIterator* Faces = Model->NewIterator(vtkModelFaceType);
  for(Faces->Begin();!Faces->IsAtEnd();Faces->Next(),FaceCounter++)
    {
    vtkSmartPointer<vtkXMLDataElement> modelFace =
      vtkSmartPointer<vtkXMLDataElement>::New();
    modelFace->SetName("ModelFace");
    vtkModelFace* Face = vtkModelFace::SafeDownCast(Faces->GetCurrentItem());
    // probably region0 is the one we want
    vtkModelRegion* Region0 = Face->GetModelRegion(0);
    vtkModelRegion* Region1 = Face->GetModelRegion(1);
    vtkIdType Region0Id = -1;
    if(Region0)
      {
      Region0Id = regionIdToOutputId[Region0->GetUniquePersistentId()];
      }
    vtkIdType Region1Id = -1;
    if(Region1)
      {
      Region1Id = regionIdToOutputId[Region1->GetUniquePersistentId()];
      }
    modelFace->SetIntAttribute("ModelRegionId0", Region0Id);
    modelFace->SetIntAttribute("ModelRegionId1", Region1Id);
    modelFace->SetIntAttribute("UniquePersistentId", Face->GetUniquePersistentId());
    const char* MFName = vtkModelUserName::GetUserName(Face);
    modelFace->SetAttribute("Name", MFName);
    modelFaces->AddNestedElement(modelFace);
    }
  Faces->Delete();

  //
  // write out the boundary condition set data
  //
  vtkSmartPointer<vtkXMLDataElement> BCSs =
    vtkSmartPointer<vtkXMLDataElement>::New();
  BCSs->SetName("BoundaryConditionSets");
  BCSs->SetIntAttribute("NumberOfObjects",
                        Model->GetNumberOfModelEntities(vtkDiscreteModelEntityGroupType));
  BCSs->SetAttribute("Description",
                     "Implicitly ordered boundary condition sets (BCSs) starting at 0 and numbered consecutively along with a Name that can be set and a Unique Persistent Id.  Each BCS element also has a nested Data element used to store the Ids of the model faces it is applied over.");
  bcsfile->AddNestedElement(BCSs);
  vtkModelItemIterator* Groups = Model->NewIterator(vtkDiscreteModelEntityGroupType);
  for(Groups->Begin();!Groups->IsAtEnd();Groups->Next())
    {
    vtkDiscreteModelEntityGroup* Group =
      vtkDiscreteModelEntityGroup::SafeDownCast(Groups->GetCurrentItem());
    if(Group->GetEntityType() != vtkModelFaceType)
      {
      vtkErrorMacro("Only currently support BCSs defined over model faces.");
      }
    vtkIdType BCSId = Group->GetUniquePersistentId();
    const char* BCSUserName = vtkModelUserName::GetUserName(Group);

    vtkSmartPointer<vtkXMLDataElement> BCS =
      vtkSmartPointer<vtkXMLDataElement>::New();
    BCS->SetName("BoundaryConditionSet");
    BCS->SetIntAttribute("UniquePersistentId", BCSId);
    BCS->SetAttribute("Name", BCSUserName);
    BCS->SetAttribute("Type", "face");
    BCSs->AddNestedElement(BCS);

    vtkSmartPointer<vtkXMLDataElement> BCSModelFaces =
      vtkSmartPointer<vtkXMLDataElement>::New();
    BCSModelFaces->SetName("Data");
    BCSModelFaces->SetAttribute("type", "Int32");
    BCSModelFaces->SetIntAttribute("NumberOfObjects", Group->GetNumberOfModelEntities());

    std::stringstream BCSMFs;
    BCSMFs << "\n" << indent4 << " ";
    vtkModelItemIterator* GroupFaces = Group->NewModelEntityIterator();
    for(GroupFaces->Begin();!GroupFaces->IsAtEnd();GroupFaces->Next())
      {
      BCSMFs << " " << vtkModelEntity::SafeDownCast(
        GroupFaces->GetCurrentItem())->GetUniquePersistentId();
      }
    GroupFaces->Delete();
    BCSMFs << "\n" << indent4;
    BCSModelFaces->SetCharacterData(BCSMFs.str().c_str(), BCSMFs.str().length());
    BCS->AddNestedElement(BCSModelFaces);
    }
  Groups->Delete();

  //
  // write out the hard points data
  //
  if(Model->GetNumberOfModelEntities(vtkModelEdgeType))
    {
    this->AddHardPointsData(bcsfile, Model, indent4);
    }

  if(this->WritingToFile)
    {
    vtkXMLUtilities::WriteElementToFile(bcsfile, this->GetFileName(), &indent);
    }
  else if(this->Stream) //only write to stream if it isn't NULL
    {
    vtkXMLUtilities::FlattenElement(bcsfile, *this->Stream, &indent);
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBXMLBCSWriter::AddHardPointsData(vtkXMLDataElement* ParentElement,
  vtkDiscreteModel* Model, vtkIndent indent4)
{
  vtkSmartPointer<vtkXMLDataElement> HardPointsSets =
    vtkSmartPointer<vtkXMLDataElement>::New();
  vtkModelItemIterator* itSets = Model->NewIterator(vtkModelEdgeType);
  int HardPointsCounter = 0;
  for(itSets->Begin();!itSets->IsAtEnd();itSets->Next())
    {
    vtkDiscreteModelEdge* HardPoints =
      vtkDiscreteModelEdge::SafeDownCast(itSets->GetCurrentItem());
    if(HardPoints->GetNumberOfAssociations(vtkModelRegionType) > 0)
      {
      HardPointsCounter++;
      vtkIdType GroupId = HardPoints->GetUniquePersistentId();
      const char* GroupUserName = vtkModelUserName::GetUserName(HardPoints);

      vtkSmartPointer<vtkXMLDataElement> GroupElement =
        vtkSmartPointer<vtkXMLDataElement>::New();
      GroupElement->SetName("HardPointsGroup");
      GroupElement->SetIntAttribute("UniquePersistentId", GroupId);
      GroupElement->SetAttribute("Name", GroupUserName);
      HardPointsSets->AddNestedElement(GroupElement);

      vtkSmartPointer<vtkXMLDataElement> locations =
        vtkSmartPointer<vtkXMLDataElement>::New();
      locations->SetName("Data");
      locations->SetIntAttribute("NumberOfValues", (HardPoints->GetLineResolution()+1)*3);
      locations->SetAttribute("type", "Float64");
      locations->SetAttribute("Description", "The point location tuples.");
      locations->SetIntAttribute("NumberOfObjects", HardPoints->GetLineResolution()+1);

      std::stringstream data;
      data << "\n";

      // write points tuples
      char str[1024];
      vtkModelVertex* vertex1 = HardPoints->GetAdjacentModelVertex(0);
      vtkModelVertex* vertex2 = HardPoints->GetAdjacentModelVertex(1);

      double xyz1[3], xyz2[3];
      if(vertex1->GetPoint(xyz1) && vertex2->GetPoint(xyz2))
        {
        double x[3], v[3], tc;
        for (int i=0; i<3; i++)
          {
          v[i] = xyz2[i] - xyz1[i];
          }
        for (int i=0; i<HardPoints->GetLineResolution()+1; i++)
          {
          tc = (static_cast<double>(i)/HardPoints->GetLineResolution());
          for (int j=0; j<3; j++)
            {
            x[j] = xyz1[j] + tc*v[j];
            }
          // make sure we write sufficient precision
          sprintf (str, "%.16lg %.16lg %.16lg\n", x[0], x[1], x[2]);
          data << indent4 << str;
          }
        }
      else
        {
        vtkErrorMacro("Could not get location of a model vertex.");
        }

      data << indent4;
      locations->AddCharacterData(data.str().c_str(), data.str().length());
      GroupElement->AddNestedElement(locations);
      }
    }
  itSets->Delete();
  HardPointsSets->SetName("HardPointsGroups");
  HardPointsSets->SetIntAttribute(
    "NumberOfObjects", HardPointsCounter);
  HardPointsSets->SetAttribute("Description",
                            "Independent HardPoints sets.  Each HardPoints element also has a nested Data element used to store the point locations (x,y,z tuples).");
  ParentElement->AddNestedElement(HardPointsSets);
}

 //-----------------------------------------------------------------------------
const char* vtkCMBXMLBCSWriter::GetDataSetName()
{
  return NULL;
}

//-----------------------------------------------------------------------------
void vtkCMBXMLBCSWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ModelWrapper: " << this->ModelWrapper << "\n";
}
