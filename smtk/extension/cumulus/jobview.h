#ifndef __smtk_extension_cumulus_jobview_h
#define __smtk_extension_cumulus_jobview_h

#include "job.h"

#include <QtGui/QTableView>
#include <QtCore/QSet>


namespace cumulus
{
class CumulusProxy;

class JobView : public QTableView
{
  Q_OBJECT

public:
  JobView(QWidget *theParent = 0);
  ~JobView();

  void contextMenuEvent(QContextMenuEvent *e);
  void setCumulusProxy(CumulusProxy *cumulusProxy);

private slots:
  void deleteJob();
  void terminateJob();

private:
  CumulusProxy *m_cumulusProxy;

};

} // end of namespace

#endif
