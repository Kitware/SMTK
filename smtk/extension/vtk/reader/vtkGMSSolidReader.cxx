//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkGMSSolidReader.h"

#include "smtk/extension/vtk/reader/vtkCMBReaderHelperFunctions.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"

#include <string>
#include <sys/stat.h>

vtkStandardNewMacro(vtkGMSSolidReader);

struct vtkGMSSolidReaderInternals
{
  ifstream* Stream;

  vtkGMSSolidReaderInternals() : Stream(0) {}
  ~vtkGMSSolidReaderInternals()
    {
      this->DeleteStream();
    }
  void DeleteStream()
    {
      delete this->Stream;
      this->Stream = 0;
    }
};

//----------------------------------------------------------------------------
vtkGMSSolidReader::vtkGMSSolidReader()
{
  this->FileName  = NULL;
  this->SetNumberOfInputPorts(0);
  this->Internals = new vtkGMSSolidReaderInternals;
}

//----------------------------------------------------------------------------
vtkGMSSolidReader::~vtkGMSSolidReader()
{
  delete[] this->FileName;
  delete this->Internals;
}

//----------------------------------------------------------------------------
int vtkGMSSolidReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if (!this->FileName || (strlen(this->FileName) == 0))
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  struct stat fs;
  if (stat(this->FileName, &fs) != 0)
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    return 0;
    }

  this->Internals->Stream = new ifstream(this->FileName, ios::in);
  if (this->Internals->Stream->fail())
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    this->Internals->DeleteStream();
    return 0;
    }

  std::string header;
  *this->Internals->Stream >> header;
  if (header != "SOLID")
    {
    vtkErrorMacro("Failed reader MESH2D card in the header.");
    this->SetErrorCode( vtkErrorCode::FileFormatError );
    this->Internals->DeleteStream();
    return 0;
    }

  unsigned int block = 0;
  while(!this->Internals->Stream->eof())
    {
    if (!this->ReadSolid(block++, output))
      {
      this->Internals->DeleteStream();
      output->Initialize();
      return 0;
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkGMSSolidReader::ReadSolid(unsigned int block,
                                 vtkMultiBlockDataSet* output)
{
  std::string cardname;

  *this->Internals->Stream >> cardname;
  if (this->Internals->Stream->eof())
    {
    return 1;
    }
  if (cardname != "BEGS")
    {
    vtkErrorMacro("Expected BEGS, found " << cardname << " instead");
    return 0;
    }

  vtkPolyData* solid = vtkPolyData::New();
  output->SetBlock(block, solid);
  solid->Delete();

  while (!this->Internals->Stream->eof())
    {
    *this->Internals->Stream >> cardname;
    if (cardname == "HIDDEN")
      {
      int flag;
      *this->Internals->Stream >> flag;

      vtkIntArray* hiddenArray = vtkIntArray::New();
      hiddenArray->SetName("Hidden");
      hiddenArray->InsertNextValue(flag);
      solid->GetFieldData()->AddArray(hiddenArray);
      hiddenArray->Delete();
      }
    else if (cardname == "ID")
      {
      int id;
      *this->Internals->Stream >> id;
      }
    else if (cardname == "MAT")
      {
      int id;
      *this->Internals->Stream >> id;

      vtkIntArray* materialArray = vtkIntArray::New();
      materialArray->SetName(ReaderHelperFunctions::GetMaterialTagName());
      materialArray->InsertNextValue(id);
      solid->GetFieldData()->AddArray(materialArray);
      materialArray->Delete();
      }
    else if (cardname == "SNAM")
      {
      char buffer[1024];
      std::string solidname;
      this->Internals->Stream->getline(buffer, 1024);

      // Spaces are allowed only is the name is quoted. Otherwise, no.
      bool spaceIsOK = false;
      bool done = false;
      for (int i=0; i<1024 && buffer[i] && !done; i++)
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
              solidname.push_back(buffer[i]);
              }
            break;
          default:
            solidname.push_back(buffer[i]);
          }
        }

      if (!solidname.empty())
        {
        vtkStringArray* nameArray = vtkStringArray::New();
        nameArray->SetName("Name");
        nameArray->InsertNextValue(solidname);
        solid->GetFieldData()->AddArray(nameArray);
        nameArray->Delete();
        }
      }
    else if (cardname == "VERT")
      {
      vtkPoints* pts = vtkPoints::New();
      pts->SetDataTypeToDouble();
      solid->SetPoints(pts);
      pts->Delete();
      vtkDoubleArray* dpts = vtkDoubleArray::SafeDownCast(pts->GetData());
      this->ReadVerts(dpts);
      }
    else if (cardname == "TRI")
      {
      vtkCellArray* ca = vtkCellArray::New();
      solid->SetPolys(ca);
      ca->Delete();
      vtkUnsignedCharArray* visArray = vtkUnsignedCharArray::New();
      visArray->SetName("Visibility");
      solid->GetCellData()->AddArray(visArray);
      visArray->Delete();
      this->ReadTriangles(ca, visArray);
      }
    else if (cardname == "LAYERS")
      {
      char buffer[1024];
      this->Internals->Stream->getline(buffer, 1024);
      }
    else if (cardname == "TOPCELLBIAS")
      {
      char buffer[1024];
      this->Internals->Stream->getline(buffer, 1024);
      }
    else if (cardname == "MINCELLTHICK")
      {
      char buffer[1024];
      this->Internals->Stream->getline(buffer, 1024);
      }
    else if (cardname == "ENDS")
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

//----------------------------------------------------------------------------
void vtkGMSSolidReader::ReadTriangles(vtkCellArray* ca,
                                      vtkUnsignedCharArray* visArray)
{
  vtkIdType numTris;
  *this->Internals->Stream >> numTris;
  for (vtkIdType i=0; i<numTris; i++)
    {
    vtkIdType pts[3];
    *this->Internals->Stream >> pts[0] >> pts[1] >> pts[2];
    vtkIdType cellId = ca->InsertNextCell(3, pts);
    int vis;
    *this->Internals->Stream >> vis;
    visArray->InsertValue(cellId, static_cast<unsigned char>(vis));
    }
}

//----------------------------------------------------------------------------
void vtkGMSSolidReader::ReadVerts(vtkDoubleArray* dpts)
{
  vtkIdType numVerts;
  *this->Internals->Stream >> numVerts;
  for (vtkIdType i=0; i<numVerts; i++)
    {
    double pts[3];
    *this->Internals->Stream >> pts[0] >> pts[1] >> pts[2];
    dpts->InsertNextTuple(pts);
    }
}

//----------------------------------------------------------------------------
void vtkGMSSolidReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << endl;
}

//----------------------------------------------------------------------------
int vtkGMSSolidReader::RequestInformation(
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
