//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkLASReader.h"

#include "vtkByteSwap.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedShortArray.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkGeoSphereTransform.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkTransform.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>

#include <sstream>

//#define LIDAR_PREVIEW_PIECE_NUM_POINTS 10000
#define LIDAR_BINARY_POINT_SIZE sizeof(double)*3

enum FileReadingStatus
  {
    READ_OK = 0,
    READ_ERROR,
    READ_ABORT
  };

vtkStandardNewMacro(vtkLASReader);


//-----------------------------------------------------------------------------
vtkLASReader::vtkLASReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->LimitReadToBounds = false;
  this->ReadBounds[0] = this->ReadBounds[2] = this->ReadBounds[4] = VTK_DOUBLE_MAX;
  this->ReadBounds[1] = this->ReadBounds[3] = this->ReadBounds[5] = VTK_DOUBLE_MIN;
  this->Origin[0] = this->Origin[1] = this->Origin[2] = 0;
  for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
    {
    this->Transform[i] = 0;
    this->PointRecordsPerClassification[i] = 0;
    }
  this->TransformOutputData = false;

  this->ScanMode = false;
  this->ConvertFromLatLongToXYZ = false;
  this->LatLongTransform1 = vtkSmartPointer<vtkGeoSphereTransform>::New();
  this->LatLongTransform2 = vtkSmartPointer<vtkTransform>::New();
  this->LatLongTransform2Initialized = false;

  this->Header.GlobalEncoding = 0; // GPS "Week" Time
  this->Header.Size = 0;
  this->Header.OffsetToPointData = 0;
  this->OutputDataTypeIsDouble = true;
}

//-----------------------------------------------------------------------------
vtkLASReader::~vtkLASReader()
{
  this->SetFileName(0);
  for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
    {
    this->SetTransform(i, static_cast<vtkTransform*>(0));
    }
}

//-----------------------------------------------------------------------------
void vtkLASReader::SetConvertFromLatLongToXYZ(bool mode)
{
  if (this->ConvertFromLatLongToXYZ == mode)
    {
    return;
    }

  this->ConvertFromLatLongToXYZ = mode;
  this->LatLongTransform2Initialized = false;

  this->Modified();
}

