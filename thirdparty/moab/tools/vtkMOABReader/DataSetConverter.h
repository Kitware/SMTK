#ifndef __smoab_DataSetConverter_h
#define __smoab_DataSetConverter_h

#include "SimpleMoab.h"
#include "CellSets.h"
#include "detail/LoadGeometry.h"
#include "detail/ReadSparseTag.h"

#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkIntArray.h>
#include <vtkIdTypeArray.h>
#include <vtkNew.h>

#include <algorithm>

namespace smoab{
class DataSetConverter
{
  const smoab::Interface& Interface;
  moab::Interface* Moab;
  const smoab::Tag *Tag;
  bool ReadMaterialIds;
  bool ReadProperties;
  std::string MaterialName;

public:
  DataSetConverter(const smoab::Interface& interface, const smoab::Tag* tag):
    Interface(interface),
    Moab(interface.Moab),
    Tag(tag),
    ReadMaterialIds(false),
    ReadProperties(false)
    {
    }

  void readMaterialIds(bool add) { this->ReadMaterialIds = add; }
  bool readMaterialIds() const { return this->ReadMaterialIds; }

  void readProperties(bool readProps) { this->ReadProperties = readProps; }
  bool readProperties() const { return this->ReadProperties; }

  //----------------------------------------------------------------------------
  //given a range of entity handles merge them all into a single unstructured
  //grid. Currently doesn't support reading properties.
  //Will read in material ids,  if no material id is assigned to an entity,
  //its cells will be given an unique id
  template<typename VTKGridType>
  bool fill(const smoab::Range& entities,
            VTKGridType* grid) const
    {
    //create a helper datastructure which can determines all the unique point ids
    //and converts moab connecitvity info to vtk connectivity


    //get all the cells for each parent entity and create
    // an entity set of those items
    int dim = this->Tag->value();
    typedef smoab::Range::const_iterator iterator;
    smoab::CellSets entitySets;
    for(iterator i=entities.begin(); i!= entities.end(); ++i)
      {
      smoab::Range entitiesCells;
      if(this->Tag->isComparable())
        {
        //if we are comparable only find the cells that match our tags dimension
        entitiesCells = this->Interface.findEntitiesWithDimension(*i,dim,true);
        }
      else
        {
        entitiesCells = this->Interface.findHighestDimensionEntities(*i,true);
        }
      smoab::CellSet set(*i,entitiesCells);
      entitySets.push_back(set);
      }

    moab::Range cells = smoab::getAllCells(entitySets);


    //convert the datastructure from a list of cells to a vtk data set
    detail::LoadGeometry loadGeom(cells,dim,this->Interface);
    loadGeom.fill(grid);

    if(this->readMaterialIds())
      {

      detail::ReadSparseTag materialTagReading(entitySets,
                                               cells,
                                               this->Interface);

      smoab::MaterialTag mtag;
      vtkNew<vtkIntArray> materials;
      materials->SetName(mtag.name());
      materialTagReading.fill(materials.GetPointer(),&mtag);
      grid->GetCellData()->AddArray(materials.GetPointer());
      }

    //by default we always try to load the default tag
    detail::ReadSparseTag sTagReading(entitySets,
                                      cells,
                                      this->Interface);

    vtkNew<vtkIntArray> sparseTagData;
    sparseTagData->SetName(this->Tag->name());
    sTagReading.fill(sparseTagData.GetPointer(),this->Tag);
    grid->GetCellData()->AddArray(sparseTagData.GetPointer());

    return true;
    }

