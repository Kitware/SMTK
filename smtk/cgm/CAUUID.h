#ifndef __smtk_cgm_CAUUID_h
#define __smtk_cgm_CAUUID_h

#include "CubitAttrib.hpp"
#include "smtk/util/UUID.h"

namespace smtk {
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
class CAUUID : public CubitAttrib
{
public:
  CAUUID(RefEntity*);
  CAUUID(RefEntity*, CubitSimpleAttrib*);
  virtual ~CAUUID();

  virtual const type_info& entity_type_info() const;
  virtual int int_attrib_type();
  virtual CubitStatus actuate();
  virtual CubitStatus update();
  virtual CubitStatus reset();
  virtual CubitSimpleAttrib* cubit_simple_attrib();

  smtk::util::UUID entityId() const;

  static CubitStatus actuate_all();

protected:
  smtk::util::UUID m_entityId;
};

  } // namespace cgm
} // namespace smtk

#endif // __smtk_cgm_CAUUID_h
