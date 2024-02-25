.. _smtk-resource-copy-template-update:

Copying, Templating, and Updating Resources
===========================================

Resources have a "template type" and "template version"
accessed with :smtk:`templateType <smtk::resource::Resource::templateType>`
and :smtk:`templateVersion <smtk::resource::Resource::templateVersion>`.
This allows resources that provide user- or developer-specified schemas to indicate
the name and version number of a schema; that information can be used to

* update resources when new versions of the schema are released;
* inform users and applications that the resource conforms to a standard structure; and
* provide information to applications on how a resource is intended to be used.

This template information allows resources to model prototypal inheritance
(where one document's structure serves as the "theme" or "style" for other documents –
similar to the way most presentation software allows users to create and edit slide styles).
Thus, resources may be

* **cloned** (which produces a new "blank" resource that has its own UUID but with
  ancillary data matching the source's ancillary data); or
* **copied** (which produces a new resource that has a different UUID and different
  component UUIDs but whose content matches the source document); or
* **updated** (which produces a new resource whose UUIDs match the source but whose
  template version has been updated).

See the :smtk:`clone <smtk::resource::Resource::clone>`, :smtk:`copyStructure <smtk::resource::Resource::copyStructure>`,
and :smtk:`copyRelations <smtk::resource::Resource::copyRelations>` methods for details on how to carry
these tasks out.
Note that, as with supports for template type and version information, the ability to clone
and copy resources must be implemented by subclasses of the base resource class.
Some facilties are provided for you to copy information such as property data, link data,
unit systems, etc.
