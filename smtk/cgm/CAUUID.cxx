#include "smtk/cgm/CAUUID.h"
#include "smtk/cgm/TDUUID.h"

#include "CGMApp.hpp"
#include "CubitAttribManager.hpp"
#include "CubitSimpleAttrib.hpp"
#include "GSaveOpen.hpp"
#include "GeometryQueryTool.hpp"
#include "RefEntity.hpp"

namespace smtk {
  namespace cgm {

CAUUID::CAUUID(RefEntity* ref)
  : CubitAttrib(ref)
{
  // m_entityId is initialized to null by default
}

CAUUID::CAUUID(RefEntity* ref, CubitSimpleAttrib* sa)
  : CubitAttrib(ref)
{
  m_entityId = smtk::util::UUID(
    std::string(sa->string_data_list()->get()->c_str()));
}

CAUUID::~CAUUID()
{
}

const type_info& CAUUID::entity_type_info() const
{
  return typeid(CAUUID);
}

int CAUUID::int_attrib_type()
{
  return CA_UUID;
}

CubitStatus CAUUID::actuate()
{
  if (this->hasActuated == CUBIT_TRUE) return CUBIT_SUCCESS;

  TDUUID* uuid = dynamic_cast<TDUUID*>(
    this->attrib_owner()->get_TD(&TDUUID::isTDUUID));
  if (uuid)
    {
    if (uuid->entityId() != this->m_entityId)
      {
      std::cerr
        << "Different UUID found for " << attrib_owner()->class_name()
        << " (" << uuid->entityId() << " vs " << this->m_entityId << ")\n";
      return CUBIT_FAILURE;
      }
    }
  else
    {
    if( !GSaveOpen::performingUndo &&
      GeometryQueryTool::importingSolidModel &&
      !GeometryQueryTool::mergeGloballyOnImport)
      {
      //Is there an entity that already has this id?
      ToolDataUser* other = TDUUID::findEntityById(this->m_entityId);
      if (other)
        {
        RefEntity* otherAsEnt = dynamic_cast<RefEntity*>(other);
        std::cerr
          << "UUID collision " << this->m_entityId << " between "
          << (otherAsEnt ? otherAsEnt->class_name() : "unknown class")
          << " and " << attrib_owner()->class_name()
          << "\n";
        return CUBIT_FAILURE;
        }
      }
    uuid = new TDUUID(attrib_owner(), this->m_entityId);
    }
  this->delete_attrib(CUBIT_TRUE);
  this->hasActuated = CUBIT_TRUE;

  return CUBIT_SUCCESS;
}

CubitStatus CAUUID::update()
{
  if (this->hasUpdated) return CUBIT_SUCCESS;
  this->hasUpdated = CUBIT_TRUE;

  TDUUID* uuid = dynamic_cast<TDUUID*>(
    this->attrib_owner()->get_TD(&TDUUID::isTDUUID));

  // No auto_id feature yet.
  if (!uuid)
    {
    this->delete_attrib(CUBIT_TRUE);
    }
  else
    {
    this->m_entityId = uuid->entityId();
    if (this->delete_attrib() == CUBIT_TRUE)
      this->delete_attrib(CUBIT_FALSE);
    }
  return CUBIT_SUCCESS;
}

CubitStatus CAUUID::reset()
{
  return CUBIT_SUCCESS;
}

CubitSimpleAttrib* CAUUID::cubit_simple_attrib()
{
  return new CubitSimpleAttrib(
    this->att_internal_name(),
    this->m_entityId.toString().c_str());
}

smtk::util::UUID CAUUID::entityId() const
{
  return this->m_entityId;
}

CubitStatus CAUUID::actuate_all()
{
  return CUBIT_SUCCESS;
}

  } // namespace cgm
} // namespace smtk
