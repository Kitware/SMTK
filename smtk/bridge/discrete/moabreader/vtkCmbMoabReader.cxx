//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCmbMoabReader.h"

#include "CellSets.h"
#include "ExtractShell.h"
#include "detail/LoadPoly.h"
#include "detail/ReadSparseTag.h"
#include "model/FaceSets.h"
#include "model/BoundaryConditions.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkIntArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkTrivialProducer.h"

#include "ModelParserHelper.h"

namespace detail
{
//------------------------------------------------------------------------------
template<typename T>
void readSparseTag(T *array,
                   const smoab::CellSets& cellSet,
                   smoab::Tag *tag,
                   const smoab::Interface& interface)
{
  smoab::Range allCells = smoab::getAllCells(cellSet);
  smoab::detail::ReadSparseTag reader(cellSet,
                                      allCells,
                                      interface);
   reader.fill(array,tag);
 }

//------------------------------------------------------------------------------
smoab::CellSets getCellRange(const smoab::Interface& interface,
                             smoab::Tag* parentTag,
                             smoab::GeomTag* dimTag=NULL)
{
  smoab::EntityHandle rootHandle = interface.getRoot();
  smoab::Range entities = interface.findEntitiesWithTag(*parentTag,rootHandle);

  smoab::CellSets sets;
  sets.reserve(entities.size());

  smoab::Range subset;
  typedef smoab::Range::const_iterator iterator;
  for(iterator i=entities.begin(); i!= entities.end(); ++i)
    {
    if(dimTag)
      {
      subset = interface.findEntitiesWithDimension(*i,dimTag->value(),true);
      }
    else
      {
      subset = interface.findHighestDimensionEntities(*i,true);
      }

    if(!subset.empty())
      {
      smoab::CellSet set(*i,subset);
      sets.push_back(set);
      }
    }
  return sets;
}

//------------------------------------------------------------------------------
smoab::CellSets extractShells(const smoab::Interface& interface,
                              smoab::Tag* shellTag)
{
  bool extractShell = false;
  smoab::GeomTag geom3Tag(3);
  smoab::GeomTag geom2Tag(2);

  //first choice is 2d shell elements, which is the shell
  smoab::CellSets shells = detail::getCellRange(interface,shellTag,
                                                          &geom2Tag);
  //second choice is shell 3d elements, which need the shell extracted
 if(shells.empty())
    {
    shells = detail::getCellRange(interface,shellTag,&geom3Tag);
    extractShell = true;
    }

  //third is 2d geom elements, which are the shell.
  if(shells.empty())
    {
    shells = detail::getCellRange(interface,&geom3Tag,&geom2Tag);
    extractShell = false;
    }

  //last choice is the 3d geom elements
  if(shells.empty())
    {
    shells = detail::getCellRange(interface,&geom3Tag);
    extractShell = true;
    }

if(extractShell && !shells.empty())
  { //scope class to remove it form memory when done

    smoab::CellSets extractedShellSet;

    smoab::ExtractShell extract(shells,interface);
    extract.findSkins(extractedShellSet);

    return extractedShellSet;
  }
  return shells;
}

}


vtkStandardNewMacro(vtkCmbMoabReader)
//------------------------------------------------------------------------------
vtkCmbMoabReader::vtkCmbMoabReader():
  FileName(NULL)
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//------------------------------------------------------------------------------
vtkCmbMoabReader::~vtkCmbMoabReader()
{
  this->SetFileName(0);
}

