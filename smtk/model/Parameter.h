#ifndef __smtk_model_Parameter_h
#define __smtk_model_Parameter_h

#include "smtk/util/UUID.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/StringData.h"
#include "smtk/model/IntegerData.h"

#include <set>
#include <string>

namespace smtk {
  namespace model {

class Parameter;
typedef std::set<Parameter> Parameters;

/// The state of validation of a parameter value.
enum ParameterValidState
{
  PARAMETER_UNKNOWN = -1,  //!< The parameter has been modified since last marked.
  PARAMETER_INVALID = 0,   //!< The owning Operator has marked the value as invalid.
  PARAMETER_VALIDATED = 1  //!< The owning Operator has marked the value as valid.
};

/// A parameter to be used by an smtk::model::Operator.
class SMTKCORE_EXPORT Parameter
{
public:
  Parameter();
  Parameter(const std::string& name);
  Parameter(const std::string& name, const Float& val);
  Parameter(const std::string& name, const FloatList& val);
  Parameter(const std::string& name, const String& val);
  Parameter(const std::string& name, const StringList& val);
  Parameter(const std::string& name, const Integer& val);
  Parameter(const std::string& name, const IntegerList& val);
  Parameter(const std::string& name, const smtk::util::UUID& val);
  Parameter(const std::string& name, const smtk::util::UUIDArray& val);

  std::string name() const;
  ParameterValidState validState() const;

  FloatList& floatValues() { return this->m_floatVals; }
  const FloatList& floatValues() const { return this->m_floatVals; }
  void setFloatValue(const Float& sval);
  void setFloatValue(const FloatList& sval);

  StringList& stringValues() { return this->m_stringVals; }
  const StringList& stringValues() const { return this->m_stringVals; }
  void setStringValue(const String& sval);
  void setStringValue(const StringList& sval);

  IntegerList& integerValues() { return this->m_integerVals; }
  const IntegerList& integerValues() const { return this->m_integerVals; }
  void setIntegerValue(const Integer& sval);
  void setIntegerValue(const IntegerList& sval);

  smtk::util::UUIDArray& uuidValues() { return this->m_uuidVals; }
  const smtk::util::UUIDArray& uuidValues() const { return this->m_uuidVals; }
  void setUUIDValue(const smtk::util::UUID& sval);
  void setUUIDValue(const smtk::util::UUIDArray& sval);

  bool operator < (const Parameter& other) const;

protected:
  friend class Operator;
  friend class OperatorResult;

  void setValidState(ParameterValidState s);

  ParameterValidState m_state;
  String m_name;
  FloatList m_floatVals;
  StringList m_stringVals;
  IntegerList m_integerVals;
  smtk::util::UUIDArray m_uuidVals;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Parameter_h
