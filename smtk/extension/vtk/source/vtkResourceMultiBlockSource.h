//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_vtk_ResourceMultiBlockSource_h
#define smtk_vtk_ResourceMultiBlockSource_h

#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "vtkMultiBlockDataSetAlgorithm.h"

/**\brief A VTK source for exposing smtk resources.
  *
  * This filter generates a vtkMultiBlockDataSet with the following structure:
  *
  * Block <Components>: a vtkMultiBlockDataSet where each block represents a
  *                     component in the resource.
  * Block <Prototypes>: a vtkMultiBlockDataSet where each block represents a
  *                     component that is to be used as a prototype for a glyph.
  * Block <Instances>:  a vtkMultiBlockDataSet where each block is a vtkPolyData
  *                     containing the points at which the prototypes should be
  *                     placed.
  */
class VTKSMTKSOURCEEXT_EXPORT vtkResourceMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkAbstractTypeMacro(vtkResourceMultiBlockSource, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Block Ids for the primary blocks that comprise the output.
  ///
  /// NOTE: avoid using a strongly typed enum so the value will be implicitly
  ///       convertable to an integer
  struct BlockId
  {
    enum Value : unsigned int
    {
      Components,
      Prototypes,
      Instances,
      NumberOfBlocks
    };
  };

  /// Key used to put entity UUID in the meta-data associated with a block.
  static vtkInformationStringKey* COMPONENT_ID();

  /// Set the COMPONENT_ID key on the given information object to a given UUID.
  static void SetDataObjectUUID(vtkInformation*, const smtk::common::UUID&);

  /// Return a UUID for the data object.
  static smtk::common::UUID GetDataObjectUUID(vtkInformation*);

  /// Store the resource UUID in the output dataset's top-level block metadata.
  static void SetResourceId(vtkMultiBlockDataSet* dataset, const smtk::common::UUID&);

  /// Fetch the resource UUID from a dataset's top-level block metadata.
  static smtk::common::UUID GetResourceId(vtkMultiBlockDataSet* dataset);

  /// Return the component corresponding to the data object.
  static smtk::resource::ComponentPtr GetComponent(
    const smtk::resource::ResourcePtr&, vtkInformation*);
  smtk::resource::ComponentPtr GetComponent(vtkInformation*);

  smtk::resource::ResourcePtr GetResource();
  void SetResource(const smtk::resource::ResourcePtr&);

  /// A debug utility to print out the block structure of a multiblock dataset
  /// annotated with UUIDs (where present) and data type.
  static void DumpBlockStructureWithUUIDs(vtkMultiBlockDataSet* dataset, int indent = 0)
  {
    int counter = 1;
    DumpBlockStructureWithUUIDsInternal(dataset, counter, indent);
  }
  static void DumpBlockStructureWithUUIDsInternal(
    vtkMultiBlockDataSet* dataset, int& counter, int indent = 0);

protected:
  vtkResourceMultiBlockSource();
  ~vtkResourceMultiBlockSource() override;

  std::weak_ptr<smtk::resource::Resource> Resource;

private:
  vtkResourceMultiBlockSource(const vtkResourceMultiBlockSource&) = delete;
  void operator=(const vtkResourceMultiBlockSource&) = delete;
};

#endif
