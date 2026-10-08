Common subsystem
================

libarchive dependency removed
-----------------------------

The ``smtk::common::Archive`` class and the dependency it introduced on
libarchive have been removed. The facility has been unused due to issues
with speed and so is being removed. This means that any zip archives saved
with older versions of SMTK will no longer be read by the ``ReadResource``
operation, which was the only class in SMTK that used the archive utility.
