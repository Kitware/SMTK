//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_Component_h
#define pybind_smtk_markup_Component_h

#include <pybind11/pybind11.h>

#include "smtk/markup/Component.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::markup::Component> pybind11_init_smtk_markup_Component(py::module &m)
{
  PySharedPtrClass< smtk::markup::Component, smtk::graph::Component> instance(m, "Component");
  instance
    .def("setName", &smtk::markup::Component::setName, py::arg("name"))
    .def("groups", [](smtk::markup::Component& comp)
      {
        std::set<smtk::markup::Group*> result;
        comp.groups().visit([&result](const smtk::markup::Group* constGroup)
          {
            auto* group = const_cast<smtk::markup::Group*>(constGroup);
            if (group)
            {
              result.insert(group);
            }
          }
        );
        return result;
      }
    )
    // .def("labels", [](smtk::markup::Component& comp) { })
    // .def("groups", &smtk::markup::Component::labels)
    // .def("importedFrom", &smtk::markup::Component::labels)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::PersistentObject>& obj)
      { return std::dynamic_pointer_cast<smtk::markup::Component>(obj); })
    ;
  return instance;
}

#endif
