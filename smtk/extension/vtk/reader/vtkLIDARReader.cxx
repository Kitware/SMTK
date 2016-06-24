//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkLIDARReader.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkStringArray.h"
#include "vtkMath.h"
#include "vtkGeoSphereTransform.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <vtksys/SystemTools.hxx>

//#define LIDAR_PREVIEW_PIECE_NUM_POINTS 10000
#define LIDAR_BINARY_POINT_SIZE sizeof(double)*3

vtkStandardNewMacro(vtkLIDARReader);

vtkCxxSetObjectMacro(vtkLIDARReader, Transform, vtkTransform);

//-----------------------------------------------------------------------------
vtkLIDARReader::vtkLIDARReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->ValuesPerLine = -1;
  this->BytesPerPoint = 0;
  this->CompleteFileHasBeenRead = false;
  this->RealNumberOfOutputPoints = 0;
  this->LastReadPieceOffset = 0;
  this->LimitReadToBounds = false;
  this->PieceIndex = -1;
  this->ReadBounds[0] = this->ReadBounds[2] = this->ReadBounds[4] = VTK_DOUBLE_MAX;
  this->ReadBounds[1] = this->ReadBounds[3] = this->ReadBounds[5] = VTK_DOUBLE_MIN;
  this->Transform = 0;
  this->TransformOutputData = false;

  this->ConvertFromLatLongToXYZ = false;
  this->LatLongTransform1 = vtkSmartPointer<vtkGeoSphereTransform>::New();
  this->LatLongTransform2 = vtkSmartPointer<vtkTransform>::New();
  this->LatLongTransform2Initialized = false;

  this->MaxNumberOfPoints = 1000000;
  this->LimitToMaxNumberOfPoints = false;

  this->OutputDataTypeIsDouble = false;
  this->FileType = VTK_ASCII;

  this->Origin[0] = this->Origin[1] = this->Origin[2] = 0;
}

//-----------------------------------------------------------------------------
vtkLIDARReader::~vtkLIDARReader()
{
  this->SetFileName(0);
  this->SetTransform(static_cast<vtkTransform*>(0));
}

//-----------------------------------------------------------------------------
void vtkLIDARReader::SetConvertFromLatLongToXYZ(bool mode)
{
  if (this->ConvertFromLatLongToXYZ == mode)
    {
    return;
    }

  this->ConvertFromLatLongToXYZ = mode;

  // need to reset bounds on each piece we know about
  for (size_t i = 0; i < this->LIDARPieces.size(); i++)
    {
    this->LIDARPieces[i].BBox.Reset();
    }

  this->LatLongTransform2Initialized = false;

  this->Modified();
}

