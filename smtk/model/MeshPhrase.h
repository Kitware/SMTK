//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_MeshPhrase_h
#define __smtk_model_MeshPhrase_h

#include "smtk/model/DescriptivePhrase.h"

namespace smtk {
  namespace model {

/**\brief Describe an meshset or collection for user presentation.
  */
class SMTKCORE_EXPORT MeshPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(MeshPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const smtk::mesh::MeshSet& meshset, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  // NOTE: This is not updating subphrases, nor markDirty, just the related mesh is changed
  void updateMesh(const smtk::mesh::MeshSet& meshset);
#ifndef SHIBOKEN_SKIP
  Ptr setup(const smtk::mesh::CollectionPtr& meshes, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  // NOTE: This is not updating subphrases, nor markDirty, just the related mesh is changed
  void updateMesh(const smtk::mesh::CollectionPtr& c);
#endif
  virtual ~MeshPhrase() { }

  virtual std::string title();
  virtual bool isTitleMutable() const;
  virtual bool setTitle(const std::string& newTitle);

  virtual smtk::mesh::MeshSet relatedMesh() const;
  virtual smtk::mesh::CollectionPtr relatedMeshCollection() const;

  virtual FloatList relatedColor() const;
  virtual bool isRelatedColorMutable() const;
  virtual bool setRelatedColor(const FloatList& rgba);

  void setMutability(int whatsMutable);
  bool isCollection() const;
protected:
  MeshPhrase();

  smtk::mesh::MeshSet m_relatedMesh;
  smtk::mesh::CollectionPtr m_relatedCollection;
  int m_mutability;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_MeshPhrase_h
