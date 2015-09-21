#ifndef __smoab_detail_ReadSparseTag_h
#define __smoab_detail_ReadSparseTag_h

#include "SimpleMoab.h"

#include <vtkIntArray.h>

#include <string>
#include <algorithm>
#include <vector>

namespace smoab { namespace detail {


struct ReadSparseTag
{

  ReadSparseTag(const smoab::CellSets& mSets,
                const smoab::Range& cells,
                const smoab::Interface& interface):
    Interface(interface),
    MeshSets(mSets),
    Cells(cells)
    {
    }

  //the cellTag describes the tag mapping between the cells
  //and meshsets.
  void fill(vtkIntArray* array, const Tag* cellTag) const;
  void fill(vtkIdTypeArray* array, const Tag* cellTag) const;

private:
  template<typename T>
  void fillRawArray(T* sparseTagArray, std::size_t length,
                    const Tag* cellTag) const;

  template<typename T>
  void singleSetRead(T *sparseTagArray,
                     const std::vector<int>& values,
                     std::size_t length) const;

  template<typename T>
  void multiSetRead(T *sparseTagArray,
                    const std::vector<int>& values,
                    std::size_t length,
                    int defaultTagValue) const;

  const smoab::Interface& Interface;
  const smoab::CellSets& MeshSets;
  const smoab::Range& Cells;
};

//----------------------------------------------------------------------------
void ReadSparseTag::fill(vtkIntArray* array, const Tag* cellTag) const
{
  const std::size_t length = this->Cells.size();
  array->SetNumberOfValues(length);
  int *raw = static_cast<int*>(array->GetVoidPointer(0));
  this->fillRawArray(raw,length,cellTag);
}

//----------------------------------------------------------------------------
void ReadSparseTag::fill(vtkIdTypeArray* array, const Tag* cellTag) const
{
  const std::size_t length = this->Cells.size();
  array->SetNumberOfValues(length);
  vtkIdType *raw = static_cast<vtkIdType*>(array->GetVoidPointer(0));
  this->fillRawArray(raw,length,cellTag);
}

//----------------------------------------------------------------------------
template<typename T>
void ReadSparseTag::fillRawArray(T *sparseTagArray,
                                 std::size_t length,
                                 const smoab::Tag* sparseTag) const
{

  typedef std::vector<int>::const_iterator IdConstIterator;
  typedef std::vector<int>::iterator IdIterator;
  typedef smoab::CellSets::const_iterator CellSetIterator;

  std::vector<int> sparseTagValues(this->MeshSets.size());
  //first off iterate the entities and determine which ones
  //have moab material ids

  //todo get proper default value from moab
  //wrap this area with scope, to remove local variables

  {
    const moab::Tag stag = this->Interface.getMoabTag(*sparseTag);
    IdIterator tagIds = sparseTagValues.begin();
    for(CellSetIterator i=this->MeshSets.begin();
        i != this->MeshSets.end();
        ++i, ++tagIds)
      {
      //getTagData clobbers defaultValue
      int defaultValue = this->Interface.getDefaultTagVaue<int>(stag);
      *tagIds = this->Interface.getTagData(stag,i->entity(),defaultValue);
      }

    //now determine ids for all entities that don't have materials
    IdConstIterator maxPos = std::max_element(sparseTagValues.begin(),
                                              sparseTagValues.end());
    int maxValue = *maxPos;
    for(IdIterator i=sparseTagValues.begin(); i!= sparseTagValues.end(); ++i)
      {
      if(*i==-1)
        {
        *i = ++maxValue;
        }
      }
  }
  if(this->MeshSets.size() == 1)
    {
    this->singleSetRead(sparseTagArray,sparseTagValues,length);
    }
  else
    {
    int defaultValue = this->Interface.getDefaultTagVaue<int>(*sparseTag);
    this->multiSetRead(sparseTagArray,sparseTagValues,length,defaultValue);
    }
}

//----------------------------------------------------------------------------
template<typename T>
void ReadSparseTag::singleSetRead(T *sparseTagArray,
                                  const std::vector<int>& values,
                                  std::size_t length) const
  {

  //now we set all the values as this has a single meshset so we
  //have no complicated logic for mapping each cell to a meshset
  T value = static_cast<T>(values[0]);
  std::fill(sparseTagArray,sparseTagArray+length,value);
  }

//----------------------------------------------------------------------------
template<typename T>
void ReadSparseTag::multiSetRead(T *sparseTagArray,
                                 const std::vector<int>& sparseTagValues,
                                 std::size_t numCells,
                                 int defaultTagValue) const
  {
  typedef std::vector<smoab::EntityHandle>::const_iterator EntityHandleIterator;
  typedef std::vector<int>::const_iterator IdConstIterator;
  typedef smoab::CellSets::const_iterator CellSetIterator;
  typedef smoab::Range::const_iterator RangeIterator;

  //create the search structure as a range is really slow to search with
  //lower_bounds
  std::vector<smoab::EntityHandle> searchableCells;
  smoab::RangeToVector(this->Cells,searchableCells);


  //pre fill with -1 to mark cells we don't touch, since some cells
  //might no have a default tag
  T defaultValue = T(defaultTagValue);
  std::fill(sparseTagArray,sparseTagArray+numCells,defaultValue);

  EntityHandleIterator s_begin = searchableCells.begin();
  EntityHandleIterator s_end = searchableCells.end();

  IdConstIterator currentTagValue = sparseTagValues.begin();
  for(CellSetIterator i=this->MeshSets.begin();
      i!=this->MeshSets.end(); ++i, ++currentTagValue)
    {
    T value = static_cast<T>(*currentTagValue);

    const smoab::Range& entitiesCells = i->cells();
    for(RangeIterator j=entitiesCells.begin(); j != entitiesCells.end();++j)
      {
      EntityHandleIterator result = std::lower_bound(s_begin,
                                                     s_end,
                                                     *j);
      std::size_t newId = std::distance(s_begin,result);
      sparseTagArray[newId] = value;
      }
    }

}

}
}

#endif // __smoab_detail_ReadSparseTag_h
