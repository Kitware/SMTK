//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_ImageData_h
#define smtk_markup_ImageData_h

#include "smtk/markup/DiscreteGeometry.h"

#include "smtk/markup/AssignedIds.h" // for point/cell IDs

#include "vtkImageData.h"

namespace smtk
{
namespace markup
{

class SMTKMARKUP_EXPORT ImageData : public smtk::markup::DiscreteGeometry
{
public:
  smtkTypeMacro(smtk::markup::ImageData);
  smtkSuperclassMacro(smtk::markup::DiscreteGeometry);

  template<typename... Args>
  ImageData(Args&&... args)
    : smtk::markup::DiscreteGeometry(std::forward<Args>(args)...)
  {
  }

  template<typename... Args>
  ImageData(
    const std::shared_ptr<AssignedIds>& pointIds,
    const std::shared_ptr<AssignedIds>& cellIds,
    Args&&... args)
    : smtk::markup::DiscreteGeometry(std::forward<Args>(args)...)
    , m_pointIds(pointIds)
    , m_cellIds(cellIds)
  {
  }

  ~ImageData() override;

  /// This is called immediately after construction by Resource::add().
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

  /// Provide all the IDs (point and cell) this data participates in.
  void assignedIds(std::vector<AssignedIds*>& assignments) const override;

  const AssignedIds& pointIds() const { return *m_pointIds; }
  const AssignedIds& cellIds() const { return *m_cellIds; }

  /// Replace the image's shape with a new one.
  ///
  /// This will call updateChildren() to add and remove Fields
  /// in order to match the point-, cell-, and field-data arrays
  /// in the new \a image.
  ///
  /// Afterward, it also releases its old assigned IDs (if any)
  /// and requests new ones.
  ///
  /// Do not call this method outside of an operation and be aware
  /// that it _may_ modify m_pointIds and m_cellIds as well.
  bool setShapeData(vtkSmartPointer<vtkImageData> image, Superclass::ShapeOptions& options);
  vtkSmartPointer<vtkImageData> shapeData() const { return m_image; }

  /// Assign this node's state from \a source.
  bool assign(const smtk::graph::Component::ConstPtr& source, smtk::resource::CopyOptions& options)
    override;

protected:
  std::shared_ptr<AssignedIds> m_pointIds;
  std::shared_ptr<AssignedIds> m_cellIds;
  vtkSmartPointer<vtkImageData> m_image;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_ImageData_h
