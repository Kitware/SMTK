//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkLIDARReader - Reader for LIDAR point files
// .SECTION Description
// Reader for binary and ascii LIDAR files.  If ascii format, the file MAY contain
// rgb information for each vertex.  The format, ascii or Binary, must be
// specifed before reading the file.
//
// It is possible to only load every nth (OnRatio) point and also, individual pieces
// can be read and appended as a single dataset.

#ifndef __smtk_vtk_LIDARReader_h
#define __smtk_vtk_LIDARReader_h

#include "smtk/extension/vtk/reader/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"

#include "vtkBoundingBox.h"
#include <vector>
#include <map>

class vtkTransform;
class vtkGeoSphereTransform;
class vtkFloatArray;

#define VTK_ASCII 1
#define VTK_BINARY 2

class VTKSMTKREADEREXT_EXPORT vtkLIDARReader : public vtkPolyDataAlgorithm
{
public:
  static vtkLIDARReader *New();
  vtkTypeMacro(vtkLIDARReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  enum FileReadingStatus
    {
    READ_OK = 0,
    READ_ERROR,
    READ_ABORT
    };

  // Description:
  // Name of the file to be read.
  void SetFileName(const char *filename);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the number of pieces in the dataset that we currently know about
  // (does NOT read the file)
  int GetKnownNumberOfPieces();

  // Description:
  // Get the total number of points in the file or -1 if the file hasn't
  // already been read (ReadFileInfo())
  vtkIdType GetTotalNumberOfPoints();

  vtkGetMacro(RealNumberOfOutputPoints, vtkIdType);
  vtkGetMacro(LastReadPieceOffset, vtkIdType);

  // Description:
  // Set/Get the index of the desired piece of data
  vtkSetClampMacro(PieceIndex, int, 0, VTK_INT_MAX);
  vtkGetMacro(PieceIndex, int);

  // Description:
  // Get the number of points in the requested piece.  Note, this returns ALL
  // the points in the piece AND the file info must already have been read.
  vtkIdType GetNumberOfPointsInPiece(int pieceIndex);

  // Description:
  // Get the number of points in the piece specified by the PieceIndex.  Note,
  // this returns ALL the points in the piece AND the file info must already
  // have been read.
  int GetNumberOfPointsInPiece()
    { return this->GetNumberOfPointsInPiece(this->PieceIndex); }

  // Description:
  // Add individual pieces and their OnRatio for reading. The pieces will
  // be read and appended together as a single dataset.
  // Index:   Add the index of the desired piece of data for reading
  // OnRatio: Perform vtkMaskPoints(like) operation as we read in the points.  By
  // default the OnRatio is 1, so we get every point, but can increase
  // such that points are skipped.
  void AddRequestedPieceForRead(int pieceIndex, int OnRatio);
  void RemoveAllRequestedReadPieces();

  // Description:
  // Read the number of pieces and points per piece.  Unfortunately, we
  // have to read though the whole file to get this information (it's not in a
  // header)
  int ReadFileInfo();

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

  // Description:
  // Transform to apply to the pts being read in for determining whether the
  // data is in/out of the ReadBounds (if LimitReadToBounds is true), or for
  // transforming data for the output (or both);  Note, the transform is
  // ignored if neither LimitReadToBounds nor TransformOutputData is true.
  void SetTransform(vtkTransform *transform);
  vtkGetObjectMacro(Transform, vtkTransform);
  void SetTransform(double elements[16]);
  void ClearTransform()
    {
    this->SetTransform(static_cast<vtkTransform*>(0));
    }

  // Description:
  // Whether or not to transform the data by this->Transform for the output
  vtkBooleanMacro(TransformOutputData, bool);
  vtkSetMacro(TransformOutputData, bool);
  vtkGetMacro(TransformOutputData, bool);

  // Description:
  // Boolean value indicates whether or not to limit number of points read
  // based on MaxNumbeOfPoints.
  vtkBooleanMacro(LimitToMaxNumberOfPoints, bool);
  vtkSetMacro(LimitToMaxNumberOfPoints, bool);
  vtkGetMacro(LimitToMaxNumberOfPoints, bool);

  // Description:
  // The maximum number of points to load if LimitToMaxNumberOfPoints is on/true.
  // Sets a temporary onRatio.
  vtkSetClampMacro(MaxNumberOfPoints,vtkIdType,1,VTK_INT_MAX);
  vtkGetMacro(MaxNumberOfPoints,vtkIdType);

  // Description:
  // Setting controls whether or not to convert from Lat/Long to x,y,z coordinates
  vtkBooleanMacro(ConvertFromLatLongToXYZ, bool);
  void SetConvertFromLatLongToXYZ(bool mode);
  vtkGetMacro(ConvertFromLatLongToXYZ, bool);

  // Description:
  // The output type defaults to float, but can instead be double.
  vtkBooleanMacro(OutputDataTypeIsDouble, bool);
  vtkSetMacro(OutputDataTypeIsDouble, bool);
  vtkGetMacro(OutputDataTypeIsDouble, bool);

protected:
  vtkLIDARReader();
  ~vtkLIDARReader();

  friend class vtkLIDARMultiFilesReader;

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int MoveToStartOfPiece(ifstream &fin, int pieceIndex);

  int ReadPiece(ifstream &fin, int pieceIndex, int onRatio, long totalNumPts,
    vtkPoints *newPts, vtkCellArray *newVerts,
    vtkUnsignedCharArray *scalars, vtkFloatArray *intensityArray,
    vtkUnsignedCharArray *pieceIndexArray);


  // Description:
  // Get file type used to do last read
  vtkGetMacro(FileType,int);
  int GetPointInfo(ifstream &fin);
  vtkIdType GetEstimatedNumOfOutPoints();

  char *FileName;
  int FileType;

  int ValuesPerLine;
  int BytesPerPoint;

  bool CompleteFileHasBeenRead;

  bool LimitReadToBounds;
  double ReadBounds[6];
  double DataBounds[6];
  vtkBoundingBox ReadBBox;

  vtkIdType MaxNumberOfPoints;
  bool LimitToMaxNumberOfPoints;
  bool OutputDataTypeIsDouble;

  struct LIDARPieceInfo
    {
    LIDARPieceInfo()
      {
      this->PiecePointsOffset = 0;
      this->NumPoints = 0;
      }
    long PiecePointsOffset;
    long PieceStartOffset;
    vtkIdType NumPoints;
    vtkBoundingBox BBox;
    };

  std::vector<LIDARPieceInfo> LIDARPieces;
  std::map<int, int> RequestedReadPieces; // < pieceIndex, onRatio>
  vtkIdType RealNumberOfOutputPoints;
  vtkIdType LastReadPieceOffset;
  int PieceIndex;

  vtkTransform *Transform;

  bool ConvertFromLatLongToXYZ;
  bool LatLongTransform2Initialized;
  vtkSmartPointer<vtkGeoSphereTransform> LatLongTransform1;
  vtkSmartPointer<vtkTransform> LatLongTransform2;

  bool TransformOutputData;

private:
  vtkLIDARReader(const vtkLIDARReader&);  // Not implemented.
  void operator=(const vtkLIDARReader&);  // Not implemented.

};

#endif
