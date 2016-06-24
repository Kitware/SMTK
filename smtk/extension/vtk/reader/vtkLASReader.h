//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkLASReader - Reader for LIDAR point files
// .SECTION Description
// Reader for binary and ascii LIDAR files.  If ascii format, the file MAY contain
// rgb information for each vertex.  The format, ascii or Binary, must be
// specifed before reading the file.
//
// It is possible to only load every nth (OnRatio) point and also, individual pieces
// can be read and appended as a single dataset.

#ifndef __smtk_vtk_LASReader_h
#define __smtk_vtk_LASReader_h

#include "smtk/extension/vtk/reader/Exports.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkSmartPointer.h"

#include "vtkBoundingBox.h"
#include <vector>
#include <map>

class vtkPolyData;
class vtkTransform;
class vtkGeoSphereTransform;

#define VTK_ASCII 1
#define VTK_BINARY 2

#define NUMBER_OF_CLASSIFICATIONS 32

class VTKSMTKREADEREXT_EXPORT vtkLASReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkLASReader *New();
  vtkTypeMacro(vtkLASReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  void SetFileName(const char *filename);
  vtkGetStringMacro(FileName);

  // Description:
  // Add individual classification and the OnRatio for reading. The
  // classification will be an individual block in the output
  // Classification:   Add the classification ID for reading
  // OnRatio: Perform vtkMaskPoints(like) operation as we read in the points.  By
  // default the OnRatio is 1, so we get every point, but can increase
  // such that points are skipped.
  void AddRequestedClassificationForRead(int classification, int OnRatio);
  void RemoveAllRequestedReadClassifications();

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

  // Description
  // Retrieve bounds for the data in the file.  More specifically, gets
  // the bounds of data/pieces that have been read.
  vtkGetVector6Macro(DataBounds, double);

  vtkSetVector3Macro(Origin, double);

  // Description:
  // Transform to apply to the pts being read in for determining whether the
  // data is in/out of the ReadBounds (if LimitReadToBounds is true), or for
  // transforming data for the output (or both);  Note, the transform is
  // ignored if neither LimitReadToBounds nor TransformOutputData is true.
  void SetTransform(int index, double elements[16]);
  void SetTransform(double elements[17]); // 1st element is index
  void SetTransform(int index, vtkTransform *transform);
  vtkTransform *GetTransform(int index);
  void ClearTransforms()
    {
    for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
      {
      this->SetTransform(i, static_cast<vtkTransform*>(0));
      }
    }

  // Description:
  // Whether or not to transform the data by this->Transform for the output
  vtkBooleanMacro(TransformOutputData, bool);
  vtkSetMacro(TransformOutputData, bool);
  vtkGetMacro(TransformOutputData, bool);

  // Description:
  // Setting controls whether or not to convert from Lat/Long to x,y,z coordinates
  vtkBooleanMacro(ConvertFromLatLongToXYZ, bool);
  void SetConvertFromLatLongToXYZ(bool mode);
  vtkGetMacro(ConvertFromLatLongToXYZ, bool);

  // Description:
  // If set (true), will only scan the file, collecting info about the data
  // different classifications that are present
  vtkBooleanMacro(ScanMode, bool);
  vtkSetMacro(ScanMode, bool);
  vtkGetMacro(ScanMode, bool);

  // Description:
  // The output type defaults to float, but can instead be double.
  vtkBooleanMacro(OutputDataTypeIsDouble, bool);
  vtkSetMacro(OutputDataTypeIsDouble, bool);
  vtkGetMacro(OutputDataTypeIsDouble, bool);

protected:
  vtkLASReader();
  ~vtkLASReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  int ReadHeaderBlock();

  int ReadPoints(vtkMultiBlockDataSet *output);

  void AddClassificationFieldData(unsigned char classification, vtkPolyData *pD);

private:
  vtkLASReader(const vtkLASReader&);  // Not implemented.
  void operator=(const vtkLASReader&);  // Not implemented.

  char *FileName;

  bool LimitReadToBounds;
  double ReadBounds[6];
  double DataBounds[6];
  vtkBoundingBox ReadBBox;

  // < classification, onRatio>
  std::map<unsigned char, int> RequestedReadClassifications;
#ifndef __WRAP__
  vtkTransform *(Transform[NUMBER_OF_CLASSIFICATIONS]);
#endif
  unsigned long PointRecordsPerClassification[NUMBER_OF_CLASSIFICATIONS];

  bool ScanMode;
  bool ConvertFromLatLongToXYZ;
  bool LatLongTransform2Initialized;
  vtkSmartPointer<vtkGeoSphereTransform> LatLongTransform1;
  vtkSmartPointer<vtkTransform> LatLongTransform2;

  bool TransformOutputData;
  bool OutputDataTypeIsDouble;

  vtkTypeUInt16 GlobalEncoding;
  vtkTypeUInt16 HeaderSize;
  vtkTypeUInt32 OffsetToPointData;
  vtkTypeUInt32 NumberOfVariableLengthRecords;
  unsigned char PointDataFormat;
  vtkTypeUInt16 PointDataRecordLength;
  vtkTypeUInt32 NumberOfPointRecords;

  double ScaleFactor[3];
  double Offset[3];
  double Origin[3];

};

#endif
