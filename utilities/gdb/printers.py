import gdb.printing

# NOTE:
# To use this file, source it in gdb or add it to your
# gdb auto-initialization. For example, if this file
# is in /path/to/smtk/utilities/gdb/printers.py, then
# add this line to your ~/.gdbinit file (or create the
# file and put this line in it):
#
#   source /path/to/smtk/utilities/gdb/printers.py
#
# Then, when you print data with UUIDs, it will print them
# out much more succinctly.


class UUIDPrinter(object):
    """Print smtk.common.UUID objects"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        vv = self.val['m_data']['data']
        ss = '-'.join((
            ''.join(['%02x' % vv[ii] for ii in range(0, 4)]),
            ''.join(['%02x' % vv[ii] for ii in range(4, 6)]),
            ''.join(['%02x' % vv[ii] for ii in range(6, 8)]),
            ''.join(['%02x' % vv[ii] for ii in range(8, 10)]),
            ''.join(['%02x' % vv[ii] for ii in range(10, 16)])))
        return "{UUID = %s}" % ss


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter('smtk')
    pp.add_printer('smtkUUID', 'smtk::common::UUID', UUIDPrinter)
    return pp


gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer())
