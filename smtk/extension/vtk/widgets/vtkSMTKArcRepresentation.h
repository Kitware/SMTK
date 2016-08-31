//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMTKArcRepresentation - Default representation for the arc widget
// .SECTION Description
// This class provides the default concrete representation for the
// vtkContourWidget. It works in conjunction with the
// vtkContourLineInterpolator and vtkPointPlacer. See vtkContourWidget
// for details.
// .SECTION See Also
// vtkOrientedGlyphContourRepresentation vtkContourRepresentation vtkContourWidget vtkPointPlacer
// vtkContourLineInterpolator

#ifndef __smtk_vtk_SMTKArcRepresentation_h
#define __smtk_vtk_SMTKArcRepresentation_h

#include "smtk/extension/vtk/widgets/Exports.h" // For export macro

#include "vtkOrientedGlyphContourRepresentation.h"

class vtkProperty;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkGlyph3D;
class vtkPoints;

class VTKSMTKWIDGETSEXT_EXPORT vtkSMTKArcRepresentation : public vtkOrientedGlyphContourRepresentation
{
public:
  // Description:
  // Instantiate this class.
  static vtkSMTKArcRepresentation *New();

  // Description:
  // Standard methods for instances of this class.
  vtkTypeMacro(vtkSMTKArcRepresentation,vtkOrientedGlyphContourRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Controls whether the contour widget should
  // generate a point array that represents if the point
  // was modifed. If used remember to turn off during construction
  // of the initial points
  // Default is to set it to false.
  vtkSetMacro( LoggingEnabled, int );
  vtkGetMacro( LoggingEnabled, int );
  vtkBooleanMacro( LoggingEnabled, int );

  //needed to make sure selected nodes are highlighted properly
  virtual int SetNthNodeSelected(int);
  int ToggleActiveNodeSelected();

  // Get the number of selected nodes
  virtual int GetNumberOfSelectedNodes();

  //overloaded for logging purposes
  virtual int DeleteNthNode(int n);
  virtual int SetActiveNodeToWorldPosition( double worldPos[3],double worldOrient[9] );
  virtual int SetActiveNodeToWorldPosition(double worldPos[3]);
  virtual int AddNodeOnContour(int X, int Y);


  // Description:
  // Get the points in this contour as a vtkPolyData.
  virtual vtkPolyData * GetContourRepresentationAsPolyData();

  //Description:
  // Get the flags for a given point
  // the flags represent if the point has been moved, inserted or deleted
  // A point can have all three flags or none of them
  int GetNodeModifiedFlags(int n);

  // Description:
  // Controls whether the contour widget can be moved to the edit mode
  vtkSetMacro( CanEdit, int );
  vtkGetMacro( CanEdit, int );
  vtkBooleanMacro( CanEdit, int );

  //Description:
  // Generate a closed rectangle arc with four points, based on the
  // points on the contour. Initially, the new rectangle is just
  // the bounding box of all available points.
  void Rectangularize();

  //Description:
  // This will make the representation a straight line containing only the
  // start and end points of the original arc.
  void Straighten();

  //Description:
  // Expose method in superClass
  void InitializeContour( vtkPolyData * poly, vtkIdList *)
  {
    this->Initialize(poly);
  }

protected:
  vtkSMTKArcRepresentation();
  ~vtkSMTKArcRepresentation();

  virtual void UpdateLines(int index);
  virtual void BuildRepresentation();

  // Description:
  // Build a contour representation from externally supplied PolyData. This
  // is very useful when you use an external program to compute a set of
  // contour nodes, let's say based on image features. Subsequently, you want
  // to build and display a contour that runs through those points.
  // This method is protected and accessible only from
  // vtkContourWidget::Initialize( vtkPolyData * )

  //Note: While this method will only render the first line cell in the polydata
  //it will compute if the contour is closed based on this first cell number of points
  //versus the number of points in the polydata. So don't have any extra points
  virtual void Initialize( vtkPolyData * );

  //support logging of point changes
  int LoggingEnabled;

  //suppor the ability to disable editing of the arc
  int CanEdit;

  class vtkInternalMap;
  vtkInternalMap *ModifiedPointMap;


  void UpdatePropertyMap(int index, int flags);


private:
  vtkSMTKArcRepresentation(const vtkSMTKArcRepresentation&);  //Not implemented
  void operator=(const vtkSMTKArcRepresentation&);  //Not implemented
};

#endif
