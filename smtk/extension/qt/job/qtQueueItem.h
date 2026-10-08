//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtQueueItemView_h
#define smtk_extension_qtQueueItemView_h

#include "smtk/extension/qt/qtItem.h"

namespace smtk
{
namespace qt
{

/**\brief A custom string-item view for selecting a job queue.
 */
class SMTKQTEXT_EXPORT qtQueueItem : public smtk::extension::qtItem
{
  Q_OBJECT

public:
  static smtk::extension::qtItem* createItemWidget(
    const smtk::extension::qtAttributeItemInfo& info);
  qtQueueItem(const smtk::extension::qtAttributeItemInfo& info);
  ~qtQueueItem() override;
  void setLabelVisible(bool) override;
  bool isFixedWidth() const override;
public Q_SLOTS:
  void updateItemData() override;
  void selectQueue(int queueIndex);

protected:
  void createWidget() override;
  class Internal;
  std::unique_ptr<Internal> m_p;
};

} // namespace qt
} // namespace smtk

#endif // smtk_extension_qtQueueItemView_h
