//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBPrepareForTriangleMesher
// .SECTION Description
//  This class is used to format vtkPolyData so that they have the necessary
//  information so that they can be used as input to the vtkCMBTriangleMesher
//  from any vtkPolyData structure.
//
//  It also has functions to get information back from this special format
//
//  The first step is to break the model down to a collection of loops.
//  A loop is a collection of arcs that describe the polygon we are building.
//  A polygon must have an exterior loop, and can have mutliple interior loops.
//  Interior loops are used to describe both holes and interior polygons.
//
//  EXAMPLE:
//
//  Say you want to mesh this polygon
//  This shape has 3 polygons and 1 hole. Polygon [P3] is a nested
//  polygon that is completely inside of Polygon [P0]. The
//
//  Let (LPX) demote a Loop
//  Let [PX]  demote a Polygon
//
//   ___________(LP0)____________________________
//   |                                          |
//  (LP0)                 [P1]                 (LP0)
//   |        ________         _________        |
//   |       |        |       |        |        |
//   |       |  <H1>  |       |  [P3]  |        |
//   |       |_(LP2)__|       |_(LP3)__|        |
//   |                                          |
//   |                                          |
//   |______________(LP0)_(LP1)_________________|
//   |                                          |
//   |                  [P2]                    |
//  (LP1)                                      (LP1)
//   |                                          |
//   |__________________(LP1)___________________|
//
//
//
//  Let [PX] denote a polygon
//  Let <HX> denote a hole with no polygon
//  Let (AX) denote an Arc
//  Let {LX} demote a line segment for an arc
//
//   _______(A1)_{L2}____________________________
//   |                                          |
//  {L1}                 [P1]                 {L13}
//   |        __(A4)__         _(A5)____        |
//  (A1)     |        |       |        |      (A1)
//   |       |  <H1>  |       |  [P3]  |        |
//   |       |__{L8}__|       |__{L12}_|        |
//   |                                          |
//   |                                          |
//   |______________(A2)__{L7}__________________|
//   |                                          |
//  (A3)                [P2]                   (A3)
//  {L4}                                       {L6}
//   |                                          |
//   |____________(A3)__{L5}____________________|

//
//  The verbal layout of the Loop picture is:
//  Loop 1 is composed of Arcs (A1,A2) and is the outside loop for Polgyon [P1]
//  Loop 2 is composed of Arcs (A2,A3) and is the outside loop for Polgyon [P2]
//  Loop 3 is composed of Arc (A4) and is the inside loop for Polgyon [P1]
//  Loop 4 is composed of Arc (A5) and is the inside loop for Polgyon [P1]
//        and is the outside loop for Polygon [P3]
//
//  The verbal layout of the Arc/Line picture is:
//  Arc 1 is composed of three consecutive lines {L1, L2, L3},
//  Arc 2 is composed of a single line {L7}
//  Arc 3 is composed of three consecutive lines {L4, L5, L6},
//  Arc 4 is composed of four consecutive lines {L8, L9, L10, L11},
//  Arc 5 is composed of four consecutive lines {L12, L13, L14, L15},
//
//
// When we combine this information together we are able to describe
// how we want the polydata meshed with a very small overhead.
//
// So now lets show how we would describe this problem using the AddLoop
// and AddArc commands of vtmCmbPrepareForTriangleMesher
//
// For: vtkIdType AddLoop( vtkIdType OutsidePoly, vtkIdType InsidePoly)
//
// Return Loop Id | OutsidePoly | InsidePoly | Descrption
//   0            |    1        |   -1     | states loop 0 is outside loop of poly 1
//   1            |    2        |   -1     | states loop 1 is outside loop of poly 2
//   2            |   -1        |    1     | states loop 2 is the outside of a hole (-1) AND inside loop of poly 1
//   3            |    3        |    1     | states loop 3 is outside loop of poly3 AND inside loop of poly 1
//
// For: vtkIdType AddArc( vtkIdType cellOffset, vtkIdType cellArraySize,
//                        vtkIdType arcId, vtkIdType loop1, vtkIdType loop2,
//                        vtkIdType endPoint1, vtkIdType endPoint2)
//
// Note endPoints are currently optional and can be just set to an incrementing
// number. You only need valid end nodes if you are using AddNode and
// want to propagate model Vertex information through the mesher
//
//  CellArrayOffset|CellArraySize|ArcId|Loop1|Loop2| Description
//       0         |      9      |  1  |  0  | -1  |
//       9         |      3      |  2  |  0  |  1  |
//       12        |      9      |  3  |  1  | -1  |
//       21        |     12      |  4  | -1  |  2  |
//       23        |     12      |  5  |  3  |  3  |
//
//
//  Todo: The AddArc method is in real need for a cleanup. Something like:
//   AddArc( Cell(0) + Cell(1) + Cell(3), LoopInfo(0,1) EndPoints(0,1) )
// Or
//  CellInArc c; c.Add(0); c.Add(1); c.Add(3);
//  AddArc(c, LoopInfo(0,1), EndPoints(0,1));
//
//
//
#ifndef __smtkdiscrete_vtkCMBPrepareForTriangleMesher_h
#define __smtkdiscrete_vtkCMBPrepareForTriangleMesher_h

#include "vtkSMTKDiscreteExtModule.h" // For export macro
#include "vtkObject.h"
#include "smtk/bridge/discrete/extension/meshing/cmbFaceMeshHelper.h"

#include <map>
#include <vector>

class vtkPolyData;
class vtkIdTypeArray;

