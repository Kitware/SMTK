//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Session_h
#define __smtk_model_Session_h
/*! \file */

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace io
{
class Logger;
}
namespace model
{

/// Bit-vector combinations of SessionInformation values for requesting information to transcribe.
typedef unsigned long SessionInfoBits;

class ArrangementHelper;
class EntityRef;
class Group;
class CellEntity;
class UseEntity;
class Instance;
class ShellEntity;
class Model;
class Session;
class SessionRef;
typedef std::map<smtk::common::UUID, smtk::shared_ptr<Session> > UUIDsToSessions;
typedef std::map<smtk::model::EntityRef, SessionInfoBits> DanglingEntities;

/**\brief Bit flags describing types of information sessiond to Resource.
  *
  * Session classes should provide individual translation for
  * each piece of information, but are allowed to transcribe
  * additional information when it is efficient or necessary
  * to do so.
  * For example, it does not make sense for an Entity record's
  * relations to be populated but not the bit-flag describing
  * its type. Thus, requesting SESSION_ENTITY_RELATIONS usually
  * also results in SESSION_ENTITY_TYPE being transcribed.
  */
enum SessionInformation
{
  // Basic types of information in smtk::model::Resource
  SESSION_ENTITY_TYPE = 0x00000001,            //!< Transcribe the Entity type.
  SESSION_ENTITY_RELATIONS = 0x00000002,       //!< Transcribe the Entity relationship vector.
  SESSION_ARRANGEMENTS = 0x00000004,           //!< Arrangement information for the relationships.
  SESSION_TESSELLATION = 0x00000008,           //!< Points and triangles used to render an entity.
  SESSION_FLOAT_PROPERTIES = 0x00000010,       //!< Floating-point properties.
  SESSION_STRING_PROPERTIES = 0x00000020,      //!< String properties.
  SESSION_INTEGER_PROPERTIES = 0x00000040,     //!< Integer properties.
  SESSION_ATTRIBUTE_ASSOCIATIONS = 0x00000080, //!< Attribute associations.

  // Extended options specific to Resource::erase():
  SESSION_USER_DEFINED_PROPERTIES =
    0x00000100, /**< Remove user-defined as well as machine-generated properties.
                                                 *
                                                 *  This bit is not used during transcription; it is only
                                                 *  significant when erasing entities. Usually, it is not
                                                 *  specified so that properties such as user-assigned
                                                 *  names, colors, and visibility are preserved.
                                                 */
  // Common combinations
  SESSION_NOTHING = 0x00000000,       //!< Transcribe nothing.
  SESSION_ENTITY_RECORD = 0x00000003, //!< Transcribe both entity type and relations.
  SESSION_ENTITY_ARRANGED =
    0x00000007, //!< Transcribe the entity record and all arrangement information.
  SESSION_PROPERTIES = 0x00000070, //!< Transcribe all properties.
  SESSION_EVERYTHING = 0x000000ff, //!< Transcribe all information about the entity.
  SESSION_EXHAUSTIVE =
    0x000001ff //!< Erase **all** information about the entity, including user-specified.
};

/**\brief A base class for bridging modelers into SMTK.
  *
  * SMTK can act as a session between other (foreign) solid modelers
  * and client applications.
  * The session must provide techniques for attaching UUIDs to foreign model
  * entities (on its own or by using facilities provided by the foreign modeler)
  * and for obtaining notification when foreign model entities are modified or
  * destroyed. In extreme cases, the SMTK model resource must be reset after
  * each modeling operation to guarantee a consistent model.
  *
  * Instances of Session subclasses should be registered with a
  * model using Resource::sessionModel(). Then, when an
  * entity cannot be resolved from a UUID created by
  * the session, the \a transcribe method will be invoked
  * to request that the session add an entry.
  *
  * This class is not intended for external use.
  * Public methods are intended for invocation by the
  * Resource instance which owns the session.
  * Protected methods are either called internally or
  * by subclasses in order to track UUIDs for which there
  * is only partial information in Resource.
  *
  * \sa smtk::model::SessionInformation
  * \sa smtkDeclareModelingKernel smtkImplementsModelingKernel
  */
class SMTKCORE_EXPORT Session : smtkEnableSharedPtr(Session)
{
public:
  smtkTypeMacroBase(smtk::model::Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);

  static std::string staticClassName() { return "smtk::model::Session"; }
  virtual std::string name() const;
  virtual std::string className() const { return Session::staticClassName(); }
  smtk::common::UUID sessionId() const;

  int transcribe(
    const EntityRef& entity, SessionInfoBits flags, bool onlyDangling = true, int depth = -1);

  virtual SessionInfoBits allSupportedInformation() const;

  const DanglingEntities& danglingEntities() const;
  void declareDanglingEntity(const EntityRef& ent, SessionInfoBits present = 0);

  virtual int setup(const std::string& optName, const StringList& optVal);

  ResourcePtr resource() const;
  smtk::io::Logger& log();

  virtual ~Session();

  virtual bool splitEntity(const EntityRef& from, const EntityRefs& to) const
  {
    bool ok = this->splitAttributes(from, to);
    ok &= this->splitProperties(from, to);
    return ok;
  }
  virtual bool mergeEntities(const EntityRefs& from, EntityRef& to) const
  {
    bool ok = this->mergeAttributes(from, to);
    ok &= this->mergeProperties(from, to);
    return ok;
  }

  virtual bool splitAttributes(const EntityRef& from, const EntityRefs& to) const;
  virtual bool mergeAttributes(const EntityRefs& from, EntityRef& to) const;

  virtual bool removeGeneratedProperties(const EntityRef& ent, SessionInfoBits propFlags);
  virtual bool splitProperties(const EntityRef& from, const EntityRefs& to) const;
  virtual bool mergeProperties(const EntityRefs& from, EntityRef& to) const;

  virtual std::string defaultFileExtension(const Model& model) const;

protected:
  friend class Resource;

  Session();

  virtual SessionInfoBits transcribeInternal(
    const EntityRef& entity, SessionInfoBits flags, int depth = -1);

  void setSessionId(const smtk::common::UUID& sessId);
  void setResource(Resource* resource);

  virtual EntityPtr addEntityRecord(const EntityRef& entRef);
  virtual ArrangementHelper* createArrangementHelper();
  int findOrAddRelatedEntities(
    const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper);
  virtual int findOrAddCellAdjacencies(
    const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddCellUses(
    const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddOwningCell(
    const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddShellAdjacencies(
    const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddUseAdjacencies(
    const ShellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddGroupOwner(
    const Group& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddFreeCells(
    const Model& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedModels(
    const Model& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddPrototype(
    const Instance& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedModels(
    const SessionRef& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedGroups(
    const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual int findOrAddRelatedInstances(
    const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper);
  virtual SessionInfoBits findOrAddArrangements(
    const EntityRef& entRef, EntityPtr entRec, SessionInfoBits flags, ArrangementHelper* helper);
  virtual SessionInfoBits updateProperties(
    const EntityRef& entRef, EntityPtr entRec, SessionInfoBits flags, ArrangementHelper* helper);
  virtual SessionInfoBits updateTessellation(
    const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper);

  virtual SessionIOPtr createIODelegate(const std::string& format);

  DanglingEntities m_dangling;
  smtk::common::UUID m_sessionId;
  Resource* m_resource;
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_Session_h
