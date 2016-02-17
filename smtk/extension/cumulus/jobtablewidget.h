#ifndef __smtk_extension_cumulus_jobtablewidget_h
#define __smtk_extension_cumulus_jobtablewidget_h

#include <QtGui/QWidget>

namespace Ui {
class JobTableWidget;
}

namespace cumulus
{
class JobTableModel;
class CumulusProxy;

class JobTableWidget : public QWidget
{
  Q_OBJECT

public:
  explicit JobTableWidget(QWidget *parentObject = 0);
  ~JobTableWidget();

  void setModel(QAbstractItemModel *model);
  void setCumulusProxy(CumulusProxy *cumulusProxy);
protected:
  Ui::JobTableWidget *ui;

};

} // end namespace

#endif
