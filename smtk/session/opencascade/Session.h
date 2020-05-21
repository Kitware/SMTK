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

#include <array>

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
  void addShape(const smtk::common::UUID& uid, TopoDS_Shape& storage);
  bool removeShape(const smtk::common::UUID& uid);

  /// Return the OpenCascade "document" for this session.
  ///
  /// The document handles management of undo/redo state and
  /// can read/write labels and attributes.
  const ::opencascade::handle<TDocStd_Document>& document() const { return m_document; }
  ::opencascade::handle<TDocStd_Document>& document() { return m_document; }

  /// Return counters used to name opencascade TopoDS_Shape items uniquely
  const std::array<std::size_t, 9>& shapeCounters() const { return m_shapeCounters; }
  std::array<std::size_t, 9>& shapeCounters() { return m_shapeCounters; }

protected:
  Session();

  std::map<smtk::common::UUID, TopoDS_Shape> m_storage;
  std::unordered_map<std::size_t, smtk::common::UUID> m_reverse;
  ::opencascade::handle<TDocStd_Document> m_document;
  std::array<std::size_t, 9> m_shapeCounters;

private:
  Session(const Session&);        // Not implemented.
  void operator=(const Session&); // Not implemented.
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_Session_h
