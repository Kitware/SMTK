#include "smtk/model/BridgeBase.h"


namespace smtk {
  namespace model {

/**\brief Transcribe an entity from a foreign modeler into SMTK storage.
  *
  */
int BridgeBase::transcribe(const Cursor& entity, BridgedInfoBits requested)
{
  int retval = 0;
  if (requested)
    {
    DanglingEntities::iterator it = this->m_dangling.find(entity);
    if (it == this->m_dangling.end())
      { // The bridge has not been told that this UUID exists.
      return retval;;
      }
    BridgedInfoBits actual(requested);
    retval = this->transcribeInternal(entity, actual);
    if (retval)
      {
      if ((actual & this->allSupportedInformation()) == this->allSupportedInformation())
        {
        // The call succeeded and every possible bit of information has
        // been transcribed; thus the entity is no longer dangling.
        this->m_dangling.erase(it);
        }
      }
    }
  return retval;
}

/**\brief Return a bit vector describing what types of information can be transcribed.
  *
  * This is used to determine when an entity has been fully transcribed into storage
  * and is no longer "dangling."
  */
BridgedInfoBits BridgeBase::allSupportedInformation() const
{
  return BRIDGE_EVERYTHING;
}

/**\brief Mark an entity, \a ent, as partially transcribed.
  *
  * Subclasses should call this method when a UUID has been assigned
  * to a model entity but ent.storage() has not yet been populated with
  * all of the information about the entity. The information which *is*
  * \a present in ent.storage() should be passed but will default to
  * zero (i.e., the UUID exists in some other entity's relations but
  * has no records in storage itself).
  *
  * The entity is added to the list of dangling entities and will be
  * removed from the list when a call to \a transcribeInternal indicates
  * that BridgeBase::allSupportedInformation() is now present in storage.
  */
void BridgeBase::declareDanglingEntity(const Cursor& ent, BridgedInfoBits present)
{
  if ((present & this->allSupportedInformation()) < this->allSupportedInformation())
    this->m_dangling[ent] = present;
}

/**\brief Transcribe information requested by \a flags into \a entity from foreign modeler.
  *
  * Subclasses must override this method.
  * This method should return a non-zero value upon success.
  * Upon success, \a flags should be modified to represent the
  * actual information transcribed (as opposed to what was requested).
  * This should always be at least the information requested but may
  * include more information.
  */
int BridgeBase::transcribeInternal(const Cursor& entity, BridgedInfoBits& flags)
{
  (void)entity;
  (void)flags;
  // Fail to transcribe anything:
  flags = 0;
  return 1;
}

  } // namespace model
} // namespace smtk
