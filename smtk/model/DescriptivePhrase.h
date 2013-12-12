#ifndef __smtk_model_DescriptivePhrase_h
#define __smtk_model_DescriptivePhrase_h

#include "smtk/util/SharedFromThis.h"
#include "smtk/model/Cursor.h"

#include <string>
#include <vector>

namespace smtk {
  namespace model {

/// Possible types of relationships that the iterator will report
enum DescriptivePhraseType {
  ENTITY_LIST,                  //!< Multiple entities should be summarized.
  ENTITY_SUMMARY,               //!< Summarize an entity by displaying its name, type, and dimension.
  ARRANGEMENT_LIST,             //!< The entity has multiple arrangements (of a single kind).
  MODEL_INCLUDES_ENTITY,        //!< The entity is a model with a "free" entity.
  MODEL_EMBEDDED_IN_MODEL,      //!< The entity is a model that has child model(s).
  CELL_INCLUDES_CELL,           //!< The entity is a cell that includes a lower-dimensional cell.
  CELL_EMBEDDED_IN_CELL,        //!< The entity is a cell embedded in a higher-dimensional cell.
  CELL_HAS_SHELL,               //!< The entity is a cell with a shell describing its boundary.
  CELL_HAS_USE,                 //!< The entity is a cell with a sense that is in use as a boundary.
  SHELL_HAS_CELL,               //!< The entity is a shell and this is its parent cell.
  SHELL_HAS_USE,                //!< The entity is a shell composed of multiple uses.
  USE_HAS_CELL,                 //!< The entity is a use and this is its parent cell.
  USE_HAS_SHELL,                //!< The entity is a use and it participates in a shell.
  FLOAT_PROPERTY_LIST,          //!< The entity has a list of floating-point properties.
  STRING_PROPERTY_LIST,         //!< The entity has a list of string properties.
  INTEGER_PROPERTY_LIST,        //!< The entity has a list of integer properties.
  ENTITY_HAS_FLOAT_PROPERTY,    //!< The entity has floating-point properties.
  ENTITY_HAS_STRING_PROPERTY,   //!< The entity has string properties.
  ENTITY_HAS_INTEGER_PROPERTY,  //!< The entity has integer properties.
  ENTITY_HAS_ATTRIBUTE,         //!< The entity is associated with an attribute.
  FLOAT_PROPERTY_VALUE,         //!< One property of an entity has a list of floating-point values.
  STRING_PROPERTY_VALUE,        //!< One property of an entity has a list of string values.
  INTEGER_PROPERTY_VALUE,       //!< One property of an entity has a list of integer values.
  ENTITY_HAS_SUBPHRASES,        //!< The entity has many phrases of one type; this phrase summarizes them.
  INVALID_DESCRIPTION           //!< This is used to indicate an invalid or empty descriptive phrase.
};

class DescriptivePhrase;
typedef std::vector<DescriptivePhrasePtr> DescriptivePhrases;

class SMTKCORE_EXPORT DescriptivePhrase : smtkEnableSharedPtr(DescriptivePhrase)
{
public:
  smtkTypeMacro(DescriptivePhrase);
  Ptr setup(DescriptivePhraseType phraseType, Ptr parent = Ptr());

  virtual std::string title()                                  { return std::string(); }
  virtual std::string subtitle()                               { return std::string(); }
  virtual DescriptivePhraseType phraseType()                   { return this->m_type; }

  virtual DescriptivePhrasePtr parent() const                  { return this->m_parent; }
  virtual DescriptivePhrases subphrases();

  virtual smtk::util::UUID relatedEntityId() const             { return smtk::util::UUID::null(); }
  virtual ArrangementKind relatedArrangementKind() const       { return KINDS_OF_ARRANGEMENTS; }
  virtual Cursor relatedEntity() const                         { return Cursor(); }
  virtual int relatedAttributeId() const                       { return -1; }
  virtual std::string relatedPropertyName() const              { return std::string(); }
  virtual PropertyType relatedPropertyType() const             { return INVALID_PROPERTY; }

protected:
  DescriptivePhrase();

  void buildSubphrases();
  virtual bool buildSubphrasesInternal() { return true; }

  DescriptivePhrasePtr m_parent;
  DescriptivePhraseType m_type;
  mutable DescriptivePhrases m_subphrases;
  mutable bool m_subphrasesBuilt;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_DescriptivePhrase_h
