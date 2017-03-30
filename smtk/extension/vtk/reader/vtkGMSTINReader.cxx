//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkGMSTINReader.h"

#include "vtkBitArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"
#include <string>
#include <sys/stat.h>

vtkStandardNewMacro(vtkGMSTINReader);

struct vtkGMSTINReaderInternals
{
  ifstream* Stream;

  vtkGMSTINReaderInternals()
    : Stream(0)
  {
  }
  ~vtkGMSTINReaderInternals() { this->DeleteStream(); }
  void DeleteStream()
  {
    delete this->Stream;
    this->Stream = 0;
  }
};

vtkGMSTINReader::vtkGMSTINReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->Internals = new vtkGMSTINReaderInternals;
}

vtkGMSTINReader::~vtkGMSTINReader()
{
  delete[] this->FileName;
  delete this->Internals;
}

int vtkGMSTINReader::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  if (!this->FileName || (strlen(this->FileName) == 0))
  {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
  }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkMultiBlockDataSet* output =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  struct stat fs;
  if (stat(this->FileName, &fs) != 0)
  {
    vtkErrorMacro(<< "Unable to open file: " << this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
  }

  this->Internals->Stream = new ifstream(this->FileName, ios::in);
  if (this->Internals->Stream->fail())
  {
    vtkErrorMacro(<< "Unable to open file: " << this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    this->Internals->DeleteStream();
    return 0;
  }

  std::string header;
  *this->Internals->Stream >> header;
  if (header != "TIN")
  {
    vtkErrorMacro("Failed reader TIN card in the header.");
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    this->Internals->DeleteStream();
    return 0;
  }

  unsigned int block = 0;
  while (!this->Internals->Stream->eof())
  {
    if (!this->ReadTIN(block++, output))
    {
      this->Internals->DeleteStream();
      output->Initialize();
      return 0;
    }
  }

  return 1;
}

int vtkGMSTINReader::ReadTIN(unsigned int block, vtkMultiBlockDataSet* output)
{
  std::string cardname;

  *this->Internals->Stream >> cardname;
  if (this->Internals->Stream->eof())
  {
    return 1;
  }
  if (cardname != "BEGT")
  {
    vtkErrorMacro("Expected BEGT, found " << cardname << " instead");
    return 0;
  }

  vtkPolyData* tin = vtkPolyData::New();
  output->SetBlock(block, tin);
  tin->Delete();

  while (!this->Internals->Stream->eof())
  {
    *this->Internals->Stream >> cardname;
    if (cardname == "ACTIVETIN")
    {
      vtkBitArray* activeTin = vtkBitArray::New();
      activeTin->SetName("ActiveTIN");
      activeTin->InsertNextValue(1);
      output->GetFieldData()->AddArray(activeTin);
      activeTin->Delete();
    }
    else if (cardname == "HIDDEN")
    {
      vtkBitArray* hidden = vtkBitArray::New();
      hidden->SetName("Hidden");
      hidden->InsertNextValue(1);
      output->GetFieldData()->AddArray(hidden);
      hidden->Delete();
    }
    else if (cardname == "ID")
    {
      int id;
      *this->Internals->Stream >> id;
      vtkIntArray* idArray = vtkIntArray::New();
      idArray->SetName("ID");
      idArray->InsertNextValue(id);
      output->GetFieldData()->AddArray(idArray);
      idArray->Delete();
    }
    else if (cardname == "MAT")
    {
      int id;
      *this->Internals->Stream >> id;

      vtkIntArray* materialArray = vtkIntArray::New();
      materialArray->SetName("Material");
      materialArray->InsertNextValue(id);
      output->GetFieldData()->AddArray(materialArray);
      materialArray->Delete();
    }
    else if (cardname == "TNAM")
    {
      char buffer[1024];
      std::string tinname;
      this->Internals->Stream->getline(buffer, 1024);

      // Spaces are allowed only is the name is quoted. Otherwise, no.
      bool spaceIsOK = false;
      bool done = false;
      for (int i = 0; i < 1024 && buffer[i] && !done; i++)
      {
        switch (buffer[i])
        {
          case '"':
            if (spaceIsOK)
            {
              done = true;
            }
            else
            {
              spaceIsOK = true;
            }
            break;
          case ' ':
            if (spaceIsOK)
            {
              tinname.push_back(buffer[i]);
            }
            break;
          default:
            tinname.push_back(buffer[i]);
        }
      }

      if (!tinname.empty())
      {
        vtkStringArray* nameArray = vtkStringArray::New();
        nameArray->SetName("Name");
        nameArray->InsertNextValue(tinname);
        output->GetFieldData()->AddArray(nameArray);
        nameArray->Delete();
      }
    }
    else if (cardname == "THORID")
    {
      char buffer[1024];
      std::string tinname;
      this->Internals->Stream->getline(buffer, 1024);

      // Spaces are allowed only is the name is quoted. Otherwise, no.
      bool spaceIsOK = false;
      bool done = false;
      for (int i = 0; i < 1024 && buffer[i] && !done; i++)
      {
        switch (buffer[i])
        {
          case '"':
            if (spaceIsOK)
            {
              done = true;
            }
            else
            {
              spaceIsOK = true;
            }
            break;
          case ' ':
            if (spaceIsOK)
            {
              tinname.push_back(buffer[i]);
            }
            break;
          default:
            tinname.push_back(buffer[i]);
        }
      }

      if (!tinname.empty())
      {
        vtkStringArray* nameArray = vtkStringArray::New();
        nameArray->SetName("ThorID");
        nameArray->InsertNextValue(tinname);
        output->GetFieldData()->AddArray(nameArray);
        nameArray->Delete();
      }
    }
    else if (cardname == "GUID")
    {
      char buffer[1024];
      std::string tinname;
      this->Internals->Stream->getline(buffer, 1024);

      // Spaces are allowed only is the name is quoted. Otherwise, no.
      bool spaceIsOK = false;
      bool done = false;
      for (int i = 0; i < 1024 && buffer[i] && !done; i++)
      {
        switch (buffer[i])
        {
          case '"':
            if (spaceIsOK)
            {
              done = true;
            }
            else
            {
              spaceIsOK = true;
            }
            break;
          case ' ':
            if (spaceIsOK)
            {
              tinname.push_back(buffer[i]);
            }
            break;
          default:
            tinname.push_back(buffer[i]);
        }
      }

      if (!tinname.empty())
      {
        vtkStringArray* nameArray = vtkStringArray::New();
        nameArray->SetName("GUID");
        nameArray->InsertNextValue(tinname);
        output->GetFieldData()->AddArray(nameArray);
        nameArray->Delete();
      }
    }
    else if (cardname == "VERT")
    {
      this->ReadVerts(tin);
    }
    else if (cardname == "TRI")
    {
      vtkCellArray* ca = vtkCellArray::New();
      tin->SetPolys(ca);
      ca->Delete();
      this->ReadTriangles(ca);
    }
    else if (cardname == "ENDT")
    {
      break;
    }
    else
    {
      vtkErrorMacro("Unknown card: " << cardname);
      return 0;
    }
  }
  return 1;
}

void vtkGMSTINReader::ReadTriangles(vtkCellArray* ca)
{
  vtkIdType numTris, pts[3];
  *this->Internals->Stream >> numTris;
  vtkIdTypeArray* cellInfo = vtkIdTypeArray::New();
  cellInfo->SetNumberOfValues(4 * numTris);
  for (vtkIdType i = 0, j = 0; i < numTris; i++)
  {
    *this->Internals->Stream >> pts[0] >> pts[1] >> pts[2];
    cellInfo->SetValue(j++, 3);
    cellInfo->SetValue(j++, pts[0] - 1);
    cellInfo->SetValue(j++, pts[1] - 1);
    cellInfo->SetValue(j++, pts[2] - 1);
  }
  ca->SetCells(numTris, cellInfo);
  cellInfo->Delete();
}

void vtkGMSTINReader::ReadVerts(vtkPolyData* tin)
{
  vtkIdType numVerts;
  *this->Internals->Stream >> numVerts;
  vtkPoints* pts = vtkPoints::New();
  pts->SetDataTypeToDouble();
  tin->SetPoints(pts);
  pts->Delete();
  vtkDoubleArray* dpts = vtkDoubleArray::SafeDownCast(pts->GetData());
  vtkBitArray* lockInfo = vtkBitArray::New();
  lockInfo->SetName("LockInfo");
  dpts->SetNumberOfTuples(numVerts);
  lockInfo->SetNumberOfComponents(1);
  lockInfo->SetNumberOfTuples(numVerts);
  int linfo;
  double pnt[3];
  for (vtkIdType i = 0; i < numVerts; i++)
  {
    *this->Internals->Stream >> pnt[0] >> pnt[1] >> pnt[2];
    dpts->SetTypedTuple(i, pnt);
    *this->Internals->Stream >> linfo;
    lockInfo->SetValue(i, linfo);
  }
  tin->GetPointData()->AddArray(lockInfo);
  lockInfo->Delete();
}

void vtkGMSTINReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << endl;
}

int vtkGMSTINReader::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  if (!this->FileName)
  {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
  }

  return 1;
}
