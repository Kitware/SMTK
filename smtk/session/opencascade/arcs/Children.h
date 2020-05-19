//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Children_h
#define smtk_session_opencascade_Children_h

#include "smtk/session/opencascade/Exports.h"

#include "smtk/graph/Resource.h"
#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class SMTKOPENCASCADESESSION_EXPORT Children
{
public:
  typedef Shape FromType;
  typedef Shape ToType;
  typedef std::vector<std::reference_wrapper<const ToType> > Container;

  Children(const FromType& from)
    : m_from(from)
  {
  }

  const FromType& from() const { return m_from; }
  Container to() const;
  bool visit(std::function<bool(const ToType&)>) const;
  bool visit(std::function<bool(ToType&)>);

  template <typename SelfType>
  class API
  {
  protected:
    SelfType& self(const FromType& lhs) const
    {
      auto& arcs = std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())->arcs();
      if (!arcs.template contains<SelfType>(lhs.id()))
      {
        arcs.template emplace<SelfType>(lhs.id(), lhs);
      }
      return arcs.template get<SelfType>().at(lhs.id());
    }

  public:
    Container get(const FromType& lhs) const { return self(lhs).to(); }

    bool contains(const FromType& lhs) const
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .contains(lhs.id());
    }

    void visit(const FromType& lhs) const {}
  };

private:
  const FromType& m_from;
};
}
}
}

#endif
