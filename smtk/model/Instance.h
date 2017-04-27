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

  EntityRef prototype() const;

  // Instance& setTransform(const smtk::common::Matrix4d&);
  // smtk::common::Matrix4d transform() const;
};

typedef std::vector<Instance> InstanceEntities;

} // namespace model
} // namespace smtk

#endif // __smtk_model_Instance_h
