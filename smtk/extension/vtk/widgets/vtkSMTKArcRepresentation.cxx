//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/widgets/vtkSMTKArcRepresentation.h"
#include "vtkCleanPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkAssemblyPath.h"
#include "vtkMath.h"
#include "vtkInteractorObserver.h"
#include "vtkLine.h"
#include "vtkCoordinate.h"
#include "vtkGlyph3D.h"
#include "vtkCursor2D.h"
#include "vtkCylinderSource.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkCamera.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkFocalPlanePointPlacer.h"
#include "vtkBezierContourLineInterpolator.h"
#include "vtkSphereSource.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkNew.h"
#include "vtkBoundingBox.h"
#include "vtkCellArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkCommand.h"

#include <map>

vtkStandardNewMacro(vtkSMTKArcRepresentation);

namespace
{
  typedef std::map<int,int> vtkInternalMapBase;
  typedef std::map<int,int>::iterator vtkInternalMapIterator;
  enum ModifiedPointFlags
    {
    Point_Moved = 1 << 1,
    Point_Deleted = 1 << 2,
    Point_Inserted = 1 << 4,
    Point_Original = 1 << 8
    };
}
class vtkSMTKArcRepresentation::vtkInternalMap : public vtkInternalMapBase {};


//----------------------------------------------------------------------
vtkSMTKArcRepresentation::vtkSMTKArcRepresentation()
{
  this->LinesMapper->SetScalarModeToUseCellData();
  this->Property->SetLineWidth(2.0);
  this->Property->SetPointSize(6.0);
  this->LinesProperty->SetLineWidth(2.0);
  this->ActiveProperty->SetLineWidth(2.0);
  this->LoggingEnabled = false;
  this->ModifiedPointMap = new vtkSMTKArcRepresentation::vtkInternalMap();
  this->CanEdit = 1;
  this->PointSelectMode = 0;
  this->pointId = 0;
  this->PointSelectCallBack = nullptr;
}

//----------------------------------------------------------------------
vtkSMTKArcRepresentation::~vtkSMTKArcRepresentation()
{
  if (PointSelectCallBack)
  {
    PointSelectCallBack->Delete();
  }
  PointSelectCallBack = nullptr;
  if ( this->ModifiedPointMap )
    {
    delete this->ModifiedPointMap;
    }
}
//----------------------------------------------------------------------
int vtkSMTKArcRepresentation::GetNumberOfSelectedNodes()
{
  int numSelected = 0;
  for (int i=0; i < this->GetNumberOfNodes()-1; ++i)
    {
    //get the flags for this selected node.
    if(this->GetNthNodeSelected(i))
      {
      numSelected++;
      }
    }

  return numSelected;
}

//----------------------------------------------------------------------
int vtkSMTKArcRepresentation::SetNthNodeSelected(int n)
{
  if ( n < 0 ||
    static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    // Failed.
    return 0;
    }

  if(this->Internal->Nodes[n]->Selected != 1)
    {
    this->Internal->Nodes[n]->Selected = 1;
    this->NeedToRender = 1;
    this->Modified();
    }
  return 1;
}

//----------------------------------------------------------------------
int vtkSMTKArcRepresentation::ToggleActiveNodeSelected()
{
  if ( this->ActiveNode < 0 ||
    static_cast<unsigned int>(this->ActiveNode) >= this->Internal->Nodes.size())
    {
    //index out of range
    return 0;
    }

  if ( this->ActiveNode == 0 ||
      static_cast<unsigned int>(this->ActiveNode) == (this->Internal->Nodes.size() -1) )
    {
    //can't toggle the first and last node off but we can turn them on
    return this->SetNthNodeSelected(this->ActiveNode);
    }

    this->Internal->Nodes[this->ActiveNode]->Selected =
      this->Internal->Nodes[this->ActiveNode]->Selected ? 0 : 1;
    this->NeedToRender = 1;
    this->Modified();
    return 1;

}

