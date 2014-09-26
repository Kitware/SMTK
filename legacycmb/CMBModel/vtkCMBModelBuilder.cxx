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

#include "vtkCMBModelBuilder.h"

#include "vtkAbstractArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkCharArray.h"
#include "vtkModel3dm2DGridRepresentation.h"
#include "vtkModel3dmGridRepresentation.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkCMBParserBase.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkInstantiator.h"
#include "vtkIntArray.h"
#include "vtkInformation.h"
#include "vtkModelEntityOperatorBase.h"
#include "vtkModelItemIterator.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkAlgorithm.h"
#include "vtkStringArray.h"
#include "vtkIdTypeArray.h"
#include "vtkPointData.h"
#include "vtkTriangulateConcavePolysFilter.h"

#include <set>
#include <sstream>

vtkStandardNewMacro(vtkCMBModelBuilder);

namespace detail
{
typedef std::pair<std::vector<vtkModelFace*>, std::vector<int> > RegionInfoPair;
typedef std::map<vtkIdType,RegionInfoPair> RegionInfoMap;

//a model domain is a collection of regions
struct ModelDomains
{
private:
  typedef std::set<vtkIdType> vregions;
public:
  typedef vregions::const_iterator const_region_id_iterator;

  typedef std::map<vtkIdType,vregions>::const_iterator const_iterator;
  typedef std::map<vtkIdType,vregions>::const_iterator iterator;

  ModelDomains()
  {
    this->UseDomainTag = false;
  }

  //create a new domain using a user defined identifier.
  //does nothing if the domain already exists
  void addDomain(vtkIdType domainId)
    {
    this->UseDomainTag = true;
    if(this->DomainsToRegions.find(domainId) == this->DomainsToRegions.end())
      {
      this->DomainsToRegions[domainId] = vregions();
      }
    }

  //create a new domain using the next unique identifier
  //return the identifier
  vtkIdType createNewDomain()
    {
    vtkIdType newDomainValue=0;
    if(this->DomainsToRegions.size() > 0)
      {
      //find the last element in the map, and increment its value
      //by one to get the new domain value. This is will not
      //be in the map otherwise it would have been the last element
      const_iterator lastElem = this->DomainsToRegions.end();
      --lastElem;
      newDomainValue = lastElem->first + 1;
      }
    this->DomainsToRegions[newDomainValue] = vregions();
    return newDomainValue;
    }

  //given a domains identifier insert a region id
  //requires the domain to already exist
  void addRegionToDomain(vtkIdType domain, vtkIdType region)
    {
    this->DomainsToRegions[domain].insert(region);
    }

  //verify that regionId isn't owned by a domain. If not create a new
  //domain for it
  void newDomainIfNotRegion(vtkIdType regionId)
    {
    if(!this->haveRegionId(regionId))
      {
      vtkIdType d = this->createNewDomain();
      this->addRegionToDomain(d,regionId);
      }
    }

  //see if we have a region id held in a domain
  bool haveRegionId(vtkIdType regionId) const
    {
    bool haveId = false;
    for(const_iterator i=this->DomainsToRegions.begin();
        i != this->DomainsToRegions.end() && !haveId;
        ++i)
      {
      haveId = i->second.count(regionId) != 0;
      }
    return haveId;
    }


  const_iterator begin() const { return this->DomainsToRegions.begin(); }
  iterator begin() { return this->DomainsToRegions.begin(); }

  const_iterator end() const  { return this->DomainsToRegions.end(); }
  iterator end()  { return this->DomainsToRegions.end(); }

  bool GetUseDomainTag()
  {
    return this->UseDomainTag;
  }

private:
  typedef vregions::const_iterator vIt;
  std::map<vtkIdType,vregions> DomainsToRegions;
  bool UseDomainTag;

};

}

//-----------------------------------------------------------------------------
vtkCMBModelBuilder::vtkCMBModelBuilder()
{
  this->OperateSucceeded = 0;
}

//-----------------------------------------------------------------------------
vtkCMBModelBuilder:: ~vtkCMBModelBuilder()
{
}

