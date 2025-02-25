//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_categories_Evaluators_h
#define smtk_common_categories_Evaluators_h

#include "smtk/io/Logger.h"

#include <iostream>
#include <set>
#include <stack>
#include <string>

namespace smtk
{
namespace common
{
namespace categories
{

class SMTKCORE_EXPORT Evaluators
{
public:
  typedef std::function<bool(const std::set<std::string>& catNames)> Eval;

  void pushEval(Eval e)
  {
    m_evalStack.push(e);
    this->processOps();
  }

  void pushOp(const char& op)
  {
    // Simple optimization - if we are trying to push a complement
    // operation onto the stack and there is a complement operation
    // on the top - then instead of pushing the second we will remove
    // in the stack
    if ((op == '!') && (!m_currentOps.empty()) && (m_currentOps.top() == '!'))
    {
      m_currentOps.pop();
    }
    else
    {
      m_currentOps.push(op);
    }
  }

  void startSubExpression()
  {
    m_subExpressionOps.push(m_currentOps);
    m_currentOps = {};
  }

  void endSubExpression()
  {
    if (!m_currentOps.empty())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Category Expression: Ending Sub Expression and Op stack is not empty!\n");
    }
    else if (m_subExpressionOps.empty())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Category Expression: Ending Sub Expression and sub Ops stack is empty!\n");
    }
    else
    {
      m_currentOps = m_subExpressionOps.top();
      m_subExpressionOps.pop();
      this->processOps();
    }
  }

  void processOps()
  {
    char op;
    while (!m_currentOps.empty())
    {
      op = m_currentOps.top();
      m_currentOps.pop();
      switch (op)
      {
        case '!':
          if (m_evalStack.empty())
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Category Expression: Can not perform Complement - Eval Stack is empty!\n");
          }
          else
          {
            auto e = m_evalStack.top();
            m_evalStack.pop();
            m_evalStack.push([e](const std::set<std::string>& catNames) { return !e(catNames); });
          }
          break;
        case '&':
          if (m_evalStack.size() < 2)
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Category Expression: Can not perform And - Eval Stack contains only "
                << m_evalStack.size() << " entries!\n");
          }
          else
          {
            auto a = m_evalStack.top();
            m_evalStack.pop();
            auto b = m_evalStack.top();
            m_evalStack.pop();
            m_evalStack.push([a, b](const std::set<std::string>& catNames) {
              return (a(catNames) && b(catNames));
            });
          }
          break;
        case '|':
          if (m_evalStack.size() < 2)
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Category Expression: Can not perform Or - Eval Stack contains only "
                << m_evalStack.size() << " entries!\n");
          }
          else
          {
            auto a = m_evalStack.top();
            m_evalStack.pop();
            auto b = m_evalStack.top();
            m_evalStack.pop();
            m_evalStack.push([a, b](const std::set<std::string>& catNames) {
              return (a(catNames) || b(catNames));
            });
          }
          break;
        default:
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Category Expression: Unsupported Operation: " << op);
      }
    }
  }

  bool isValid() const
  {
    return (m_evalStack.size() == 1) && m_currentOps.empty() && m_subExpressionOps.empty();
  }

  Eval top() const
  {
    if (m_evalStack.empty())
    {
      return ([](const std::set<std::string>&) { return false; });
    }
    return m_evalStack.top();
  }

  void addCategoryName(const std::string& name) { m_categoryNames.insert(name); }

  const std::set<std::string>& categoryNames() const { return m_categoryNames; }

private:
  std::stack<std::stack<char>> m_subExpressionOps;
  std::stack<char> m_currentOps;
  std::stack<Eval> m_evalStack;
  std::set<std::string> m_categoryNames;
};
} // namespace categories
} // namespace common
} // namespace smtk

#endif
