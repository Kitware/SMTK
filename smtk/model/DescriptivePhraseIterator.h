//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_DescriptivePhraseIterator_h
#define __smtk_model_DescriptivePhraseIterator_h

#include "smtk/model/Cursor.h"

#include <string>

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

class SMTKCORE_EXPORT DescriptivePhraseIterator
{
public:
  DescriptivePhraseIterator(const Cursor& entity);
  DescriptivePhraseIterator(const DescriptivePhraseIterator& other);
  DescriptivePhraseIterator(DescriptivePhraseIterator* parent, DescriptivePhraseType phraseType, int phraseNum);

  void init();
  bool advance();
  bool done();

  std::string phrase() const;
  DescriptivePhraseType phraseType() const;
  int numberOfSubphrases() const;
  smtk::common::UUID relatedEntityId() const;
  Cursor relatedEntity() const;
  smtk::attribute::AttributeId relatedAttributeId() const;
  std::string relatedPropertyName() const;
  PropertyType relatedPropertyType() const;

  DescriptivePhraseIterator& operator ++ ()
    { this->advance(); return *this; }

protected:
  DescriptivePhraseIterator* m_parent; // If we are a subphrase, the parent phrase. Or NULL.
  Cursor m_context; // The entity we are describing
  DescriptivePhraseType m_currentPhraseType; // The current type of phrase we are iterating
  int m_currentPhrase; // The index of the phrase of the current type we are iterating
  int m_numberOfPhrasesOfCurrentType; // An upper bound for m_currentPhrase.
  shared_ptr<DescriptivePhraseIterator> m_currentChild; // If we are at a subphrase, a pointer to it. Or NULL.
  PropertyNameWithConstFloats m_currentFloatProperty;
  PropertyNameWithConstStrings m_currentStringProperty;
  PropertyNameWithConstIntegers m_currentIntegerProperty;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_DescriptivePhraseIterator_h