//-----------------------------------------------------------------------------
// vtkSetStringMacro except we clear some variables if we update the value
void vtkLIDARReader::SetFileName(const char *filename)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting FileName to " << filename );
  if (this->FileName == NULL && filename == NULL)
    {
    return;
    }
  if (this->FileName && filename && !strcmp(this->FileName, filename))
    {
    return;
    }
  if (this->FileName)
    {
    delete [] this->FileName;
    }
  if (filename)
    {
    size_t n = strlen(filename) + 1;
    char *cp1 =  new char[n];
    const char *cp2 = (filename);
    this->FileName = cp1;
    do
      {
      *cp1++ = *cp2++;
      } while ( --n );
    }
   else
    {
    this->FileName = NULL;
    }

  this->CompleteFileHasBeenRead = false;

  // only want to clear these values if FileName changes!
  this->LIDARPieces.clear();
  this->DataBounds[0] = this->DataBounds[2] = this->DataBounds[4] = VTK_DOUBLE_MAX;
  this->DataBounds[1] = this->DataBounds[3] = this->DataBounds[5] = VTK_DOUBLE_MIN;
  this->RequestedReadPieces.clear();
  this->ValuesPerLine = -1;
  this->BytesPerPoint = 0;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkLIDARReader::AddRequestedPieceForRead(int pieceIdx, int onRatio)
{
  if(pieceIdx >= 0 && onRatio > 0)
    {
    this->RequestedReadPieces[pieceIdx] = onRatio;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkLIDARReader::RemoveAllRequestedReadPieces()
{
  this->RequestedReadPieces.clear();
  this->Modified();
}

//-----------------------------------------------------------------------------
int vtkLIDARReader::GetKnownNumberOfPieces()
{
  return static_cast<int>(this->LIDARPieces.size());
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARReader::GetTotalNumberOfPoints()
{
  if (this->LIDARPieces.size() == 0)
    {
    // -1 indicates that ReadFileInfo not yet done
    return -1;
    }

  vtkIdType totalNumPts = 0;
  for(std::vector<LIDARPieceInfo>::iterator it=
    this->LIDARPieces.begin();
    it!=this->LIDARPieces.end();it++)
    {
    totalNumPts += (*it).NumPoints;
    }

  return totalNumPts;
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARReader::GetNumberOfPointsInPiece(int pieceIndex)
{
  if (pieceIndex < 0 || pieceIndex >= static_cast<int>(this->LIDARPieces.size()))
    {
    return -1;
    }

  return this->LIDARPieces[pieceIndex].NumPoints;
}

//-----------------------------------------------------------------------------
void vtkLIDARReader::SetTransform(double elements[16])
{
  vtkTransform *tmpTransform = vtkTransform::New();
  tmpTransform->SetMatrix(elements);
  this->SetTransform(tmpTransform);
  tmpTransform->Delete();
}

//-----------------------------------------------------------------------------
int vtkLIDARReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  int res = this->ReadFileInfo();
  if(res == READ_ERROR)
    {
    return 0;
    }
  else if(res == READ_ABORT)
    {
    return 1;
    }

  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (this->RequestedReadPieces.size() > 0 &&
    this->RequestedReadPieces.begin()->second == VTK_INT_MAX)
    {
    return 1; // just scanning to get file info, which already done in ReadFileInfo
    }

  ifstream fin;

  // always open in binary mode, because we want tellg/seekg to work
  // "correctly" on all platforms
  fin.open(this->FileName, ios::binary);

  if(!fin)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    this->SetFileName(NULL);
    return 0;
    }

  int numOutputPts = this->GetEstimatedNumOfOutPoints();
  if (numOutputPts == 0)
    {
    numOutputPts = 1;
    }

  vtkPoints *newPts = vtkPoints::New();
  if (this->OutputDataTypeIsDouble)
    {
    newPts->SetDataTypeToDouble();
    }
  else
    {
    newPts->SetDataTypeToFloat();
    }
  vtkCellArray *newVerts = vtkCellArray::New();
  output->SetPoints( newPts );
  output->SetVerts( newVerts );
  newPts->UnRegister(this);
  newVerts->UnRegister(this);

  vtkUnsignedCharArray *scalars = 0;
  vtkFloatArray *intensityArray = 0;
  if (this->FileType == VTK_ASCII && this->ValuesPerLine == 7)
    {
    scalars = vtkUnsignedCharArray::New();
    scalars->SetNumberOfComponents(3);
    scalars->SetName("Color");
    output->GetPointData()->SetScalars( scalars );
    scalars->UnRegister(this);
    }
  if (this->FileType == VTK_ASCII &&
    (this->ValuesPerLine == 7 || this->ValuesPerLine == 4))
    {
    intensityArray = vtkFloatArray::New();
    intensityArray->SetName("Intensity");
    intensityArray->SetNumberOfComponents(1);
    output->GetPointData()->AddArray( intensityArray );
    intensityArray->UnRegister(this);
    }

  this->UpdateProgress(0);
  if (this->GetAbortExecute())
    {
    fin.close();
    this->UpdateProgress( 1.0 );
    return 1;
    }

  newPts->Allocate( numOutputPts );
  newVerts->Allocate( numOutputPts );
  if (scalars)
    {
    scalars->Allocate( numOutputPts * 3 );
    intensityArray->Allocate( numOutputPts );
    }

  vtkUnsignedCharArray *pieceIndexArray = vtkUnsignedCharArray::New();
  pieceIndexArray->SetNumberOfComponents(1);
  pieceIndexArray->Allocate( numOutputPts );
  pieceIndexArray->SetName("PieceIndex");
  output->GetPointData()->AddArray( pieceIndexArray );
  pieceIndexArray->UnRegister(this);

  // setup the ReadBBox, IF we're limiting the read to specifed ReadBounds
  if (this->LimitReadToBounds)
    {
    this->ReadBBox.Reset();
    this->ReadBBox.SetMinPoint(this->ReadBounds[0], this->ReadBounds[2],
      this->ReadBounds[4]);
    this->ReadBBox.SetMaxPoint(this->ReadBounds[1], this->ReadBounds[3],
      this->ReadBounds[5]);
    // the ReadBBox is guaranteed to be "valid", regardless of the whether
    // ReadBounds is valid.  If any of the MonPoint values are greater than
    // the corresponding MaxPoint, the MinPoint component will be set to be
    // the same as the MaxPoint during the SetMaxPoint fn call.
    }

  if (this->RequestedReadPieces.size()==0) // read all pieces
    {
    int j=0;
   int onRationForAllPieces = 1;
    if (this->LimitToMaxNumberOfPoints)
      {
      onRationForAllPieces =
        ceil(static_cast<double>(this->GetTotalNumberOfPoints()) /
        this->MaxNumberOfPoints);
      }
    do
      {
      res = this->ReadPiece(fin, j, onRationForAllPieces,
        numOutputPts, newPts, newVerts, scalars, intensityArray, pieceIndexArray);
      if(res != READ_OK)
        {
        fin.close();
        this->UpdateProgress( 1.0 );
        return res==READ_ABORT ? 1 : 0;
        }
      } while (++j < this->GetKnownNumberOfPieces());
    }
  else // read single pieces
    {
    for(std::map<int, int>::iterator it=this->RequestedReadPieces.begin();
      it != this->RequestedReadPieces.end(); it++)
      {
      int onRatio = it->second;
      if (this->LimitToMaxNumberOfPoints)
        {
        onRatio = ceil(static_cast<double>(this->LIDARPieces[it->first].NumPoints) /
          this->MaxNumberOfPoints);
        }
      res = this->ReadPiece(fin, it->first, onRatio, numOutputPts, newPts,
        newVerts, scalars, intensityArray, pieceIndexArray);
      if(res != READ_OK)
        {
        fin.close();
        this->UpdateProgress( 1.0 );
        return res == READ_ABORT ? 1 : 0;
        }
      }
    }

  fin.close();

  newPts->Squeeze();
  this->RealNumberOfOutputPoints = output->GetNumberOfPoints();
  this->UpdateProgress( 1.0 );

  std::string fileNameStr =this->FileName;

  vtksys::SystemTools::ConvertToUnixSlashes(fileNameStr);
  std::string fullName = vtksys::SystemTools::CollapseFullPath(fileNameStr.c_str());

  // Append File name to output
  vtkSmartPointer<vtkStringArray> filenameFD =
    vtkSmartPointer<vtkStringArray>::New();
  filenameFD->SetName("FileName");
  filenameFD->InsertNextValue(fullName);
  output->GetFieldData()->AddArray( filenameFD );


  // set our DataBounds to be bounds from all pieces we've read data from (this
  // RequestData as well as previous of the same dataset)
  vtkBoundingBox bbox;
  for(std::vector<LIDARPieceInfo>::iterator it=
    this->LIDARPieces.begin(); it!=this->LIDARPieces.end();it++)
    {
    if (it->BBox.IsValid())
      {
      bbox.AddBox(it->BBox);
      }
    }
  bbox.GetBounds(this->DataBounds);
  if(intensityArray)
    {
    output->GetPointData()->SetActiveScalars(
      intensityArray->GetName());
    }

  return 1;
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARReader::GetEstimatedNumOfOutPoints()
{
  vtkIdType numOutputPts = 0;
  if (this->RequestedReadPieces.size()>0)
    {
    for(std::map<int, int>::iterator it=this->RequestedReadPieces.begin();
      it != this->RequestedReadPieces.end(); it++)
      {
      // previous logic here used an estimate total of points... but must have
      // read ReadFileInfo by this point, and thus either had error or we know
      // what the total is for each piece
      if (it->first < static_cast<int>(this->LIDARPieces.size()))
        {
        numOutputPts += this->LIDARPieces[it->first].NumPoints / it->second;
        }
      }
    }
  else
    {
    if(this->CompleteFileHasBeenRead)
      {
      numOutputPts = this->GetTotalNumberOfPoints();
      }
    else
      {
      numOutputPts = 0;
      }
    }

  return numOutputPts;
}

//-----------------------------------------------------------------------------
int vtkLIDARReader::ReadFileInfo()
{
  if (this->CompleteFileHasBeenRead)
    {
    return READ_OK;
    }

  // set/determine the filetype
  std::string fileNameStr = this->FileName;
  if (fileNameStr.find("bin.pts") != std::string::npos ||
    fileNameStr.find(".bin") != std::string::npos)
    {
    this->FileType = VTK_BINARY;
    }
  else
    {
    this->FileType = VTK_ASCII;
    }

  if((this->FileType == VTK_ASCII && this->ValuesPerLine > 0) ||
    this->BytesPerPoint > 0)
    {
    return READ_OK;
    }

  ifstream fin;

  // always open in binary mode, because we want tellg/seekg to work
  // "correctly" on all platforms
  fin.open(this->FileName, ios::binary);

  if(!fin)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return READ_ERROR;
    }

  struct stat fs;
  if (stat(this->FileName, &fs) != 0)
    {
    fin.close();
    vtkErrorMacro(<< "stat() failed on file, " << this->FileName);
    return READ_ERROR;
    }

  if(this->GetPointInfo(fin) != VTK_OK)
    {
    fin.close();
    vtkErrorMacro(<< "Invalid BytesPerPoint, " << this->BytesPerPoint);
    return READ_ERROR;
    }


  fin.seekg(0, ios::beg );
  this->UpdateProgress(0.0);
  this->SetProgressText("Reading All Pieces Info ...");
  // force to read whole file
  int res = this->MoveToStartOfPiece(fin, -1);
  if( res == READ_OK)
    {
    this->CompleteFileHasBeenRead = true;
    }
  else if(res == READ_ABORT)
    {
    }
  else
    {
    vtkErrorMacro("Unable to read file?");
    }

  this->UpdateProgress(0.99);
  fin.close();
  return res;
}

//-----------------------------------------------------------------------------
int vtkLIDARReader::GetPointInfo(ifstream &fin)
{
  if((this->FileType == VTK_ASCII && this->ValuesPerLine > 0) ||
    this->BytesPerPoint > 0)
    {
    return VTK_OK;
    }

  LIDARPieceInfo pieceInfo;
  fin.seekg(0, ios::beg );
  vtkTypeInt32 numPts = -1;
  char buffer[2048];
  //get the first piece info
  pieceInfo.PieceStartOffset = fin.tellg();
  while(!fin.eof() && numPts <= 0)
    {
    if (this->FileType == VTK_ASCII)
      {
      fin.getline(buffer, 2048);
      long tempNumPts;
      char temp[2048];
      // In the case of an ASCII File the first non-empty line
      // should be the number of points - consider the file
      // invalid if the line does not contain a single Int
      do
        {
        // Scanf should match the interger part but not the string
        int numArgs = sscanf(buffer, "%ld%s", &tempNumPts, temp);
        if (numArgs == 1)
          {
          numPts = static_cast<vtkTypeInt32>(tempNumPts);
          }
        else if (numArgs != -1)
          {
          vtkErrorMacro(<< "Invalid Pts Format (missing number of points) in the file:" << this->FileName);
          return VTK_ERROR;
          }
        else
          {
          fin.getline(buffer, 2048);
          }
        }
      while(!fin.eof() && numPts <= 0);
      }
    else
      {
      fin.read(reinterpret_cast<char *>(&numPts), sizeof(vtkTypeInt32));
      }
    // Add the first piece info
    if(this->LIDARPieces.size()==0)
      {
      pieceInfo.PiecePointsOffset = fin.tellg();
      pieceInfo.NumPoints = numPts;
      this->LIDARPieces.push_back(pieceInfo);
      }
    }

  if (numPts < 0)
    {
    }
  else if (numPts == 0)
    {
    // so no points, but still a "valid" file, set some defaults so we won't
    // go through this (fn, other than 1st if statement) again
    this->ValuesPerLine = 3;
    this->BytesPerPoint = LIDAR_BINARY_POINT_SIZE;
    return VTK_OK;
    }

  float jnk;
  double rgb[3], pt[3];
  if (this->FileType == VTK_ASCII && this->ValuesPerLine <=0)
    {
    fin.getline(buffer, 2048);
    this->BytesPerPoint = strlen(buffer);
    this->ValuesPerLine =
      sscanf(buffer, "%lf %lf %lf %f %lf %lf %lf", pt, pt+1, pt+2,
      &jnk, rgb, rgb+1, rgb+2);
    }

  if(this->FileType == VTK_BINARY)
    {
    this->BytesPerPoint = LIDAR_BINARY_POINT_SIZE;
    }

  if(this->BytesPerPoint <=0 ||
    (this->FileType == VTK_ASCII && this->ValuesPerLine <=0))
    {
    vtkErrorMacro(<< "Invalid Point Info in the file:" << this->FileName);
    return VTK_ERROR;
    }

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkLIDARReader::ReadPiece(ifstream &fin, int pieceIndex, int onRatio,
                              long totalNumPts,
                              vtkPoints *newPts, vtkCellArray *newVerts,
                              vtkUnsignedCharArray *scalars,
                              vtkFloatArray *intensityArray,
                              vtkUnsignedCharArray *pieceIndexArray)
{
  int res = this->MoveToStartOfPiece(fin, pieceIndex);
  if(res != READ_OK)
    {
    return res;
    }

  // IF we are limiting the read to the specified bounds, the bounds
  // of this piece are valid (have we read this piece previously), AND
  // the specified ReadBounds do NOT intersect the bounds of this piece
  // THEN we can skip the piece
  if (this->LimitReadToBounds &&
    this->LIDARPieces[pieceIndex].BBox.IsValid())
    {
    vtkBoundingBox tmpBBox;
    if (this->Transform) // transform the bounds of the dataset
      {
      double bounds[6], *tmpPt;
      this->LIDARPieces[pieceIndex].BBox.GetBounds(bounds);
      for (int i = 0; i < 2; i++)
        {
        for (int j = 0; j < 2; j++)
          {
          for (int k = 0; k < 2; k++)
            {
            tmpPt = this->Transform->TransformPoint(bounds[i], bounds[2+j], bounds[4+k]);
            tmpBBox.AddPoint(tmpPt);
            }
          }
        }
      }
    else
      {
      tmpBBox = this->LIDARPieces[pieceIndex].BBox;
      }

    if (this->ReadBBox.Intersects( tmpBBox ) == 0)
      {
      return READ_OK;
      }
    }


  long numPts = this->LIDARPieces[pieceIndex].NumPoints;
  float intensity;
  // initialized in case we have piece that doesn't have rgb but 1st did
  double rgb[3] = {0, 0, 0};
  double pt[3];
  char buffer[2048];
  char progressText[100];
  sprintf(progressText, "%s %d", "Reading Piece ", pieceIndex);
  this->SetProgressText(progressText);
  vtkIdType idx;
  for (long i = 0; i < numPts; i ++)
    {
    if (this->FileType == VTK_ASCII)
      {
      fin.getline(buffer, 2048);
      }
    else
      {
      fin.read(reinterpret_cast<char*>(pt),sizeof(double)*3);
      }
    if (i % onRatio == 0)
      {
      if (this->FileType == VTK_ASCII)
        {
        sscanf(buffer, "%lf %lf %lf %f %lf %lf %lf", pt, pt+1, pt+2,
          &intensity, rgb, rgb+1, rgb+2);
        }
      else if (onRatio > 1)
        {
        // binary data, so we can skip ahead to the next point we want
        fin.seekg(sizeof(double)*3*(onRatio-1), ios::cur);
        i += onRatio - 1;
        }

      if (this->ConvertFromLatLongToXYZ)
        {
        this->LatLongTransform1->TransformPoint(pt, pt);
        if (!this->LatLongTransform2Initialized)
          {
          this->LatLongTransform2Initialized = true;
          this->LatLongTransform2->Identity();
          double rotationAxis[3], zAxis[3] = {0, 0, 1};
          double tempPt[3] = {pt[0], pt[1], pt[2]};
          vtkMath::Normalize(tempPt);
          vtkMath::Cross(tempPt, zAxis, rotationAxis);
          double angle = vtkMath::DegreesFromRadians( acos(tempPt[2]) );

          this->LatLongTransform2->PreMultiply();
          this->LatLongTransform2->RotateWXYZ(angle, rotationAxis);
          this->LatLongTransform2->Translate(-pt[0], -pt[1], -pt[2]);
          }
        this->LatLongTransform2->TransformPoint(pt, pt);
        }

      pt[0] -= this->Origin[0];
      pt[1] -= this->Origin[1];
      pt[1] -= this->Origin[2];

      // always computing/updating the bounds... which can/will frequently
      // be wasted effort; done before transformation
      this->LIDARPieces[pieceIndex].BBox.AddPoint(pt);

      // add the point, but 1st make sure it is in the ReadBounds (if specified);
      // consider the Transform if set (and "on")
      double transformedPt[3];
      if (this->Transform)
        {
        // only need the transformed pt if we're limiting read based on bounds or
        // we're transforming the output
        if (this->LimitReadToBounds || this->TransformOutputData)
          {
          this->Transform->TransformPoint(pt, transformedPt);
          }
        if (this->LimitReadToBounds && !this->ReadBBox.ContainsPoint(transformedPt))
          {
          continue;
          }
        }
      else // not transformed, use as read in
        {
        if (this->LimitReadToBounds && !this->ReadBBox.ContainsPoint(pt))
          {
          continue;
          }
        }

      if (this->Transform && this->TransformOutputData)
        {
        idx = newPts->InsertNextPoint(transformedPt);
        }
      else
        {
        idx = newPts->InsertNextPoint(pt);
        }
      newVerts->InsertNextCell(1, &idx);
      if (scalars)
        {
        scalars->InsertNextTuple(rgb);
        }
      if (intensityArray)
        {
        intensityArray->InsertNextValue( intensity );
        }
      pieceIndexArray->InsertNextValue( pieceIndex );

      if ((idx%100)==0)
        {
        this->UpdateProgress( static_cast<double>(idx) / static_cast<double>(totalNumPts) );
        if (this->GetAbortExecute())
          {
          fin.close();
          return READ_ABORT;
          }
        }
      }
    }

  // we've read this far... the farthest we've been thus far;  read a little
  // farther to get info on the next piece (if present)
  if (!this->CompleteFileHasBeenRead && static_cast<size_t>(pieceIndex) == this->LIDARPieces.size() - 1)
    {
     if (fin.eof())
      {
      this->CompleteFileHasBeenRead = true;
      return res;
      }

    vtkTypeInt32 nextNumPts = -1;
    LIDARPieceInfo pieceInfo;
    pieceInfo.PieceStartOffset = fin.tellg();
    if (this->FileType == VTK_ASCII)
      {
      fin.getline(buffer, 2048);
      sscanf(buffer, "%d", &nextNumPts);
      }
    else
      {
      fin.read(reinterpret_cast<char *>(&nextNumPts), sizeof(vtkTypeInt32));
      }

    if (nextNumPts < 0)
      {
      this->CompleteFileHasBeenRead = true;
      return res;
      }
    pieceInfo.PiecePointsOffset = fin.tellg();
    pieceInfo.NumPoints = nextNumPts;
    this->LastReadPieceOffset = pieceInfo.PieceStartOffset;
    this->LIDARPieces.push_back(pieceInfo);
    //this->PieceOffset.push_back(  );
    //this->PieceNumPoints.push_back( nextNumPts );
    }

  return res;
}

//-----------------------------------------------------------------------------
//  attempt to move to specified piece
int vtkLIDARReader::MoveToStartOfPiece(ifstream &fin, int pieceIndex)
{
  if (this->CompleteFileHasBeenRead &&
    pieceIndex >= static_cast<int>(this->LIDARPieces.size()) )
    {
    return READ_ERROR;
    }

  if (pieceIndex >= 0 && pieceIndex < static_cast<int>(this->LIDARPieces.size()))
    {
    fin.seekg( this->LIDARPieces[pieceIndex].PiecePointsOffset, ios::beg );
    return READ_OK;
    }

  int currentPieceIndex = -1;
  vtkTypeInt32 numPts = -1;
  // move to the beginning of the last piece we're aware of
  if (this->LIDARPieces.size() > 0)
    {
    fin.seekg( this->LIDARPieces.back().PiecePointsOffset, ios::beg );
    currentPieceIndex = this->LIDARPieces.size() - 1;
    numPts = this->LIDARPieces.back().NumPoints;
    }

  char buffer[2048];
  while (!fin.eof())
    {
    // only time this might not be true is the 1st time, if we've already read
    // part of the file
    if (numPts < 0)
      {
      LIDARPieceInfo pieceInfo;
      pieceInfo.PieceStartOffset = fin.tellg();
      if (this->FileType == VTK_ASCII)
        {
        fin.getline(buffer, 2048);
        sscanf(buffer, "%d", &numPts);
        }
      else
        {
        fin.read(reinterpret_cast<char *>(&numPts), sizeof(vtkTypeInt32));
        }

      if (numPts <= 0)
        {
        break;
        }
      currentPieceIndex++;
      pieceInfo.PiecePointsOffset = fin.tellg();
      pieceInfo.NumPoints = numPts;
      this->LIDARPieces.push_back(pieceInfo);

     // this->PieceOffset.push_back( fin.tellg() );
     // this->PieceNumPoints.push_back( numPts );
      }

    // if 1st time, read number of values per line, and then back up
    if (this->FileType == VTK_ASCII && this->ValuesPerLine == -1)
      {
      float jnk;
      double rgb[3], pt[3];
      fin.getline(buffer, 2048);
      this->ValuesPerLine =
        sscanf(buffer, "%lf %lf %lf %f %lf %lf %lf", pt, pt+1, pt+2,
        &jnk, rgb, rgb+1, rgb+2);
      this->BytesPerPoint = strlen(buffer);
      fin.seekg( this->LIDARPieces.back().PiecePointsOffset, ios::beg );
      }

    if (currentPieceIndex == pieceIndex)
      {
      return READ_OK;
      }

    char progressText[1024];
    sprintf(progressText, "Scanning File: %s, Piece: %d",
      this->FileName, currentPieceIndex);
    this->SetProgressText(progressText);
    // read to next piece of data
    if (this->FileType == VTK_ASCII)
      {
      this->UpdateProgress(0);
      for (long j = 0; j < numPts; j++)
        {
        fin.getline(buffer, 2048);
        if ((j%100)==0)
          {
          this->UpdateProgress( static_cast<double>(j) / static_cast<double>(numPts) );
          if (this->GetAbortExecute())
            {
            return READ_ABORT;
            }
          }
        }
      //this->UpdateProgress(1.0);
      }
    else
      {
      // binary data, so we can skip ahead to the next piece (read the rest
      // of this piece)
      fin.seekg(sizeof(double)*3*numPts, ios::cur);
      }

    numPts = -1;
    }

  return READ_OK;
}


//-----------------------------------------------------------------------------
void vtkLIDARReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Convert From Lat/Long to xyz: " <<
    (this->ConvertFromLatLongToXYZ ? "On" : "Off");
}


//----------------------------------------------------------------------------
int vtkLIDARReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return 1;
}
