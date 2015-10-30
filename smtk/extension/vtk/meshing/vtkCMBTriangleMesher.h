//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBTriangleMesher
// .SECTION Description
// This class will mesh a given 2 dimensional polygon
//
// Input: vtkPolyData
//   The input vtkPolyData must have certain properties.
//   It is assumed that all lines form closed polygons.
//   Any polygon that you want to be meshed must be stored
//   as a series of VTK_LINE's with the following properties
//
//   The input vtkPolyData needs to be processed using
//   the vtkCMBPrepareForTriangleMesher class that helps you describe
//   how the poly data will be meshed
//
//   See vtkCMBPrepareForTriangleMesher for how to format the input poly data

#ifndef __smtk_vtk_vtkCMBTriangleMesher_h
#define __smtk_vtk_vtkCMBTriangleMesher_h

#include "smtk/extension/vtk/meshing/vtkSMTKMeshingExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

namespace smtk {
  namespace vtk {

class vtkCMBMeshServerLauncher;

class VTKSMTKMESHINGEXT_EXPORT vtkCMBTriangleMesher : public vtkPolyDataAlgorithm
{

  public:
    static vtkCMBTriangleMesher *New();
    vtkTypeMacro(vtkCMBTriangleMesher,vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);
    // Description:
    // When relativly calculating maximum areas the size of
    // each individual polygon is used when this is true.
    // When false the bounds of all objects are used
    // default: false
    vtkSetMacro(UseUniqueAreas,bool);
    vtkGetMacro(UseUniqueAreas,bool);

    // Description:
    // A way to add points or preserve boundaries of polygons
    // WARNING: This option assume that no boundaries are
    // touching. Things will break if two boundaries of different
    // polygons are in contact.
    // default: true
    vtkSetMacro(PreserveBoundaries,bool);
    vtkGetMacro(PreserveBoundaries,bool);

    // Description:
    // The returned mesh will have elementIds returned with the vtkPolyData
    // for nodes and edges, as well as faces
    vtkSetMacro(PreserveEdgesAndNodes,bool);
    vtkGetMacro(PreserveEdgesAndNodes,bool);

    // Description:
    // A way to absolutely or relativly set the maximum triangle
    // area used by triangle. Whether this is absolute or relative
    // depends on the current configuration
    // default: 1/8
    vtkSetMacro(MaxArea, double);
    vtkGetMacro(MaxArea, double);

    // Description:
    // Get the max area that is sent to triangle. This returns the result
    // of the combination of the MaxArea and the MaxAreaMode.
    vtkGetMacro(ComputedMaxArea,double);

    enum MaxAreaModeOptions
      {
      NoMaxArea,
      AbsoluteArea,
      RelativeToBounds,
      RelativeToBoundsAndSegments
      };
    // Description:
    //This tells triangle how to constrain the maximum area of a
    //triangle
    //0 - No max Area
    //     >No area constraints will generate a very coarse
    //     >mesh. Not advised
    //1 - Use Absolute Area
    //     >Tells triangle to make sure there are no
    //     >triangles generated bigger than MaxArea
    //2 - Use Area Relative to Bounds
    //     >Tells triangle to make sure there are no
    //     >triangles generated  bigger than Bounds * MaxArea
    //3 - Use Area Relative to Bounds And Segments
    //     >Tells triangle to make sure there are no triangles
    //     >generated bigger than (Bounds / NumSegments) * MaxArea
    //default: 3
    vtkSetClampMacro(MaxAreaMode, int , 0 , 3);
    vtkGetMacro(MaxAreaMode,int);

    // Description:
    // A way to enable verbose console output from the mesher
    // default: false
    vtkSetMacro(VerboseOutput,bool);
    vtkGetMacro(VerboseOutput,bool);
    vtkBooleanMacro(VerboseOutput,bool);

    // Description:
    // A way to generate the maximum triangle area constraint
    // currently uses 0.01 of the area of the bounds
    // default: false
    vtkSetMacro(UseMinAngle,bool);
    vtkGetMacro(UseMinAngle,bool);
    vtkSetClampMacro(MinAngle,double,0,VTK_DOUBLE_MAX);  //defaults to 20.0
    vtkGetMacro(MinAngle,double);

    // Description:
    // Set/get the mesh server launcher class used to
    // submit data to be triangulated.
    //
    // Initially NULL, this will be set to a non-NULL value (if required),
    // when RequestData() is invoked.
    virtual void SetLauncher(vtkCMBMeshServerLauncher* launcher);
    vtkGetObjectMacro(Launcher,vtkCMBMeshServerLauncher);

  protected:
    vtkCMBTriangleMesher();
    ~vtkCMBTriangleMesher();

    //Used to configure triangle's 'q' flag
    double MinAngle;
    bool UseMinAngle;

    //Used to configure triangle's 'Y' flag
    bool PreserveBoundaries;

    //Used to configure triangle's segment markers
    //and point attribute list
    bool PreserveEdgesAndNodes;

    //Used to configure triangle's 'a' flag
    double MaxArea;
    //the actual paramater that is sent to triangle for max area
    double ComputedMaxArea;
    //Can either be
    //0 - No max Area
    //1 - Use Absolute Area
    //2 - Use Area Relative to Bounds
    //3 - Use Area Relative to Bounds And Segments
    int MaxAreaMode;
    bool UseUniqueAreas;

    //Used to configure triangle's 'V' flag
    bool VerboseOutput;

    // Allow the same launcher for multiple meshing operations:
    vtkCMBMeshServerLauncher* Launcher;

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  private:
    vtkCMBTriangleMesher(const vtkCMBTriangleMesher&);  // Not implemented.
    void operator=(const vtkCMBTriangleMesher&);  // Not implemented.
};
  } // namespace vtk
} // namespace smtk

#endif
