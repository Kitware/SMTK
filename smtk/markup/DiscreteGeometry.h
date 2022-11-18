//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_DiscreteGeometry_h
#define smtk_markup_DiscreteGeometry_h

#include "smtk/markup/SpatialData.h"

#include "smtk/operation/Operation.h"

#include "vtkDataObject.h"
#include "vtkSmartPointer.h"

#include <vector>

namespace smtk
{
namespace markup
{

class AssignedIds;

/** Represent a discrete geometric shape as a modeling entity.
  *
  * This class and its subclasses are implemented using VTK data.
  */
class SMTKMARKUP_EXPORT DiscreteGeometry : public smtk::markup::SpatialData
{
public:
  smtkTypeMacro(smtk::markup::DiscreteGeometry);
  smtkSuperclassMacro(smtk::markup::SpatialData);

  template<typename... Args>
  DiscreteGeometry(Args&&... args)
    : smtk::markup::SpatialData(std::forward<Args>(args)...)
  {
  }

  ~DiscreteGeometry() override;

  /// Return all of the IdSpace allotments this instance has.
  ///
  /// Generally, this will return IDs in the "point" and "cell" domains
  /// since that is how most VTK data is modeled.
  /// However, side sets and other indirect geometries may return more.
  ///
  /// It is up to each subclass to implement this method, which is
  /// used to look up domains (IdSpaces in particular) that this
  /// instance participates in.
  ///
  /// Note that callers must hold a lock on the resource for the duration
  /// that they use the resulting \a assignments as otherwise the returned
  /// assignments could be invalidated in a separate thread.
  virtual void assignedIds(std::vector<AssignedIds*>& assignments) const { (void)assignments; }

  /// Return the VTK data defining the shape of this component.
  virtual vtkSmartPointer<vtkDataObject> shape() const;

  /**\brief A base class for subclasses of DiscreteGeometry to use
    *       when passing options to updateChildren() and setShapeData().
    *
    * This allows callers of setShapeData to provide information used
    * in other methods that setShapeData calls to allocate IDs in an
    * IdSpace and index them.
    */
  class ShapeOptions
  {
  public:
    /// An operation result to insert modified objects
    /// related to the shape being assigned to this node.
    smtk::operation::Operation::Result trackedChanges;
  };

protected:
  /** Method for subclasses to use when updating their VTK data.
    *
    * You **must** call this method before modifying any internal
    * variable affecting the return value of DiscreteGeometry::shape()
    * since it will return with no effect if \a newShape matches the
    * existing shape.
    *
    * This is only intended to be called from within an operation
    * whose result is accepted here since this method will remove
    * no-longer-referenced Field nodes and add new ones. It will
    * also update the assigned point and cell IDs for data inheriting
    * vtkDataSet. If changes were made, this method returns true (and
    * adds this object to \a options.trackedChanges's "modified" item).
    *
    * Note that this method does **not** modify referential geometry
    * linked to this (primary) geometry. You are responsible for
    * doing this since the nature of your operation determines whether
    * reference geometry should be removed or rewritten to use the
    * newly-assigned IDs (which this method does **not** update for you).
    */
  bool updateChildren(vtkSmartPointer<vtkDataObject> newShape, ShapeOptions& options);
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_DiscreteGeometry_h