//------------------------------------------------------------------------------
int vtkCmbMoabReader::RequestData(vtkInformation *vtkNotUsed(request),
                vtkInformationVector **vtkNotUsed(inputVector),
                vtkInformationVector *outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output =
    vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  smoab::Interface interface(this->FileName);

  //lets go over the rules to create domains, regions and faces.

  //Domains have a one to one mapping with material sets inside moab.
  //for each material set in moab, we create a new domain.

  //The regions of a domain are the unique entity sets that are in the
  //domain. Which means that for each material we want to find each entity
  //set and give that id as the region.

  //now each face for the region is described by intersecting each
  //domain with all other domains (from all regions) and than each
  //domain with each boundary condition

  //we have all the domains now in CellSets each labeled with the
  //entity id that it came from.
  smoab::MaterialTag shellTag; //we must delete this
  smoab::CellSets shells = detail::extractShells(interface,&shellTag);

  //we skip directly to splitting the cell sets into both unique faces
  //and regions
  smoab::NeumannTag neTag;
  smoab::CellSets neumannCellSets = detail::getCellRange(interface,&neTag);
  smoab::model::FaceCellSets regionFaceCellSets;
  //we need to properly label each unique face in shells
  //we do this by intersecting each shell with each other shell
  //to find shell on shell contact, and than we intersect each
  //resulting shell with the boundary conditions
  //the end result of these intersections will be the new modelfaces
  vtkNew<vtkIdTypeArray> faceIds;
  vtkNew<vtkIdTypeArray> faceAdjRegionIds;
  vtkNew<vtkIdTypeArray> domainRegionIds;

  faceIds->SetName(ModelParserHelper::GetModelFaceTagName());
  faceAdjRegionIds->SetName(ModelParserHelper::GetModelFaceRegionsMapString());
  domainRegionIds->SetName(ModelParserHelper::GetModelPredefinedDomainSets());
  {
    std::set<smoab::model::FacesAdjRegions> faceRegionMapping;
    regionFaceCellSets = smoab::model::findFaceSets(shells,
                                             neumannCellSets,
                                             faceRegionMapping);

    //return a vector that holds the region face ids value for each cell
    //this maps to the new unique region face id.
    std::vector<vtkIdType> faceIdsForEachCell =
                    smoab::model::faceIdsPerCell<vtkIdType>(regionFaceCellSets);

    //copy the data from the vector to the vtkArray
    faceIds->SetNumberOfValues(faceIdsForEachCell.size());
    vtkIdType *raw = static_cast<vtkIdType*>(faceIds->GetVoidPointer(0));
    std::copy(faceIdsForEachCell.begin(),faceIdsForEachCell.end(),raw);


    //now we have to convert the mapping from region entity ids into material ids
    //and save it out to a vtkIdTypeArray
    faceAdjRegionIds->SetNumberOfComponents(3);
    faceAdjRegionIds->SetNumberOfTuples(faceRegionMapping.size());


    //get the real region values based on the geometric parent child
    //relationship graph
    std::map<smoab::EntityHandle,vtkIdType> regions;
    regions = smoab::model::findRegions<vtkIdType>(faceRegionMapping,interface);

    vtkIdType index=0;
    typedef std::set<smoab::model::FacesAdjRegions>::const_iterator iterator;
    for(iterator i = faceRegionMapping.begin(); i != faceRegionMapping.end();
        ++i,++index)
      {
      vtkIdType ids[3];
      ids[0] = i->FaceId;
      ids[1] = regions[i->Region0];
      ids[2] = regions[i->Region1];
      faceAdjRegionIds->SetTupleValue(index,ids);
      }


    //now that we have the real region ids, we can identify the domains
    //we are mapping region entity handle id to domain id
    std::map<smoab::EntityHandle,vtkIdType> regionDomainMap;
    regionDomainMap = smoab::model::findDomains<vtkIdType>(shells,
                                                           &shellTag,interface);
    domainRegionIds->SetNumberOfComponents(2);
    domainRegionIds->SetNumberOfTuples(regionDomainMap.size());

    index=0;
    typedef std::map<smoab::EntityHandle,vtkIdType>::const_iterator miterator;
    for(miterator i = regionDomainMap.begin(); i != regionDomainMap.end();
        ++i,++index)
      {
      vtkIdType ids[2];
      ids[0] = i->second; //the domain id
      //the region enity handle passed through the region id lookup table
      ids[1] = regions[i->first];

      domainRegionIds->SetTupleValue(index,ids);
      }
  }
  //generate the boundary condition sets, using the information we have
  //from all the faces. The combination that we are looking for is all
  //faces with an entity id that is in the neumman sets is a face to
  //be added to the array

  //BC 100 -> 4,7,8,9
  vtkNew<vtkIdTypeArray> boundaryConditionArray;
  boundaryConditionArray->SetName(
        ModelParserHelper::GetModelPredefinedBoundarySets());

  {
    std::vector<vtkIdType> neummannTagValues;
    neummannTagValues =smoab::getTagValues<vtkIdType>(&neTag,
                                                      neumannCellSets,
                                                      interface);

    smoab::model::BoundaryConditions<vtkIdType> bc = smoab::model::extractBCS(
                                              regionFaceCellSets,
                                              neumannCellSets,
                                              neummannTagValues);

    //fill inverts the mapping so to that we have:
    //face 4 -> 100,
    //face 7 -> 100,
    //face 23 -> 40
    bc.fill(boundaryConditionArray.GetPointer());
  }


  //release some memory we don't need anymore
  neumannCellSets.clear();


  //now that all the labeling is finished lets actually load the data
  //as polydata
  {
    smoab::Range allFaceCells = smoab::getAllCells(regionFaceCellSets);
    smoab::detail::LoadPoly loader(allFaceCells,interface);
    loader.fill(output);
  }

  output->GetCellData()->AddArray(faceIds.GetPointer());
  output->GetFieldData()->AddArray(faceAdjRegionIds.GetPointer());
  output->GetFieldData()->AddArray(domainRegionIds.GetPointer());

  if(boundaryConditionArray->GetNumberOfTuples() > 0)
    {
    output->GetFieldData()->AddArray(boundaryConditionArray.GetPointer());
    }

  return 1;
}

//------------------------------------------------------------------------------
void vtkCmbMoabReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
