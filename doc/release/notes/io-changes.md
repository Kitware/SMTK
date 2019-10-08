## I/O Changes

### smtk::io::Logger is now thread-safe
The class now does a mutex lock when modifying or accessing its records or its underlying stream.  Care must be taking when redirecting the logger's stream to avoid deadlocks.  For example using smtk::extension::qtEmittingStringBuffer, you should make sure to use Qt::QueuedConnection when doing a QObject::connect to the buffer's flush signal.  See smtk/extension/qt/cxx/testing/UnitTestEmittingStringBuffer.{h,cxx} for an example.
