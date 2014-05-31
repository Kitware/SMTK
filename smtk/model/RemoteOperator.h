#ifndef __smtk_model_RemoteOperator_h
#define __smtk_model_RemoteOperator_h

#include "smtk/util/SharedFromThis.h"
#include "smtk/model/Operator.h"

namespace smtk {
  namespace model {

class RemoteOperator;
typedef smtk::shared_ptr<RemoteOperator> RemoteOperatorPtr;

/**\brief A class for describing remote solid modeling operations.
  *
  * Applications that present model information from a separate process
  * may subclass DefaultBridge and add RemoteOperator instances to it
  * which mirror the other process' list of operators.
  * The ImportJSON class aids in this respect by creating
  * RemoteOperator instances from JSON descriptions of operators
  * when a bridge session inherits DefaultBridge.
  *
  * The ableToOperate() and operateInternal() methods of this class
  * will call delegate methods on their bridge (provided it inherits
  * DefaultBridge) so that the results may be obtained from the
  * separate process.
  * The bridge is responsible for managing communication with the
  * separate process.
  */
class SMTKCORE_EXPORT RemoteOperator : public Operator
{
public:
  smtkTypeMacro(RemoteOperator);
  smtkCreateMacro(RemoteOperator);
  smtkSharedFromThisMacro(Operator);

  virtual std::string name() const;
  virtual bool ableToOperate();

  virtual OperatorPtr clone() const
    { return create()->cloneInternal(shared_from_this()); }

protected:
  friend class ImportJSON;
  friend class DefaultBridge;

  Ptr setName(const std::string& opName);

  virtual OperatorResult operateInternal();
  virtual OperatorPtr cloneInternal(Operator::ConstPtr);

  std::string m_name;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Operator_h