//-----------------------------------------------------------------------------
// vtkSetStringMacro except we clear some variables if we update the value
void vtkLASReader::SetFileName(const char *filename)
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

  // only want to clear these values if FileName changes!
  this->DataBounds[0] = this->DataBounds[2] = this->DataBounds[4] = VTK_DOUBLE_MAX;
  this->DataBounds[1] = this->DataBounds[3] = this->DataBounds[5] = VTK_DOUBLE_MIN;
  this->RequestedReadClassifications.clear();
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkLASReader::AddRequestedClassificationForRead(int classification,
                                                     int onRatio)
{
  if(classification >= 0 && classification < 128 && onRatio > 0)
    {
    unsigned char classificationId = static_cast<unsigned char>(classification);
    this->RequestedReadClassifications[classificationId] = onRatio;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkLASReader::RemoveAllRequestedReadClassifications()
{
  this->RequestedReadClassifications.clear();
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkLASReader::SetTransform(int index, double elements[16])
{
  if (index >= 0 && index < NUMBER_OF_CLASSIFICATIONS)
    {
    vtkTransform *tmpTransform = vtkTransform::New();
    tmpTransform->SetMatrix(elements);
    this->SetTransform(index, tmpTransform);
    tmpTransform->Delete();
    }
}

//-----------------------------------------------------------------------------
void vtkLASReader::SetTransform(double elements[17])
{
  this->SetTransform(static_cast<int>(elements[0]), elements + 1);
}

//-----------------------------------------------------------------------------
void vtkLASReader::SetTransform(int index, vtkTransform *transform)
{
  if (index >= 0 && index < NUMBER_OF_CLASSIFICATIONS &&
    this->Transform[index] != transform)
    {
    vtkTransform *tempTransform = this->Transform[index];
    this->Transform[index] = transform;
    if (this->Transform[index] != NULL)
      {
      this->Transform[index]->Register(this);
      }
    if (tempTransform != NULL)
      {
      tempTransform->UnRegister(this);
      }
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vtkTransform *vtkLASReader::GetTransform(int index)
{
  return (index >= 0 && index < NUMBER_OF_CLASSIFICATIONS) ?
    this->Transform[index] : 0;
}

//-----------------------------------------------------------------------------
int vtkLASReader::ReadHeaderBlock()
{
  ifstream fin(this->FileName, ios::binary);
  if(!fin)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return READ_ERROR;
    }

  fin.read(this->Header.FileSignature, 4);
  this->Header.FileSignature[4] = 0;
  if (strcmp(this->Header.FileSignature, "LASF"))
    {
    vtkErrorMacro("File " << this->FileName << " doesn't appear to be LAS file!");
    return READ_ERROR;
    }

  fin.read(reinterpret_cast<char *>(&this->Header.FileSourceId), 2);
  fin.read(reinterpret_cast<char *>(&this->Header.GlobalEncoding), 2);
  vtkByteSwap::Swap2LE(&this->Header.GlobalEncoding);


  fin.read(reinterpret_cast<char *>(&this->Header.ProjectIDGUID1), 4);
  fin.read(reinterpret_cast<char *>(&this->Header.ProjectIDGUID2), 2);
  fin.read(reinterpret_cast<char *>(&this->Header.ProjectIDGUID3), 2);
  fin.read(reinterpret_cast<char *>(this->Header.ProjectIDGUID4), 8);

  fin.read(reinterpret_cast<char *>(&this->Header.VersionMajor), 1);
  fin.read(reinterpret_cast<char *>(&this->Header.VersionMinor), 1);
  if (this->Header.VersionMajor != 1 || (this->Header.VersionMinor != 1 &&
                                         this->Header.VersionMinor != 2))
    {
    vtkErrorMacro("Invalid version; must be 1.1 or 1.2!");
    return READ_ERROR;
    }

  fin.read(this->Header.SystemIdentifier, 32);
  this->Header.SystemIdentifier[32] = 0;

  fin.read(this->Header.GeneratingSoftware, 32);
  this->Header.GeneratingSoftware[32] = 0;

  fin.read(reinterpret_cast<char *>(&this->Header.CreationDay), 2);
  fin.read(reinterpret_cast<char *>(&this->Header.CreationYear), 2);

  fin.read(reinterpret_cast<char *>(&this->Header.Size), 2);
  vtkByteSwap::Swap2LE(&this->Header.Size);

  fin.read(reinterpret_cast<char *>(&this->Header.OffsetToPointData), 4);
  vtkByteSwap::Swap4LE(&this->Header.OffsetToPointData);

  fin.read(reinterpret_cast<char *>(&this->Header.NumberOfVariableLengthRecords), 4);
  vtkByteSwap::Swap4LE(&this->Header.NumberOfVariableLengthRecords);

  fin.read(reinterpret_cast<char *>(&this->Header.PointDataFormat), 1);

  fin.read(reinterpret_cast<char *>(&this->Header.PointDataRecordLength), 2);
  vtkByteSwap::Swap2LE(&this->Header.PointDataRecordLength);

  if ((this->Header.PointDataFormat == 0 && this->Header.PointDataRecordLength < 20) ||
      (this->Header.PointDataFormat == 1 && this->Header.PointDataRecordLength < 28) ||
      (this->Header.PointDataFormat == 2 && this->Header.PointDataRecordLength < 26) ||
      (this->Header.PointDataFormat == 3 && this->Header.PointDataRecordLength < 34) ||
      this->Header.PointDataFormat > 4)
    {
    vtkErrorMacro("Invalid data format (" << static_cast<int>(this->Header.PointDataFormat)
      << ") / record lengh (" << this->Header.PointDataRecordLength << ") combo!");
    return READ_ERROR;
    }

  fin.read(reinterpret_cast<char *>(&this->Header.NumberOfPointRecords), 4);
  vtkByteSwap::Swap4LE(&this->Header.NumberOfPointRecords);
  for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
    {
    this->PointRecordsPerClassification[i] = this->Header.NumberOfPointRecords;
    }

  fin.read(reinterpret_cast<char *>(this->Header.NumberOfPointsByReturn), 4 * 5);
  vtkByteSwap::Swap4LERange(this->Header.NumberOfPointsByReturn, 5);

  fin.read(reinterpret_cast<char *>(this->Header.ScaleFactor), 8 * 3);
  fin.read(reinterpret_cast<char *>(this->Header.Offset), 8 * 3);
  // read bounds into memory as VTK wants it!
  fin.read(reinterpret_cast<char *>(this->DataBounds + 1), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 0), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 3), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 2), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 5), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 4), 8);

  this->DataBounds[0] -= Origin[0];
  this->DataBounds[1] -= Origin[0];
  this->DataBounds[2] -= Origin[1];
  this->DataBounds[3] -= Origin[1];
  this->DataBounds[4] -= Origin[2];
  this->DataBounds[5] -= Origin[2];


  fin.close();

  return READ_OK;
}

