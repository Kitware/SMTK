//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_paraview_appcomponents_GeometricVisibilityBadge_h
#define smtk_extension_paraview_appcomponents_GeometricVisibilityBadge_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"
#include "smtk/extension/qt/VisibilityBadge.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/io/Logger.h"

#include <QObject>

class pqView;
class pqRepresentation;

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

///\brief A badge to show/toggle a Phrase's renderable geometry's visibility.
class SMTKPQCOMPONENTSEXT_EXPORT GeometricVisibilityBadge
  : public smtk::extension::qt::VisibilityBadge
{
  Q_OBJECT
public:
  ///\brief Represents possible visibility
  enum class VisibilityState
  {
    Neither =
      -1, //!< Indicates it is neither visible or invisible.  For example can be used to indicate partial visibility
    Invisible = 0, //!< Indicates it is invisible.
    Visible = 1    //!< Indicates it is visible.
  };

  ///\brief store the visibility information related to a Phrase
  ///
  /// Note that the phrase itself is not stored since it is used in maintaining the Phrase Visibility Cache
  struct PhraseInfo
  {
    std::size_t m_numberOfVisibleChildren = 0;
    std::size_t m_numberOfInvisibleChildren = 0;
    VisibilityState m_geometricVisibility = VisibilityState::Neither;
    bool m_isResource = false;

    ///\brief Calculate the hierarchical visibility of a phrase
    ///
    /// If decrementNumChildren is true, then the "adjust" the phrase's number of children by
    /// the offset.  This is to support the case of calculating visibility when removing some of the
    /// phrase's children.
    ///
    ///  Note that is does not take into consideration of the phrase's geometric visibility
    VisibilityState calculateHierarchicalVisibility(
      const smtk::view::DescriptivePhrase* phrase,
      bool decrementNumChildren = false,
      std::size_t offset = 0) const;
    ///\brief Calculate the visibility of phrase that takes both its geometric and hierarchical
    /// visibilities into consideration
    ///
    /// If decrementNumChildren is true, then the "adjust" the phrase's number of children by
    /// the offset.  This is to support the case of calculating visibility when removing some of the
    /// phrase's children.
    VisibilityState calculateCombinedVisibility(
      const smtk::view::DescriptivePhrase* phrase,
      bool decrementNumChildren = false,
      std::size_t offset = 0) const;
  };

  ///\brief Caches the phrase visibility information for Phrases in a Phrase Model
  struct PhraseCache
  {
    mutable std::map<smtk::view::DescriptivePhrase*, PhraseInfo> m_phraseInfos;

    ///\brief Update the cache for a Phrase based on the visibility change of one of the Phrase's children.
    ///
    /// If okToCreate is true, then if the Phrase is not already in the cache, then create a new PhraseInfo
    /// and insert it.
    void update(
      smtk::view::DescriptivePhrase* phrase,
      const VisibilityState& childPreviousVisibility,
      const VisibilityState& childNewVisibility,
      bool okToCreate = true);

    ///\brief Update the cache for a Phrase based on its new geometric visibility.
    ///
    /// If okToCreate is true, then if the Phrase is not already in the cache, then create a new PhraseInfo
    /// and insert it.
    void update(
      smtk::view::DescriptivePhrase* phrase,
      const VisibilityState& geometricVisibility,
      bool isResource = false);

    ///\brief Update the cache for a Phrase, based on the number of the Phrase's children being removed
    /// for each visibility state.
    void updateForRemoval(
      smtk::view::DescriptivePhrase* phrase,
      std::size_t numVisibleChildrenToBeRemoved,
      std::size_t numInvisibleChildrenToBeRemoved,
      std::size_t numNeitherChildrenToBeRemoved);

    ///\brief Clear the entire cache/
    void clear() { m_phraseInfos.clear(); }

    ///\brief Remove the cache entries for a sub-tree of Phrases.
    ///
    /// isTopOfSubTree indicates that the Phrase represents the top of the sub-tree
    /// being removed.  The Phrase that owns that phrase will have its visibility information
    /// updated.
    ///
    ///uuidToRepVisibility is a map of Persistent Object UUID's to Representation Visibilities.
    void removeSubTreeVisibilities(
      view::DescriptivePhrase* phrase,
      std::map<smtk::common::UUID, int>& uuidToRepVisibility,
      bool isTopOfSubTree);

    ///\brief Removed a Phase and all of its descendants from the cache.
    ///
    /// NOTE - this does not update the phase's parent's visibility!  If you need to do that you
    /// should first call updateForRemoval on the parent phrase.
    void removePhrase(smtk::view::DescriptivePhrase* phrase);

    ///\brief Inserts a newly inserted Descriptive Phrase into the cache.
    void insertNewPhrase(smtk::view::DescriptivePhrase* phrase);

    ///\brief Return the cache of PhaseInfos.
    std::map<smtk::view::DescriptivePhrase*, PhraseInfo>& phraseInfos() const
    {
      return m_phraseInfos;
    }
  };

  smtkTypeMacro(smtk::extension::paraview::appcomponents::GeometricVisibilityBadge);
  smtkSuperclassMacro(smtk::extension::qt::VisibilityBadge);
  smtkSharedFromThisMacro(smtk::view::Badge);
  smtkCreateMacro(smtk::view::Badge);

  using DescriptivePhrase = smtk::view::DescriptivePhrase;
  using BadgeAction = smtk::view::BadgeAction;

  GeometricVisibilityBadge();
  GeometricVisibilityBadge(
    smtk::view::BadgeSet& parent,
    const smtk::view::Configuration::Component&);

  ~GeometricVisibilityBadge() override;

  bool appliesToPhrase(const DescriptivePhrase* phrase) const override;

  std::string tooltip(const DescriptivePhrase*) const override
  {
    return std::string("Click to toggle this object's visibility and not its children.");
  }
  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>&) const override;

  /// take an action when the badge is clicked.
  bool action(const DescriptivePhrase* phrase, const BadgeAction& act) override;

  ///\brief Set and get the visibility for this phrase.
  ///@{
  void setPhraseVisibility(const DescriptivePhrase* phrase, int val);
  bool phraseVisibility(const DescriptivePhrase* phrase) const;
  ///@}

  ///\brief Get the visibility cache.
  ///@{
  GeometricVisibilityBadge::PhraseCache& phraseCache();
  const GeometricVisibilityBadge::PhraseCache& phraseCache() const;
  ///@}

  ///\brief Class method for returning a string representation of a visibility emum value.
  static std::string convertVisibilityToString(const VisibilityState& state);

protected Q_SLOTS:
  /// handle the active view changing, change the visible icons.
  void activeViewChanged(pqView* view);
  void representationAddedToActiveView(pqRepresentation* rep);
  void representationRemovedFromActiveView(pqRepresentation* rep);
  void componentVisibilityChanged(const smtk::resource::ComponentPtr& comp, bool visible);

protected:
  smtk::view::PhraseModel* phraseModel() const { return this->m_parent->phraseModel(); }
  std::map<smtk::common::UUID, int> m_uuidToRepVisibility;

  ///\brief Observer handler method.
  void updateObserver(
    smtk::view::DescriptivePhrasePtr phrase,
    smtk::view::PhraseModelEvent event,
    const std::vector<int>& src,
    const std::vector<int>& dst,
    const std::vector<int>& range);
  smtk::view::PhraseModel::Observers::Key m_key;

private:
  // borrowed from paraview Qt/Components
  std::string m_icon;
  std::string m_iconClosed;
  smtk::view::BadgeSet* m_parent{ nullptr };
};
} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk
#endif
