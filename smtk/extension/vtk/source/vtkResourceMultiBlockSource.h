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

#include <memory>
#include <set>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{

class Geometry;
}
}
}
}

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
  *
  * This class provides methods to cache blocks so that resource-specific
  * subclasses need only regenerate data for modified components.
  */
class VTKSMTKSOURCEEXT_EXPORT vtkResourceMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkAbstractTypeMacro(vtkResourceMultiBlockSource, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using UUID = smtk::common::UUID;

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

  /// Sequence numbers are integers describing how recent a given component's data is.
  using SequenceType = int;

  /// Reserve a special value for invalid (non-existent) sequence numbers.
  constexpr static SequenceType InvalidSequence = -1;

  /// Cache entries hold VTK data and a sequence number for a given UUID.
  struct CacheEntry
  {
    /// Data being cached. Null pointers are disallowed.
    vtkDataObject* Data;
    /// The "timestamp" or "generation number" of the cache data.
    SequenceType SequenceNumber;
  };

  /// Key used to put entity UUID in the meta-data associated with a block.
  static vtkInformationStringKey* COMPONENT_ID();

  /// Set the COMPONENT_ID key on the given information object to a given UUID.
  static void SetDataObjectUUID(vtkInformation*, const UUID&);

  /// Return a UUID for the data object.
  static UUID GetDataObjectUUID(vtkInformation*);

  /// Store the resource UUID in the output dataset's top-level block metadata.
  static void SetResourceId(vtkMultiBlockDataSet* dataset, const UUID&);

  /// Fetch the resource UUID from a dataset's top-level block metadata.
  static UUID GetResourceId(vtkMultiBlockDataSet* dataset);

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

  /// Given a UUID and a data object, insert it into the cache, overwriting any pre-existing entry.
  ///
  /// The \a sequenceNumber specifies how recent the provided data object is.
  /// If a more recent (larger) sequenceNumber number is in the map, this method does nothing
  /// and returns false.
  /// This method returns true when the cache is modified.
  bool SetCachedData(const UUID& uid, vtkDataObject* data, SequenceType sequenceNumber);
  /// Fetch a cache entry's sequence number.
  SequenceType GetCachedDataSequenceNumber(const UUID& uid) const;
  /// Fetch a cache entry's data.
  vtkDataObject* GetCachedDataObject(const UUID& uid);
  /// Remove a single cache entry. Note that this effectively resets the sequence numbers for \a uid.
  ///
  /// This method returns true when the cache was modified.
  bool RemoveCacheEntry(const UUID& uid);
  /// Remove cache entries not listed in the provided set.
  bool RemoveCacheEntriesExcept(const std::set<UUID>& exceptions);
  /// Remove all cache entries (releasing their VTK data objects).
  void ClearCache();

protected:
  vtkResourceMultiBlockSource();
  ~vtkResourceMultiBlockSource() override;

  /// If any subclass determines that the resource has an
  /// appropriate geometry provider, it can call this method
  /// for the body of its RequestData() implementation.
  int RequestDataFromGeometry(vtkInformation* request, vtkInformationVector* outputData,
    const smtk::extension::vtk::source::Geometry& provider);

  std::weak_ptr<smtk::resource::Resource> Resource;
  std::map<UUID, CacheEntry> Cache;
  std::set<UUID> Visited; // Populated with extant entities during RequestData.

private:
  vtkResourceMultiBlockSource(const vtkResourceMultiBlockSource&) = delete;
  void operator=(const vtkResourceMultiBlockSource&) = delete;
};

#endif
