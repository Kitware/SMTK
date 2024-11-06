//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_json_Helper_h
#define smtk_common_json_Helper_h

#include "smtk/common/UUID.h"
#include <set>

namespace smtk
{
namespace common
{

/// A helper for controlling how common links are serialized.
///
/// There are cases where only as subset of links need to be serialized.
/// For example if an operator needs to serialize its parameter information,
/// only a subset of its specification attribute resource links would be
/// needed.
template<typename LeftIdType = smtk::common::UUID, typename RightIdType = smtk::common::UUID>
class SMTKCORE_EXPORT Helper
{
public:
  Helper() = default;
  /// Destructor is public, but you shouldn't use it.
  ~Helper() = default;
  /// Copy construction and assignment are disallowed.
  Helper(const Helper&) = delete;
  void operator=(const Helper&) = delete;

  /// Return the set of required Persistent Object
  /// Ids that should be used to determine which links
  /// should be serialized.
  ///
  std::set<LeftIdType>& requiredIds() { return m_requiredIds; }
  const std::set<LeftIdType>& requiredIds() const { return m_requiredIds; }

  /// Return true if Id are to be filter when
  /// processing links
  bool hasRequiredIds() const { return !m_requiredIds.empty(); }

  ///@{
  /// A left place holder ID indicates that a link who left hand side contains that ID
  /// should use the left placeholder text instead when serializing it
  void setLeftPlaceholderId(const LeftIdType& newId)
  {
    m_hasLeftPlaceholderId = true;
    m_leftPlaceholderId = newId;
  }

  void unsetLeftPlaceholderId() { m_hasLeftPlaceholderId = false; }

  LeftIdType leftPlaceholderId() const { return m_leftPlaceholderId; }
  bool hasLeftPlaceholderId() const { return m_hasLeftPlaceholderId; }

  const std::string& leftPlaceholderText() const { return m_leftPlaceholderText; }
  void setLeftPlaceholderText(const std::string& placeholderText)
  {
    m_leftPlaceholderText = placeholderText;
  }
  ///@}

  ///@{
  /// A right place holder ID indicates that a link who right hand side contains that ID
  /// should use the right placeholder text instead when serializing it
  void setRightPlaceholderId(const RightIdType& newId)
  {
    m_hasRightPlaceholderId = true;
    m_rightPlaceholderId = newId;
  }

  void unsetRightPlaceholderId() { m_hasRightPlaceholderId = false; }

  RightIdType rightPlaceholderId() const { return m_rightPlaceholderId; }
  bool hasRightPlaceholderId() const { return m_hasRightPlaceholderId; }

  const std::string& rightPlaceholderText() const { return m_rightPlaceholderText; }
  void setRightPlaceholderText(const std::string& placeholderText)
  {
    m_rightPlaceholderText = placeholderText;
  }
  ///@}

  /// Return the helper "singleton".
  ///
  static Helper& instance() { return s_instancedHelper; }

  /// Reset the helper's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these resources is recommended since
  /// it will free memory.
  void clear();

protected:
  std::set<LeftIdType> m_requiredIds;
  LeftIdType m_leftPlaceholderId;
  bool m_hasLeftPlaceholderId = false;
  std::string m_leftPlaceholderText = "0000000!0000!0000!0000!000000000000";
  RightIdType m_rightPlaceholderId;
  bool m_hasRightPlaceholderId = false;
  std::string m_rightPlaceholderText = "FFFFFFF!FFFF!FFFF!FFFF!FFFFFFFFFFFF";
  thread_local static smtk::common::Helper<LeftIdType, RightIdType> s_instancedHelper;
};

template<typename LeftIdType, typename RightIdType>
thread_local smtk::common::Helper<LeftIdType, RightIdType>
  Helper<LeftIdType, RightIdType>::s_instancedHelper;
template<typename LeftIdType, typename RightIdType>
void Helper<LeftIdType, RightIdType>::clear()
{
  m_requiredIds.clear();
  m_hasLeftPlaceholderId = false;
  m_hasRightPlaceholderId = false;
  m_leftPlaceholderText = "0000000!0000!0000!0000!000000000000";
  m_rightPlaceholderText = "FFFFFFF!FFFF!FFFF!FFFF!FFFFFFFFFFFF";
}

} // namespace common
} // namespace smtk

#endif // smtk_resource_json_Helper_h
