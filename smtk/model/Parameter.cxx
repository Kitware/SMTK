#include "smtk/model/Parameter.h"

namespace smtk {
  namespace model {

Parameter::Parameter()
  : m_state(PARAMETER_UNKNOWN), m_name("unnamed")
{
}

Parameter::Parameter(const std::string& pname)
  : m_state(PARAMETER_UNKNOWN), m_name(pname)
{
}

Parameter::Parameter(const std::string& pname, const Float& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_floatVals(1, val)
{
}

Parameter::Parameter(const std::string& pname, const FloatList& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_floatVals(val)
{
}

Parameter::Parameter(const std::string& pname, const String& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_stringVals(1, val)
{
}

Parameter::Parameter(const std::string& pname, const StringList& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_stringVals(val)
{
}

Parameter::Parameter(const std::string& pname, const Integer& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_integerVals(1, val)
{
}

Parameter::Parameter(const std::string& pname, const IntegerList& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_integerVals(val)
{
}

Parameter::Parameter(const std::string& pname, const smtk::util::UUID& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_uuidVals(1, val)
{
}

Parameter::Parameter(const std::string& pname, const smtk::util::UUIDArray& val)
  : m_state(PARAMETER_UNKNOWN), m_name(pname), m_uuidVals(val)
{
}

/// Return the name of the parameter.
std::string Parameter::name() const
{
  return this->m_name;
}

/**\brief Returns whether an Operator has validated the parameter value.
  *
  * The default value is PARAMETER_UNKNOWN.
  * The state is reset to the default whenever the parameter value is changed.
  */
ParameterValidState Parameter::validState() const
{
  return this->m_state;
}

void Parameter::setFloatValue(const Float& fval)
{
  this->m_floatVals.clear();
  this->m_floatVals.push_back(fval);
  this->setValidState(PARAMETER_UNKNOWN);
}

void Parameter::setFloatValue(const FloatList& fval)
{
  this->m_floatVals = fval;
  this->setValidState(PARAMETER_UNKNOWN);
}

void Parameter::setStringValue(const String& sval)
{
  this->m_stringVals.clear();
  this->m_stringVals.push_back(sval);
  this->setValidState(PARAMETER_UNKNOWN);
}

void Parameter::setStringValue(const StringList& sval)
{
  this->m_stringVals = sval;
  this->setValidState(PARAMETER_UNKNOWN);
}

void Parameter::setIntegerValue(const Integer& ival)
{
  this->m_integerVals.clear();
  this->m_integerVals.push_back(ival);
  this->setValidState(PARAMETER_UNKNOWN);
}

void Parameter::setIntegerValue(const IntegerList& ival)
{
  this->m_integerVals = ival;
  this->setValidState(PARAMETER_UNKNOWN);
}

void Parameter::setUUIDValue(const smtk::util::UUID& ival)
{
  this->m_uuidVals.clear();
  this->m_uuidVals.push_back(ival);
  this->setValidState(PARAMETER_UNKNOWN);
}

void Parameter::setUUIDValue(const smtk::util::UUIDArray& ival)
{
  this->m_uuidVals = ival;
  this->setValidState(PARAMETER_UNKNOWN);
}

bool Parameter::operator < (const Parameter& other) const
{
  return this->name() < other.name();
}

void Parameter::setValidState(ParameterValidState s)
{
  this->m_state = s;
}

  } // model namespace
} // smtk namespace