std::string vtkLASReader::GetHeaderInfo()
{
  if(this->ReadHeaderBlock() == READ_ERROR)
  {
    return "";
  }
  std::stringstream ss;
  ss << "\tNumber of Points:\t" << this->Header.NumberOfPointRecords << std::endl
     << "\tBounds\t\t\t\tX:\t" << this->DataBounds[0] << "\t" << this->DataBounds[1] << std::endl
     << "\t\t\t\t\t\t\tY:\t" << this->DataBounds[2] << "\t" << this->DataBounds[3] << std::endl
     << "\t\t\t\t\t\t\tZ:\t" << this->DataBounds[4] << "\t" << this->DataBounds[5] << std::endl;
  return ss.str();
}

//-----------------------------------------------------------------------------
int vtkLASReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the ouptut
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(this->ReadHeaderBlock() == READ_ERROR)
    {
    return 0;
    }

  // setup the ReadBBox, If we're limiting the read to specifed ReadBounds
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

  this->ReadPoints(output);

  return 1;

}


//-----------------------------------------------------------------------------
int vtkLASReader::ReadPoints(vtkMultiBlockDataSet *output)
{

  struct LASPieceInfo
    {
    LASPieceInfo()
      {
      this->PointsInClassification = 0;
      this->PolyData = 0;
      this->SkipCount = 0;
      this->ReadRatio = 1;
      }
    vtkPolyData *PolyData;
    vtkIdType PointsInClassification;
    int SkipCount;
    int ReadRatio;
    };

  // up to 32 possible classfications (not counting "special" bits); although
  // only 0-8 really used, allocated array for all 32
  LASPieceInfo pieceInfo[NUMBER_OF_CLASSIFICATIONS];
  // initialize with readRatios
  if (this->RequestedReadClassifications.size() == 0)
    {
    for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
      {
      pieceInfo[i].ReadRatio = 1;
      }
    }
  else
    {
    std::map<unsigned char, int>::iterator pieceReadRatio;
    for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
      {
      pieceReadRatio = this->RequestedReadClassifications.find(i);
      if (pieceReadRatio == this->RequestedReadClassifications.end())
        {
        pieceInfo[i].ReadRatio = 0;  // don't read, not present in request list
        }
      else
        {
        pieceInfo[i].ReadRatio = pieceReadRatio->second;
        }
      }
    }


  this->UpdateProgress(0);

  // We already succesfully read the header, so know file exists
  ifstream fin(this->FileName, ios::binary);
  fin.seekg( this->Header.OffsetToPointData, ios::beg );

  vtkTypeInt32 *ptRaw;
  double pt[3];
  char pointDataRecord[34];
  unsigned char classification, classificationField;
  unsigned char classificationMask = 0x1F;
//  char scanAngle;
  vtkTypeUInt32 progressInterval = this->Header.NumberOfPointRecords / 1000;
  // If the progressInterval is 0 then lets set it to a sane value
  if (!progressInterval)
    {
    progressInterval = 1;
    }

  vtkCellArray *verts = NULL;
  vtkPoints *pts = NULL;
  vtkUnsignedCharArray *colorArray = NULL;
  vtkUnsignedShortArray *intensityArray = NULL;
  vtkIdType idx;
  for (vtkTypeUInt32 ptIndex = 0; ptIndex < this->Header.NumberOfPointRecords; ptIndex++)
    {
    if (ptIndex % progressInterval == 0)
      {
      this->UpdateProgress( static_cast<double>(ptIndex) /
                           static_cast<double>(this->Header.NumberOfPointRecords) );
      if (this->GetAbortExecute())
        {
        fin.close();
        for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
          {
          if (pieceInfo[i].PolyData != 0)
            {
            pieceInfo[i].PolyData->Delete();
            }
          }
        return READ_ABORT;
        }
      }

    fin.read(pointDataRecord, this->Header.PointDataRecordLength);
    classificationField = *reinterpret_cast<unsigned char *>(pointDataRecord + 15);
    if (classificationField > 127)
      {
      continue; // withheld
      }
    classification =  classificationField & classificationMask;
//    scanAngle = pointDataRecord[16];

    if (pieceInfo[classification].ReadRatio == 0)
      {
      continue;
      }

    if (!pieceInfo[classification].PolyData)
      {
      vtkPolyData *pD = vtkPolyData::New();
      if (!this->ScanMode)
        {
        pts = vtkPoints::New();
        if (this->OutputDataTypeIsDouble)
          {
          pts->SetDataTypeToDouble();
          }
        else
          {
          pts->SetDataTypeToFloat();
          }

        verts = vtkCellArray::New();
        pD->SetPoints( pts );
        pD->SetVerts( verts );
        pts->UnRegister(this);
        verts->UnRegister(this);

        unsigned long numberOfValuesEstimate = 2 +
          this->PointRecordsPerClassification[classification] /
          pieceInfo[classification].ReadRatio;

        pts->Allocate( numberOfValuesEstimate );
        verts->Allocate( numberOfValuesEstimate );

        // scalars for color
        colorArray = vtkUnsignedCharArray::New();
        colorArray->SetNumberOfComponents(3);
        colorArray->Allocate( numberOfValuesEstimate * 3 );
        colorArray->SetName("Color");
        pD->GetPointData()->SetScalars( colorArray );
        colorArray->UnRegister(this);

        // array for intensity
        intensityArray = vtkUnsignedShortArray::New();
        intensityArray->SetName("Intensity");
        intensityArray->SetNumberOfComponents(1);
        intensityArray->Allocate( numberOfValuesEstimate );
        pD->GetPointData()->AddArray( intensityArray );
        intensityArray->UnRegister(this);
        }
      this->AddClassificationFieldData(classification, pD);
      pieceInfo[classification].PolyData = pD;
      }
    else
      {
      pts = pieceInfo[classification].PolyData->GetPoints();
      verts = pieceInfo[classification].PolyData->GetVerts();
      intensityArray = vtkUnsignedShortArray::SafeDownCast(
        pieceInfo[classification].PolyData->GetPointData()->GetArray("Intensity"));
      colorArray = vtkUnsignedCharArray::SafeDownCast(
        pieceInfo[classification].PolyData->GetPointData()->GetArray("Color"));
      }

    pieceInfo[classification].PointsInClassification++;
    if (this->ScanMode)
      {
      continue;
      }
    if (classificationField < 64 &&
      pieceInfo[classification].SkipCount % pieceInfo[classification].ReadRatio)
      {
      pieceInfo[classification].SkipCount++;
      continue;
      }

    // set skip count such that we'll read next point based on ReadRatio
    pieceInfo[classification].SkipCount = 1;

    // THE point
    ptRaw = reinterpret_cast<vtkTypeInt32 *>(pointDataRecord);
    vtkByteSwap::Swap4LERange(ptRaw, 3);
    pt[0] = ptRaw[0] * this->Header.ScaleFactor[0] + this->Header.Offset[0];
    pt[1] = ptRaw[1] * this->Header.ScaleFactor[1] + this->Header.Offset[1];
    pt[2] = ptRaw[2] * this->Header.ScaleFactor[2] + this->Header.Offset[2];

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

    pt[0] -= Origin[0];
    pt[1] -= Origin[1];
    pt[2] -= Origin[2];

    // add the point, but 1st make sure it is in the ReadBounds (if specified);
    // consider the Transform if set (and "on")
    double transformedPt[3];
    if (this->Transform[classification])
      {
      // only need the transformed pt if we're limiting read based on bounds or
      // we're transforming the output
      if (this->LimitReadToBounds || this->TransformOutputData)
        {
        this->Transform[classification]->TransformPoint(pt, transformedPt);
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

    if (this->Transform[classification] && this->TransformOutputData)
      {
      idx = pts->InsertNextPoint(transformedPt);
      }
    else
      {
      idx = pts->InsertNextPoint(pt);
      }

    verts->InsertNextCell(1, &idx);

    // intensity
    vtkTypeUInt16 intensity = *reinterpret_cast<vtkTypeUInt16 *>(pointDataRecord + 12);
    vtkByteSwap::Swap2LE(&intensity);
    intensityArray->InsertNextValue( intensity );

    // color
    unsigned char bytergb[3] = {0, 0, 0};
    if (this->Header.PointDataFormat == 2 || this->Header.PointDataFormat == 3)
      {
      vtkTypeUInt16 *rgb;
      if (this->Header.PointDataFormat == 2)
        {
        rgb = reinterpret_cast<vtkTypeUInt16 *>(pointDataRecord + 20);
        }
      else
        {
        rgb = reinterpret_cast<vtkTypeUInt16 *>(pointDataRecord + 28);
        }
      vtkByteSwap::Swap2LERange(rgb, 3);
      // rgb values are supposed to be normalized to 16 bits, but not always done
      bytergb[0] = (static_cast<double>(rgb[0]) / 65535.0) * 255.0 + 0.5;
      bytergb[1] = (static_cast<double>(rgb[1]) / 65535.0) * 255.0 + 0.5;
      bytergb[2] = (static_cast<double>(rgb[2]) / 65535.0) * 255.0 + 0.5;
      }
    else // color based on classifcation
      {
      switch(classification)
        {
        case 0: // Created, never classified
          {
          bytergb[0] = bytergb[1] = bytergb[2] = 160;
          break;
          }
        case 1: // Unclassified
          {
          bytergb[0] = bytergb[1] = bytergb[2] = 215;
          break;
          }
        case 2: // Ground
          {
          bytergb[0] = 138;
          bytergb[1] = 69;
          bytergb[2] = 19;
          break;
          }
        case 3: // Low vegetation
          {
          bytergb[0] = 124;
          bytergb[1] = 252;
          bytergb[2] = 0;
          break;
          }
        case 4: // Medium vegetation
          {
          bytergb[0] = 0;
          bytergb[1] = 250;
          bytergb[2] = 154;
          break;
          }
        case 5: // High vegetation
          {
          bytergb[0] = 34;
          bytergb[1] = 139;
          bytergb[2] = 34;
          break;
          }
        case 6: // Building
          {
          bytergb[0] = 175;
          bytergb[1] = 196;
          bytergb[2] = 222;
          break;
          }
        case 7: // Low Point (noise)
          {
          bytergb[0] = bytergb[1] = bytergb[2] = 123;
          break;
          }
        case 8: // Model Key-point (mass-point)
          {
          bytergb[0] = 255;
          break;
          }
        case 9: // Water
          {
          bytergb[2] = 255;
          break;
          }
        case 12: // Overlap Points
          {
          bytergb[0] = bytergb[1] = bytergb[2] = 78;
          break;
          }
        default: // Reserved
          {
          bytergb[0] = bytergb[1] = bytergb[2] = 60;
          break;
          }
        }
      }
    colorArray->InsertNextTypedTuple(bytergb);
    }

  // iterate through any sets we created, adding them to the output
  for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
    {
    this->PointRecordsPerClassification[i] = pieceInfo[i].PointsInClassification;

    if (pieceInfo[i].PolyData != 0)
      {
      vtkIdTypeArray *pointsInClassification = vtkIdTypeArray::New();
      pointsInClassification->SetName( "NumberOfPointsInClassification" );
      pointsInClassification->InsertNextValue(pieceInfo[i].PointsInClassification);
      pieceInfo[i].PolyData->GetFieldData()->AddArray(pointsInClassification);
      pointsInClassification->Delete();

      output->SetBlock(output->GetNumberOfBlocks(), pieceInfo[i].PolyData);
      pieceInfo[i].PolyData->Delete();
      }
    }

  return READ_OK;
}

//-----------------------------------------------------------------------------
void vtkLASReader::AddClassificationFieldData(unsigned char classification,
                                              vtkPolyData *pD)
{
  // add some field data regarding the classification of this block
  vtkUnsignedCharArray *classificationFD = vtkUnsignedCharArray::New();
  classificationFD->SetName( "Classification" );
  classificationFD->InsertNextValue(classification);
  pD->GetFieldData()->AddArray(classificationFD);
  classificationFD->Delete();

  vtkStringArray *classificationNameFD = vtkStringArray::New();
  classificationNameFD->SetName("ClassificationName");
  switch(classification)
    {
    case 0:
      {
      classificationNameFD->InsertNextValue("Created, never classfied");
      break;
      }
    case 1:
      {
      classificationNameFD->InsertNextValue("Unclassified");
      break;
      }
    case 2:
      {
      classificationNameFD->InsertNextValue("Ground");
      break;
      }
    case 3:
      {
      classificationNameFD->InsertNextValue("Low Vegetation");
      break;
      }
    case 4:
      {
      classificationNameFD->InsertNextValue("Medium Vegetation");
      break;
      }
    case 5:
      {
      classificationNameFD->InsertNextValue("High Vegetation");
      break;
      }
    case 6:
      {
      classificationNameFD->InsertNextValue("Building");
      break;
      }
    case 7:
      {
      classificationNameFD->InsertNextValue("Low Point (noise)");
      break;
      }
    case 8:
      {
      classificationNameFD->InsertNextValue("Model Key-point (mass-point)");
      break;
      }
    case 9:
      {
      classificationNameFD->InsertNextValue("Water");
      break;
      }
    case 12:
      {
      classificationNameFD->InsertNextValue("Overlap Points");
      break;
      }
    default:
      {
      classificationNameFD->InsertNextValue("Reserved");
      }
    }
  pD->GetFieldData()->AddArray(classificationNameFD);
  classificationNameFD->Delete();
}

//-----------------------------------------------------------------------------
void vtkLASReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Convert From Lat/Long to xyz: " <<
    (this->ConvertFromLatLongToXYZ ? "On" : "Off");
}


//----------------------------------------------------------------------------
int vtkLASReader::RequestInformation(
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
