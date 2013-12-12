#ifndef __smtk_model_PropertyValuePhrase_h
#define __smtk_model_PropertyValuePhrase_h

#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/BRepModel.h" // For PropertyType enum.

#include <string>
#include <vector>

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT PropertyValuePhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(PropertyValuePhrase);
  smtkCreateMacro(DescriptivePhrase);
  PropertyValuePhrase();
  Ptr setup(const std::string& propName, PropertyListPhrase::Ptr parent);

  virtual std::string title();
  virtual std::string subtitle();

  virtual smtk::util::UUID relatedEntityId() const;
  virtual Cursor relatedEntity() const;
  virtual std::string relatedPropertyName() const;
  virtual PropertyType relatedPropertyType() const;

  static DescriptivePhraseType propertyToPhraseType(PropertyType p);

protected:
  PropertyListPhrase::Ptr m_plist;
  std::string m_propertyName;

  virtual bool buildSubphrasesInternal();
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_DescriptivePhrase_h
