//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_OperationDecorator_h
#define smtk_view_OperationDecorator_h

#include "smtk/operation/Operation.h"
#include "smtk/string/Token.h"
#include "smtk/view/Configuration.h"

#include <functional>
#include <initializer_list>
#include <set>

namespace smtk
{
namespace view
{

/**\brief Decorate operations that should be presented to the user.
  *
  * This class should be used to whitelist, decorate, group, and/or
  * sort sets of operations that will be presented to users.
  *
  * You may construct an operation decorator in three ways:
  * 1. from an smtk::view::Configuration::Component as described below;
  * 2. from an initializer list of OperationDecorator::Entry objects; or
  * 3. with no arguments (allowing programmatic configuration).
  *
  * If you choose to configure an operation decorator using
  * view configuration information, it should follow the example
  * below
  *
  * ```xml
  *   <OperationDecorator>
  *     <!-- Include operations explicitly -->
  *     <Operation TypeRegex="smtk::model.*"/>
  *     <Operation TypeRegex="smtk::attribute.*"/>
  *     <!-- Overrides are allowed for operations like so: -->
  *     <Operation Type="smtk::geometry::MeshInspector"/>
  *     <Operation Type="smtk::geometry::ImageInspector"/>
  *     <Operation Type="smtk::operation::AssignColors">
  *       <Overrides>
  *         <Label>choose a color</Label>
  *         <ButtonLabel>choose\ncolor</Label>
  *         <Tooltip>Choose colors for parts.</Tooltip>
  *         <!-- Icon overrides are not implemented yet, but should include: -->
  *         <Icon Source="inline">(insert svg here)</Icon>
  *         <Icon Source="qrc">:/path/to/icon</Icon>
  *         <Icon Source="factory" Arguments="iconName">functorName</Icon>
  *       </Overrides>
  *     </Operation>
  *   </OperationDecorator>
  * ```
  */
class SMTKCORE_EXPORT OperationDecorator
{
public:
  /// An entry indicating how to present an operation to users.
  class Entry
  {
  public:
    /// The index of the underlying operation.
    smtk::operation::Operation::Index m_index{ 0 };
    /// An alternate name for the operation (overriding its native label).
    mutable std::string m_label;
    /// Same as m_label but with linebreaks to squarify it for presentation.
    mutable std::string m_buttonLabel;
    /// An alternate tool-tip string.
    mutable std::string m_toolTip;
    /// Groupings in which this operation should appear.
    mutable std::set<smtk::string::Token> m_groups;

    // TODO: Provide a way to override the operation's icon?

    /// An optional sort order.
    mutable int m_precedence{ -1 };

    /**\brief A constructor that takes an operation index.
      *
      * This includes the operation but does not provide any overrides.
      * The templated variant is preferred where the operation type
      * is available at compile time.
      */
    Entry(smtk::operation::Operation::Index index)
      : m_index(index)
    {
    }

    /**\brief A constructor that accepts overrides for all parameters.
      *
      * Note that \a buttonLabel is last and defaults to an empty string.
      * The templated variant is preferred where the operation type
      * is available at compile time.
      */
    Entry(
      smtk::operation::Operation::Index index,
      const std::string& label,
      const std::string& toolTip,
      const std::set<smtk::string::Token>& groups = {},
      const std::string& buttonLabel = std::string())
      : m_index(index)
      , m_label(label)
      , m_buttonLabel(buttonLabel)
      , m_toolTip(toolTip)
      , m_groups(groups)
    {
    }

    bool operator<(const Entry& other) const { return m_index < other.m_index; }

  protected:
    friend class OperationDecorator;

    /// Used by at() to create a return value for range errors.
    Entry() = default;
  };

  using Override = std::pair<bool, std::reference_wrapper<const Entry>>;

  OperationDecorator() = default;
  OperationDecorator(std::initializer_list<Entry> entries);
  OperationDecorator(
    const std::shared_ptr<smtk::operation::Manager>& manager,
    const Configuration::Component& config);
  virtual ~OperationDecorator() = default;

  /// Print debug info.
  void dump() const;

  /// Add an entry for an operation.
  int insert(const Entry& entry);

  /// Return the entry for an operation.
  template<typename OperationType>
  Override at() const
  {
    return this->at(std::type_index(typeid(OperationType)).hash_code());
  }

  /// Return the entry for an operation.
  Override at(smtk::operation::Operation::Index index) const;

  /// Return the number of operation entries.
  std::size_t size() const { return m_entries.size(); }

  static Override none();

protected:
  std::set<Entry> m_entries;
};

/**\brief An Entry-constructor that takes an operation as a parameter.
  *
  * Includes the operation but does not provide any overrides.
  * This method is a free function because templated constructors are disallowed.
  */
template<typename OperationType>
OperationDecorator::Entry wrap()
{
  OperationDecorator::Entry result(std::type_index(typeid(OperationType)).hash_code());
  return result;
}

/**\brief An Entry-constructor that accepts overrides for all parameters.
  *
  * Note that \a buttonLabel is last and defaults to an empty string.
  * This method is a free function because templated constructors are disallowed.
  */
template<typename OperationType>
OperationDecorator::Entry wrap(
  const std::string& label,
  const std::string& toolTip,
  const std::set<smtk::string::Token>& groups = {},
  const std::string& buttonLabel = std::string())
{
  OperationDecorator::Entry result(
    std::type_index(typeid(OperationType)).hash_code(), label, toolTip, groups, buttonLabel);
  return result;
}

} // namespace view
} // namespace smtk
#endif // smtk_view_OperationDecorator_h
