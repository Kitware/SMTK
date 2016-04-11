//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshReader - Reader for CMB ADH, PT123, WASH123D, and XMS
// mesh files
// .SECTION Description
// vtkCMBMeshReader reads 1D, 2D, and 3D meshes written for CMB's ADH,
// PT123, and WASH123D models; and by XMS in ASCII.
// It is assumed that node and element indices start from 1 and that there
// are no gaps in indices.
//
// Supported file cards are:
//   ADH, XMS format:
//     MESH2D or MESH3D (file declaration)
//     E2L (1D line) - SMS only
//     E3L (1D quadratic line) - SMS only
//     E3T (2D triangle)
//     E4Q (2D quadrilateral) - XMS only
//     E6T (2D quadrautic triangle) - XMS only
//     E8Q (2D quadrautic quadrilateral) - XMS only
//     E9Q (2D biquadrautic quadrilateral) - SMS only
//     E4T (3D tetrathedron)
//     E5P (3D pyramid) - GMS only
//     E6W (3D wedge or prism) - GMS only
//     E8H (3D hexahedron) - GMS only
//     ND  (node)
//
//   PT123, WASH123D format:
//     MESH (file declaration) - PT123
//      or
//     WMS1DM, WMS2DM, or WMS3DM (file declaration) - WASH123D
//
//     GE2 (1D line with 2 nodes)
//     GE3 (2D triangle with 3 nodes)
//     GE4 (2D quadrilateral or 3D tetrahedron) (file declaration card or
//          file extension determines the dimension)
//     GE6 (3D wedge or prism)
//     GE8 (3D hexahedron)
//     GN  (node)
//
//     ENDR (end of file) - PT123
//      or
//     END (end of file) - WASH123D
//
//   Meta data (optional; expected on lines right after file declaration)
//     #NNODE (number of nodes declared in file)
//     #NELEM (number of elements declared in file)

#ifndef __smtk_vtk_vtkCMBMeshReader_h
#define __smtk_vtk_vtkCMBMeshReader_h

#include "smtk/extension/vtk/reader/Exports.h" // For export macro
#include "vtkUnstructuredGridAlgorithm.h"

class vtkDoubleArray;
class vtkIntArray;
class vtkUnstructuredGrid;
class vtkIdTypeArray;

namespace smtk {
  namespace vtk {

//BTX
struct vtkCMBMeshReaderInternals;
//ETX

class VTKSMTKREADEREXT_EXPORT vtkCMBMeshReader : public vtkUnstructuredGridAlgorithm
{
public:
//BTX
  enum vtkCMBMeshDimension { MESH1D = 1, MESH2D = 2, MESH3D = 3 };
//ETX

  static vtkCMBMeshReader *New();
  vtkTypeMacro(vtkCMBMeshReader,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Determine whether the file can be read by this reader.
  int CanReadFile(const char *);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the mesh dimension.
  vtkGetMacro(MeshDimension, int);

  // Description:
  // Turn on/off the generation of the Mesh Element ID array.
  vtkBooleanMacro(CreateMeshElementIdArray, bool);
  vtkSetMacro(CreateMeshElementIdArray, bool);
  vtkGetMacro(CreateMeshElementIdArray, bool);

  // Description:
  // Turn on/off the generation of the Mesh Material ID (or Region) array.
  vtkBooleanMacro(CreateMeshMaterialIdArray, bool);
  vtkSetMacro(CreateMeshMaterialIdArray, bool);
  vtkGetMacro(CreateMeshMaterialIdArray, bool);

  // Description:
  // Turn on/off the renaming of the Mesh Material ID array to the Region array.
  vtkBooleanMacro(RenameMaterialAsRegion, bool);
  vtkSetMacro(RenameMaterialAsRegion, bool);
  vtkGetMacro(RenameMaterialAsRegion, bool);

  // Description:
  // Turn on/off the generation of the Mesh Node ID array.
  vtkBooleanMacro(CreateMeshNodeIdArray, bool);
  vtkSetMacro(CreateMeshNodeIdArray, bool);
  vtkGetMacro(CreateMeshNodeIdArray, bool);

protected:
  vtkCMBMeshReader();
  ~vtkCMBMeshReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

  int PreviewFile(vtkIdType& ncells, vtkIdType& npts);
  void ReadCell(int cellType, int npts, vtkUnstructuredGrid* output,
                vtkIntArray* cellMaterialArray, vtkIdTypeArray* cellIdArray);
  void ReadNode(vtkDoubleArray* dpts, vtkIdTypeArray* nodeIdArray);

  char * FileName;
  bool CreateMeshElementIdArray;
  bool CreateMeshMaterialIdArray;
  bool CreateMeshNodeIdArray;
  bool RenameMaterialAsRegion;
  int MeshDimension;

private:
  vtkCMBMeshReader(const vtkCMBMeshReader&);  // Not implemented.
  void operator=(const vtkCMBMeshReader&);  // Not implemented.

  vtkCMBMeshReaderInternals* Internals;
};

  } // namespace vtk
} // namespace smtk

#endif