//-----------------------------------------------------------------------------
void vtkCMBModelBuilder::Operate(
  vtkDiscreteModelWrapper* modelWrapper, vtkAlgorithm* inputPoly)
{
  if(!inputPoly)
    {
    vtkErrorMacro("Passed in a null poly algorithm input.");
    return;
    }
  if(!modelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return;
    }

  inputPoly->Update();
  vtkPolyData* modelPoly = vtkPolyData::SafeDownCast(
    inputPoly->GetOutputDataObject(0));
  if(!modelPoly)//!this->ProcessForParsing(MasterPoly))
    {
    vtkErrorMacro("Output from the input poly algorithm is not a polydata.");
    return;
    }

  // see if it has data we're expecting for a 2D model
  if (modelPoly->GetFieldData()->GetArray("Vertices"))
    {
    this->ProcessAs2DMesh(modelWrapper, modelPoly);
    return;
    }

  detail::ModelDomains domains;

  //if this optional array exists use it to populate the domain/region mapping
  vtkIdTypeArray* modelRegionDomainMap = vtkIdTypeArray::SafeDownCast(
    modelPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelPredefinedDomainSets()));
  if(modelRegionDomainMap)
    {
    //for each region we are given the domain it part of
    vtkIdType ids[2];
    for(vtkIdType i=0;i<modelRegionDomainMap->GetNumberOfTuples();i++)
      {
      modelRegionDomainMap->GetTupleValue(i,ids);
      domains.addDomain(ids[0]);
      domains.addRegionToDomain(ids[0],ids[1]);
      }

    modelPoly->GetFieldData()->RemoveArray(
          vtkCMBParserBase::GetModelPredefinedDomainSets());
    }

  vtkIdTypeArray* modelFaceRegionsMap = vtkIdTypeArray::SafeDownCast(
    modelPoly->GetFieldData()->GetArray(vtkCMBParserBase::GetModelFaceRegionsMapString()));
  if(!modelFaceRegionsMap)//!this->ProcessForParsing(MasterPoly))
    {
    vtkErrorMacro("Output from the input poly algorithm does not have modelFaceRegionsMap field data.");
    return;
    }

  if (!modelPoly->GetCellData()->GetNormals())
    {
    vtkWarningMacro("Normals not specified on input polydata!  Best if " <<
      "normals calculated with vtkMasterPolyDataNormals to be sure they are " <<
      "pointing out!  Will assume normals do point out and will compute " <<
      "locally for the point inside computation.");
    }

  // now fill in the model information.
  vtkDiscreteModel* model = modelWrapper->GetModel();
  model->Reset();

  vtkIdType faceRegions[3];
  std::map<vtkIdType, std::pair<vtkIdType, vtkIdType> > faceToRegionsMap;
  for(vtkIdType i=0;i<modelFaceRegionsMap->GetNumberOfTuples();i++)
    {
    modelFaceRegionsMap->GetTupleValue(i, faceRegions);
    std::pair<vtkIdType, vtkIdType> regionIds;
    regionIds.first = faceRegions[1];
    regionIds.second = faceRegions[2];
    faceToRegionsMap[faceRegions[0]] = regionIds;
    }

 // vtkPolyData* modelPoly =vtkPolyData::SafeDownCast(MergeFilter->GetOutput(0));
  vtkIdTypeArray* modelFaceIds = vtkIdTypeArray::SafeDownCast(
    modelPoly->GetCellData()->GetArray(vtkCMBParserBase::GetModelFaceTagName()));

  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > modelFaceCells;
  vtkIdType faceId;
  vtkIdType maxModelEntityId = -1;
  for(vtkIdType i=0;i<modelFaceIds->GetNumberOfTuples();i++)
    {
    faceId = modelFaceIds->GetValue(i);
    std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it=
      modelFaceCells.find(faceId);
    if(it==modelFaceCells.end())
      {
      vtkSmartPointer<vtkIdList> Cells = vtkSmartPointer<vtkIdList>::New();
      Cells->InsertNextId(i);
      modelFaceCells[faceId] = Cells;
      }
    else
      {
      it->second->InsertNextId(i);
      }

    if(faceId > maxModelEntityId)
      {
      maxModelEntityId = faceId;
      }
    vtkIdType regionIds[] = {faceToRegionsMap[faceId].first, faceToRegionsMap[faceId].second};
    for(int j=0;j<2;j++)
      {
      if(regionIds[j] > maxModelEntityId)
        {
        maxModelEntityId = regionIds[j];
        }
      if(regionIds[j] != -1)
        {
        domains.newDomainIfNotRegion(regionIds[j]);
        }
      }
    }
  // Does the polydata have lines?  If not we need to add them
  // in case model edges are created
  if (modelPoly->GetNumberOfLines() == 0)
      {
      vtkNew<vtkCellArray> lines;
      modelPoly->SetLines(lines.GetPointer());
      }
  DiscreteMesh mesh(modelPoly);
  model->SetMesh(mesh);
  vtkIdTypeArray* pointMapArray = vtkIdTypeArray::SafeDownCast(
      modelPoly->GetPointData()->GetArray("vtkOriginalPointIds"));
  vtkIdTypeArray* cellMapArray = vtkIdTypeArray::SafeDownCast(
    modelPoly->GetCellData()->GetArray("OrigCellIds"));
  vtkCharArray* canonicalSideArray = vtkCharArray::SafeDownCast(

    modelPoly->GetCellData()->GetArray("CellFaceIds"));
  if(pointMapArray || cellMapArray || canonicalSideArray)
    {
    if(pointMapArray && cellMapArray && canonicalSideArray)
      {
      vtkModel3dmGridRepresentation* analysisGridInfo =
        vtkModel3dmGridRepresentation::New();
      vtkStringArray *solidFileName = vtkStringArray::SafeDownCast(
        modelPoly->GetFieldData()->GetAbstractArray( "FileName" ) );
      const char* analysisGridFile = solidFileName ?
        solidFileName->GetValue(0).c_str() : NULL;
      analysisGridInfo->Initialize(analysisGridFile, model, pointMapArray,
                                   cellMapArray, canonicalSideArray);
      model->SetAnalysisGridInfo(analysisGridInfo);
      analysisGridInfo->Delete();
      }
    else
      {
      vtkWarningMacro("There seems to be some information missing for"
                      << " mapping back to the analysis mesh.");
      }
    }
  model->SetLargestUsedUniqueId(maxModelEntityId+1);

  detail::RegionInfoMap RegionInfo;
  vtkIdType counter = 0;
  for(std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it=
        modelFaceCells.begin();it!=modelFaceCells.end();it++,counter++)
    {
    faceId = it->first;
    vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(
      model->BuildModelFace(0, 0, 0, faceId));
    Face->AddCellsToGeometry(it->second);
    // store the region adjacency info
    vtkIdType RegionIds[] = {faceToRegionsMap[faceId].first, faceToRegionsMap[faceId].second};
    for(int i=0;i<2;i++)
      {
      if(RegionIds[i] >= 0)
        {
        RegionInfo[RegionIds[i]].first.push_back(Face);
        RegionInfo[RegionIds[i]].second.push_back(i);
        }
      }
    }

  // setup cell locator for "point inside" computation (to confirm the computed
  // point inside isclosest to cell used to generate it... as expected
  vtkSmartPointer<vtkCellLocator> cellLocator =
    vtkSmartPointer<vtkCellLocator>::New();
  cellLocator->SetDataSet( modelPoly );
  cellLocator->BuildLocator();

  typedef detail::ModelDomains::const_iterator md_iterator;
  for(md_iterator it=domains.begin();
      it!=domains.end();it++, counter++)
    {
    //really randomly assigning a material to this region
    vtkModelMaterial* material = model->BuildMaterial();
    if(domains.GetUseDomainTag())
      {
      int moabTag = it->first;
      vtkInformation* info = material->GetAttributes();
      std::stringstream buffer;
      buffer << moabTag;
      std::string data = buffer.str();
      info->Set(vtkModelEntity::USERDATA(),data.c_str());
      // set the name according to the neumann set id
      // this makes it easier to debug moab input files
      vtkNew<vtkModelEntityOperatorBase> op;
      op->SetId(material->GetUniquePersistentId());
      op->SetItemType(material->GetType());
      data = "material set " + data;
      op->SetUserName(data.c_str());
      op->Operate(model);
      }

    //we need to do a sub loop here over each region in the
    //in the domain
    detail::ModelDomains::const_region_id_iterator regionIterator;
    for(regionIterator = it->second.begin();
        regionIterator != it->second.end();
        ++regionIterator)
      {
      vtkIdType regionId = *regionIterator;
      int numFaces = RegionInfo[regionId].first.size();

      if(regionId < 0)
        {
        regionId = model->GetNextUniquePersistentId();
        }
      vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(
        model->BuildModelRegion(numFaces, &(RegionInfo[regionId].first[0]),
                        &(RegionInfo[regionId].second[0]), regionId, material));

      // compute the point inside for this model region
      this->ComputePointInsideForRegion(region, cellLocator);
      }
    }

  // we should be able to clear out point and cell data from the
  // poly data to save on memory.
  //vtkWarningMacro("Deleting all field data may not be ready for public consumption yet!!!");
  modelPoly->GetPointData()->RemoveArray("vtkOriginalPointIds");
  modelPoly->GetCellData()->RemoveArray("Region");
  modelPoly->GetCellData()->RemoveArray("OrigCellIds");
  modelPoly->GetCellData()->RemoveArray("CellFaceIds");
  modelPoly->GetCellData()->RemoveArray("Normals");
  modelPoly->GetCellData()->RemoveArray("modelfaceids");
  modelPoly->GetFieldData()->RemoveArray("FileName");
  modelPoly->GetFieldData()->RemoveArray("ModelFaceRegionsMap");

