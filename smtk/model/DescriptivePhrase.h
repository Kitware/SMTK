#ifndef __smtk_model_DescriptivePhrase_h
#define __smtk_model_DescriptivePhrase_h

#include "smtk/util/SharedFromThis.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/AttributeAssignments.h" // for AttributeId

#include <string>
#include <vector>

namespace smtk {
  namespace model {

/// Possible types of relationships that the iterator will report
enum DescriptivePhraseType {
  ENTITY_LIST,                  //!< Multiple entities should be summarized.
  ENTITY_SUMMARY,               //!< Summarize an entity by displaying its name, type, and dimension.
  ARRANGEMENT_LIST,             //!< The entity has multiple arrangements (of a single kind).
  ATTRIBUTE_LIST,               //!< The entity has multiple attribute values defined on it.
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

/**\brief A base class for phrases describing an SMTK model.
  *
  * Instances of subclasses serve as the basis for user interfaces
  * to display information about the model in a hierarchical fashion.
  * Each phrase may have zero-to-many subphrases of any type and
  * zero-or-one parent phrases.
  *
  * The title(), subtitle(), and phraseType() report information
  * for the user interface to present. In the Qt layer, these are
  * to be used for list item text (title and subtitle) and icon
  * (phraseType).
  *
  * Subphrases are built on demand (and subclasses must implement
  * buildSubphrasesInternal() to do so) so that portions of the model
  * may be presented without loading the full model.
  *
  * An external (yet-to-be-designed) filter is given the chance to
  * modify each phrase's list of subphrases prior to its presentation.
  * This can be used to limit presentation; inject additional information
  * or functional components; or even modify the phrases (e.g., to
  * eliminate vacuous phrases like "0 attributes associated" or to
  * compact verbose phrases).
  */
class SMTKCORE_EXPORT DescriptivePhrase : smtkEnableSharedPtr(DescriptivePhrase)
{
public:
  smtkTypeMacro(DescriptivePhrase);
  Ptr setup(DescriptivePhraseType phraseType, Ptr parent = Ptr());

  virtual std::string title()                                  { return std::string(); }
  virtual std::string subtitle()                               { return std::string(); }
  virtual DescriptivePhraseType phraseType()                   { return this->m_type; }

  virtual DescriptivePhrasePtr parent() const                  { return this->m_parent; }
  virtual DescriptivePhrases& subphrases();
  virtual int argFindChild(DescriptivePhrase* child) const;

  virtual Cursor relatedEntity() const                         { return Cursor(); }
  virtual smtk::util::UUID relatedEntityId() const             { return this->relatedEntity().entity(); }
  virtual ArrangementKind relatedArrangementKind() const       { return KINDS_OF_ARRANGEMENTS; }
  virtual AttributeId relatedAttributeId() const               { return -1; }
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
