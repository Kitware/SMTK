//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Resource_h
#define smtk_session_opencascade_Resource_h

#include "smtk/graph/Resource.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/Traits.h"

#include "smtk/session/opencascade/Exports.h"

#include <TopoDS_Compound.hxx>

namespace smtk
{
namespace session
{
/// OpenCASCADE session
namespace opencascade
{

// Forward-declare node types
class Vertex;
class Edge;
class Face;
class Volume;
class Model;
class Shape;

// Forward-declare bond types
struct CellBoundary;
struct FreeCell;

using GraphResource = smtk::graph::Resource<Traits>;
/**\brief A resource for boundary representations via OpenCASCADE.
  *
  */
class SMTKOPENCASCADESESSION_EXPORT Resource
  : public smtk::resource::DerivedFrom<smtk::session::opencascade::Resource, GraphResource>
{
public:
  smtkTypeMacro(smtk::session::opencascade::Resource);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<Resource, GraphResource>);
  smtkSharedPtrCreateMacro(smtk::resource::PersistentObject);

  virtual ~Resource() = default;

  const Session::Ptr& session() const { return m_session; }
  void setSession(const Session::Ptr&);

  /// set/get this resource's top-level modeling object
  TopoDS_Compound& compound() { return m_compound; }
  const TopoDS_Compound& compound() const { return m_compound; }
  void setCompound(const TopoDS_Compound& compound) { m_compound = compound; }

  // wrap to avoid name conflict in msvc
  template <typename componentT>
  smtk::shared_ptr<componentT> createShape()
  {
    return GraphResource::create<componentT>();
  }

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);

  TopoDS_Compound m_compound;

  Session::Ptr m_session;
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_Resource_h
