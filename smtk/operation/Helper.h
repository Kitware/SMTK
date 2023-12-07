//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_Helper_h
#define smtk_operation_Helper_h

#include "smtk/operation/Operation.h"

#include <vector>

namespace smtk
{
namespace operation
{

/// A helper for dealing with serialization/deserialization done within an operation.
///
/// This is needed in order to internally call operations with the serialization/deserialization functions.
class SMTKCORE_EXPORT Helper
{
public:
  /// Destructor is public, but you shouldn't use it.
  ~Helper();
  /// Copy construction and assignment are disallowed.
  Helper(const Helper&) = delete;
  void operator=(const Helper&) = delete;

  /// Return the helper "singleton".
  ///
  /// The object returned is a per-thread instance
  /// at the top of a stack that may be altered using
  /// the pushInstance() and popInstance() methods.
  /// This allows nested deserializers to each have
  /// their own context that appears to be globally
  /// available.
  static Helper& instance();

  /// Push a new helper instance on the local thread's stack.
  ///
  /// The returned \a Helper will have the same managers as
  /// the previous (if any) helper.
  static Helper& pushInstance(Operation::Key* opsKey);

  /// Pop a helper instance off the local thread's stack.
  static void popInstance();

  /// Return the nesting level (i.e., the number of helper instances in the stack).
  ///
  /// The outermost helper will return 1 (assuming you have called instance() first).
  static std::size_t nestingDepth();

  /// Returns true if the helper is for deserializing top-level or child operations.
  bool topLevel() const { return m_topLevel; }

  /// Return the key currently being used.
  const Operation::Key* key() const { return m_key; }

protected:
  Helper();
  Operation::Key* m_key;
  /// m_topLevel indicates whether pushInstance() (false) or instance() (true)
  /// was used to create this helper.
  bool m_topLevel = true;
};

} // namespace operation
} // namespace smtk

#endif // smtk_operation_Helper_h
