//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_vtk_vtkPolyFileReader_h
#define __smtk_vtk_vtkPolyFileReader_h

#include "smtk/extension/vtk/reader/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

#include <string>

/**\brief The name of the array storing boundary marker values on points.
  *
  * These are optional values specified after each node's coordinates.
  */
#define VTK_POLYFILE_NODE_GROUP "NodeGroups"

/**\brief The name of the array storing boundary marker values on polygonal facets.
  *
  * These are optional values specified after each node's coordinates.
  */
#define VTK_POLYFILE_FACE_GROUP "boundarysets"

/**\brief The name of the array storing an optional grouping for regions.
  *
  * These are optional values specified <b>after</b> each region's coordinates
  * and are treated as region <b>group</b> identifiers since it is possible
  * to assign the same number to different regions.
  */
#define VTK_POLYFILE_REGION_GROUP "RegionGroups"

/**\brief The name of an array storing the node numbers specified with each node.
  *
  * These are the numbers that appear <b>before</b> each node's coordinates.
  */
#define VTK_POLYFILE_GLOBAL_NODE_ID "GlobalNodeIds"

/**\brief The name of an array storing the numbers specified with each hole point.
  *
  * These are the numbers that appear before each volumetric hole's coordinates.
  * They are marked as global point IDs and should be unique but this is not
  * enforced by the reader.
  */
#define VTK_POLYFILE_GLOBAL_HOLE_ID "GlobalHoleIds"

/**\brief The name of an array storing a unique integer for each polygonal facet.
  *
  * This array can be used as global model-face identifier, but is a
  * pedigree ID on the reader's output cells since a single facet may
  * be composed of multiple polyline loops.
  *
  * It is also the name of a pedigree point IDs on the hole
  * coordinate output of the reader.
  */
#define VTK_POLYFILE_MODEL_FACE_ID "modelfaceids"

/**\brief The base name of arrays storing per-node attributes.
  *
  * This name will have a zero-based integer counter appended
  * to it as multiple nodal attributes may be specified.
  */
#define VTK_POLYFILE_NODE_GROUP_ATTRIBUTES "NodeAttribute"

/**\brief The name of an array storing per-region-group attributes.
  *
  * Only one region-group attribute can be specified in the file and
  * it is usually a sizing function.
  */
#define VTK_POLYFILE_REGION_GROUP_ATTRIBUTES "RegionGroupAttribute"

/**\brief The name of the array storing a region group identifier.
  *
  * This is the number that appears <b>before</b> the coordinates
  * of a point in the region.
  */
#define VTK_POLYFILE_REGION_GROUP_NUMBER "RegionGroupNumbers"

/**\brief Read TetGen polyfiles (.poly) and surface meshes (.smesh).
  *
  */
class VTKSMTKREADEREXT_EXPORT vtkPolyFileReader : public vtkPolyDataAlgorithm
{
public:
  static vtkPolyFileReader* New();
  vtkTypeMacro(vtkPolyFileReader, vtkPolyDataAlgorithm);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/set the filename of the polyfile to read.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  // Description:
  // Get/set the filename of the file containing nodal coordinates.
  //
  // This is only used if the polyfile specified by \a FileName
  // indicates that nodal coordinates are in a separate file.
  // If NULL (the default) or empty and the polyfile does not contain nodal
  // coordinates, ".node" will be appended to base name of \a FileName.
  // For example if \a FileName is "/foo/bar.baz.poly" and
  // \a NodeFileName is NULL or empty, then "/foo/bar.baz.node" will
  // be used.
  vtkGetStringMacro(NodeFileName);
  vtkSetStringMacro(NodeFileName);

  // Description:
  // Set/get whether the file should be treated as a ".poly" file
  // (which allows facets with multiple loops and holes) or as
  // a ".smesh" file (which allows only a single loop per facet).
  //
  // The default is 0 (read as a poly file).
  // When set to -1, the filename will be used to infer file type.
  // When set to any other non-zero value, the simple mesh format
  // will be expected.
  vtkGetMacro(SimpleMeshFormat, int);
  vtkSetMacro(SimpleMeshFormat, int);
  vtkBooleanMacro(SimpleMeshFormat, int);

  // Description:
  // Should boundary markers on facets be output as cell data or
  // as field data? The latter may be more compact but will not be
  // accessible by most filters processing cells.
  //
  // The default is 0 (boundary markers will be output as field data).
  vtkGetMacro(FacetMarksAsCellData, int);
  vtkSetMacro(FacetMarksAsCellData, int);
  vtkBooleanMacro(FacetMarksAsCellData, int);

protected:
  vtkPolyFileReader();
  virtual ~vtkPolyFileReader();

  virtual int RequestData(
    vtkInformation* request, vtkInformationVector** inVec, vtkInformationVector* outVec);

  template <typename T>
  void ReadFile(std::istream& in, int isSimpleMesh, T& errorReporter,
    const std::string& nodeFileName = std::string());

  template <typename T>
  bool ReadNodes(std::istream& in, vtkIdType numPts, const std::string& nodeSpec, int& dimension,
    int& numAttribs, T& errorReporter);

  template <typename T>
  bool ReadSegments(std::istream& in, int& bdyMarkers, T& errorReporter);

  template <typename T>
  bool ReadFacets(std::istream& in, int dimension, int& bdyMarkers, T& errorReporter);

  template <typename T>
  bool ReadSimpleFacets(std::istream& in, int dimension, int& bdyMarkers, T& errorReporter);

  template <typename T>
  bool ReadHoles(std::istream& in, int dimension, T& err);

  template <typename T>
  bool ReadRegionAttributes(std::istream& in, int dimension, T& err);

  class Private;
  Private* P;
  class Builder;
  Builder* B;
  char* FileName;
  char* NodeFileName;
  int SimpleMeshFormat;
  int FacetMarksAsCellData;

private:
  vtkPolyFileReader(const vtkPolyFileReader&); // Not implemented.
  void operator=(const vtkPolyFileReader&);    // Not implemented.
};

#endif // __vtkPolyFileReader_h
