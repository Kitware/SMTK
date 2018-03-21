//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "CGMApp.hpp"
#include "CubitAttribManager.hpp"
#include "CubitSimpleAttrib.hpp"
#include "GSaveOpen.hpp"
#include "GeometryQueryTool.hpp"
#include "RefEntity.hpp"

namespace smtk
{
namespace bridge
{
namespace cgm
{

/**\brief Register the CAUUID attribute type with the attribute manager.
  *
  * You must call this method before attempting to instantiate a CAUUID.
  */
void CAUUID::registerWithAttributeManager()
{
  static bool caUUIDRegistered = false;
  if (!caUUIDRegistered)
  {
    CGMApp::instance()->attrib_manager()->register_attrib_type(CA_UUID, "uuid", "UUID",
      &CAUUID::creator,
      CUBIT_TRUE,   // auto_actuate_flag
      CUBIT_TRUE,   // auto_update_flag
      CUBIT_TRUE,   // auto_write_flag
      CUBIT_TRUE,   // auto_read_flag
      CUBIT_TRUE,   // actuate_in_constructor
      CUBIT_FALSE); // actuate_after_geom_changes
    caUUIDRegistered = true;
  }
}

/// Construct a CAUUID and attach it to \a ref.
CAUUID::CAUUID(RefEntity* ref)
  : CubitAttrib(ref)
{
  // m_entityId is initialized to null by default
}

#if CGM_MAJOR_VERSION >= 14
/// Construct a CAUUID and restore the UUID from the simple attribute's string data.
CAUUID::CAUUID(RefEntity* ref, const CubitSimpleAttrib& sa)
  : CubitAttrib(ref)
{
  if (!sa.string_data_list().empty())
  {
    m_entityId = smtk::common::UUID(std::string(sa.string_data_list().back().c_str()));
  }
}
#else
/// Construct a CAUUID and restore the UUID from the simple attribute's string data.
CAUUID::CAUUID(RefEntity* ref, CubitSimpleAttrib* sa)
  : CubitAttrib(ref)
{
  if (sa && sa->string_data_list() && sa->string_data_list()->size() != 0)
  {
    m_entityId = smtk::common::UUID(std::string(sa->string_data_list()->last_item()->c_str()));
  }
}
#endif

CAUUID::~CAUUID()
{
}

const std::type_info& CAUUID::entity_type_info() const
{
  return typeid(CAUUID);
}

int CAUUID::int_attrib_type()
{
  return CA_UUID;
}

CubitStatus CAUUID::actuate()
{
  if (this->hasActuated == CUBIT_TRUE)
    return CUBIT_SUCCESS;

  TDUUID* uuid = dynamic_cast<TDUUID*>(this->attrib_owner()->get_TD(&TDUUID::isTDUUID));
  if (uuid)
  {
    if (uuid->entityId() != m_entityId)
    {
      std::cerr << "Different UUID found for " << attrib_owner()->class_name() << " ("
                << uuid->entityId() << " vs " << m_entityId << ")\n";
      return CUBIT_FAILURE;
    }
  }
  else
  {
    /*
    std::cout
      << " undo?   " << (GSaveOpen::performingUndo ? "Y" : "N")
      << " import? " << (GeometryQueryTool::importingSolidModel ? "Y" : "N")
      << " merge?  " << (GeometryQueryTool::mergeGloballyOnImport ? "Y" : "N")
      << "\n";
      */
    if (!GSaveOpen::performingUndo && GeometryQueryTool::importingSolidModel &&
      !GeometryQueryTool::mergeGloballyOnImport)
    {
      //Is there an entity that already has this id?
      ToolDataUser* other = TDUUID::findEntityById(m_entityId);
      if (other)
      {
        RefEntity* otherAsEnt = dynamic_cast<RefEntity*>(other);
        std::cerr << "UUID collision " << m_entityId << " between "
                  << (otherAsEnt ? otherAsEnt->class_name() : "unknown class") << " and "
                  << attrib_owner()->class_name() << "\n";
        return CUBIT_FAILURE;
      }
    }
    uuid = new TDUUID(attrib_owner(), m_entityId);
    {
      RefEntity* ownerAsEnt = dynamic_cast<RefEntity*>(attrib_owner());
      if (ownerAsEnt)
      {
        //std::cout << "Restored " << ownerAsEnt->entity_name().c_str() << " (" << ownerAsEnt->class_name() << ") " << m_entityId << "\n";
      }
    }
  }
  this->delete_attrib(CUBIT_TRUE);
  this->hasActuated = CUBIT_TRUE;

  return CUBIT_SUCCESS;
}

CubitStatus CAUUID::update()
{
  if (this->hasUpdated)
    return CUBIT_SUCCESS;
  this->hasUpdated = CUBIT_TRUE;

  TDUUID* uuid = dynamic_cast<TDUUID*>(this->attrib_owner()->get_TD(&TDUUID::isTDUUID));

  // No auto_id feature yet.
  if (!uuid)
  {
    this->delete_attrib(CUBIT_TRUE);
  }
  else
  {
    m_entityId = uuid->entityId();
    if (this->delete_attrib() == CUBIT_TRUE)
      this->delete_attrib(CUBIT_FALSE);
  }
  return CUBIT_SUCCESS;
}

CubitStatus CAUUID::reset()
{
  return CUBIT_SUCCESS;
}

#if CGM_MAJOR_VERSION >= 14
CubitSimpleAttrib CAUUID::cubit_simple_attrib()
{
  RefEntity* ownerAsEnt = dynamic_cast<RefEntity*>(attrib_owner());
  if (ownerAsEnt)
  {
    //std::cout << "Exported " << ownerAsEnt->entity_name().c_str() << " (" << ownerAsEnt->class_name() << ") " << m_entityId << "\n";
  }
  return CubitSimpleAttrib(this->att_internal_name(), m_entityId.toString().c_str());
}
#else
CubitSimpleAttrib* CAUUID::cubit_simple_attrib()
{
  RefEntity* ownerAsEnt = dynamic_cast<RefEntity*>(attrib_owner());
  if (ownerAsEnt)
  {
    //std::cout << "Exported " << ownerAsEnt->entity_name().c_str() << " (" << ownerAsEnt->class_name() << ") " << m_entityId << "\n";
  }
  return new CubitSimpleAttrib(this->att_internal_name(), m_entityId.toString().c_str());
}
#endif

smtk::common::UUID CAUUID::entityId() const
{
  return m_entityId;
}

CubitStatus CAUUID::actuate_all()
{
  std::cerr << "CAUUID actuate_all not implemented.\n";
  return CUBIT_SUCCESS;
}

#if CGM_MAJOR_VERSION >= 14
CubitAttrib* CAUUID::creator(RefEntity* entity, const CubitSimpleAttrib& p_csa)
{
  CAUUID* attrib = new CAUUID(entity, p_csa);
  return attrib;
}
#else
CubitAttrib* CAUUID::creator(RefEntity* entity, CubitSimpleAttrib* p_csa)
{
  CAUUID* attrib = p_csa ? new CAUUID(entity, p_csa) : new CAUUID(entity);
  return attrib;
}
#endif

} // namespace cgm
} //namespace bridge
} // namespace smtk
