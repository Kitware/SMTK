//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_PortData_h
#define smtk_task_PortData_h

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

namespace smtk
{
namespace task
{

/**\brief PortData is a base class for all types of information passed through Ports.
  *
  * Port objects are passed as keys to Task objects to fetch PortData.
  *
  * PortData objects are intended to be lightweight and refer to large data by reference.
  * PortData does not inherit smtk::resource::PersistentObject but frequently needs
  * to refer to persistent data. The base Port class has a virtual method defined
  * that allows ports to construct an appropriate PortData subclass from a persistent object.
  *
  * PortData objects should **not** be held by objects. They should be consumed and
  * then discarded by tasks requesting them; objects in PortData subclasses are
  * generally referenced by raw pointer, so holding a PortData beyond the life of the
  * function consuming the data may result in stale pointers being dereferenced.
  */
class SMTKCORE_EXPORT PortData : smtkEnableSharedPtr(PortData)
{
public:
  smtkTypeMacroBase(smtk::task::PortData);

  /// This default virtual destructor forces PortData to be polymorphic.
  virtual ~PortData() = default;

  /// Merge another \a data instance with this one.
  ///
  /// Some subclasses of PortData may provide the ability to merge
  /// with other PortData objects. If so, they should implement this
  /// method.
  ///
  /// The default is for merging to fail (returning false).
  virtual bool merge(const PortData* data)
  {
    (void)data;
    return false;
  }
};

} // namespace task
} // namespace smtk

#endif // smtk_task_PortData_h
