//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Component_h
#define smtk_markup_Component_h

#include "smtk/markup/Exports.h"

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/Component.h"

#include "vtkSmartPointer.h"

#include "nlohmann/json.hpp"

#include <type_traits>

class vtkDataObject;

namespace smtk
{
namespace resource
{
namespace json
{
class Helper;
}
} // namespace resource
namespace markup
{

template<typename ArcTraits, typename Constness, typename Outgoingness>
using ArcEndpointInterface = smtk::graph::ArcEndpointInterface<ArcTraits, Constness, Outgoingness>;
using ConstArc = smtk::graph::ConstArc;
using NonConstArc = smtk::graph::NonConstArc;
using IncomingArc = smtk::graph::IncomingArc;
using OutgoingArc = smtk::graph::OutgoingArc;

class AssignedIds;
class Group;
class Label;
namespace arcs
{
struct GroupsToMembers;
struct LabelsToSubjects;
struct OntologyIdentifiersToIndividuals;
struct URLsToImportedData;
} // namespace arcs

class SMTKMARKUP_EXPORT Component : public smtk::graph::Component
{
public:
  smtkTypeMacro(smtk::markup::Component);
  smtkSuperclassMacro(smtk::graph::Component);

  using Serialize = std::true_type; // Mark this node for JSON serialization.
  using Index = std::size_t;

  template<typename... Args>
  Component(Args&&... args)
    : smtk::graph::Component(std::forward<Args>(args)...)
  {
  }
  ~Component() override;

  /// Provide an initializer for resources to call after construction.
  virtual void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper);

  /// The index is a compile-time intrinsic of the derived resource; as such, it cannot be set.
  Component::Index index() const;

  /// Return the component's name.
  std::string name() const override;

  /// Set the component's name.
  bool setName(const std::string& name);

  /// A unique integer corresponding to the component type.
  static const Component::Index type_index;

  /**\brief Return the container of groups this component belongs to.
    */
  //@{
  ArcEndpointInterface<arcs::GroupsToMembers, ConstArc, IncomingArc> groups() const;
  ArcEndpointInterface<arcs::GroupsToMembers, NonConstArc, IncomingArc> groups();
  //@}

  /**\brief Return the container of labels that reference this component.
    */
  //@{
  ArcEndpointInterface<arcs::LabelsToSubjects, ConstArc, IncomingArc> labels() const;
  ArcEndpointInterface<arcs::LabelsToSubjects, NonConstArc, IncomingArc> labels();
  //@}

  /**\brief Return the container of URLs from which this component was imported (if any).
    */
  //@{
  ArcEndpointInterface<arcs::URLsToImportedData, ConstArc, IncomingArc> importedFrom() const;
  ArcEndpointInterface<arcs::URLsToImportedData, NonConstArc, IncomingArc> importedFrom();
  //@}

  /**\brief Return the container of ontology identifiers this component realizes.
    *
    * Any component may be marked as a member of an ontology class;
    * return the set of classes (ontology identifiers) this component belongs to.
    */
  //@{
  ArcEndpointInterface<arcs::OntologyIdentifiersToIndividuals, ConstArc, IncomingArc>
  ontologyClasses() const;
  ArcEndpointInterface<arcs::OntologyIdentifiersToIndividuals, NonConstArc, IncomingArc>
  ontologyClasses();
  //@}

  /// Assign this node's state from \a source.
  bool assign(const smtk::graph::Component::ConstPtr& source, smtk::resource::CopyOptions& options)
    override;

protected:
  /// A functor for changing the name of a component.
  struct ModifyName;

  /// Copy the \a sourceAssignment's IDs into the \a targetAssignment.
  ///
  /// \a sourceAssignment is assumed to originate from an external resource
  /// while \a targetAssignment is held by a component of this resource.
  /// The target domain is looked up by name from the source domain's name
  /// and the matching IDs are requested.
  /// On success, true is returned.
  /// If false is returned, the matching IDs could not be allocated from
  /// the target domain.
  bool copyAssignment(
    const AssignedIds& sourceAssignment,
    std::shared_ptr<AssignedIds>& targetAssignment);

  /// Use copyData<T>() instead. It calls this method.
  bool copyBaseData(
    const vtkSmartPointer<vtkDataObject>& baseSourceData,
    vtkSmartPointer<vtkDataObject>& baseTargetData,
    smtk::resource::CopyOptions& options);

  /// Copy the \a sourceData into the \a targetData.
  ///
  /// The \a options are used to determine whether to perform a shallow
  /// or deep copy.
  template<typename DataType>
  bool copyData(
    const vtkSmartPointer<DataType>& sourceData,
    vtkSmartPointer<DataType>& targetData,
    smtk::resource::CopyOptions& options)
  {
    vtkSmartPointer<vtkDataObject> baseTargetData = targetData;
    bool didCopy = this->copyBaseData(sourceData, baseTargetData, options);
    if (didCopy)
    {
      targetData = dynamic_cast<DataType*>(baseTargetData.GetPointer());
    }
    return didCopy;
  }

  std::string m_name;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Component_h
