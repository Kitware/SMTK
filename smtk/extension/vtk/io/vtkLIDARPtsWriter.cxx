//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkLIDARPtsWriter.h"

#include "vtkAppendPolyData.h"
#include "vtkDataArray.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnsignedCharArray.h"

#define LIDAR_ASCII_SEPERATOR " "

enum FileWritingStatus
{
  WRITE_OK = 0,
  WRITE_ERROR,
  WRITE_ABORT
};

vtkStandardNewMacro(vtkLIDARPtsWriter);

vtkLIDARPtsWriter::vtkLIDARPtsWriter()
{
  this->FileName = nullptr;
  this->OutputIsBinary = 0;
  this->WriteAsSinglePiece = false;
}

vtkLIDARPtsWriter::~vtkLIDARPtsWriter()
{
  this->SetFileName(nullptr);
}

void vtkLIDARPtsWriter::WriteData()
{
  ofstream* outfile = this->OpenOutputFile();

  if (!outfile)
  {
    vtkErrorMacro("Can not open output file for writing!");
  }
  else
  {
    if (this->WriteFile(*outfile) == WRITE_ERROR)
    {
      vtkErrorMacro("Write failed");
    }
  }

  this->CloseFile(outfile);
}

int vtkLIDARPtsWriter::WriteFile(ofstream& ofp)
{
  int numInputs = this->GetNumberOfInputConnections(0);
  int returnValue = WRITE_OK;

  if (numInputs > 1 && this->WriteAsSinglePiece)
  {
    // 1st append all pieces, then write
    vtkNew<vtkAppendPolyData> append;
    for (int idx = 0; idx < numInputs; ++idx)
    {
      vtkPolyData* inputPoly = vtkPolyData::SafeDownCast(this->GetInputFromPort0(idx));
      if (!inputPoly)
      {
        continue;
      }
      append->AddInputData(inputPoly);
    }
    append->Update();

    returnValue = this->WritePoints(ofp, append->GetOutput());
  }
  else
  {
    for (int idx = 0; idx < numInputs; ++idx)
    {
      vtkPolyData* inputPoly = vtkPolyData::SafeDownCast(this->GetInputFromPort0(idx));
      if (!inputPoly)
      {
        continue;
      }

      if (this->WritePoints(ofp, inputPoly) == WRITE_ABORT)
      {
        return WRITE_ABORT;
      }
    }
  }

  return returnValue;
}

int vtkLIDARPtsWriter::ComputeRequiredAxisPrecision(double min, double max)
{
  double maxComponent = fabs(min) > max ? fabs(min) : max;
  double logRange = max - min != 0 ? log10(max - min) : 0;
  double logMaxComponent = log10(maxComponent);
  int minPrecision = 7;
  if (logMaxComponent - logRange > 4)
  {
    minPrecision = static_cast<int>(ceil(logMaxComponent + (4 - logRange)));
    if (minPrecision < 7)
    {
      minPrecision = 7;
    }
  }
  // a bit of a balancing act with other axes (may end up with a larger file
  // than if we just left as already calculated), but make some effort to
  // avoid needlessly writing in scienticfic format (1.234568e+008), since
  // (for this axis) less digits if write extra precision
  // (1.234568e+008 is more digits than 123456789, 9 versus 13).  Also,
  // we don't push this as far as we "could", because it doesn mean we're
  // putting out more digits on the other axes
  if (logMaxComponent > minPrecision && logMaxComponent < minPrecision + 3)
  {
    minPrecision = ceil(logMaxComponent);
  }

  return minPrecision;
}

