#ifndef __smoab_detail_LoadPoly_h
#define __smoab_detail_LoadPoly_h

#include "SimpleMoab.h"
#include "LinearCellConnectivity.h"

#include <vtkCellArray.h>
#include <vtkIdTypeArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>

namespace smoab { namespace detail {

//basic class that given a range of moab cells will convert them to a
//vtk poly data. If given any high order cells will do a simple linearization
//of the cells
class LoadPoly
{
  moab::Interface* Interface;
  smoab::detail::LinearCellConnectivity CellConn;
  smoab::Range Points;

public:
  //----------------------------------------------------------------------------
  //warning cells input to constructor is held by reference
  LoadPoly(const smoab::Range& cells,
           const smoab::Interface& interface):
    Interface(interface.Moab),
    CellConn(cells,interface.Moab),
    Points()
    {
    }

  const smoab::Range& moabPoints() const { return this->Points; }

  //----------------------------------------------------------------------------
  //todo: have support for using only a subsection of the input cells
  void fill(vtkPolyData *dataSet)
    {
    //now that CellConn has all the cells properly stored, lets fixup
    //the ids so that they start at zero and keep the same logical ordering
    //as before.
    vtkIdType numCells, connLen;
    this->CellConn.compactIds(numCells,connLen);
    this->addGridsTopology(dataSet,numCells,connLen);
    this->addCoordinates(dataSet);
    }

private:
  //----------------------------------------------------------------------------
  void addCoordinates(vtkPointSet *grid)
    {
    //this is sorta of hackish as moabPoints is only valid
    //after compactIds has been called
    this->CellConn.moabPoints(this->Points);

    //since the smoab::range are always unique and sorted
    //we can use the more efficient coords_iterate
    //call in moab, which returns moab internal allocated memory
    vtkNew<vtkPoints> newPoints;
    newPoints->SetDataTypeToDouble();
    newPoints->SetNumberOfPoints(this->Points.size());

    //need a pointer to the allocated vtkPoints memory so that we
    //don't need to use an extra copy and we can bypass all vtk's check
    //on out of bounds
    double *rawPoints = static_cast<double*>(newPoints->GetVoidPointer(0));
    this->Interface->get_coords(this->Points,rawPoints);

    grid->SetPoints(newPoints.GetPointer());
    }

  //----------------------------------------------------------------------------
  void addGridsTopology(vtkPolyData* data,
                        vtkIdType numCells,
                        vtkIdType numConnectivity) const
    {
    //correct the connectivity size to account for the vtk padding
    const vtkIdType vtkConnectivity = numCells + numConnectivity;

    vtkNew<vtkIdTypeArray> cellArray;
    cellArray->SetNumberOfValues(vtkConnectivity);

    vtkIdType* rawArray = static_cast<vtkIdType*>(cellArray->GetVoidPointer(0));
    this->CellConn.copyToVtkCellInfo(rawArray);

    vtkNew<vtkCellArray> cells;
    cells->SetCells(numCells,cellArray.GetPointer());


    data->SetPolys(cells.GetPointer());
    }
};


} }

#endif // __smoab_detail_LoadPoly_h
