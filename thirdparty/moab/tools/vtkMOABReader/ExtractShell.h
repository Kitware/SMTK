#ifndef __smoab_ExtractShell_h
#define __smoab_ExtractShell_h

#include "SimpleMoab.h"
#include "detail/UsageTable.h"

#include <algorithm>

namespace smoab{

class ExtractShell
{
  const smoab::Interface& Interface;
  smoab::CellSets VCells;

public:
  ExtractShell(const smoab::CellSets volCells,
               const smoab::Interface& interface):
    Interface(interface),
    VCells(volCells)
    {
    }

  bool findSkins(smoab::CellSets &surfaceCellSets);
};


//----------------------------------------------------------------------------
bool ExtractShell::findSkins(smoab::CellSets &surfaceCellSets)
{
  typedef smoab::Range::const_iterator Iterator;

  typedef smoab::CellSets::const_iterator SetIterator;


  smoab::Range cellsToRemove;
  for(SetIterator set = this->VCells.begin();
      set != this->VCells.end();
      ++set)
    {
    const smoab::Range &cells = set->cells();
    this->Interface.createAdjacencies(set->cells(),2);


    //we create the usage table for each iteration so that we only
    //get the shell of each cell set. If we used the table between
    //sets we would get the shell of the combined sets
    smoab::detail::UsageTable table;
    for(Iterator i = cells.begin(); i != cells.end(); ++i)
      {
      std::vector<smoab::EntityHandle> faceCells =
                        this->Interface.sideElements(*i,2);

      //the usage id allows you to label cells when going into the table
      //so that you can extract multiple shells where each is based on
      //a single region id.
      std::vector<int> regionId(1,faceCells.size());
      table.incrementUsage(faceCells,regionId);
      }
    smoab::Range surfaceCells = table.singleUsage();

    //create a new cell set that
    smoab::CellSet surfaceSet(set->entity(),surfaceCells);
    surfaceCellSets.push_back(surfaceSet);

    smoab::Range subsetToRemove = table.multipleUsage();
    cellsToRemove.insert(subsetToRemove.begin(),subsetToRemove.end());
    }

  //we will remove all cells that have multiple usages from the moab database
  //I really don't care if they already existed or not.
  this->Interface.remove(cellsToRemove);
  return true;
}



}

#endif // __smoab_ExtractShell_h
