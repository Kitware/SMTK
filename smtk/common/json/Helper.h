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

#include "smtk/common/CompilerInformation.h" // for SMTK_ALWAYS_EXPORT
#include "smtk/common/UUID.h"

#include "nlohmann/json.hpp"

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
class SMTK_ALWAYS_EXPORT Helper
{
public:
  Helper() = default;
  /// Destructor is public, but you shouldn't use it.
  ~Helper() = default;
  /// Copy construction and assignment are disallowed.
  Helper(const Helper&) = delete;
  void operator=(const Helper&) = delete;

  /// Return the active helper instance.
  static Helper* instance() { return nullptr; }
  /// Create a new helper, making it active.
  static Helper* activate() { return nullptr; }
  static bool deactivate() { return false; }

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

  /// Reset the helper's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these resources is recommended since
  /// it will free memory.
  void clear();

  /// Check whether a link should be included.
  bool includeLink(const LeftIdType& leftId) const
  {
    return m_requiredIds.find(leftId) != m_requiredIds.end();
  }

  /// Return JSON for the given LeftIdType value, substituting placeholder text if needed.
  nlohmann::json serializeLeft(LeftIdType value) const
  {
    if (m_hasLeftPlaceholderId && value == m_leftPlaceholderId)
    {
      return m_leftPlaceholderText;
    }
    return value;
  }

  /// Return JSON for the given RightIdType value, substituting placeholder text if needed.
  nlohmann::json serializeRight(RightIdType value) const
  {
    if (m_hasRightPlaceholderId && value == m_rightPlaceholderId)
    {
      return m_rightPlaceholderText;
    }
    return value;
  }

  /// Return LeftIdType value, replacing placeholder text if needed.
  LeftIdType deserializeLeft(const nlohmann::json& value) const
  {
    if (
      m_hasLeftPlaceholderId && value.is_string() &&
      value.get<std::string>() == m_leftPlaceholderText)
    {
      return m_leftPlaceholderId;
    }
    return value.get<LeftIdType>();
  }

  /// Return RightIdType value, replacing placeholder text if needed.
  RightIdType deserializeRight(const nlohmann::json& value) const
  {
    if (
      m_hasRightPlaceholderId && value.is_string() &&
      value.get<std::string>() == m_rightPlaceholderText)
    {
      return m_rightPlaceholderId;
    }
    return value.get<RightIdType>();
  }

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
void Helper<LeftIdType, RightIdType>::clear()
{
  m_requiredIds.clear();
  m_hasLeftPlaceholderId = false;
  m_hasRightPlaceholderId = false;
  m_leftPlaceholderText = "0000000!0000!0000!0000!000000000000";
  m_rightPlaceholderText = "FFFFFFF!FFFF!FFFF!FFFF!FFFFFFFFFFFF";
}

/// Specialize the helper for UUIDs.
///
/// The default implementation will always return a null pointer,
/// causing from_json/to_json to fall back to serializing all links
/// with no replacement. This implementation may return a null pointer
/// if no helper has been pushed.
template<>
SMTKCORE_EXPORT Helper<smtk::common::UUID, smtk::common::UUID>*
Helper<smtk::common::UUID, smtk::common::UUID>::instance();
template<>
SMTKCORE_EXPORT Helper<smtk::common::UUID, smtk::common::UUID>*
Helper<smtk::common::UUID, smtk::common::UUID>::activate();
template<>
SMTKCORE_EXPORT bool Helper<smtk::common::UUID, smtk::common::UUID>::deactivate();

} // namespace common
} // namespace smtk

#endif // smtk_resource_json_Helper_h
