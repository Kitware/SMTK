Using TypeContainers instead of ViewInfo
----------------------------------------

In order to make the View System more flexible and to work with the new Task System, the following changes were made:

* smtk::view::Information is now derived from TypeContainer and is no longer an abstract class.  As a result it can now do the job that ViewInfo and OperationViewInfo does
* ViewInfo and OperationViewInfo are no longer needed.
* qtBaseView's m_viewInfo is now an instance of smtk::view::Information and not ViewInfo

Developer changes
~~~~~~~~~~~~~~~~~~

Unless the qtView is directly accessing m_viewInfo, there should be no required changes.

When dealing with smtk::view::information, it is important that the type you insert into the container exactly matches the type you use to get information from the container.  For example if you insert a QPushButton* into the container and attempt to get a QWidget* back, it will fail and throw an exception.

So it is recommended you explicitly state the template type instead of having the compiler determine it. In the above example you would need to do an insert<QWidget*>(myQtPushButton) in order to get a QWidget* back.

Removed Data Structures
+++++++++++++++++++++++
smtk::external::ViewInfo and smtk::external::OperatorViewInfo are no longer needed and have been removed.  smtk::view::Information object should be used instead.
