//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EntityTypeSubphrases_h
#define __smtk_model_EntityTypeSubphrases_h

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
class SMTKCORE_EXPORT EntityTypeSubphrases : public SubphraseGenerator
{
public:
  smtkTypeMacro(EntityTypeSubphrases);
  smtkSharedPtrCreateMacro(SubphraseGenerator);
  virtual ~EntityTypeSubphrases() { }

  virtual DescriptivePhrases subphrases(DescriptivePhrase::Ptr src);
  virtual bool shouldOmitProperty(
    DescriptivePhrase::Ptr parent, PropertyType ptype, const std::string& pname) const;

protected:
  EntityTypeSubphrases();

  virtual void childrenOfEntity(EntityPhrase::Ptr, DescriptivePhrases&);
  virtual void childrenOfEntityList(EntityListPhrase::Ptr, DescriptivePhrases&);
  virtual void childrenOfPropertyList(PropertyListPhrase::Ptr, DescriptivePhrases&);
  virtual void childrenOfMesh(MeshPhrase::Ptr, DescriptivePhrases&);
  virtual void childrenOfMeshList(MeshListPhrase::Ptr, DescriptivePhrases&);

  bool m_abridgeUses;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EntityTypeSubphrases_h
