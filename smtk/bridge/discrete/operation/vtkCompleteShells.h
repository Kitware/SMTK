//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


// .NAME vtkCompleteShells - search for holes in shells and "complete" them
// .SECTION Description
// Filter to search for holes in shells and complete them, for further
// processing into a CMBModel via the vtkCMBModelBuilder.  Holes are
// identified as an edge of a cell that does not have a neighbor that is
// of the same region (shell).  To fill the hole it is expected that there
// will be two neighbors at the edge, both belonging to another shell.
// The hole is filled by following the neighbor that has its normal
// oriented as expected.
// NOTE: it is required that all shells have their normals pointing outward
// (and are already present on the input... probably from passing through
// vtkMasterPolyDataNormals).  Also, assuming no concave cells.

#ifndef __vtkCompleteShells_h
#define __vtkCompleteShells_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkCell;
class vtkCellLocator;
class vtkIdTypeArray;
class vtkIntArray;
class vtkPolyData;
//BTX
struct vtkCompleteShellsInternals;
//ETX

class SMTKDISCRETESESSION_EXPORT vtkCompleteShells : public vtkPolyDataAlgorithm
{
public:
  static vtkCompleteShells* New();
  vtkTypeMacro(vtkCompleteShells, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // If true (which is the default), after initial fixup, another pass
  // is made on solids that only havea region defined on one side of a
  // face (and the region is made up of one face).  Main use case is
  // a "submerged solid" (completely buried in the "soil").  If this
  // flag is false, then there is no boundary for the soil at this
  // interface.  Note, the algorithm was also implemented to handle a
  // solid within a solid, but in limited testing omicron complained
  // before the this filter.
  vtkBooleanMacro(DetectAndFixSubmergedSolids, bool);
  vtkSetMacro(DetectAndFixSubmergedSolids, bool);
  vtkGetMacro(DetectAndFixSubmergedSolids, bool);

  // Description:
  // If DetectAndFixSubmergedSolids is true (on), then 6 search/test directions
  // are considered for each potentially submerged solid.  We expect/hope that
  // we'll get a unanimous decision, but in case we don't (there are some
  // "edge" cases not currently being handled), decide at what point we want to
  // warn the user.  Defaults to 6 (meaning if not unanimous, user is warned).
  vtkSetClampMacro(MinimumSubmergedVoteCountToAvoidWarning, int, 2, 6);
  vtkGetMacro(MinimumSubmergedVoteCountToAvoidWarning, int);

  vtkSetStringMacro(ModelRegionArrayName);
  vtkGetStringMacro(ModelRegionArrayName);

  vtkSetStringMacro(ModelFaceArrayName);
  vtkGetStringMacro(ModelFaceArrayName);

protected:
  vtkCompleteShells();
  ~vtkCompleteShells();

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  /// Implementation of the algorithm.
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

//BTX

private:
  vtkCompleteShells(const vtkCompleteShells&); // Not implemented.
  void operator=(const vtkCompleteShells&); // Not implemented.

  void FindRegionEdge(vtkPolyData *input, vtkDataArray *cellNormals,
    vtkIntArray *regionArray, vtkIdTypeArray *modelFaceArray,
    vtkIdType cellIndex, char *visited);

  vtkIdType FindHoleFillingModelFace(vtkPolyData *input,
    vtkDataArray *cellNormals, vtkIdType currentCellId,
    vtkIdList *neighborIds, vtkIdType *pts, int ptIndex, int otherPtIndex);

  void FindClosestEnclosingRegion( int regionId, vtkIdType modelFaceId,
    vtkPolyData *input, vtkDataArray *cellNormals, vtkIntArray *regionArray,
    vtkIdTypeArray *modelFaceArray, vtkCellLocator *locator );

  // Description:
  // The name of the vtkIntArray that specifies what region each mesh facet/cell
  // is associated with in the input to the filter.
  char *ModelRegionArrayName;

  // Description:
  // The name of the vtkCellData vtkIdTypeArray used to store the
  // model face that each mesh facet/cell is assigned to.
  char* ModelFaceArrayName;

  // Used if ModelFaceArray isn't passed in
  int NextModelFaceId;

  bool DetectAndFixSubmergedSolids;
  int MinimumSubmergedVoteCountToAvoidWarning;

  vtkCompleteShellsInternals* Internals;
//ETX

};

#endif
