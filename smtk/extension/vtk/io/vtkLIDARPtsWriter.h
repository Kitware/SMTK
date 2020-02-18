//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkLIDARPtsWriter - Writer for LIDAR point files
// .SECTION Description

#ifndef __LIDARPtsWriter_h
#define __LIDARPtsWriter_h

#include "smtk/extension/vtk/io/IOVTKExports.h" // For export macro
#include "vtkWriter.h"
#include <fstream>
#include <map>

class vtkPolyData;

#define VTK_ASCII 1
#define VTK_BINARY 2

class SMTKIOVTK_EXPORT vtkLIDARPtsWriter : public vtkWriter
{
public:
  static vtkLIDARPtsWriter* New();
  vtkTypeMacro(vtkLIDARPtsWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Add an input to this writer
  void AddInputData(vtkDataObject* input) { this->AddInputData(0, input); }
  void AddInputData(int, vtkDataObject*);

  // Description:
  // Set/Get whether or not to write multiple pieces as a single piece
  vtkBooleanMacro(WriteAsSinglePiece, bool);
  vtkSetMacro(WriteAsSinglePiece, bool);
  vtkGetMacro(WriteAsSinglePiece, bool);

  //BTX
  // Description:
  // Unlike vtkWriter which assumes data per port - this Writer can have multiple connections
  // on Port 0
  vtkDataObject* GetInputFromPort0(int connection);
  vtkDataObject* GetInputFromPort0() { return this->GetInputFromPort0(0); };
  //ETX

  //BTX

protected:
  vtkLIDARPtsWriter();
  ~vtkLIDARPtsWriter() override;

  // Actual writing.
  void WriteData() override;
  // return write_status: OK, Abort, or Error
  int WriteFile(std::ofstream& ofp);
  int ComputeRequiredAxisPrecision(double min, double max);
  int WritePoints(std::ofstream& ofp, vtkPolyData* inputPoly);

  std::ofstream* OpenOutputFile();
  bool IsBinaryType(const char* filename);

  void CloseFile(ios* fp);

  char* FileName;
  int OutputIsBinary;
  bool WriteAsSinglePiece;

  int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkLIDARPtsWriter(const vtkLIDARPtsWriter&); // Not implemented.
  void operator=(const vtkLIDARPtsWriter&);    // Not implemented.

  //ETX
};

#endif
