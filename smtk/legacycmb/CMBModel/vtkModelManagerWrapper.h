#ifndef __vtkModelManagerWrapper_h
#define __vtkModelManagerWrapper_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelMultiBlockSource.h"

struct cJSON;

// .NAME vtkModelManagerWrapper - The *really* new CMB model
// .SECTION Description
// This class exists to wrap an SMTK model into a vtkObject
// subclass whose methods can be wrapped
// in order to use ParaView's client/server framework to
// synchronize remote instances.
//
// An instance of this class is tied to a vtkSMModelManagerProxy
// on the client side. They exchange information with
// proxied calls of JSON strings.
//
// Model synchronization is accomplished by serializing the
// SMTK model into a JSON string maintained as field data on
// an instance of this class.
// Operators are also serialized (1) by this instance in order
// for the client to enumerate them and (2) by the client in
// order for this object to execute them.
//
// This model also serves as a ParaView pipeline source that
// generates multiblock data of the model for rendering.
class VTKCMBDISCRETEMODEL_EXPORT vtkModelManagerWrapper : public vtkModelMultiBlockSource
{
public:
  static vtkModelManagerWrapper* New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkModelManagerWrapper,vtkModelMultiBlockSource);

  vtkGetStringMacro(JSONRequest);
  vtkSetStringMacro(JSONRequest);

  void ProcessJSONRequest();

  vtkGetStringMacro(JSONResponse);

  // Eventually allow partial model fetches like so:
  //std::string GetModelEntity(const std::string& uuid);

  std::string CanOperatorExecute(const std::string& jsonOperator);
  std::string ApplyOperator(const std::string& jsonOperator);

protected:
  vtkModelManagerWrapper();
  virtual ~vtkModelManagerWrapper();

  vtkSetStringMacro(JSONResponse);

  void GenerateError(cJSON* err, const std::string& errMsg, const std::string& reqId);

  char* JSONRequest;
  char* JSONResponse;

private:
  vtkModelManagerWrapper(const vtkModelManagerWrapper&); // Not implemented.
  void operator = (const vtkModelManagerWrapper&); // Not implemented.
};

#endif // __vtkModelManagerWrapper_h
