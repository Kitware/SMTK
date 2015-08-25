#ifndef __smoab_LinearCellConnectivity_h
#define __smoab_LinearCellConnectivity_h

#include "CellTypeToType.h"
#include "ContinousCellInfo.h"

#include <algorithm>
#include <vector>

namespace smoab { namespace detail {

namespace internal
{
  //we want a subset of the real connetivity array,
  //this does that for use with a super easy wrapper
  struct SubsetArray
  {
    SubsetArray(EntityHandle* realConn,
                int numCells,
                int currentVertsPerCell,
                int newVertsPerCell):
      Array()
    {
      const int size = numCells*newVertsPerCell;
      this->Array.reserve(size);
      if(currentVertsPerCell == newVertsPerCell)
        {
        std::copy(realConn,realConn+size, std::back_inserter(this->Array));
        }
      else
        {
        //skip copy only the first N points which we want
        //since moab stores linear points first per cell
        EntityHandle *pos = realConn;
        for(int i=0; i < numCells;++i)
          {
          std::copy(pos,pos+newVertsPerCell,std::back_inserter(this->Array));
          pos += currentVertsPerCell;
          }
        }
    }
    typedef std::vector<EntityHandle>::const_iterator const_iterator;
    typedef std::vector<EntityHandle>::iterator iterator;

    const_iterator begin() const { return this->Array.begin(); }
    iterator begin() { return this->Array.begin(); }

    const_iterator end() const { return this->Array.end(); }
    iterator end(){ return this->Array.end(); }

  private:
    std::vector<EntityHandle> Array;
  };
}

class LinearCellConnectivity
{
public:

  LinearCellConnectivity(smoab::Range const& cells, moab::Interface* moab):
    Connectivity(),
    UniquePoints(),
    Info()
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

      int vtkNumVerts;
      int vtkCellType = smoab::detail::vtkLinearCellType(type,vtkNumVerts);

      ContinousCellInfo info = { vtkCellType, vtkNumVerts, 0, iterationCount };
      this->Info.push_back(info);


      //we need to copy only a subset of the connectivity array
      internal::SubsetArray conn(connectivity,iterationCount,numVerts,vtkNumVerts);
      this->Connectivity.push_back(conn);

      count += iterationCount;
      }
    }

  //----------------------------------------------------------------------------
  void compactIds(vtkIdType& numCells, vtkIdType& connectivityLength)
    {
    //converts all the ids to be ordered starting at zero, and also
    //keeping the orginal logical ordering. Stores the result of this
    //operation in the unstrucutred grid that is passed in

    //lets determine the total length of the connectivity
    connectivityLength = 0;
    numCells = 0;
    for(InfoConstIterator i = this->Info.begin();
        i != this->Info.end();
        ++i)
      {
      connectivityLength += (*i).numCells * (*i).numVerts;
      numCells += (*i).numCells;
      }

    this->UniquePoints.reserve(connectivityLength);

    this->copyConnectivity(this->UniquePoints);
    std::sort(this->UniquePoints.begin(),this->UniquePoints.end());

    typedef std::vector<EntityHandle>::iterator EntityIterator;
    EntityIterator newEnd = std::unique(this->UniquePoints.begin(),
                                        this->UniquePoints.end());

    const std::size_t newSize = std::distance(this->UniquePoints.begin(),newEnd);
    this->UniquePoints.resize(newSize);
    }

  //----------------------------------------------------------------------------
  void moabPoints(smoab::Range& range) const
    {
    //from the documentation a reverse iterator is the fastest way
    //to insert into a range.
    std::copy(this->UniquePoints.rbegin(),
              this->UniquePoints.rend(),
              moab::range_inserter(range));
    }

  //----------------------------------------------------------------------------
  //copy the connectivity from the moab held arrays to the user input vector
  void copyConnectivity(std::vector<EntityHandle>& output) const
    {
    //walk the info to find the length of each sub connectivity array,
    //and insert them into the vector, ordering is implied by the order
    //the connecitivy sub array are added to this class
    ConnConstIterator c = this->Connectivity.begin();
    for(InfoConstIterator i = this->Info.begin();
        i != this->Info.end();
        ++i,++c)
      {
      //remember our Connectivity is a vector of pointers whose
      //length is held in the info vector.
      const int numUnusedPoints = (*i).numUnusedVerts;
      const int connLength = (*i).numCells * (*i).numVerts;
      std::copy(c->begin(),c->end(),std::back_inserter(output));
      }
    }

  //copy the information from this contianer to a vtk cell array, and
  //related lookup information
  void copyToVtkCellInfo(vtkIdType* cellArray) const
    {
    ConnConstIterator c = this->Connectivity.begin();
    for(InfoConstIterator i = this->Info.begin();
        i != this->Info.end();
        ++i, ++c)
      {
      //for this group of the same cell type we need to fill the cellTypes
      const int numCells = (*i).numCells;
      const int numVerts = (*i).numVerts;

      //for each cell in this collection that have the same type
      //grab the raw array now, so we can properly increment for each vert in each cell
      internal::SubsetArray::const_iterator moabConnectivity = c->begin();
      for(int j=0;j < numCells; ++j)
        {
        //cell arrays start and end are different, since we
        //have to account for element that states the length of each cell
        cellArray[0]=numVerts;


        for(int k=0; k < numVerts; ++k, ++moabConnectivity )
          {
          //this is going to be a root of some failures when we start
          //reading really large datasets under 32bit.


          //fyi, don't use a range ds for unique points, distance
          //function is horribly slow they need to override it
          EntityConstIterator result = std::lower_bound(
                                         this->UniquePoints.begin(),
                                         this->UniquePoints.end(),
                                         *moabConnectivity);
          std::size_t newId = std::distance(this->UniquePoints.begin(),
                                            result);
          cellArray[k+1] = static_cast<vtkIdType>(newId);
          }
        cellArray += numVerts+1;
        }
      }
    }

private:
  std::vector<internal::SubsetArray> Connectivity;
  std::vector<EntityHandle> UniquePoints;

  std::vector<detail::ContinousCellInfo> Info;

  typedef std::vector<EntityHandle>::const_iterator EntityConstIterator;
  typedef std::vector<internal::SubsetArray>::const_iterator ConnConstIterator;
  typedef std::vector<detail::ContinousCellInfo>::const_iterator InfoConstIterator;
};
} }  //namespace smoab::detail

#endif // __smoab_LinearCellConnectivity_h