//----------------------------------------------------------------------
int vtkSMTKArcRepresentation::ActivateNode( double displayPos[2] )
{
  // if there is no nodes yet, nothing to activate.
  if(this->GetNumberOfNodes() == 0)
    {
    return 0;
    }

  return this->vtkContourRepresentation::ActivateNode(displayPos);
}

//----------------------------------------------------------------------
int vtkSMTKArcRepresentation::DeleteNthNode(int n)
{
  if(PointSelectMode)
  {
    return 0;
  }
  if (n <= 0)
    {
    //you can't delete this first node ever!
    return 0;
    }
  int lastNode = static_cast<int>(this->Internal->Nodes.size()) -1;
  bool deletingLastNode = (n == lastNode);
  if (deletingLastNode && !this->CanEdit)
    {
    //you can't delete this node since it is connected to other things
    return 0;
    }

  int good = this->Superclass::DeleteNthNode(n);
  if ( good )
    {
    this->UpdatePropertyMap(n,Point_Deleted);
    }

  if (deletingLastNode)
    {
    //in this case we need to promote the new last node to be selected
    this->SetNthNodeSelected(lastNode-1);
    }
  return good;
}

//-----------------------------------------------------------------------------
void vtkSMTKArcRepresentation::UpdateLines(int index)
{
  this->Superclass::UpdateLines(index);
}

//-----------------------------------------------------------------------------
void vtkSMTKArcRepresentation::BuildRepresentation()
{
  this->Superclass::BuildRepresentation();
  if(this->ShowSelectedNodes && this->SelectedNodesGlypher)
    {
    // If the SelectedNodesCursorShape is not set from SuperClass,
    // set it here.
    if(!this->SelectedNodesCursorShape ||
      !this->SelectedNodesCursorShape->GetPoints())
      {
      vtkNew<vtkSphereSource> sphere;
      sphere->SetThetaResolution(12);
      sphere->SetRadius(0.3);
      sphere->Update();
      if(this->SelectedNodesCursorShape)
        {
        this->SelectedNodesCursorShape->Delete();
        }
      this->SelectedNodesCursorShape = sphere->GetOutput();
      this->SelectedNodesCursorShape->Register(this);
      this->SelectedNodesGlypher->SetSourceData(
        this->SelectedNodesCursorShape);
      this->SelectedNodesGlypher->Update();
      this->SelectedNodesMapper->SetInputConnection(
        this->SelectedNodesGlypher->GetOutputPort());
      }
    }
}

//-----------------------------------------------------------------------------
int vtkSMTKArcRepresentation::SetActiveNodeToWorldPosition( double worldPos[3],double worldOrient[9] )
{
  int ret = this->Superclass::SetActiveNodeToWorldPosition(worldPos,worldOrient);
  if (ret==1)
    {
    this->UpdatePropertyMap(this->ActiveNode,Point_Moved);
    }
  return ret;
}

//-----------------------------------------------------------------------------
int vtkSMTKArcRepresentation::SetActiveNodeToWorldPosition(double worldPos[3])
{
  int ret = this->Superclass::SetActiveNodeToWorldPosition(worldPos);
  if (ret==1)
    {
    this->UpdatePropertyMap(this->ActiveNode,Point_Moved);
    }
  return ret;
}

void vtkSMTKArcRepresentation::StartWidgetInteraction(double startEventPos[2])
{
  if(PointSelectMode)
  {
    if( this->GetCurrentOperation() == vtkContourRepresentation::Translate )
    {
      this->SetCurrentOperationToInactive();
    }
    PointSelectCallBack->Execute(NULL, this->ActiveNode, NULL);
  }
  else
  {
    this->Superclass::StartWidgetInteraction(startEventPos);
  }
}