#ifdef DEBUG
  if(modelPoly->GetPointData()->GetNumberOfArrays() != 0)
    {
    vtkWarningMacro("There is point data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<modelPoly->GetPointData()->GetNumberOfArrays();ii++)
      {
      vtkWarningMacro("Point data array name is " <<
                      modelPoly->GetPointData()->GetArrayName(ii));
      }
    }
  if(modelPoly->GetCellData()->GetNumberOfArrays() != 0)
    {
    vtkWarningMacro("There is cell data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<modelPoly->GetCellData()->GetNumberOfArrays();ii++)
      {
      vtkWarningMacro("Cell data array name is " <<
                      modelPoly->GetCellData()->GetArrayName(ii));
      }
    }
  if(modelPoly->GetFieldData()->GetNumberOfArrays() != 0)
    {
    vtkWarningMacro("There is field data on the master poly -- we may be wasting memory.");
    for(int ii=0;ii<modelPoly->GetFieldData()->GetNumberOfArrays();ii++)
      {
      vtkWarningMacro("Field data array name is " <<
                      modelPoly->GetFieldData()->GetArrayName(ii));
      }
    }
#endif

  vtkIdTypeArray* predefinedBoundarySet = vtkIdTypeArray::SafeDownCast(
    modelPoly->GetFieldData()->GetArray(
    vtkCMBParserBase::GetModelPredefinedBoundarySets()));
  if(predefinedBoundarySet)
    {
    typedef std::vector<vtkDiscreteModelEntity*> idVec;
    typedef std::map<vtkIdType,idVec> BCMap;

    BCMap boundaryMapping;
    vtkModelItemIterator* faceIterator = model->NewIterator(vtkModelFaceType);

    faceIterator->Begin();
    for(vtkIdType faceIndex=0;faceIndex<predefinedBoundarySet->GetNumberOfTuples() && !faceIterator->IsAtEnd();
        faceIndex++, faceIterator->Next())
      {
      vtkDiscreteModelEntity *entity = vtkDiscreteModelFace::SafeDownCast(
                                        faceIterator->GetCurrentItem());

      const vtkIdType boundaryCondition =
          predefinedBoundarySet->GetValue(faceIndex);
      boundaryMapping[boundaryCondition].push_back(entity);
      }
    faceIterator->Delete();
    //remove the mapping
    boundaryMapping.erase(-1);
    for(BCMap::iterator mit=boundaryMapping.begin();mit!=boundaryMapping.end();
          mit++)
      {
      // check to see that the first argument is correct -- acbauer
      vtkDiscreteModelEntityGroup* bcSet = model->BuildModelEntityGroup(
        vtkModelFaceType, static_cast<int>(mit->second.size()), &(mit->second[0]));
      vtkInformation* info = bcSet->GetAttributes();
      std::stringstream buffer;
      buffer << mit->first;
      std::string data = buffer.str();
      info->Set(vtkModelEntity::USERDATA(),data.c_str());
      // set the name according to the neumann set id
      // this makes it easier to debug moab input files
      vtkNew<vtkModelEntityOperatorBase> op;
      op->SetId(bcSet->GetUniquePersistentId());
      op->SetItemType(bcSet->GetType());
      data = "boundary set " + data;
      op->SetUserName(data.c_str());
      op->Operate(model);
      }
    modelPoly->GetFieldData()->RemoveArray(
          vtkCMBParserBase::GetModelPredefinedBoundarySets());
    }

  this->OperateSucceeded = 1;
  modelWrapper->InitializeWithModelGeometry();

  return;
}

//-----------------------------------------------------------------------------
void vtkCMBModelBuilder::ProcessAs2DMesh(vtkDiscreteModelWrapper* modelWrapper,
                                         vtkPolyData *modelPolyData)
{
  vtkDiscreteModel* model = modelWrapper->GetModel();
  model->Reset();

  DiscreteMesh mesh(modelPolyData);
  model->SetMesh(mesh);

  // get the number of loops now so that we can know how many material
  // persistent ids to set aside
  // NOTE: We should use the maximum value of region array, instead of
  // the size of the array ("Loop Names")

  // before we process loops, go through the polys, and create map for each region
  std::map< vtkIdType, vtkIdList* > polyRegionMap;
  vtkIntArray* regionArray = vtkIntArray::SafeDownCast(
    modelPolyData->GetCellData()->GetArray("Region") );
  // keep mapping between new region and old region if it exists.
  std::map< vtkIdType, vtkIdType> newToOldRegionMap;
  vtkIntArray* oldRegionArray = vtkIntArray::SafeDownCast(
    modelPolyData->GetCellData()->GetArray("Original Region") );
  vtkIdType maxRegionId = -1;
  vtkIdList *regionIdList;

  const vtkIdType lineOffset = modelPolyData->GetNumberOfLines();
  for (vtkIdType i = lineOffset; i < modelPolyData->GetNumberOfCells(); i++)
    {
    int regionId = regionArray->GetValue(i);
    std::map< vtkIdType, vtkIdList* >::iterator iter = polyRegionMap.find(regionId);
    if (iter == polyRegionMap.end())
      {
      regionIdList = vtkIdList::New();
      polyRegionMap[regionId] = regionIdList;
      maxRegionId = regionId>maxRegionId ? regionId : maxRegionId;
      regionIdList->Allocate( modelPolyData->GetNumberOfPolys() );
      if(oldRegionArray && newToOldRegionMap.find(regionId)==newToOldRegionMap.end())
        {
        vtkIdType oldRegionId = oldRegionArray->GetValue(i);
        newToOldRegionMap[regionId] = oldRegionId;
        maxRegionId = oldRegionId>maxRegionId ? oldRegionId : maxRegionId;
        }
      }
    else
      {
      regionIdList = iter->second;
      }
    //Edges and Faces now have different id spaces, so we have
    //convert this back to a zero based index
    const vtkIdType meshFaceId = i - lineOffset;
    regionIdList->InsertNextId( meshFaceId  );
    }
  model->SetLargestUsedUniqueId( maxRegionId + 1 ); // +1 just to be sure

  // 1st Vertices
  vtkIdTypeArray *vertices =
    vtkIdTypeArray::SafeDownCast( modelPolyData->GetFieldData()->GetArray("Vertices") );
  std::vector< vtkModelVertex* > modelVertices;
  for (int i = 0; i < vertices->GetNumberOfTuples(); i++)
    {
    vtkDiscreteModelVertex* cmbVertex = vtkDiscreteModelVertex::SafeDownCast(
      model->BuildModelVertex( vertices->GetValue(i) ));
    if(cmbVertex)
      {
      cmbVertex->CreateGeometry();
      }
    modelVertices.push_back( cmbVertex );
    }
  modelPolyData->GetFieldData()->RemoveArray( "Vertices" );

  // now edges

  std::vector< vtkModelEdge* > modelEdges;
  vtkStringArray *edgeNames =
    vtkStringArray::SafeDownCast( modelPolyData->GetFieldData()->GetAbstractArray("Edge Names") );
  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
  for (int i = 0; i < edgeNames->GetNumberOfValues(); i++)
    {
    vtkIdTypeArray *edgeArray = vtkIdTypeArray::SafeDownCast(
      modelPolyData->GetFieldData()->GetArray(edgeNames->GetValue(i).c_str()) );
    vtkModelEdge *modelEdge =
      model->BuildModelEdge( modelVertices[ edgeArray->GetValue(0) ],
                             modelVertices[ edgeArray->GetValue(1) ] );
    modelEdges.push_back( modelEdge );

    cellIds->Allocate( edgeArray->GetNumberOfTuples() - 2 );
    cellIds->Reset();
    for (int j = 2; j < edgeArray->GetNumberOfTuples(); j++)
      {
      //We are currently presuming that at write time, that all writes will
      //combine the edge and face mesh poly data's into a single combined
      //polydata. This way all readers can presume that all edge ids come
      //before any face id
      cellIds->InsertNextId( edgeArray->GetValue(j) );
      }
    vtkDiscreteModelEdge::SafeDownCast( modelEdge )->AddCellsToGeometry( cellIds );
    modelPolyData->GetFieldData()->RemoveArray( edgeNames->GetValue(i).c_str() );
    }
  modelPolyData->GetFieldData()->RemoveArray( "Edge Names" );

  // now faces / loops
  vtkStringArray *loopNames =
    vtkStringArray::SafeDownCast(
    modelPolyData->GetFieldData()->GetAbstractArray("Loop Names") );
  vtkIdType mainLoopIndex, innerLoopIndex;
  for (mainLoopIndex = 0; mainLoopIndex < loopNames->GetNumberOfValues();
    mainLoopIndex = innerLoopIndex)
    {
    vtkIdTypeArray *loopArray = vtkIdTypeArray::SafeDownCast(
      modelPolyData->GetFieldData()->GetArray(
      loopNames->GetValue(mainLoopIndex).c_str()) );

    vtkIdType regionId = loopArray->GetValue(0);
    vtkIdType numberOfEdges = (loopArray->GetNumberOfTuples() - 2) / 2;
    vtkModelEdge **edges = new vtkModelEdge* [numberOfEdges];
    int *edgesDir = new int [numberOfEdges];
    for (vtkIdType j = 0; j < numberOfEdges; j++)
      {
      edges[j] = modelEdges[ loopArray->GetValue( j*2 + 2 ) ];
      edgesDir[j] = loopArray->GetValue( j*2 + 3 );
      }
    // If original region array exist, we should use those region Ids to
    // create materials.
    vtkModelMaterial *newMaterial = NULL;
    if(oldRegionArray && newToOldRegionMap.size()>0)
      {
      vtkIdType oldRegionId = newToOldRegionMap[regionId];
      newMaterial = vtkModelMaterial::SafeDownCast(
        model->GetModelEntity(vtkModelMaterialType, oldRegionId));
      if(!newMaterial)
        {
        newMaterial = model->BuildMaterial(oldRegionId);
        }
      }
    else
      {
      newMaterial = model->BuildMaterial(regionId);
      }
    vtkModelFace* modelFace = model->BuildModelFace(numberOfEdges, edges, edgesDir, newMaterial);
    delete [] edges;
    delete [] edgesDir;
    modelPolyData->GetFieldData()->RemoveArray(
      loopNames->GetValue(mainLoopIndex).c_str() );

    // the polygons that go with the face
    if(polyRegionMap.find(regionId) != polyRegionMap.end())
      {
      vtkDiscreteModelFace::SafeDownCast( modelFace )->AddCellsToGeometry(
        polyRegionMap.find(regionId)->second );
      }
    else
      {
      vtkWarningMacro("missing info");
      }

    // now see if there are any inner loops
    for (innerLoopIndex = mainLoopIndex + 1;
      innerLoopIndex < loopNames->GetNumberOfValues(); innerLoopIndex++)
      {
      vtkIdTypeArray *innerLoopArray = vtkIdTypeArray::SafeDownCast(
        modelPolyData->GetFieldData()->GetArray(loopNames->GetValue(innerLoopIndex).c_str()) );
      if (innerLoopArray->GetValue(0) != regionId)
        {
        break;
        }
      vtkIdType numOfEdges = (innerLoopArray->GetNumberOfTuples() - 2) / 2;
      edges = new vtkModelEdge* [numOfEdges];
      edgesDir = new int [numOfEdges];
      for (vtkIdType j = 0; j < numOfEdges; j++)
        {
        edges[j] = modelEdges[ innerLoopArray->GetValue( j*2 + 2 ) ];
        edgesDir[j] = innerLoopArray->GetValue( j*2 + 3 );
        }
      modelFace->AddLoop(numOfEdges, edges, edgesDir);
      delete [] edges;
      delete [] edgesDir;
      modelPolyData->GetFieldData()->RemoveArray( loopNames->GetValue(innerLoopIndex).c_str() );
      }
    }
  modelPolyData->GetFieldData()->RemoveArray( "Loop Names" );

  // free poly id lists
  modelPolyData->GetPointData()->RemoveArray( "Region" );

  std::map< vtkIdType, vtkIdList* >::const_iterator mapIter;
  for (mapIter = polyRegionMap.begin(); mapIter != polyRegionMap.end(); mapIter++)
    {
    mapIter->second->Delete();
    }

  vtkModel3dm2DGridRepresentation* gridRepresentation =
    vtkModel3dm2DGridRepresentation::New();
  model->SetAnalysisGridInfo(gridRepresentation);
  gridRepresentation->Delete();

  this->OperateSucceeded = 1;
  modelWrapper->InitializeWithModelGeometry();
}

//-----------------------------------------------------------------------------
void vtkCMBModelBuilder::ComputePointInsideForRegion(vtkDiscreteModelRegion *region,
                                                     vtkCellLocator *locator)
{
  double ptInside[3];

  vtkSmartPointer<vtkIdTypeArray> ptIdsArray =
    vtkSmartPointer<vtkIdTypeArray>::New();
  vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
  vtkSmartPointer<vtkIdList> triangulateIds = vtkSmartPointer<vtkIdList>::New();

  vtkDataArray *normals = locator->GetDataSet()->GetCellData()->GetNormals();

  double centroid[3];
  vtkIdType masterCellId = -1;
  vtkModelItemIterator* faces = region->NewAdjacentModelFaceIterator();
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkDiscreteModelFace *modeFace = vtkDiscreteModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkPolyData *faceGeometry = vtkPolyData::SafeDownCast( modeFace->GetGeometry() );
    vtkCellArray *polys = faceGeometry->GetPolys();
    vtkIdType numPts, *pts;
    vtkIdType cellIndex = 0;
    for (polys->InitTraversal(); polys->GetNextCell(numPts, pts); cellIndex++)
      {
      // Hmmm... shouldn't have concave polys at this point, since won't be displayed
      // correctly... just warn and skip
      if (vtkTriangulateConcavePolysFilter::IsPolygonConcave(
        faceGeometry->GetPoints(), numPts, pts))
        {
        vtkWarningMacro("Concave polygon encountered (shouldn't happen)");
        continue;
        }
      else
        {
        ptIdsArray->SetArray(pts, numPts, 1);
        vtkPolygon::ComputeCentroid(ptIdsArray, faceGeometry->GetPoints(), centroid);
        }

      // get the cell id of the cell in the master polydata matching this cell
      // in the face polydata
      masterCellId = modeFace->GetMasterCellId( cellIndex );

      double normal[3];
      if (normals)
        {
        normals->GetTuple(masterCellId, normal);
        }
      else
        {
        // normals array not specified, so need to compute normal from poly
        vtkPolygon::ComputeNormal(faceGeometry->GetPoints(), numPts, pts, normal);
        }

      // try 3 steps (1/100, 1/1000, and 1/10000)... if all fail (closest poly
      // isn't this poly) move to next poly
      polygon->Initialize(numPts, pts, faceGeometry->GetPoints());
      double delta = sqrt( polygon->GetLength2() ) / 100;

      // if the "2nd" face use, then we need to reverse the direction we step
      if (modeFace->GetModelRegion(1) == region)
        {
        delta *= -1;
        }

      vtkIdType closestCellId;
      int subId;
      double dist2, closestPt[3];
      for (int i = 0; i < 3; i++, delta /= 10.0)
        {
        // step away from the polygon, opposite direction of the normal
        ptInside[0] = centroid[0] - normal[0] * delta;
        ptInside[1] = centroid[1] - normal[1] * delta;
        ptInside[2] = centroid[2] - normal[2] * delta;

        // find closest point to our pointInside, to make sure it is on cell
        // used to generate the pointInside
        locator->FindClosestPoint(ptInside, closestPt, closestCellId, subId, dist2);
        // if not, then closer to another cell and thus MAY be outside the region
        if (masterCellId == closestCellId)
          {
          region->SetPointInside(ptInside);
          faces->Delete();
          return;
          }
        }
      }
    }
  vtkErrorMacro("Failed to set point inside for region " <<
    region->GetUniquePersistentId());
  faces->Delete();
}

//-----------------------------------------------------------------------------
void vtkCMBModelBuilder::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
