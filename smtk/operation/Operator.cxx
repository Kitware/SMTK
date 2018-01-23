//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Operator.h"

namespace smtk
{
namespace operation
{

Operator::Operator()
{
}

Operator::~Operator()
{
}

bool Operator::ableToOperate()
{
  return false;
}

Operator::Result Operator::operate()
{
  Operator::Result result;
  return result;
}

void Operator::observe(EventType, Callback, void*)
{
}

void Operator::observe(EventType, CallbackWithResult, void*)
{
}

void Operator::unobserve(EventType, Callback, void*)
{
}

void Operator::unobserve(EventType, CallbackWithResult, void*)
{
}

int Operator::trigger(EventType)
{
  int status = 0;
  return status;
}

int Operator::trigger(EventType, const Operator::Result&)
{
  return 0;
}

Operator::Definition Operator::definition() const
{
  return Operator::Definition();
}

Operator::Specification Operator::specification() const
{
  return this->m_specification;
}

bool Operator::setSpecification(attribute::AttributePtr)
{
  return true;
}

bool Operator::ensureSpecification() const
{
  return false;
}

Operator::Result Operator::createResult(Operator::Outcome)
{
  return Operator::Result();
}

void Operator::setResultOutcome(Operator::Result, Operator::Outcome)
{
}

bool Operator::operator<(const Operator&) const
{
  return true;
}

void Operator::eraseResult(Operator::Result)
{
}

void Operator::setManager(ManagerPtr)
{
}

void Operator::postProcessResult(Operator::Result&)
{
}

void Operator::copyModelEntityItemToComponentItem(
  Operator::Result&, const std::string&, const std::string&)
{
}

void Operator::generateSummary(Operator::Result&)
{
}

std::string outcomeAsString(int)
{
  return "outcome unknown";
}

Operator::Outcome stringToOutcome(const std::string&)
{
  return Operator::OUTCOME_UNKNOWN;
}

} // model namespace
} // smtk namespace
