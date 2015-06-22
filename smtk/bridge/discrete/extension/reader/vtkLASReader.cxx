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

namespace smtk {
  namespace bridge {
    namespace discrete {

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

  this->GlobalEncoding = 0; // GPS "Week" Time
  this->HeaderSize = 0;
  this->OffsetToPointData = 0;
  this->OutputDataTypeIsDouble = false;
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

  char fileSignature[5];
  fin.read(fileSignature, 4);
  fileSignature[4] = 0;
  if (strcmp(fileSignature, "LASF"))
    {
    vtkErrorMacro("File " << this->FileName << " doesn't appear to be LAS file!");
    return READ_ERROR;
    }

  unsigned short fileSourceId;
  fin.read(reinterpret_cast<char *>(&fileSourceId), 2);
  fin.read(reinterpret_cast<char *>(&this->GlobalEncoding), 2);
  vtkByteSwap::Swap2LE(&this->GlobalEncoding);


  vtkTypeUInt32 projectIDGUID1;
  vtkTypeUInt16 projectIDGUID2, projectIDGUID3;
  unsigned char projectIDGUID4[8];
  fin.read(reinterpret_cast<char *>(&projectIDGUID1), 4);
  fin.read(reinterpret_cast<char *>(&projectIDGUID2), 2);
  fin.read(reinterpret_cast<char *>(&projectIDGUID3), 2);
  fin.read(reinterpret_cast<char *>(projectIDGUID4), 8);

  unsigned char versionMajor, versionMinor;
  fin.read(reinterpret_cast<char *>(&versionMajor), 1);
  fin.read(reinterpret_cast<char *>(&versionMinor), 1);
  if (versionMajor != 1 || (versionMinor != 1 && versionMinor != 2))
    {
    vtkErrorMacro("Invalid version; must be 1.1 or 1.2!");
    return READ_ERROR;
    }

  char systemIdentifier[33];
  fin.read(systemIdentifier, 32);
  systemIdentifier[32] = 0;

  char generatingSoftware[33];
  fin.read(generatingSoftware, 32);
  generatingSoftware[32] = 0;

  vtkTypeUInt16 creationDay, creationYear;
  fin.read(reinterpret_cast<char *>(&creationDay), 2);
  fin.read(reinterpret_cast<char *>(&creationYear), 2);

  fin.read(reinterpret_cast<char *>(&this->HeaderSize), 2);
  vtkByteSwap::Swap2LE(&this->HeaderSize);

  fin.read(reinterpret_cast<char *>(&this->OffsetToPointData), 4);
  vtkByteSwap::Swap4LE(&this->OffsetToPointData);

  fin.read(reinterpret_cast<char *>(&this->NumberOfVariableLengthRecords), 4);
  vtkByteSwap::Swap4LE(&this->NumberOfVariableLengthRecords);

  fin.read(reinterpret_cast<char *>(&this->PointDataFormat), 1);

  fin.read(reinterpret_cast<char *>(&this->PointDataRecordLength), 2);
  vtkByteSwap::Swap2LE(&this->PointDataRecordLength);

  if ((this->PointDataFormat == 0 && this->PointDataRecordLength != 20) ||
    (this->PointDataFormat == 1 && this->PointDataRecordLength != 28) ||
    (this->PointDataFormat == 2 && this->PointDataRecordLength != 26) ||
    (this->PointDataFormat == 3 && this->PointDataRecordLength != 34) ||
    this->PointDataFormat > 4)
    {
    vtkErrorMacro("Invalid data format (" << this->PointDataFormat
      << ") / record lengh (" << this->PointDataRecordLength << ") combo!");
    return READ_ERROR;
    }

  fin.read(reinterpret_cast<char *>(&this->NumberOfPointRecords), 4);
  vtkByteSwap::Swap4LE(&this->NumberOfPointRecords);
  for (int i = 0; i < NUMBER_OF_CLASSIFICATIONS; i++)
    {
    this->PointRecordsPerClassification[i] = this->NumberOfPointRecords;
    }

  vtkTypeUInt32 numberOfPointsByReturn[5];
  fin.read(reinterpret_cast<char *>(numberOfPointsByReturn), 4 * 5);
  vtkByteSwap::Swap4LERange(numberOfPointsByReturn, 5);

  fin.read(reinterpret_cast<char *>(this->ScaleFactor), 8 * 3);
  fin.read(reinterpret_cast<char *>(this->Offset), 8 * 3);
  // read bounds into memory as VTK wants it!
  fin.read(reinterpret_cast<char *>(this->DataBounds + 1), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 0), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 3), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 2), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 5), 8);
  fin.read(reinterpret_cast<char *>(this->DataBounds + 4), 8);

  fin.close();

  return READ_OK;
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
  fin.seekg( this->OffsetToPointData, ios::beg );

  vtkTypeInt32 *ptRaw;
  double pt[3];
  char pointDataRecord[34];
  unsigned char classification, classificationField;
  unsigned char classificationMask = 0x1F;
//  char scanAngle;
  vtkTypeUInt32 progressInterval = this->NumberOfPointRecords / 1000;
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
  for (vtkTypeUInt32 ptIndex = 0; ptIndex < this->NumberOfPointRecords; ptIndex++)
    {
    if (ptIndex % progressInterval == 0)
      {
      this->UpdateProgress( static_cast<double>(ptIndex) / static_cast<double>(this->NumberOfPointRecords) );
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

    fin.read(pointDataRecord, this->PointDataRecordLength);
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
    pt[0] = ptRaw[0] * this->ScaleFactor[0] + this->Offset[0];
    pt[1] = ptRaw[1] * this->ScaleFactor[1] + this->Offset[1];
    pt[2] = ptRaw[2] * this->ScaleFactor[2] + this->Offset[2];

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
    if (this->PointDataFormat == 2 || this->PointDataFormat == 3)
      {
      vtkTypeUInt16 *rgb;
      if (this->PointDataFormat == 2)
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
    colorArray->InsertNextTupleValue(bytergb);
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

    } // namespace discrete
  } // namespace bridge
} // namespace smtk