namespace smtk {
  namespace bridge {
    namespace discrete {

class VTKSMTKDISCRETEEXT_EXPORT vtkCMBPrepareForTriangleMesher : public vtkObject
{
public:
  static vtkCMBPrepareForTriangleMesher *New();
  vtkTypeMacro(vtkCMBPrepareForTriangleMesher,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetPolyData(vtkPolyData* pd);//Set the polydata to work on

  //-----------------------------------------------------------
  //Functions for describing what to mesh
  //
  // Functions are generally called in this order
  //
  // Set your pd to work on (your input will be modified):
  //   mapInterface->SetPolyData(pd)
  // Set information for proper memory allocation (optional):
  //   mapInterface->SetNumberOfArcs(25);
  //   mapInterface->SetNumberOfLoops(4);
  // Create the arrays:
  //   mapInterface->InitializeNewMapInfo();
  // Tell the interface which lines belong to which arcs:
  //   mapInterface->AddArc(o,s,id,l1,l2,e1,e2);
  //   ...
  // Tell the interface which arcs belong to which loops:
  //   mapInterface->AddLoop(1,-1);
  //   ...
  // Add the arrays to your polydata:
  //   mapInterface->FinalizeNewMapInfo();

  void SetNumberOfArcs(const vtkIdType& num);//For preallocating arrays
  void SetNumberOfLoops(const vtkIdType& num);//For preallocating arrays
  void SetNumberOfCells(const vtkIdType& num);//#nodes+#lines, Used for cellElementIds

  void InitializeNewMapInfo();//make the polydata a map formated polydata
  void FinalizeNewMapInfo();//Add the new field data to the polydata

  //Description:
  //Input: id of a model vertex/node
  //Output: the node index in the cell array
  //IMPORTANT: You must add the nodes in the same order in which you created
  //a VTK_VERTEX for them in the polydata
  vtkIdType AddNode(const vtkIdType& nodeId);

  //Description:
  //Input: See https://www.kitware.com/CMB/index.php/Storing_Map_information_in_vtkPolyData
  //       If a datapoint is currently unknown use a -1 as a placeholder
  //Output: the arc index / the number of arcs currently added - 1
  vtkIdType AddArc(const vtkIdType& CellArrayOffset,
                   const vtkIdType& CellArraySize,
                   const vtkIdType& ArcId,
                   const vtkIdType& Loop1,
                   const vtkIdType& Loop2,
                   const vtkIdType& Endpoint1,
                   const vtkIdType& Endpoint2);

  //Description:
  //Input: the outer and inner polygon id that this loop describes
  //Output: the loop index/id
  vtkIdType AddLoop(const vtkIdType& OuterPolyId,
                    const vtkIdType& InnerPolyId );

  //Description:
  //Input: The polygon id that this loop is a part of
  //       Whether this is an outer or and inner loop
  //       the arc indexes that make up this polygon
  //Output: the loop index/id
  //ETX
  vtkIdType AddLoopWithArcs(const vtkIdType& PolyId,
                            const bool& isOuter,
                            const std::vector<vtkIdType>& arcIndexes);
  //BTX

  //-----------------------------------------------------------
  //Functions for querying a map file
  bool IsValidForReading();
  //BTX
  //returns true if the map is valid and has the face rep
  bool GetPolyId2ModelFaceRepMap(std::map<vtkIdType, CmbFaceMesherClasses::ModelFaceRep* >& pid2Face);
  //ETX
  void GetArc(vtkIdType arcId, vtkPolyData* toReturn);
protected:
  //BTX
  bool BuildLoopId2ArcIndexMap(std::map<vtkIdType, std::vector<vtkIdType> >& loopId2ArcIndex);
  bool BuildPolygonId2ModelFaceMap(const std::map<vtkIdType, std::vector<vtkIdType> >& loopId2ArcIndex, std::map<vtkIdType, CmbFaceMesherClasses::ModelFaceRep* >& pid2Face);
  //ETX
protected:
  vtkCMBPrepareForTriangleMesher();
  ~vtkCMBPrepareForTriangleMesher();

  vtkPolyData* PolyData;

  //Field data that gives arc information
  vtkIdTypeArray* fieldCellArrayOffset;
  vtkIdTypeArray* fieldCellArraySize;
  vtkIdTypeArray* fieldArcId;
  vtkIdTypeArray* fieldLoop1;
  vtkIdTypeArray* fieldLoop2;
  vtkIdTypeArray* fieldEndpoint1;
  vtkIdTypeArray* fieldEndpoint2;

  //Field data that gives loop information
  vtkIdTypeArray* fieldLoopInfo;

  vtkIdTypeArray* cellElementIds; // Cell data telling the id of each node/arc/polygon

  vtkIdType arcArraySize;
  vtkIdType loopArraySize;
  vtkIdType numCells; //used to allocate space for cellElementIds

  vtkIdType numNodesAdded;
  vtkIdType numArcsAdded;
  vtkIdType numLoopsAdded;

  bool mapInfoInitialized; //Set to true once the allocation has been done for
                           //the map info fielddata/celldata

  //BTX
  // maps a vector of arcids that make up a loop to an index
  // This is only used when using AddLoopWithArcs
  std::map< std::vector<vtkIdType>, vtkIdType > loop2loopIndex;
  //ETX
private:
  vtkCMBPrepareForTriangleMesher(const vtkCMBPrepareForTriangleMesher&);  // Not implemented.
  void operator=(const vtkCMBPrepareForTriangleMesher&);  // Not implemented.
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif
