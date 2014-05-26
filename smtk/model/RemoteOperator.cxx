#include "smtk/model/RemoteOperator.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Parameter.h"

namespace smtk {
  namespace model {

std::string RemoteOperator::name() const
{
  return this->m_name;
}

Operator::Ptr RemoteOperator::cloneInternal(Operator::ConstPtr src)
{
  this->setName(src->name());
  return this->Operator::cloneInternal(src);
}

RemoteOperator::Ptr RemoteOperator::setName(const std::string& opName)
{
  this->m_name = opName;
  return shared_from_this();
}

  } // model namespace
} // smtk namespace
