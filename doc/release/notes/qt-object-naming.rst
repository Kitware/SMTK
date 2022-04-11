Added names to qt UI objects to allow for more repeatable automatic testing
---------------------------------------------------------------------------

Automated tests were not giving repeatable results due to unpredicatble Qt
implicit naming of UI widgets.  To avoid this, all UI widgets were manually
assigned names using QObject::setObjectName()

Developer changes
~~~~~~~~~~~~~~~~~~

It would be good to use QObject::setObjectName() for any new Items to
avoid breaking this new "fully named" status.

User-facing changes
~~~~~~~~~~~~~~~~~~~

N/A
