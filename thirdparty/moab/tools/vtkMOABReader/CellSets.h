
#ifndef __smoab_CellSets_h
#define __smoab_CellSets_h

#include "SimpleMoab.h"

namespace smoab
{
//----------------------------------------------------------------------------
class CellSet
{
public:
  CellSet(smoab::EntityHandle p,const smoab::Range& cells):
    Entity(p),
    Cells(cells)
    {}

  const smoab::Range& cells() const { return this->Cells; }
  EntityHandle entity() const { return this->Entity; }

  bool contains(smoab::EntityHandle c) const
    {
    return this->Cells.find(c) != this->Cells.end();
    }

  void erase(smoab::Range cells)
    {
    //seems that erase() has a bug, so use subtract
    this->Cells = smoab::subtract(this->Cells,cells);
    }

private:
  smoab::EntityHandle Entity;
  smoab::Range Cells;
};

//----------------------------------------------------------------------------
//CellSets are just a vector of CellSets
typedef std::vector<CellSet> CellSets;

//----------------------------------------------------------------------------
//templated so it works with FaceCellSets and CellSets
template<typename T>
smoab::Range getParents(const T& set)
{
  typedef typename T::const_iterator iterator;
  smoab::Range result;

  for(iterator i=set.begin(); i != set.end(); ++i)
    {
    result.insert(i->entity());
    }
  return result;
}

//----------------------------------------------------------------------------
//templated so it works with FaceCellSets and CellSets
template<typename T>
smoab::Range getAllCells(const T& set)
{
  typedef typename T::const_iterator iterator;
  smoab::Range result;

  for(iterator i=set.begin(); i != set.end(); ++i)
    {
    smoab::Range c = i->cells();
    result.insert(c.begin(),c.end());
    }
  return result;
}


}

#endif