//----------------------------------------------------------------------
int vtkSMTKArcRepresentation::AddNodeOnContour(int X, int Y)
{
  if(PointSelectMode)
  {
    return 0;
  }
  int idx;

  double worldPos[3];
  double worldOrient[9] = {1.0,0.0,0.0,
                           0.0,1.0,0.0,
                           0.0,0.0,1.0};

  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  double displayPos[2];
  displayPos[0] = X;
  displayPos[1] = Y;
  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, worldPos,
                                                 worldOrient) )
    {
    return 0;
    }

  double pos[3];
  if ( !this->FindClosestPointOnContour( X, Y, pos, &idx ) )
    {
    return 0;
    }

  if ( idx > this->GetNumberOfNodes() - 1 && !this->CanEdit)
    {
    //we disable editing currently under this use case
    // "idx == this->GetNumberOfNodes() - 1" should be a valid case.
    // Check the logic in vtkContourRepresentation::FindClosestPointOnContour()
    return 0;
    }

  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos,
                                                 pos,
                                                 worldPos,
                                                 worldOrient) )
    {
    return 0;
    }

  // Add a new point at this position
  vtkContourRepresentationNode *node = new vtkContourRepresentationNode;
  node->WorldPosition[0] = worldPos[0];
  node->WorldPosition[1] = worldPos[1];
  node->WorldPosition[2] = worldPos[2];
  node->Selected = 0;
  node->PointId = this->pointId;
  pointId++;

  this->GetRendererComputedDisplayPositionFromWorldPosition(
          worldPos, worldOrient, node->NormalizedDisplayPosition );
  this->Renderer->DisplayToNormalizedDisplay(
         node->NormalizedDisplayPosition[0],
         node->NormalizedDisplayPosition[1] );
  memcpy(node->WorldOrientation, worldOrient, 9*sizeof(double) );

  // For a special case when trying to insert a point between the end node
  // and the start node, it should be appended at the end, not front
  if(idx == 0 && this->ClosedLoop)
    {
    this->Internal->Nodes.push_back(node);
    idx = this->GetNumberOfNodes() - 1;
    }
  else// if(idx == this->GetNumberOfNodes() - 1)
    {
    this->Internal->Nodes.insert(this->Internal->Nodes.begin() + idx, node);
    }

  this->UpdatePropertyMap(idx,Point_Inserted);
  this->Superclass::UpdateLines( idx );
  this->NeedToRender = 1;

  return 1;
}

int vtkSMTKArcRepresentation::AddNodeAtDisplayPosition(int X, int Y)
{
  int addNode = GetNumberOfNodes();
  int r = vtkOrientedGlyphContourRepresentation::AddNodeAtDisplayPosition(X,Y);
  if(r)
  {
    vtkContourRepresentationNode * node = GetNthNode( addNode );
    node->PointId = this->pointId;
    pointId++;
  }
  return r;
}

//----------------------------------------------------------------------
void vtkSMTKArcRepresentation::UpdatePropertyMap(int index, int flags)
{
  if ( this->GetLoggingEnabled() == 1 )
    {
    vtkInternalMap::iterator it;
    it = this->ModifiedPointMap->find(index);
    if ( it == this->ModifiedPointMap->end() )
      {
      int value = 0;
      value |= flags;
      this->ModifiedPointMap->insert(it,
        std::pair<int,int>(index,value));
      }
    else if ( flags & Point_Inserted )
      {
      //special use case, we have to insert a value into the map
      //this means we have to recurse a bit
      int oldFlags = it->second;
      it->second = Point_Inserted;
      this->UpdatePropertyMap(index+1,oldFlags);
      }
    else
      {
      it->second |= flags;
      }
    }
}

