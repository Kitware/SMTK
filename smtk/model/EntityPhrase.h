//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EntityPhrase_h
#define __smtk_model_EntityPhrase_h

#include "smtk/model/DescriptivePhrase.h"

namespace smtk
{
namespace model
{

/**\brief Describe an entity for user presentation.
  */
class SMTKCORE_EXPORT EntityPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(EntityPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const EntityRef& entity, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  virtual ~EntityPhrase() {}

  std::string title() override;
  bool isTitleMutable() const override;
  bool setTitle(const std::string& newTitle) override;
  std::string subtitle() override;

  EntityRef relatedEntity() const override;
  FloatList relatedColor() const override;
  bool isRelatedColorMutable() const override;
  bool setRelatedColor(const FloatList& rgba) override;

  void setMutability(int whatsMutable);

  static DescriptivePhrases PhrasesFromUUIDs(smtk::model::ManagerPtr, const smtk::common::UUIDs&);

protected:
  EntityPhrase();

  EntityRef m_entity;
  int m_mutability;
};

} // model namespace
} // smtk namespace

#endif // __smtk_model_EntityPhrase_h
