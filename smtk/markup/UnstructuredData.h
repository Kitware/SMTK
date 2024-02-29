//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_UnstructuredData_h
#define smtk_markup_UnstructuredData_h

#include "smtk/markup/DiscreteGeometry.h"

#include "smtk/markup/AssignedIds.h" // for point/cell IDs

namespace smtk
{
namespace markup
{
namespace arcs
{
struct BoundariesToShapes;
}

class SMTKMARKUP_EXPORT UnstructuredData : public smtk::markup::DiscreteGeometry
{
public:
  smtkTypeMacro(smtk::markup::UnstructuredData);
  smtkSuperclassMacro(smtk::markup::DiscreteGeometry);

  template<typename... Args>
  UnstructuredData(Args&&... args)
    : smtk::markup::DiscreteGeometry(std::forward<Args>(args)...)
  {
  }

  template<typename... Args>
  UnstructuredData(
    const std::shared_ptr<AssignedIds>& pointIds,
    const std::shared_ptr<AssignedIds>& cellIds,
    Args&&... args)
    : smtk::markup::DiscreteGeometry(std::forward<Args>(args)...)
    , m_pointIds(pointIds)
    , m_cellIds(cellIds)
  {
  }

  ~UnstructuredData() override;

  class ShapeOptions : public Superclass::ShapeOptions
  {
  public:
    std::shared_ptr<AssignedIds> sharedCellIds;
    std::shared_ptr<AssignedIds> sharedPointIds;
  };

  /// Used by Resource::addNode and others immediately after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  /// Return the domains this data participates in (points and cells).
  ///
  /// Note that you should own a lock on the resource for the duration
  /// of this call plus the time you dereference any of its returned pointers.
  std::unordered_set<Domain*> domains() const override;

  /// Return the AssignedIds object given a domain.
  ///
  /// Note that you should own a lock on the resource for the duration
  /// of this call plus the time you dereference any of its returned pointers.
  AssignedIds* domainExtent(smtk::string::Token domainName) const override;

  /// Return all the point and cell IDs assigned to this instance.
  void assignedIds(std::vector<AssignedIds*>& assignments) const override;

  /// Assign the \a mesh data as this object's shape and update Field children to match.
  bool setShapeData(vtkSmartPointer<vtkDataObject> mesh, ShapeOptions& options);
  vtkSmartPointer<vtkDataObject> shapeData() const;

  const AssignedIds& pointIds() const { return *m_pointIds; }
  const AssignedIds& cellIds() const { return *m_cellIds; }

  /// Return arcs pointing to parent (higher-dimensional) shapes that this shape bounds.
  ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, OutgoingArc> parents() const;
  ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, OutgoingArc> parents();

  /// Return arcs pointing to children (lower-dimensional) shapes that bound this shape.
  ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, IncomingArc> children() const;
  ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, IncomingArc> children();

  /// Assign this node's state from \a source.
  bool assign(const smtk::graph::Component::ConstPtr& source, smtk::resource::CopyOptions& options)
    override;

protected:
  std::shared_ptr<AssignedIds> m_pointIds;
  std::shared_ptr<AssignedIds> m_cellIds;
  vtkSmartPointer<vtkDataObject> m_mesh;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_UnstructuredData_h
