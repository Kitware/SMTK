#ifndef __smtk_extension_cumulus_jobrequest_h
#define __smtk_extension_cumulus_jobrequest_h

#include "job.h"

#include <QtCore/QList>

class QNetworkReply;

namespace cumulus
{

class JobRequest : public QObject
{
  Q_OBJECT

public:
  JobRequest(const QString &girderUrl, const QString &girderToken,
      Job job, QObject *parent = 0);
  ~JobRequest();

  void virtual send() = 0;
  Job job() const { return this->m_job; };

signals:
  void complete();
  void error(const QString &msg);

protected:
  Job m_job;
  QString m_girderUrl;
  QString m_girderToken;

};

class DeleteJobRequest: public JobRequest
{
  Q_OBJECT

public:
  DeleteJobRequest(const QString &girderUrl, const QString &girderToken,
      Job job, QObject *parent = 0);
  ~DeleteJobRequest();

  void send();

private slots:
  void finished(QNetworkReply *reply);

};

class TerminateJobRequest: public JobRequest
{
  Q_OBJECT

public:
  TerminateJobRequest(const QString &girderUrl, const QString &girderToken,
      Job job, QObject *parent = 0);
  ~TerminateJobRequest();

  void send();

private slots:
  void finished(QNetworkReply *reply);

};

} // end namespace

#endif
