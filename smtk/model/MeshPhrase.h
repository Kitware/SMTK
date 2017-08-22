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

namespace smtk
{
namespace model
{

/**\brief Describe an meshset or collection for user presentation.
  */
class SMTKCORE_EXPORT MeshPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(MeshPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(
    const smtk::mesh::MeshSet& meshset, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  // NOTE: This is not updating subphrases, nor markDirty, just the related mesh is changed
  void updateMesh(const smtk::mesh::MeshSet& meshset);
  Ptr setup(const smtk::mesh::CollectionPtr& meshes,
    DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  // NOTE: This is not updating subphrases, nor markDirty, just the related mesh is changed
  void updateMesh(const smtk::mesh::CollectionPtr& c);
  virtual ~MeshPhrase() {}

  std::string title() override;
  bool isTitleMutable() const override;
  bool setTitle(const std::string& newTitle) override;

  smtk::mesh::MeshSet relatedMesh() const override;
  smtk::mesh::CollectionPtr relatedMeshCollection() const override;

  FloatList relatedColor() const override;
  bool isRelatedColorMutable() const override;
  bool setRelatedColor(const FloatList& rgba) override;

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
