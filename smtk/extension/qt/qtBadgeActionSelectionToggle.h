//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtBadgeActionSelectionToggle_h
#define smtk_extension_qtBadgeActionSelectionToggle_h

#include "smtk/extension/qt/qtBadgeActionToggle.h"

#include "smtk/view/Selection.h"

#include "smtk/common/Visit.h"

#include <QItemSelection>

namespace smtk
{
namespace extension
{

/**\brief A bulk-capable toggle action for badges.
  *
  * This class inherits BadgeActionToggle to add an implementation
  * of visitRelatedPhrases() that will traverse the selected model
  * indices passed by the constructor.
  */
class SMTKQTEXT_EXPORT qtBadgeActionSelectionToggle : public qtBadgeActionToggle
{
public:
  qtBadgeActionSelectionToggle(
    std::shared_ptr<smtk::view::PhraseModel>& model,
    std::shared_ptr<smtk::view::Selection>& seln,
    const std::string& selnLabel,
    bool exactMatch = false);
  ~qtBadgeActionSelectionToggle() override = default;

  /**\brief This is implemented so older badges continue to work as expected, however
    *       it is not as fast as visitedRelatedObjects() and thus not preferred.
    *
    * Because the selection only has a list of objects (not a list of phrases
    * that refer to those objects), visiting the phrases requires us to construct
    * a list of matching phrases and act as if all of them were selected.
    * Since this lookup takes time and multiple phrases can refer to the same object,
    * this method can take longer than simply visited the related objects.
    * It is also possible for the selection to refer to objects that have no matching
    * phrase; in that case, using this method instead of visitRelatedObjects() may
    * lead to different results.
    */
  void visitRelatedPhrases(PhraseVisitor visitor) const override;

  /// Invoke a functor, \a ff, on each object in the selection at the time the badge was clicked.
  template<typename Functor>
  smtk::common::Visited visitRelatedObjects(Functor ff)
  {
    smtk::common::Visited result = smtk::common::Visited::Empty;
    if (!m_selection)
    {
      return result;
    }
    smtk::common::VisitorFunctor<Functor> visitor(ff);
    auto items =
      m_selection->currentSelectionByValueAs<std::set<smtk::resource::PersistentObject::Ptr>>(
        m_selectionLabel, m_exactMatch);
    for (const auto& item : items)
    {
      result = smtk::common::Visited::Some;
      if (visitor(item) == smtk::common::Visit::Halt)
      {
        return result;
      }
    }
    result =
      (result == smtk::common::Visited::Some ? smtk::common::Visited::All
                                             : smtk::common::Visited::Empty);
    return result;
  }

protected:
  std::shared_ptr<smtk::view::PhraseModel> m_model;
  std::shared_ptr<smtk::view::Selection> m_selection;
  std::string m_selectionLabel{ "selected" };
  bool m_exactMatch{ false };
};
} // namespace extension
} // namespace smtk

#endif
