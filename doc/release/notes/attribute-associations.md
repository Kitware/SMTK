## Replace legacy connections between attribute and model with links.

This commit Introduces resource associations to `smtk::attribute::Resource`
and uses them in place of `refModelManager()`. It also changes `ModelEntityItem`
into a subclass of `ComponentItem`, facilitating its use of resource links.
