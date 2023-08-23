//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Status_h
#define smtk_common_Status_h

#include "smtk/CoreExports.h"

namespace smtk
{
namespace common
{

/**\brief A return value for methods that need to indicate both
 *        success/failure and modification/stasis.
 *
 * This convenience class provides a default cast to a boolean that returns
 * whether the method succeeded; this way it is backwards compatible with
 * any API that previously returned a success/failure boolean.
 *
 * Upon construction, a Status object indicates success without modification.
 * You may then mark whether failure or modification occurred.
 */
class SMTKCORE_EXPORT Status
{
public:
  Status() = default;
  Status(const Status&) = default;
  Status(Status&&) = default;
  Status& operator=(const Status&) = default;

  /// Return whether or not a method succeeded (true) or not (false).
  bool success() const { return m_success; }
  /// Return whether a method modified its object (true) or not (false).
  bool modified() const { return m_modified; }
  /// Mark the Status to indicate an object was modified.
  bool markModified();
  /// Mark the Status to indicate a method failed.
  bool markFailed();
  /// Combine this status object with another instance.
  ///
  /// This AND-s the success bits together (both must succeed for either to succeed)
  /// while OR-ing the modified bits together (if either is modified, the result is).
  Status& operator&=(const Status& other);
  operator bool() const;

protected:
  bool m_success{ true };
  bool m_modified{ false };
};

} // namespace common
} // namespace smtk

#endif // smtk_common_Status_h
