## Hide advanced items in operation view for "New Resource" menu option

Currently, creation operators all fallow the pattern of optionally creating
an entity within an extant resource. Since this functionality doesn't make
sense for a "New Resource" menu option, we flag the input items involved
with that choice as advanced and disable the advanced items when the
operation is launched via the "New Resource" menu option. Since the
choice of filtering by advance level is persistent for the operation, we
unset the flag after the operation window returns.
