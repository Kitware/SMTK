//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_MeshListPhrase_h
#define __smtk_model_MeshListPhrase_h

#include "smtk/model/DescriptivePhrase.h"

namespace smtk
{
namespace model
{

/**\brief Describe a list of meshes or collections for user presentation.
  *
  */
class SMTKCORE_EXPORT MeshListPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(MeshListPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const std::vector<smtk::mesh::MeshSet>&,
    DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
#ifndef SHIBOKEN_SKIP
  Ptr setup(const std::vector<smtk::mesh::CollectionPtr>&,
    DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
#endif
  virtual ~MeshListPhrase() {}

  virtual std::string title();
  virtual std::string subtitle();
  std::vector<smtk::mesh::CollectionPtr> relatedCollections() const;
  std::vector<smtk::mesh::MeshSet> relatedMeshes() const;

protected:
  MeshListPhrase();

  std::vector<smtk::mesh::MeshSet> m_meshes;
  std::vector<smtk::mesh::CollectionPtr> m_collections;
};

} // model namespace
} // smtk namespace

#endif // __smtk_model_MeshListPhrase_h
