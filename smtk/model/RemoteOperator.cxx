#include "smtk/model/RemoteOperator.h"

#include "smtk/model/DefaultBridge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Parameter.h"
#include "smtk/model/OperatorResult.h"

namespace smtk {
  namespace model {

/// Return the name of this operator.
std::string RemoteOperator::name() const
{
  return this->m_name;
}

/// Set the name of this operator (should only be called by ImportJSON::ofOperator).
RemoteOperator::Ptr RemoteOperator::setName(const std::string& opName)
{
  this->m_name = opName;
  return shared_from_this();
}

/**\brief Call the bridge's delegate method to see if the
  *       remote process can perform the operation.
  *
  * Since the actual ableToOperate method on the remote process may
  * alter the parameter values, the delegate returns an OperatorResult
  * instance that contains possibly-updated Parameter values obtained
  * remotely. Those replace the local values.
  */
bool RemoteOperator::ableToOperate()
{
  DefaultBridge* fwdBridge = dynamic_cast<DefaultBridge*>(this->bridge());
  if (!fwdBridge)
    return false;

  OperatorResult result = fwdBridge->ableToOperateDelegate(shared_from_this());
  this->m_parameters = result.parameters();
  return result.outcome() == OPERATION_SUCCEEDED ? true : false;
}

OperatorResult RemoteOperator::operateInternal()
{
  DefaultBridge* fwdBridge = dynamic_cast<DefaultBridge*>(this->bridge());
  if (!fwdBridge)
    return OperatorResult(OPERATION_FAILED);

  return fwdBridge->operateDelegate(shared_from_this());
}

/// A method to clone this instance so it may be attached to a different bridge.
Operator::Ptr RemoteOperator::cloneInternal(Operator::ConstPtr src)
{
  this->setName(src->name());
  return this->Operator::cloneInternal(src);
}

  } // model namespace
} // smtk namespace
