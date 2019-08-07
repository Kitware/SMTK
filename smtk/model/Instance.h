//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Instance_h
#define __smtk_model_Instance_h

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
  /// a string used to store/fetch placements.
  static constexpr const char* const placements = "placements";
  /// a string used to store/fetch orientation as float property
  /// It will be retranscribed as vtkDoubleArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const orientations = "orientations";
  /// a string used to store/fetch scales as float property
  /// It will be retranscribed as vtkDoubleArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const scales = "scales";
  /// a string used to store/fetch masks(AKA visibility) as int property
  /// It will be retranscribed as vtkUnsignedCharArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const masks = "masks";
  /// a string used to store/fetch colors in rgb 0~255 as double property
  /// It will be retranscribed as vtkUnsignedCharArray when passing into vtkGlyph3DMapper if specified.
  static constexpr const char* const colors = "colors";

  EntityRef prototype() const;

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

  EntityRefs snapEntities() const;
  bool addSnapEntity(const EntityRef& snapTo);
  bool removeSnapEntity(const EntityRef& snapTo);
  bool setSnapEntity(const EntityRef& snapTo);
  bool setSnapEntities(const EntityRefs& snapTo);

  // Instance& setTransform(const smtk::common::Matrix4d&);
  // smtk::common::Matrix4d transform() const;
};

typedef std::vector<Instance> InstanceEntities;

} // namespace model
} // namespace smtk

#endif // __smtk_model_Instance_h
