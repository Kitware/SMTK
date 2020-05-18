//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_Instance_h
#define smtk_model_Instance_h

#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace model
{

/**\brief A entityref subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT Instance : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(Instance, EntityRef, isInstance);
  /// A float-property key used to store/fetch placements.
  static constexpr const char* const placements = "placements";
  /// A float-property key used to store/fetch orientation.
  /// It will be retranscribed as vtkDoubleArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const orientations = "orientations";
  /// A float-property key used to store/fetch scales.
  /// It will be retranscribed as vtkDoubleArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const scales = "scales";
  /// An int-property key used to store/fetch masks(AKA visibility).
  /// It will be retranscribed as vtkUnsignedCharArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const masks = "masks";
  /// A float-property key used to store/fetch colors in rgb 0~255.
  /// It will be retranscribed as vtkUnsignedCharArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const colors = "colors";
  /// An int-property key used to fetch-store clone subset indices.
  /// It is used by the divide() method to operate on interactive selections.
  static constexpr const char* const subset = "subset";

  /**\brief Return the model entity whose geometry serves
    *       as the prototype for this instance's placements.
    */
  EntityRef prototype() const;
  /// Change source of geometry for this instance.
  bool setPrototype(const EntityRef& prototype);

  /**\brief Apply rules (stored in properties) to recompute the tessellation for this instance.
    *
    * This will always update the instance's tessellation whether it needs it or not.
    * This method is called by EntityRef::hasTessellation() when no tessellation exists,
    * so if you change properties that govern an instance's placements/transforms you
    * may just remove the current tessellation and the tessellation will be created when
    * asked for.
    */
  Tessellation* generateTessellation();

  std::string rule() const;
  bool setRule(const std::string& nextRule);

  bool setSampleSurface(const EntityRef& surface);
  EntityRef sampleSurface() const;

  EntityRefs snapEntities() const;
  bool addSnapEntity(const EntityRef& snapTo);
  bool removeSnapEntity(const EntityRef& snapTo);
  bool setSnapEntity(const EntityRef& snapTo);
  bool setSnapEntities(const EntityRefs& snapTo);

  /// Return the number of placements (building a tessellation as needed).
  std::size_t numberOfPlacements();

  /// Return whether this instance is a temporary clone of another instance.
  bool isClone() const;

  /**\brief Create a new instance duplicating a subset of this instance's placements.
    *
    * This creates and returns a new instance.
    * The new instance may have a different rule than the original
    * as placement-subsetting may not be possible otherwise.
    *
    * The \a begin and \a end arguments are iterators whose values are 0-based
    * integer offsets into the sequence of placements.
    *
    * The new instance will have an Instance::subset integer property listing the points
    * selected from the original and will be related to the original as if it
    * was a group member (i.e., with ArrangementKind::SUBSET_OF).
    */
  template<typename I>
  Instance clonePlacements(I begin, I end, bool relate = true);

  /**\brief Divide an instance into several based on "subset" clones.
    *
    * Once you are done creating subsets of an instance (via the clonePlacements
    * method), you can call divideInstance() on the source instance (i.e., the
    * parent of all the clones). It will inspect the clones and create new
    * instances for all of the placements subsets (including the remainder of
    * placements not in any subset).
    *
    * If \a merge is true, then all clones marked as subsets of this instance
    * will result in a single output instance. Otherwise, each clone will have
    * its own output instance.
    * The clones included in the division can be reported; if you pass in
    * a non-null pointer for \a clonesIncluded, it will have the set of clones
    * appended.
    * Note that \a clonesIncluded is not reset by this method;
    * any pre-existing entries will be preserved.
    *
    * If divide is called on an instance with no clone-children, the
    * returned set will be empty.
    */
  template<typename Container>
  Container divide(bool merge = false, Container* clonesIncluded = nullptr);

  /**\brief Merge two or more instances that have everything but placements in common.
    *
    * If instances have the same prototype, rule, and other settings
    * in common, this method will attempt to append their placements
    * (and orientations, scalings, and colors) into a common output instance.
    *
    * If instances cannot be merged, an invalid instance is returned.
    *
    * NB: The current implementation will always merge instances into
    * an instance with a tabular rule.
    */
  template<typename Container>
  static Instance merge(const Container& instances);

  // Instance& setTransform(const smtk::common::Matrix4d&);
  // smtk::common::Matrix4d transform() const;

protected:
  template<typename Container>
  void divideMapInternal(
    Instance& clone,
    std::set<int>& taken,
    bool merge,
    std::vector<std::vector<int>>& output,
    Container* clonesIncluded);

  bool checkMergeable(const Instance& other) const;
  bool mergeInternal(const Instance& other);
};

typedef std::vector<Instance> InstanceEntities;

} // namespace model
} // namespace smtk

#endif // smtk_model_Instance_h
