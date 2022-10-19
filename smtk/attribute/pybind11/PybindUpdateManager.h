//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_UpdateManager_h
#define pybind_smtk_attribute_UpdateManager_h

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "smtk/attribute/UpdateManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/resource/Resource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::UpdateManager> pybind11_init_smtk_attribute_UpdateManager(py::module &m)
{
  PySharedPtrClass< smtk::attribute::UpdateManager> instance(m, "UpdateManager");
  instance
    .def_static("create", [](){ return smtk::attribute::UpdateManager::create(); }, py::return_value_policy::take_ownership)
    .def("registerItemUpdater", [](
        smtk::attribute::UpdateManager& manager,
        const std::string& resourceTemplateType, const std::string& attributeType,
        const std::string& itemPath,
        std::size_t appliesToVersionMin, std::size_t appliesToVersionMax,
        std::size_t producesVersion, smtk::attribute::update::ItemUpdater functor)
      {
        return
          manager.itemUpdaters(resourceTemplateType, attributeType).registerUpdater(
            itemPath, appliesToVersionMin, appliesToVersionMax, producesVersion, functor);
      }
    )
    .def("findItemUpdater", [](
        smtk::attribute::UpdateManager& manager,
        const std::string& resourceTemplateType, const std::string& attributeType,
        const std::string& itemPath,
        std::size_t producesVersion,
        std::size_t appliesToVersion)
      {
        auto updater =
          manager.itemUpdaters(resourceTemplateType, attributeType).find(
            itemPath, producesVersion, appliesToVersion);
        return updater.Update;
      }
    );
  return instance;
}

#endif
