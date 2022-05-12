//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_vtk_io_vtkMedHelper_h
#define smtk_vtk_io_vtkMedHelper_h

#include "smtk/extension/vtk/io/IOVTKExports.h"

#include "vtk_hdf5.h"

#include "vtkCellType.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"

#include <list>
#include <unordered_map>

class vtkCellArray;
class vtkCellData;
class vtkPointData;
class vtkPoints;
class vtkStringArray;

// Mappings from med string type to vertex count
static std::unordered_map<std::string, vtkIdType> vertexCount{
  { "PO1", 1 },   // Vertex
  { "SE2", 2 },   // Line
  { "TR3", 3 },   // Triangle
  { "QU4", 4 },   // Quad
  { "TE4", 4 },   // Tetra
  { "HE8", 8 },   // Hexahedron
  { "PY5", 5 },   // Pyramid
  { "PE6", 6 },   // Wedge
  { "SE3", 3 },   // Quadratic Line
  { "TR6", 6 },   // Quadratic Triangle
  { "QU8", 8 },   // Quadratic Quad
  { "T10", 10 },  // Quadratic Tetrahedron
  { "HE20", 20 }, // Quadratic Hexahedron
  { "PY13", 13 }, // Quadratic Pyramid
  { "P15", 15 }   // Quadratic Wedge
};

// Mappings from med string cell type to vtk cell type
static std::unordered_map<std::string, int> medToVtkCellType{ { "P01", VTK_VERTEX },
                                                              { "SE2", VTK_LINE },
                                                              { "TR3", VTK_TRIANGLE },
                                                              { "QU4", VTK_QUAD },
                                                              { "TE4", VTK_TETRA },
                                                              { "HE8", VTK_HEXAHEDRON },
                                                              { "PY5", VTK_PYRAMID },
                                                              { "PE6", VTK_WEDGE },
                                                              { "SE3", VTK_QUADRATIC_EDGE },
                                                              { "TR6", VTK_QUADRATIC_TRIANGLE },
                                                              { "QU8", VTK_QUADRATIC_QUAD },
                                                              { "T10", VTK_QUADRATIC_TETRA },
                                                              { "HE20", VTK_QUADRATIC_HEXAHEDRON },
                                                              { "PY13", VTK_QUADRATIC_PYRAMID },
                                                              { "P15", VTK_QUADRATIC_WEDGE } };

static std::unordered_map<int, std::string> vtkToMedCellType{ { VTK_VERTEX, "P01" },
                                                              { VTK_LINE, "SE2" },
                                                              { VTK_TRIANGLE, "TR3" },
                                                              { VTK_QUAD, "QU4" },
                                                              { VTK_TETRA, "TE4" },
                                                              { VTK_HEXAHEDRON, "HE8" },
                                                              { VTK_PYRAMID, "PY5" },
                                                              { VTK_WEDGE, "PE6" },
                                                              { VTK_QUADRATIC_EDGE, "SE3" },
                                                              { VTK_QUADRATIC_TRIANGLE, "TR6" },
                                                              { VTK_QUADRATIC_QUAD, "QU8" },
                                                              { VTK_QUADRATIC_TETRA, "T10" },
                                                              { VTK_QUADRATIC_HEXAHEDRON, "HE20" },
                                                              { VTK_QUADRATIC_PYRAMID, "PY13" },
                                                              { VTK_QUADRATIC_WEDGE, "P15" } };

struct vtkMedCellData
{
  vtkSmartPointer<vtkCellArray> cells;
  vtkSmartPointer<vtkCellData> cellData;
  std::string cellType;
};

struct vtkMedPointData
{
  vtkSmartPointer<vtkPoints> points;
  vtkSmartPointer<vtkPointData> pointData;
};

class SMTKIOVTK_EXPORT HdfNode
{
public:
  HdfNode(
    hid_t loc = -1,
    std::string nm = std::string(),
    std::string pp = std::string(),
    HdfNode* pr = nullptr)
    : locId(loc)
    , name(std::move(nm))
    , path(std::move(pp))
    , parent(pr)
  {
  }

  HdfNode* findChild(const std::string& name);

  hid_t locId = -1;
  std::string name;
  std::string path;
  HdfNode* parent = nullptr;
  std::list<HdfNode*> children;
};

using HdfNodeIterator = std::list<HdfNode*>::iterator;

// Builds a dynamic tree from a Hdf5 file, must be deleted by user
SMTKIOVTK_EXPORT HdfNode* rootBuildTree(hid_t rootId);

// Recursively deletes a tree of HdfNodes
SMTKIOVTK_EXPORT void deleteTree(HdfNode* node);

#endif
