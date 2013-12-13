#ifndef __smtk_model_EntityPhrase_h
#define __smtk_model_EntityPhrase_h

#include "smtk/model/DescriptivePhrase.h"

namespace smtk {
  namespace model {

class EntityPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(EntityPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const Cursor& entity, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());

  virtual std::string title();
  virtual std::string subtitle();

  virtual Cursor relatedEntity() const;

protected:
  EntityPhrase();

  virtual bool buildSubphrasesInternal();

  Cursor m_entity;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_EntityPhrase_h
