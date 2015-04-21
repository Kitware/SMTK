//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_CAUUID_h
#define __smtk_session_cgm_CAUUID_h

#include "smtk/Options.h" // for CGM_HAVE_VERSION_H
#include "smtk/bridge/cgm/Exports.h" // for SMTKCGMSESSION_EXPORT
#ifdef CGM_HAVE_VERSION_H
#  include "cgm_version.h"
#endif
#include "CubitAttrib.hpp"
#include "smtk/common/UUID.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

/// An extension of the CubitAttributeType enum
enum CubitAttributeTypeExtensions
{
  CA_UUID = 128 //!< An attribute identifier; must be distinct from CubitAttributeType values.
};

/**\brief A Cubit attribute to make TDUUIDs persistent.
  *
  * Instances of this class are attached to solid model entities
  * and saved as persistent data, just as TDUniqueId uses CAUniqueId
  * to preserve IDs.
  */
class SMTKCGMSESSION_EXPORT CAUUID : public CubitAttrib
{
public:
  static void registerWithAttributeManager();
  CAUUID(RefEntity*);
#if CGM_MAJOR_VERSION >= 14
  CAUUID(RefEntity*, const CubitSimpleAttrib&);
#else
  CAUUID(RefEntity*, CubitSimpleAttrib*);
#endif
  virtual ~CAUUID();

  virtual const std::type_info& entity_type_info() const;
  virtual int int_attrib_type();
  virtual CubitStatus actuate();
  virtual CubitStatus update();
  virtual CubitStatus reset();
#if CGM_MAJOR_VERSION >= 14
  virtual CubitSimpleAttrib cubit_simple_attrib();
#else
  virtual CubitSimpleAttrib* cubit_simple_attrib();
#endif

  smtk::common::UUID entityId() const;

  static CubitStatus actuate_all();

protected:
#if CGM_MAJOR_VERSION >= 14
  static CubitAttrib* creator(RefEntity*, const CubitSimpleAttrib&);
#else
  static CubitAttrib* creator(RefEntity*, CubitSimpleAttrib*);
#endif

  smtk::common::UUID m_entityId;
};

    } // namespace cgm
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_CAUUID_h
