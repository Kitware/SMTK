#ifndef __smtk_model_RemoteOperator_h
#define __smtk_model_RemoteOperator_h

#include "smtk/util/SharedFromThis.h"
#include "smtk/model/Operator.h"

namespace smtk {
  namespace model {

class RemoteOperator;
typedef smtk::shared_ptr<RemoteOperator> RemoteOperatorPtr;

/**\brief A base class for describing remote solid modeling operations.
  *
  * This class serves as a base class for applications that present
  * model information from a separate process.
  * Subclasses must implement name(), ableToOperate(), and operateInternal().
  * Additionally, their constructors should call setParameter() with
  * the list of accepted parameters.
  */
class SMTKCORE_EXPORT RemoteOperator : public Operator
{
public:
  smtkTypeMacro(RemoteOperator);
  smtkSharedFromThisMacro(Operator);

  virtual std::string name() const;

protected:
  std::string m_name;

  virtual OperatorPtr cloneInternal(Operator::ConstPtr);
  Ptr setName(const std::string& opName);
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Operator_h
