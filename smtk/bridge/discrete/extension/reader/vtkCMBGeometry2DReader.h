//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBGeometry2DReader - "reader" for various SceneGen geometry formats
// .SECTION Description
// Not actually a reader in the sense that it internally creates the appropriate
// reader based on the filename's extension.

#ifndef __smtkdiscrete_vtkCMBGeometry2DReader_h
#define __smtkdiscrete_vtkCMBGeometry2DReader_h

#include "smtk/bridge/discrete/extension/vtkSMTKDiscreteExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class VTKSMTKDISCRETEEXT_EXPORT vtkCMBGeometry2DReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBGeometry2DReader *New();
  vtkTypeMacro(vtkCMBGeometry2DReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  /// BoundaryStyle enumerants
  enum BoundaryStyleValue
    {
    NONE,
    RELATIVE_MARGIN,
    ABSOLUTE_MARGIN,
    ABSOLUTE_BOUNDS,
    IMPORTED_POLYGON
    };

  // Description:
  // Set/get whether or how a clip boundary should be added to the model.
  //
  // The default is NONE.
  // \sa BoundaryStyleValue
  vtkSetClampMacro(BoundaryStyle,int,NONE,IMPORTED_POLYGON);
  vtkGetMacro(BoundaryStyle,int);
  void SetBoundaryStyleToNone()            { this->SetBoundaryStyle(NONE); }
  void SetBoundaryStyleToAbsoluteBounds()  { this->SetBoundaryStyle(ABSOLUTE_BOUNDS); }
  void SetBoundaryStyleToAbsoluteMargin()  { this->SetBoundaryStyle(ABSOLUTE_MARGIN); }
  void SetBoundaryStyleToRelativeMargin()  { this->SetBoundaryStyle(RELATIVE_MARGIN); }
  void SetBoundaryStyleToImportedPolygon() { this->SetBoundaryStyle(IMPORTED_POLYGON); }

  // Description:
  // Set/get the relative margin to use when BoundaryStyle is RELATIVE_MARGIN.
  //
  // The default is 5; the units are percent of the length of the bounding-box diagonal.
  //
  // When specified as a string, either 1, 2, or 4 comma-separated values may be passed.
  // Each is a percentage. When one number is passed, it is applied uniformly to all
  // margins. When two are passed, the first is applied to the horizontal margins and
  // the second to the vertical. When all 4 are passed, each margin is explicitly specified.
  vtkSetVector4Macro(RelativeMargin,double);
  vtkGetVector4Macro(RelativeMargin,double);
  virtual void SetRelativeMarginString(const char* text);

  // Description:
  // Set/get the absolute margin to use when BoundaryStyle is ABSOLUTE_MARGIN.
  //
  // The default is 1.0 and the units are world coordinate units.
  //
  // When specified as a string, either 1, 2, or 4 comma-separated values may be passed.
  // When one number is passed, it is applied uniformly to all
  // margins. When two are passed, the first is applied to the horizontal margins and
  // the second to the vertical. When all 4 are passed, each margin is explicitly specified.
  vtkSetVector4Macro(AbsoluteMargin,double);
  vtkGetVector4Macro(AbsoluteMargin,double);
  virtual void SetAbsoluteMarginString(const char* text);

  // Description:
  // Set/get the absolute coordinates to use when BoundaryStyle is ABSOLUTE_BOUNDS.
  //
  // The default is the invalid tuple (+1, -1, +1, -1) and the units are world coordinate units.
  // If fewer or more than 4 values are specified, the bounds are set to
  // the invalid tuple (+1, -1, +1, -1).
  vtkSetVector4Macro(AbsoluteBounds,double);
  vtkGetVector4Macro(AbsoluteBounds,double);
  virtual void SetAbsoluteBoundsString(const char* text);

  // Description:
  // Set/get the name of a second shapefile to use as a boundary.
  //
  // The default is NULL.
  // This value is only used when BoundaryStyle is set to IMPORTED_POLYGON.
  vtkSetStringMacro(BoundaryFile);
  vtkGetStringMacro(BoundaryFile);

protected:
  vtkCMBGeometry2DReader();
  ~vtkCMBGeometry2DReader();

  int GetMarginFromString(const char* text, double margin[4]);

  int RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);
  int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  char* FileName;
  int BoundaryStyle;
  double RelativeMargin[4];
  double AbsoluteMargin[4];
  double AbsoluteBounds[4];
  char* BoundaryFile;

private:
  vtkCMBGeometry2DReader(const vtkCMBGeometry2DReader&);  // Not implemented.
  void operator=(const vtkCMBGeometry2DReader&);  // Not implemented.
};
    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __vtkCMBGeometry2DReader_h
