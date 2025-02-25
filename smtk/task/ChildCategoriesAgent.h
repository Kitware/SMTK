//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_ChildCategoriesAgent_h
#define smtk_task_ChildCategoriesAgent_h

#include "smtk/task/Agent.h"

#include "smtk/common/Categories.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace task
{

///\brief ChildCategoriesAgent represents category-based constraints
/// that can be used to determine if a task can be a child of the
/// agent's task.
///
class SMTKCORE_EXPORT ChildCategoriesAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::ChildCategoriesAgent);

  ChildCategoriesAgent(Task* owningTask);
  ~ChildCategoriesAgent() override = default;

  ///\brief Return the current state of the agent.
  ///
  /// This agent is always completable.
  State state() const override;

  ///\brief Configure the agent based on a provided JSON configuration.
  void configure(const Configuration& config) override;

  ///\brief Return JSON configuration for this agent's current state.
  Configuration configuration() const override;

  ///\brief Evaluates a set of categories that are associated with potential children
  /// tasks.
  CategoryEvaluation acceptsChildCategories(const std::set<std::string>& cats) const override;

  ///\brief Returns the category expression being used as the category constraint
  smtk::common::Categories::Expression& expression() { return m_expression; }

protected:
  smtk::common::Categories::Expression m_expression;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
