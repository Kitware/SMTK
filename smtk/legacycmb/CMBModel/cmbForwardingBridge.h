#ifndef __cmbForwardingBridge_h
#define __cmbForwardingBridge_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "smtk/model/DefaultBridge.h"

class vtkSMModelManagerProxy;

class VTKCMBDISCRETEMODEL_EXPORT cmbForwardingBridge : public smtk::model::DefaultBridge
{
public:
  smtkTypeMacro(cmbForwardingBridge);
  smtkCreateMacro(cmbForwardingBridge);
  smtkSharedFromThisMacro(Bridge);
  smtkDeclareModelingKernel();

  virtual ~cmbForwardingBridge();

  vtkSMModelManagerProxy* proxy() { return this->m_proxy; }
  virtual void setProxy(vtkSMModelManagerProxy* proxy);

protected:
  cmbForwardingBridge();

  virtual smtk::model::BridgedInfoBits transcribeInternal(const smtk::model::Cursor& entity, smtk::model::BridgedInfoBits flags);
  virtual bool ableToOperateDelegate(smtk::model::RemoteOperatorPtr op);
  virtual smtk::model::OperatorResult operateDelegate(smtk::model::RemoteOperatorPtr op);

  vtkSMModelManagerProxy* m_proxy;

private:
  cmbForwardingBridge(const cmbForwardingBridge&); // Not implemented.
  void operator = (const cmbForwardingBridge&); // Not implemented.
};

#endif // __cmbForwardingBridge_h
