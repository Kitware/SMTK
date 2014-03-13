#include "smtk/model/OperatorResult.h"

namespace smtk {
  namespace model {

OperatorResult::OperatorResult()
  : m_outcome(UNABLE_TO_OPERATE)
{
}

OperatorResult::OperatorResult(OperatorOutcome oc)
  : m_outcome(oc)
{
}

OperatorOutcome OperatorResult::outcome() const
{
  return this->m_outcome;
}

  } // model namespace
} // smtk namespace
