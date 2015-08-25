#ifndef __smoab_detail_ReduceSpectralMesh_h
#define __smoab_detail_ReduceSpectralMesh_h

#include "SimpleMoab.h"
#include "vtkCellType.h"

#include <algorithm>
#include <vector>

namespace smoab{ namespace detail{


class ReduceSpectralMesh
{
public:
  ReduceSpectralMeshReduceHighOrderCell(smoab::Range const& cells, moab::Interface* moab):
    LinearCellConnectivity(),
    HighOrderCellConnectivity()
    {
    int count = 0;
    const std::size_t cellSize=cells.size();
    while(count != cellSize)
      {
      EntityHandle* connectivity;
      int numVerts=0, iterationCount=0;
      //use the highly efficent calls, since we know that are of the same dimension
      moab->connect_iterate(cells.begin()+count,
                            cells.end(),
                            connectivity,
                            numVerts,
                            iterationCount);
      //if we didn't read anything, break!
      if(iterationCount == 0)
        {
        break;
        }

      //identify the cell type that we currently have,
      //store that along with the connectivity in a temp storage vector
      const moab::EntityType type = moab->type_from_handle(*cells.begin()+count);

      //instead of storing the connectivity in a single array,
      //what my goal is to have multiple vectors. The first vector
      //will hold all the linear cells, while the second vector
      //will hold the high order cells.

      //Since connect_iterate returns the number of cells of a given
      //type that we have read we can also store the RunLegthInfo
      //for all the cells allowing us to apply transformations to the
      //high order cells efficently in bulk.

      //while all these cells are contiously of the same type,
      //quadric hexs in vtk have 20 points, but moab has 21 so we
      //need to store this difference
      }
    }

private:
  std::vector<EntityHandle> LinearCellConnectivity;
  std::vector<EntityHandle> HighOrderCellConnectivity;

  std::vector<detail::ContinousCellInfo> LinearCellInfo;
  std::vector<detail::ContinousCellInfo> HighOrderCellInfo;


} } //namespace smoab::detail

#endif
