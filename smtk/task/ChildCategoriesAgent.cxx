//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/ChildCategoriesAgent.h"

#include "smtk/io/Logger.h"

#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/Managers.h"

#include "smtk/common/json/jsonUUID.h"
#include "smtk/string/json/jsonToken.h"

namespace smtk
{
namespace task
{

ChildCategoriesAgent::ChildCategoriesAgent(Task* owningTask)
  : Agent(owningTask)
{
}

State ChildCategoriesAgent::state() const
{
  return State::Completable;
}

void ChildCategoriesAgent::configure(const Configuration& config)
{
  auto result = config.find("expression");
  if (result != config.end())
  {
    if (!m_expression.setExpression(*result))
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not set ChildCategoriesAgent expression to: " << *result);
      m_expression.reset();
    }
  }
  else
  {
    result = config.find("pass-mode");
    {
      if (result != config.end())
      {
        if (*result == "all")
        {
          m_expression.setAllPass();
        }
        else if (*result == "none")
        {
          m_expression.setAllReject();
        }
        else
        {
          // Unsupported passMode
          m_expression.reset();
        }
      }
      else
      {
        // No valid expression information
        m_expression.reset();
      }
    }
  }
}

ChildCategoriesAgent::Configuration ChildCategoriesAgent::configuration() const
{
  Configuration config = this->Superclass::configuration();
  if (m_expression.isSet())
  {
    if (m_expression.expression().empty())
    {
      if (m_expression.allPass())
      {
        config["pass-mode"] = "all";
      }
      else
      {
        config["pass-mode"] = "none";
      }
    }
    else
    {
      config["expression"] = m_expression.expression();
    }
  }
  return config;
}

ChildCategoriesAgent::CategoryEvaluation ChildCategoriesAgent::acceptsChildCategories(
  const std::set<std::string>& cats) const
{
  if (m_expression.passes(cats))
  {
    return CategoryEvaluation::Pass;
  }
  return CategoryEvaluation::Reject;
}

} // namespace task
} // namespace smtk
