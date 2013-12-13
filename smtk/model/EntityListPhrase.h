#ifndef __smtk_model_EntityListPhrase_h
#define __smtk_model_EntityListPhrase_h

#include "smtk/model/DescriptivePhrase.h"

namespace smtk {
  namespace model {

/**\brief Describe a list of entities for user presentation.
  *
  */
class EntityListPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(EntityListPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  template<typename T>
  Ptr setup(const T& entities, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());

  virtual std::string title();
  virtual std::string subtitle();

protected:
  EntityListPhrase();

  virtual bool buildSubphrasesInternal();

  CursorArray m_entities;
  BitFlags m_commonFlags;
  BitFlags m_unionFlags;
};

/**\brief Initialize an entity list with an iterable container of cursors.
  *
  * This templated method is provided so that arrays of **subclasses** of
  * Cursors are also accepted.
  */
template<typename T>
EntityListPhrase::Ptr EntityListPhrase::setup(const T& entities, DescriptivePhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(ENTITY_LIST, parnt);
  for (
    typename T::const_iterator it = entities.begin();
    it != entities.end();
    ++it)
    {
    this->m_entities.push_back(*it);
    }
  this->m_commonFlags = INVALID;
  this->m_unionFlags = 0;
  return shared_from_this();
}

  } // model namespace
} // smtk namespace

#endif // __smtk_model_EntityListPhrase_h
