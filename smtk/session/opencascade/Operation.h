//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Operation_h
#define smtk_session_opencascade_Operation_h

#include "smtk/common/UUID.h"
#include "smtk/graph/Component.h"
#include "smtk/operation/ResourceManagerOperation.h"
#include "smtk/session/opencascade/Exports.h"

#include "TopoDS_Shape.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;
class Session;
class Shape;

/**\brief A parent operation for opencascade operators.
  *
  * This class exists to provide utilities common to all
  * opencascade operations; it is abstract.
  */
class SMTKOPENCASCADESESSION_EXPORT Operation : public smtk::operation::ResourceManagerOperation
{
public:
  smtkTypeMacro(smtk::session::opencascade::Operation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::ResourceManagerOperation);

protected:
  /// Use the resource and session of associated objects
  /// or create a resource and session otherwise.
  ///
  /// If a new resource is created, this method will look
  /// in the provided result object for a "resource" item
  /// and append it.
  void prepareResourceAndSession(
    Result& result, std::shared_ptr<Resource>& rsrc, std::shared_ptr<Session>& sess);

  /// Create a node of the proper type, optionally naming it.
  ///
  /// If \a mark is true, then — when appropriate (only vertices, edges, and faces) —
  /// the node will have its geometry marked as modified.
  /// If \a name is empty, one will be generated automatically.
  Shape* createNode(TopoDS_Shape& shape, Resource* resource, bool mark = true,
    const std::string& name = std::string());

  /// Add children of the given shape to the session.
  ///
  /// When creating a new top-level topological model entity
  /// (such as a solid, volume, or face) whose children have
  /// not been added to SMTK, invoke this method to transcribe
  /// the opencascade entities into SMTK.
  void iterateChildren(Shape& parent, Result& result);
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_Operation_h