int vtkLIDARPtsWriter::WritePoints(ofstream& ofp, vtkPolyData* inputPoly)
{
  vtkDataArray* scalars =
    inputPoly->GetPointData() ? inputPoly->GetPointData()->GetScalars("Color") : nullptr;

  vtkUnsignedCharArray* rgbScalars =
    scalars ? vtkUnsignedCharArray::SafeDownCast(scalars) : nullptr;

  vtkFloatArray* intensityArray = inputPoly->GetPointData()
    ? vtkFloatArray::SafeDownCast(inputPoly->GetPointData()->GetArray("Intensity"))
    : nullptr;

  vtkPoints* points = inputPoly->GetPoints();
  if (!points)
  {
    return WRITE_OK; // no points to write for this input
  }
  vtkTypeInt32 numPts = points->GetNumberOfPoints();
  if (numPts <= 0)
    return WRITE_OK;

  if (this->OutputIsBinary)
  {
    ofp.write(reinterpret_cast<char*>(&numPts), 4);
  }
  else
  {
    // determine the precision we need to write... want 4+ digits on each axis
    double* bounds = inputPoly->GetBounds();
    int xPrecision = this->ComputeRequiredAxisPrecision(bounds[0], bounds[1]);
    int yPrecision = this->ComputeRequiredAxisPrecision(bounds[2], bounds[3]);
    int zPrecision = this->ComputeRequiredAxisPrecision(bounds[4], bounds[5]);
    int requiredPrecision = xPrecision > yPrecision ? xPrecision : yPrecision;
    requiredPrecision = zPrecision > requiredPrecision ? zPrecision : requiredPrecision;
    ofp.precision(requiredPrecision);
    ofp << numPts << endl;
  }

  double pts[3], rgb[3];
  vtkIdType numColorTuples = rgbScalars ? rgbScalars->GetNumberOfTuples() : 0;
  for (vtkIdType cc = 0; cc < numPts; cc++)
  {
    points->GetPoint(cc, pts);
    if (this->OutputIsBinary)
    {
      ofp.write((char*)pts, sizeof(double) * 3);
    }
    else
    {
      ofp << pts[0] << LIDAR_ASCII_SEPERATOR << pts[1] << LIDAR_ASCII_SEPERATOR << pts[2];
      if (intensityArray)
      {
        ofp << LIDAR_ASCII_SEPERATOR << intensityArray->GetValue(cc);
      }
      else
      {
        ofp << LIDAR_ASCII_SEPERATOR << "1";
      }
      if (rgbScalars && cc < numColorTuples)
      {
        rgbScalars->GetTuple(cc, rgb);
        ofp << LIDAR_ASCII_SEPERATOR << rgb[0] << LIDAR_ASCII_SEPERATOR << rgb[1]
            << LIDAR_ASCII_SEPERATOR << rgb[2];
      }

      if ((cc % 100) == 0)
      {
        this->UpdateProgress(static_cast<double>(cc) / static_cast<double>(numPts));
        if (this->GetAbortExecute())
        {
          return WRITE_ABORT;
        }
      }

      ofp << endl;
    }
  }
  return WRITE_OK;
}

ofstream* vtkLIDARPtsWriter::OpenOutputFile()
{
  if (!this->FileName || !this->FileName[0])
  {
    vtkErrorMacro("Output FileName has to be specified.");
    return nullptr;
  }

  ofstream* fptr = nullptr;
  if (this->IsBinaryType(this->FileName))
  {
    fptr = new ofstream(this->FileName, ios::out | ios::binary);
    this->OutputIsBinary = 1;
  }
  else
  {
    fptr = new ofstream(this->FileName, ios::out);
    this->OutputIsBinary = 0;
  }

  if (fptr->fail())
  {
    vtkErrorMacro(<< "Unable to open file: " << this->FileName);
    delete fptr;
    return nullptr;
  }
  return fptr;
}

void vtkLIDARPtsWriter::CloseFile(ios* fp)
{
  delete fp;
}

bool vtkLIDARPtsWriter::IsBinaryType(const char* filename)
{
  std::string fileNameStr = filename;
  if (fileNameStr.find("bin.pts") != std::string::npos ||
    fileNameStr.find(".bin") != std::string::npos)
  {
    return true;
  }
  return false;
}

void vtkLIDARPtsWriter::AddInputData(int index, vtkDataObject* input)
{
  if (input)
  {
    this->AddInputDataInternal(index, input);
  }
}

int vtkLIDARPtsWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return 1;
}

void vtkLIDARPtsWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Write As Single Piece: " << (this->WriteAsSinglePiece ? "On" : "Off") << "\n";
}

vtkDataObject* vtkLIDARPtsWriter::GetInputFromPort0(int connection)
{
  return this->GetExecutive()->GetInputData(0, connection);
}
