#ifndef __smtk_model_DefaultBridge_h
#define __smtk_model_DefaultBridge_h

#include "smtk/model/Bridge.h"

namespace smtk {
  namespace model {

/**\brief A bridge that does no transcription.
  *
  * In other words, this bridge marks models as being "native" to SMTK.
  */
class DefaultBridge : public Bridge
{
public:
  smtkTypeMacro(DefaultBridge);
  smtkCreateMacro(Bridge);
  smtkSharedFromThisMacro(Bridge);
  smtkDeclareModelingKernel();

  void backsRemoteBridge(
    const std::string& remoteBridgeName,
    const smtk::util::UUID& bridgeSessionId);
  virtual std::string remoteName() const;
  virtual OperatorPtr op(const std::string& opName, ManagerPtr manager) const;

protected:
  friend class RemoteOperator;
  friend class ImportJSON;

  virtual BridgedInfoBits transcribeInternal(const Cursor& entity, BridgedInfoBits flags);

  virtual OperatorResult ableToOperateDelegate(RemoteOperatorPtr op);
  virtual OperatorResult operateDelegate(RemoteOperatorPtr op);

  void setImportingOperators(bool isImporting);

  std::string m_remoteBridgeName;
  bool m_importingOperators;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_DefaultBridge_h