//-------------------------------------------------------------------------
int vtkSMTKArcRepresentation::ComputeInteractionState(int X, int Y, int modified)
{
  if(this->FocalPoint->GetNumberOfPoints() == 0)
  {
    int numPoints = this->GetNumberOfNodes();
    //Some times the focal point has no data with it.  This results in a
    //segfault in Superclass::ComputeInteractionState, since when there is
    //no data, part of FocalPoint is NULL. there is no check on FocalPoint
    //in the superclass.
    for ( int i = 0; i < numPoints; i++ )
    {
      if ( i != this->ActiveNode )
      {
        double worldPos[3];
        double worldOrient[9];
        this->GetNthNodeWorldPosition( i, worldPos );
        this->GetNthNodeWorldOrientation( i, worldOrient );
        this->FocalPoint->InsertNextPoint(worldPos );
        this->FocalData->GetPointData()->GetNormals()->InsertNextTuple(worldOrient+6);
      }
    }
  }
  if(this->FocalPoint->GetNumberOfPoints() == 0)
  {
    return this->InteractionState; // not pretty sure.
  }
  return Superclass::ComputeInteractionState(X,Y,modified);
}

//-----------------------------------------------------------------------------
vtkPolyData* vtkSMTKArcRepresentation::GetContourRepresentationAsPolyData()
{
  // Make sure we are up to date with any changes made in the placer
  this->UpdateContour();
  this->BuildLines();

  vtkSmartPointer<vtkUnsignedIntArray> ids =
                              vtkSmartPointer<vtkUnsignedIntArray>::New();
  ids->SetNumberOfComponents(1);
  ids->SetName ("PointIDs");
  //std::set<vtkIdType> checker;

  for(int i = 0; i < GetNumberOfNodes(); ++i)
  {
    //assert( checker.find(GetNthNode( i )->PointId) == checker.end());
    //checker.insert(GetNthNode( i )->PointId);
    ids->InsertNextTuple1(GetNthNode( i )->PointId);
  }

  this->Lines->GetPointData()->SetScalars(ids);

  return Lines;
 }

//-----------------------------------------------------------------------------
int vtkSMTKArcRepresentation::GetNodeModifiedFlags(int n)
{
  int flag = 0;
  vtkInternalMap::iterator it;
  it = this->ModifiedPointMap->find(n);
  if ( it != this->ModifiedPointMap->end() )
    {
    flag = it->second;
    }
   return flag;
}

//-----------------------------------------------------------------------------
void vtkSMTKArcRepresentation::SetPointSelectCallBack(vtkCommand * cp)
{
  if(this->PointSelectCallBack)
  {
    this->PointSelectCallBack->Delete();
  }
  this->PointSelectCallBack = cp;
}

