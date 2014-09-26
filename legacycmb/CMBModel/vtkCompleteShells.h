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

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCell;
class vtkCellLocator;
class vtkIdTypeArray;
class vtkIntArray;
class vtkPolyData;
//BTX
struct vtkCompleteShellsInternals;
//ETX

class VTKCMBDISCRETEMODEL_EXPORT vtkCompleteShells : public vtkPolyDataAlgorithm
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


