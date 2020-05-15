//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Session_h
#define smtk_session_opencascade_Session_h

#include "smtk/SharedFromThis.h"
#include "smtk/common/UUID.h"
#include "smtk/graph/Component.h"
#include "smtk/session/opencascade/Exports.h"

#include "Standard_Handle.hxx"
#include "TDocStd_Document.hxx"
#include "TopoDS_Shape.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;

/**\brief The OpenCASCADE (OCC) session holds maps to and from
  *       OCC shapes and SMTK model nodes.
  *
  * This bidirectional map is created and updated by operations
  * as needed.
  */
class SMTKOPENCASCADESESSION_EXPORT Session : smtkEnableSharedPtr(Session)
{
public:
  smtkTypeMacroBase(smtk::session::opencascade::Session);
  smtkCreateMacro(smtk::session::opencascade::Session);

  virtual ~Session();

  const TopoDS_Shape* findShape(const smtk::common::UUID& uid) const;
  TopoDS_Shape* findShape(const smtk::common::UUID& uid);
  smtk::common::UUID findID(const TopoDS_Shape& shape) const;
  void addStorage(const smtk::common::UUID& uid, TopoDS_Shape& storage);
  bool removeStorage(const smtk::common::UUID& uid);

  /// Return the OpenCascade "document" for this session.
  ///
  /// The document handles management of undo/redo state and
  /// can read/write labels and attributes.
  const ::opencascade::handle<TDocStd_Document>& document() const { return m_document; }
  ::opencascade::handle<TDocStd_Document>& document() { return m_document; }

protected:
  Session();

  /// A functor to hash TopoDS_Shape objects into std::size_t
  /// (as opposed to Standard_Integer).
  ///
  /// Note that TopoDS_Shape::HashCode takes an argument (related to the number
  /// of buckets in OCC's maps) that we do not use (unordered_map wants the
  /// hash code to be wide at all times) but must provide a sane default for --
  /// one that fits in both OCC's Standard_Integer and std::size_t, whichever
  /// is smaller.
  struct ShapeHash
  {
    std::size_t operator()(const TopoDS_Shape& shape) const
    {
      using bigger_int_t = std::common_type<std::size_t, Standard_Integer>::type;
      constexpr bigger_int_t stmbi = std::numeric_limits<std::size_t>::max();
      constexpr bigger_int_t simbi = std::numeric_limits<Standard_Integer>::max();
      constexpr bigger_int_t hashmax = stmbi < simbi ? stmbi : simbi;
      auto code = shape.HashCode(static_cast<Standard_Integer>(hashmax));
      return static_cast<std::size_t>(code);
    }
  };

  std::map<smtk::common::UUID, TopoDS_Shape> m_storage;
  std::unordered_map<TopoDS_Shape, smtk::common::UUID, ShapeHash> m_reverse;
  ::opencascade::handle<TDocStd_Document> m_document;

private:
  Session(const Session&);        // Not implemented.
  void operator=(const Session&); // Not implemented.
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_Session_h