//----------------------------------------------------------------------
void vtkSMTKArcRepresentation::Initialize( vtkPolyData * pd )
{
  //make sure to reset the mapping each time we init
  this->ModifiedPointMap->clear();

  vtkPoints *points   = pd->GetPoints();
  vtkIdType nPoints = points->GetNumberOfPoints();
  if (nPoints <= 0)
    {
    return; // Yeah right.. build from nothing !
    }

  // Clear all existing nodes.
  for(unsigned int i=0;i<this->Internal->Nodes.size();i++)
    {
    for (unsigned int j=0;j<this->Internal->Nodes[i]->Points.size();j++)
      {
      delete this->Internal->Nodes[i]->Points[j];
      }
    this->Internal->Nodes[i]->Points.clear();
    delete this->Internal->Nodes[i];
    }
  this->Internal->Nodes.clear();
  pointId = 0;

  vtkPolyData *tmpPoints = vtkPolyData::New();
  tmpPoints->DeepCopy(pd);
  this->Locator->SetDataSet(tmpPoints);
  tmpPoints->Delete();

  //reserver space in memory to speed up vector push_back
  this->Internal->Nodes.reserve(nPoints);

  //account for the offset if the input has vert cells
  vtkIdList *pointIds = pd->GetCell(pd->GetNumberOfVerts())->GetPointIds();
  vtkDataArray * vda = pd->GetPointData()->GetScalars();
  vtkIdType numPointsInLineCells = pointIds->GetNumberOfIds();

  // Get the worldOrient from the point placer
  double ref[3], displayPos[2], worldPos[3];
  double worldOrient[9] = {1.0,0.0,0.0,
                           0.0,1.0,0.0,
                           0.0,0.0,1.0};
  ref[0] = 0.0; ref[1] = 0.0; ref[2] = 0.0;
  displayPos[0] = 0.0; displayPos[1] = 0.0;
  this->PointPlacer->ComputeWorldPosition(this->Renderer,
                                 displayPos, ref, worldPos, worldOrient );

  // Add nodes without calling rebuild lines
  // to improve performance dramatically(~15x) on large datasets

  double *pos;
  //we use nPoints so we don't add the last point
  //if it is a closed loop, since that is covered as an exception
  //after the for loop
  for ( vtkIdType i=0; i < nPoints; i++ )
    {
    if (i == 0)
      {
      //mark the first point as an original point
      int value = 0;
      value |= Point_Original;
      this->ModifiedPointMap->insert(std::pair<int,int>(0,value));
      }
    pos = points->GetPoint( pointIds->GetId(i) );
    this->GetRendererComputedDisplayPositionFromWorldPosition(
                          pos, worldOrient, displayPos );

    // Add a new point at this position
    vtkContourRepresentationNode *node = new vtkContourRepresentationNode;
    node->WorldPosition[0] = pos[0];
    node->WorldPosition[1] = pos[1];
    node->WorldPosition[2] = pos[2];
    node->Selected = 0;
    node->PointId = vda->GetTuple1(i);
    pointId = std::max(pointId, static_cast<unsigned int>(node->PointId+1));

    node->NormalizedDisplayPosition[0] = displayPos[0];
    node->NormalizedDisplayPosition[1] = displayPos[1];

    this->Renderer->DisplayToNormalizedDisplay(
      node->NormalizedDisplayPosition[0],
      node->NormalizedDisplayPosition[1] );

    memcpy(node->WorldOrientation, worldOrient, 9*sizeof(double) );

    this->Internal->Nodes.push_back(node);

    if ( this->LineInterpolator && this->GetNumberOfNodes() > 1 )
      {
      // Give the line interpolator a chance to update the node.
      int didNodeChange = this->LineInterpolator->UpdateNode(
        this->Renderer, this, node->WorldPosition, this->GetNumberOfNodes()-1 );

      // Give the point placer a chance to validate the updated node. If its not
      // valid, discard the LineInterpolator's change.
      if ( didNodeChange && !this->PointPlacer->ValidateWorldPosition(
                node->WorldPosition, worldOrient ) )
        {
        node->WorldPosition[0] = worldPos[0];
        node->WorldPosition[1] = worldPos[1];
        node->WorldPosition[2] = worldPos[2];
        }
      }
    }

  if (pointIds->GetId(0) == pointIds->GetId(numPointsInLineCells-1))
    {
    this->SetClosedLoop(1);
    }
  else
    {
    //we have a unique end node mark it
    int value = 0;
    value |= Point_Original;
    this->ModifiedPointMap->insert(
          std::pair<int,int>(numPointsInLineCells-1,value));
    this->SetClosedLoop(0);
    }

  // Update the contour representation from the nodes using the line interpolator.
  // NOTE: Don't use UpdateLines(i) for every point because it will call BuildLines(),
  // which means re-building the line nPoints times!!!
  if (this->LineInterpolator)
    {
    int indices[2];
    vtkNew<vtkIntArray> arr;
    for (vtkIdType i=1; i < nPoints; ++i)
      {
      this->LineInterpolator->GetSpan( i, arr.GetPointer(), this );
      int nNodes = arr->GetNumberOfTuples();
      for (int j = 0; j < nNodes; j++)
        {
        arr->GetTypedTuple( j, indices );
        this->UpdateLine( indices[0], indices[1] );
        }
      }
    }

  this->UpdateLines(nPoints);
  this->BuildRepresentation();

  // Show the contour.
  this->VisibilityOn();
}

//-----------------------------------------------------------------------------
void vtkSMTKArcRepresentation::PrintSelf(ostream& os,
                                                      vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Logging Enabled: "
     << (this->LoggingEnabled ? "On\n" : "Off\n");
}
