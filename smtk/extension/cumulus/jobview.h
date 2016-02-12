#ifndef __smtk_extension_cumulus_jobview_h
#define __smtk_extension_cumulus_jobview_h

#include <QtGui/QTableView>

namespace cumulus
{

class JobView : public QTableView
{
  Q_OBJECT

public:
  JobView(QWidget *theParent = 0);
  ~JobView();

};

} // end of namespace

#endif
