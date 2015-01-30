#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
import smtk
mgr = smtk.model.Manager.create()
sess = mgr.createSession('exodus')
rdr = sess.op('read')
rdr.findAsFile('filename').setValue('disk_out_ref.ex2')
res = rdr.operate()
me = smtk.model.Model(res.findModelEntity('model').value(0))
print me.groups()
