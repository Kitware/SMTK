Supporting MarkedForRemoval
--------------------------

Resources can now be markedForRemoval indicating that the resource will be removed from memory (as apposed to deletion which also means it is being deleted from storage as well).  This can be used in the UI to determine if a View needs to worry about keeping its contents up to date if the reason it is using is going to be removed.  This also gets around a current issue with the Resource Links system which will cause a resource to be pulled back into memory even though the resources that its associated with is going to be removed.

Another benefit is to potentially optimize the removal of components when its resource is targeted for removal.

Developer changes
~~~~~~~~~~~~~~~~~~

* Added the following API to smtk::resource::Resource
  * setMarkedForRemoval - sets the resource's marked for removal state
  * isMarkedForRemoval - returns the resource's marked for removal state
* UI class changes
  * All Attribute Related Qt classes that handle operations will now check to see if the resource is marked for removal.
