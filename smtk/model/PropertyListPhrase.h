#ifndef __smtk_model_PropertyListPhrase_h
#define __smtk_model_PropertyListPhrase_h

#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/BRepModel.h" // For PropertyType enum.

#include <string>
#include <vector>

namespace smtk {
  namespace model {

/**\brief Describe a list of property (name,value) pairs
  * associated with an entity for user presentation.
  *
  * This enumerates properties of a single primitive storage
  * type (e.g. only string properties or only integer properties),
  * not all of an entity's properties at once.
  */
class SMTKCORE_EXPORT PropertyListPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(PropertyListPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const Cursor& entity, PropertyType ptype, DescriptivePhrasePtr parent);
  Ptr setup(const Cursor& entity, PropertyType ptype, const std::set<std::string>& pnames, DescriptivePhrasePtr parent);

  virtual std::string title();
  virtual std::string subtitle();

  virtual smtk::util::UUID relatedEntityId() const;
  virtual Cursor relatedEntity() const;
  virtual PropertyType relatedPropertyType() const;

  std::set<std::string>& propertyNames() { return this->m_propertyNames; }
  const std::set<std::string>& propertyNames() const { return this->m_propertyNames; }

  static DescriptivePhraseType propertyToPhraseType(PropertyType p);

protected:
  PropertyListPhrase();

  Cursor m_entity;
  PropertyType m_propertyType;
  std::set<std::string> m_propertyNames; // an optional subset of m_entity's properties
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_DescriptivePhrase_h
