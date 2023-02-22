//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_UIStateGenerator_h
#define smtk_task_UIStateGenerator_h

#include "smtk/CoreExports.h"

#include "smtk/task/Task.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace task
{

/**\brief Pure virtual class for read/write UI state data. */

class SMTKCORE_EXPORT UIStateGenerator
{
public:
  UIStateGenerator();
  ~UIStateGenerator() = default;

  /** \brief Returns state not tied to a particular task (e.g. background color). */
  virtual nlohmann::json globalState() const = 0;
  /** \brief Returns state attached to a particular \a task. */
  virtual nlohmann::json taskState(const std::shared_ptr<smtk::task::Task>& task) const = 0;

protected:
};

} // namespace task
} // namespace smtk

#endif
