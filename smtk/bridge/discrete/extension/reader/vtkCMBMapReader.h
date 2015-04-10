//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMapReader - reader for map files
// .SECTION Description
// Reads in vertexes, arcs, and polygons described in map files
// reader based on the filename's extension.

#ifndef __smtkdiscrete_vtkCMBMapReader_h
#define __smtkdiscrete_vtkCMBMapReader_h

#include "smtk/bridge/discrete/extension/vtkSMTKDiscreteExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkIntArray;

namespace smtk {
  namespace bridge {
    namespace discrete {

class VTKSMTKDISCRETEEXT_EXPORT vtkCMBMapReader : public vtkPolyDataAlgorithm
{
  public:
    static vtkCMBMapReader *New();
    vtkTypeMacro(vtkCMBMapReader,vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Name of the file to be read.
    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);
    vtkGetMacro(NumArcs,int);
    vtkIntArray* GetArcIds()
    {
      return ArcIds;
    }

  protected:
    vtkCMBMapReader();
    ~vtkCMBMapReader();

    int RequestInformation(vtkInformation *,
        vtkInformationVector **,
        vtkInformationVector *);
    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
    char *FileName;

    int NumArcs;
    vtkIntArray* ArcIds;

  private:
    vtkCMBMapReader(const vtkCMBMapReader&);  // Not implemented.
    void operator=(const vtkCMBMapReader&);  // Not implemented.
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif
