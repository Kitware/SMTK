#ifndef __smtk_model_SimpleModelSubphrases_h
#define __smtk_model_SimpleModelSubphrases_h

#include "smtk/model/SubphraseGenerator.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/PropertyListPhrase.h"

namespace smtk {
  namespace model {

/**\brief A subphrase-generator for presenting an abbreviated model.
  *
  * This subphrase generator omits cell-use records to simplify traversal.
  */
class SMTKCORE_EXPORT SimpleModelSubphrases : public SubphraseGenerator
{
public:
  smtkTypeMacro(SimpleModelSubphrases);
  smtkSharedPtrCreateMacro(SubphraseGenerator);

  virtual DescriptivePhrases subphrases(DescriptivePhrase::Ptr src);

protected:
  virtual void ChildrenOfEntity(EntityPhrase::Ptr, DescriptivePhrases&);
  virtual void ChildrenOfEntityList(EntityListPhrase::Ptr, DescriptivePhrases&);
  virtual void ChildrenOfPropertyList(PropertyListPhrase::Ptr, DescriptivePhrases&);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_SimpleModelSubphrases_h
