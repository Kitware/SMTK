//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkAttribute.h - Represents a standalone piece of simulation information
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_simulation_UserData_h
#define __smtk_simulation_UserData_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace simulation
{
//derive from this class to create custom user data.
class SMTKCORE_EXPORT UserData
{
public:
  static smtk::simulation::UserDataPtr New()
  {
    return smtk::simulation::UserDataPtr(new UserData());
  }

  virtual ~UserData();

protected:
  UserData();
};

// User Data Representing Integers
class SMTKCORE_EXPORT UserDataInt : public UserData
{
public:
  static smtk::simulation::UserDataPtr New()
  {
    return smtk::simulation::UserDataPtr(new UserDataInt());
  }

  int value() const { return m_value; }

  void setValue(int val) { m_value = val; }

  ~UserDataInt() override;

protected:
  UserDataInt();
  int m_value;
};

// User Data Representing Doubles
class SMTKCORE_EXPORT UserDataDouble : public UserData
{
public:
  static smtk::simulation::UserDataPtr New()
  {
    return smtk::simulation::UserDataPtr(new UserDataDouble());
  }

  double value() const { return m_value; }

  void setValue(double val) { m_value = val; }

  ~UserDataDouble() override;

protected:
  UserDataDouble();
  double m_value;
};

// User Data Representing Strings
class SMTKCORE_EXPORT UserDataString : public UserData
{
public:
  static smtk::simulation::UserDataPtr New()
  {
    return smtk::simulation::UserDataPtr(new UserDataString());
  }

  const std::string& value() const { return m_value; }

  void setValue(const std::string& val) { m_value = val; }

  ~UserDataString() override;

protected:
  UserDataString();
  std::string m_value;
};
} // namespace simulation
} // namespace smtk

#endif // __smtk_simulation_UserData_h
