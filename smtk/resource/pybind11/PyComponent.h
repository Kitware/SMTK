//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_PyComponent_h
#define smtk_resource_PyComponent_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <type_traits>

#include "smtk/io/Logger.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/PythonInterpreter.h"

#include "smtk/resource/pybind11/PyResource.h"

namespace smtk
{
namespace resource
{
class PyComponent : public Component
{
public:
  PyComponent() {}
  virtual ~PyComponent() {}

  std::string typeName() const override { return m_typeName; }
  void setTypeName(const std::string& typeName) { m_typeName = typeName; }

  const Resource::Ptr resource() const override
  {
    PYBIND11_OVERLOAD_PURE(Resource::Ptr, Component, resource, );
  }

  const smtk::common::UUID& id() const override
  {
    PYBIND11_OVERLOAD_PURE(const smtk::common::UUID&, Component, id, );
  }

  bool setId(const smtk::common::UUID& newId) override
  {
    PYBIND11_OVERLOAD_PURE(bool, Component, setId, newId);
  }

  std::string name() const override { PYBIND11_OVERLOAD(std::string, Component, name, ); }

private:
  std::string m_typeName;
};
}
}

#endif
