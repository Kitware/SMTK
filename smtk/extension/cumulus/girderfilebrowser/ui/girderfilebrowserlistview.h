//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME girderfilebrowserlistview.h
// .SECTION Description
// .SECTION See Also

#ifndef girderfilebrowser_girderfilebrowserlistview_h
#define girderfilebrowser_girderfilebrowserlistview_h

#include <QListView>

class QPaintEvent;

namespace cumulus
{

// We subclass QListView just to overload the paintEvent() method,
// so that we can print "Empty" to the screen when a folder or item
// is empty.
class GirderFileBrowserListView : public QListView
{
public:
  explicit GirderFileBrowserListView(QWidget* parent = nullptr);

private:
  // We only overload this method so we can print "Empty" when a
  // folder or item is empty.
  void paintEvent(QPaintEvent* e);
};

} // end namespace cumulus

#endif
