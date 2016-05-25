//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkGDALRasterPolydataWrapper - Wraps reading of dem images for pointsbuilder
// .SECTION Description

#ifndef __smtk_vtk_vtkGDALRasterPolydataWrapper_h
#define __smtk_vtk_vtkGDALRasterPolydataWrapper_h

#include "smtk/extension/vtk/reader/Exports.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"

#include "vtkBoundingBox.h"
#include <string>
#include <vector>

#include "vtkGDALRasterReader.h"

namespace smtk {
  namespace vtk {

class VTKSMTKREADEREXT_EXPORT vtkGDALRasterPolydataWrapper : public vtkDataSetAlgorithm
{
public:
  static vtkGDALRasterPolydataWrapper *New();
  vtkTypeMacro(vtkGDALRasterPolydataWrapper, vtkDataSetAlgorithm);

  vtkGDALRasterPolydataWrapper();
  virtual ~vtkGDALRasterPolydataWrapper();

  // Description:
  // Set input file name
  void SetFileName(std::string const& fname);
  // Get input file name
  std::string GetFileName();

  // Description:
  // Return proj4 spatial reference.
  const char*  GetProjectionString() const;

  // Description:
  // Return geo-referenced corner points (Upper left,
  // lower left, lower right, upper right)
  const double* GetGeoCornerPoints();

  // Description:
  // Return extent of the data
  int* GetDataExtent();

  // Description:
  // Return metadata as reported by GDAL
  const std::vector<std::string>& GetMetaData();

  double GetInvalidValue();

  // Description:
  // Return domain metadata
  std::vector<std::string> GetDomainMetaData(const std::string& domain);

  // Description:
  // Return driver name which was used to read the current data
  const std::string& GetDriverShortName();
  const std::string& GetDriverLongName();

  vtkIdType GetTotalNumberOfPoints();

  vtkGetVector6Macro(DataBounds, double);

  vtkSetClampMacro(OnRatio,int,1,VTK_INT_MAX);
  vtkGetMacro(OnRatio, int);

  vtkIdType GetRealNumberOfOutputPoints()
  { return RealNumberOfOutputPoints; }

  vtkBooleanMacro(LimitToMaxNumberOfPoints, bool);
  vtkSetMacro(LimitToMaxNumberOfPoints, bool);
  vtkGetMacro(LimitToMaxNumberOfPoints, bool);

  vtkSetClampMacro(MaxNumberOfPoints,vtkIdType,1,VTK_INT_MAX);
  vtkGetMacro(MaxNumberOfPoints,vtkIdType);

  void SetTransform(vtkTransform *transform);
  vtkGetObjectMacro(Transform, vtkTransform);
  void SetTransform(double elements[16]);
  void ClearTransform()
  {
    this->SetTransform(static_cast<vtkTransform*>(0));
  }

  vtkBooleanMacro(TransformOutputData, bool);
  vtkSetMacro(TransformOutputData, bool);
  vtkGetMacro(TransformOutputData, bool);

  // Description:
  // Boolean value indicates whether or not to limit points read to a specified
  // (ReadBounds) region.
  vtkBooleanMacro(LimitReadToBounds, bool);
  vtkSetMacro(LimitReadToBounds, bool);
  vtkGetMacro(LimitReadToBounds, bool);

  // Description:
  // Bounds to use if LimitReadToBounds is On
  vtkSetVector6Macro(ReadBounds, double);
  vtkGetVector6Macro(ReadBounds, double);

  vtkGetMacro(Zone, int);
  vtkGetMacro(IsNorth, bool);

protected:

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestDataObject(vtkInformation *, vtkInformationVector **,
                                vtkInformationVector *);

  virtual int FillOutputPortInformation(int port,
                                        vtkInformation* info);
  
protected:
  vtkSmartPointer<vtkGDALRasterReader> Reader;
  double DataBounds[6];
  int OnRatio;
  int Ratio;
  std::string FileName;

  vtkIdType RealNumberOfOutputPoints;
  vtkIdType MaxNumberOfPoints;

  bool LimitToMaxNumberOfPoints;

  bool LimitReadToBounds;
  double ReadBounds[6];
  
  vtkBoundingBox ReadBBox;

  vtkTransform *Transform;
  bool TransformOutputData;

  int Zone;
  bool IsNorth;

private:
  vtkGDALRasterPolydataWrapper(const vtkGDALRasterPolydataWrapper&); // Not implemented.
  vtkGDALRasterPolydataWrapper& operator=(const vtkGDALRasterPolydataWrapper&); // Not implemented.
};

  } // namespace vtk
} // namespace smtk

#endif
