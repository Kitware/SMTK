'''
A script for debugging lldb output when debugging shared_ptrs that get away from you.

$ script out.txt
$ lldb bin/modelbuilder.app/Contents/MacOS/modelbuilder

$ breakpoint set --name std::__1::shared_ptr<smtk::resource::Resource>::shared_ptr --name std::__1::shared_ptr<smtk::resource::Resource>::~shared_ptr
$ br command add -s
$ p this
$ th ba -c 10
$ continue
$ DONE
'''

import re
import sys

f = open(sys.argv[1], "r")
content = f.read().split('(lldb)  p this\n')
print('there are %d backtraces' % len(content))

constructors = []
destructors = []
for c in content:
    if c.find(str('libsmtkCore.dylib`std::__1::shared_ptr<smtk::resource::Resource>::shared_ptr')) != -1:
        constructors.append(c)
    elif c.find(str('libsmtkCore.dylib`std::__1::shared_ptr<smtk::resource::Resource>::~shared_ptr')) != -1:
        destructors.append(c)
print('there are %d constructors and %d destructors' %
      (len(constructors), len(destructors)))

constructed_ptrs = []
for c in constructors:
    x = re.search(
        '(std::__1::shared_ptr<smtk::resource::Resource> *) .*\n\n', c)
    index_str = x.group(0)[-27:-20]
    index_str = index_str[index_str.find('$') + 1:index_str.find('=')]
    index = int(index_str)
    constructed_ptrs.append((x.group(0)[-20:-2], index, c))

destructed_ptrs = []
for c in destructors:
    x = re.search(
        '(std::__1::shared_ptr<smtk::resource::Resource> *) .*\n\n', c)
    index_str = x.group(0)[-27:-20]
    index_str = index_str[index_str.find('$') + 1:index_str.find('=')]
    index = int(index_str)
    destructed_ptrs.append((x.group(0)[-20:-2], index, c))

d_toRemove = set()
c_toRemove = set()
for c_tuple in constructed_ptrs:
    (c_address, c_index, c_trace) = c_tuple
    for d_tuple in destructed_ptrs:
        (d_address, d_index, d_trace) = d_tuple

        if d_index > c_index and d_address == c_address:
            d_toRemove.add(d_tuple)
            c_toRemove.add(c_tuple)

d_trimmed = [d for d in destructed_ptrs if not d in d_toRemove]
c_trimmed = [c for c in constructed_ptrs if not c in c_toRemove]

print('there are %d unmatched constructors and %d unmatched destructors' %
      (len(c_trimmed), len(d_trimmed)))

for (c_address, c_index, c_trace) in c_trimmed:
    print('%s\n\n' % c_trace)
