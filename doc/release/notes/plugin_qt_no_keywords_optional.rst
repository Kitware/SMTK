Make QT_NO_KEYWORDS optional for external plugins
-------------------------------------------------

External plugins can add the option `ALLOW_QT_KEYWORDS` to their use of
`smtk_add_plugin` to skip the enforcement of `QT_NO_KEYWORDS` for
the plugin. For example:

```
smtk_add_plugin(
  smtkAEVASessionPlugin
  ALLOW_QT_KEYWORDS
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
  ...
)
```
