//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_DescriptivePhrase_h
#define __smtk_model_DescriptivePhrase_h

#include "smtk/SharedFromThis.h"
#include "smtk/model/EntityRef.h"
#include "smtk/mesh/MeshSet.h"

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
  FLOAT_PROPERTY_LIST,          //!< The entity has a list of floating-point properties.
  STRING_PROPERTY_LIST,         //!< The entity has a list of string properties.
  INTEGER_PROPERTY_LIST,        //!< The entity has a list of integer properties.
  ENTITY_HAS_FLOAT_PROPERTY,    //!< The entity has floating-point properties.
  ENTITY_HAS_STRING_PROPERTY,   //!< The entity has string properties.
  ENTITY_HAS_INTEGER_PROPERTY,  //!< The entity has integer properties.
  ATTRIBUTE_ASSOCIATION,        //!< The entity is associated with an attribute.
  FLOAT_PROPERTY_VALUE,         //!< One property of an entity has a list of floating-point values.
  STRING_PROPERTY_VALUE,        //!< One property of an entity has a list of string values.
  INTEGER_PROPERTY_VALUE,       //!< One property of an entity has a list of integer values.
  ENTITY_HAS_SUBPHRASES,        //!< The entity has many phrases of one type; this phrase summarizes them.
  MESH_SUMMARY,                 //!< Summarize an mesh by displaying its name, type, and dimension.
  MESH_LIST,              //!< Summarize an mesh collection by displaying its name.
  INVALID_DESCRIPTION           //!< This is used to indicate an invalid or empty descriptive phrase.
};

class DescriptivePhrase;
class SubphraseGenerator;
typedef smtk::shared_ptr<SubphraseGenerator> SubphraseGeneratorPtr;
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
  virtual ~DescriptivePhrase() { }

  Ptr setup(DescriptivePhraseType phraseType, Ptr parent = Ptr());
  Ptr setDelegate(SubphraseGeneratorPtr delegate);

  virtual std::string title()                                  { return std::string(); }
  virtual bool isTitleMutable() const                          { return false; }
  virtual bool setTitle(const std::string& newTitle)           { (void)newTitle; return false; }

  virtual std::string subtitle()                               { return std::string(); }
  virtual bool isSubtitleMutable() const                       { return false; }
  virtual bool setSubtitle(const std::string& newSubtitle)     { (void)newSubtitle; return false; }

  virtual DescriptivePhraseType phraseType()                   { return this->m_type; }

  virtual DescriptivePhrasePtr parent() const                  { return this->m_parent.lock(); }
  virtual DescriptivePhrases& subphrases();
  virtual DescriptivePhrases subphrases() const;
  virtual bool areSubphrasesBuilt() const                      { return this->m_subphrasesBuilt; }
  virtual void markDirty(bool dirty = true)                    { this->m_subphrasesBuilt = !dirty; }
  virtual int argFindChild(const DescriptivePhrase* child) const;
  virtual int argFindChild(const EntityRef& child) const;
  int indexInParent() const;

  virtual EntityRef relatedEntity() const                      { return EntityRef(); }
  virtual smtk::common::UUID relatedEntityId() const           { return this->relatedEntity().entity(); }
  virtual ArrangementKind relatedArrangementKind() const       { return KINDS_OF_ARRANGEMENTS; }
  virtual smtk::common::UUID relatedAttributeId() const        { return smtk::common::UUID::null(); }
  virtual std::string relatedPropertyName() const              { return std::string(); }
  virtual PropertyType relatedPropertyType() const             { return INVALID_PROPERTY; }

  virtual FloatList relatedColor() const                       { return FloatList(4, -1.); }
  virtual bool isRelatedColorMutable() const                   { return false; }
  virtual bool setRelatedColor(const FloatList& rgba)          { (void)rgba; return false; }

  virtual smtk::mesh::CollectionPtr relatedMeshCollection() const { return smtk::mesh::CollectionPtr(); }
  virtual smtk::mesh::MeshSet relatedMesh() const              { return smtk::mesh::MeshSet(); }

  unsigned int phraseId() const                                { return this->m_phraseId; }

  SubphraseGeneratorPtr findDelegate();

protected:
  DescriptivePhrase();

  void buildSubphrases();

  WeakDescriptivePhrasePtr m_parent;
  DescriptivePhraseType m_type;
  SubphraseGeneratorPtr m_delegate;
  unsigned int m_phraseId;
  mutable DescriptivePhrases m_subphrases;
  mutable bool m_subphrasesBuilt;

private:
  static unsigned int s_nextPhraseId;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_DescriptivePhrase_h