  //----------------------------------------------------------------------------
  //given a single entity handle create a unstructured grid from it.
  //optional third parameter is the material id to use if readMaterialIds
  //is on, and no material sparse tag is found for this entity
  template<typename VTKGridType>
  bool fill(const smoab::EntityHandle& entity,
            VTKGridType* grid,
            const int materialId=0) const
    {
    //create a helper datastructure which can determines all the unique point ids
    //and converts moab connecitvity info to vtk connectivity

    smoab::Range cells;
    int dim = this->Tag->value();
    if(this->Tag->isComparable())
      {
      //if we are comparable only find the cells that match our tags dimension
      cells = this->Interface.findEntitiesWithDimension(entity,dim,true);
      }
    else
      {
      //load subentities
      cells = this->Interface.findHighestDimensionEntities(entity,true);
      }

    //convert the datastructure from a list of cells to a vtk data set
    detail::LoadGeometry loadGeom(cells,dim,this->Interface);
    loadGeom.fill(grid);

    const smoab::Range& points = loadGeom.moabPoints();

    if(this->readProperties())
      {
      this->readProperties(cells,grid->GetCellData());
      this->readProperties(points,grid->GetPointData());
      }

    smoab::CellSets cellSets;
    smoab::CellSet set(entity,cells);
    cellSets.push_back(set);
    if(this->readMaterialIds())
      {
      smoab::MaterialTag mtag;
      detail::ReadSparseTag materialTagReading(cellSets,
                                               cells,
                                               this->Interface);

      vtkNew<vtkIntArray> materials;
      materials->SetName(mtag.name());
      materialTagReading.fill(materials.GetPointer(),&mtag);
      grid->GetCellData()->AddArray(materials.GetPointer());

      }

    //by default we always try to load the default tag
    detail::ReadSparseTag sTagReading(cellSets,
                                      cells,
                                      this->Interface);

    vtkNew<vtkIntArray> sparseTagData;
    sparseTagData->SetName(this->Tag->name());
    sTagReading.fill(sparseTagData.GetPointer(),this->Tag);
    grid->GetCellData()->AddArray(sparseTagData.GetPointer());

    return true;
    }

private:
  //----------------------------------------------------------------------------
  void readProperties(smoab::Range const& entities,
                           vtkFieldData* field) const
    {
    if(entities.empty()) { return; }

    //so we get all the tags and parse out the sparse and dense tags
    //that we support
    typedef std::vector<moab::Tag>::const_iterator iterator;
    std::vector<moab::Tag> tags;
    this->Moab->tag_get_tags_on_entity(entities.front(),tags);

    this->readDenseTags(tags,entities,field);
    }

  //----------------------------------------------------------------------------
  void readDenseTags(std::vector<moab::Tag> &tags,
                     smoab::Range const& entities,
                     vtkFieldData* field) const
    {
    typedef std::vector<moab::Tag>::const_iterator iterator;

    for(iterator i=tags.begin();i!=tags.end();++i)
      {
      moab::TagType tagType;
      moab::DataType tagDataType;

      this->Moab->tag_get_type(*i,tagType);
      this->Moab->tag_get_data_type(*i,tagDataType);

      //make sure it is only dense
      if(tagType != moab::MB_TAG_DENSE)
        {
        continue;
        }
      //and only integer and double
      if(tagDataType != moab::MB_TYPE_DOUBLE &&
         tagDataType != moab::MB_TYPE_INTEGER)
        {
        //unsupported type, skip to next tag
        continue;
        }

      //read the name of the tag
      std::string name;
      name.reserve(32);
      this->Moab->tag_get_name(*i,name);

      //read the number of components of the tag
      int numComps = 1;

      this->Moab->tag_get_length(*i,numComps);

      //read the data if it is one of the two types we support
      int size = entities.size();
      if(tagDataType == moab::MB_TYPE_DOUBLE)
        {
        vtkNew<vtkDoubleArray> array;
        array->SetName(name.c_str());
        array->SetNumberOfComponents(numComps);
        array->SetNumberOfTuples(size);

        //read directly into the double array
        this->Moab->tag_get_data(*i,entities,
                                 array->GetVoidPointer(0));
        field->AddArray(array.GetPointer());
        }
      else if(tagDataType == moab::MB_TYPE_INTEGER)
        {
        vtkNew<vtkIntArray> array;
        array->SetName(name.c_str());
        array->SetNumberOfComponents(numComps);
        array->SetNumberOfTuples(size);

        //read directly into the double array
        this->Moab->tag_get_data(*i,entities,
                                 array->GetVoidPointer(0));
        field->AddArray(array.GetPointer());
        }
      else
        {
        }
      }
    }


};

}
#endif // __smoab_DataSetConverter_h
