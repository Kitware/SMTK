//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_PhraseListContent_h
#define smtk_view_PhraseListContent_h

#include "smtk/view/PhraseContent.h"

#include "smtk/model/Entity.h"

namespace smtk
{
namespace view
{

/**\brief Describe a list of child phrases to the user.
  *
  * Subphrase generators create instances of this class when a parent phrase
  * has more than one child, or when a list of phrases should be grouped
  * together because they have some commonality.
  *
  * Instances of this class generate their own titles by default
  * based on the number and type of their children.
  * For example, if a PhraseListContent has 3 children and they are all
  * SMTK model faces, its default title will be "3 faces".
  */
class SMTKCORE_EXPORT PhraseListContent : public PhraseContent
{
public:
  smtkTypeMacro(PhraseListContent);
  smtkSharedPtrCreateMacro(PhraseContent);

  /**\brief Initialize this list with an iterable container of subphrases.
    *
    * This templated method is provided so that arrays of shared-pointers
    * to **subclasses** of DescriptivePhrase are also accepted.
    * It is mostly intended for use by SubphraseGenerator and its subclasses.
    */
  Ptr setup(
    DescriptivePhrasePtr parent,
    smtk::model::BitFlags commonFlags,
    smtk::model::BitFlags unionFlags,
    int mutability = 0);

  /**\brief Create a descriptive phrase whose content is a PhraseListContent instance.
    *
    * This is for creating a phrase that a subphrase generator knows how to populate
    * by inspecting the commonFlags() and unionFlags() values.
    */
  static DescriptivePhrasePtr createPhrase(
    DescriptivePhrasePtr parent,
    smtk::model::BitFlags commonFlags,
    smtk::model::BitFlags unionFlags,
    int mutability = 0,
    const DescriptivePhrases& children = DescriptivePhrases());

  virtual ~PhraseListContent() {}

  bool displayable(ContentType attr) const override;
  bool editable(ContentType attr) const override
  {
    return (m_mutability & static_cast<int>(attr)) ? true : false;
  }

  std::string stringValue(ContentType attr) const override;
  int flagValue(ContentType attr) const override;

  bool editStringValue(ContentType attr, const std::string& val) override;
  bool editFlagValue(ContentType attr, int val) override;

  /// A convenience method for obtaining related components for each of the phrases in the list.
  smtk::resource::ComponentArray relatedComponents() const;

  /** \brief A convenience method for obtaining related components in a user-specified container.
    *
    * This method dynamically casts each phrase's relatedComponent() to the given container's
    * type and back-inserts it into a new container of the templated type.
    * Use this method when you know the components of interest in the array are a given
    * subclass of smtk::resource::Component (e.g., smtk::model::Entity) by passing in
    * a container of that type (e.g., relatedComponentsAs<smtk::model::EntityArray>()).
    */
  template<typename T>
  T relatedComponentsAs() const
  {
    T result;
    auto comps = this->relatedComponents();
    for (auto comp : comps)
    {
      result.insert(result.end(), dynamic_pointer_cast<typename T::value_type::element_type>(comp));
    }
    return result;
  }

  /**\brief Allow subphrase generators to set summary information for model components in the list.
    *
    * If an instance of PhraseListContent contains model entities, the SubphraseGenerator
    * subclass (or other code) which populates the list of components should call this
    * method.
    *
    * The flags are used to generate the title summary in situations
    * where setCustomTitle() is not used.
    */
  virtual void setModelFlags(smtk::model::BitFlags commonFlags, smtk::model::BitFlags unionFlags);

  /// Return the bit-vector common to all smtk::model children in this list.
  smtk::model::BitFlags commonModelFlags() const { return m_commonFlags; }

  /// Return the bit-vector holding the union of all smtk::model children in this list.
  smtk::model::BitFlags unionModelFlags() const { return m_unionFlags; }

  void setMutability(int whatsMutable);

  /**\brief This method is for subphrase generators that wish to customize the list title.
    *
    * This method is only intended for setup and ignores the mutability of the title,
    * unlike setTitle(). Generally, this method should only be called when the title
    * is **not** mutable.
    */
  void setCustomTitle(const std::string& title) { m_title = title; }

  bool operator==(const PhraseContent& other) const override;

protected:
  PhraseListContent();

  std::string generateTitle(smtk::model::BitFlags& flagCommon, smtk::model::BitFlags& flagUnion)
    const;

  int m_mutability;
  mutable smtk::model::BitFlags m_commonFlags;
  mutable smtk::model::BitFlags m_unionFlags;
  mutable std::string m_title;
};

} // namespace view
} // namespace smtk

#endif
