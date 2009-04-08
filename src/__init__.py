# Python bindings for libindicate.
# 
# Copyright 2009 Canonical Ltd.
# 
# Authors:
#     Eitan Isaacson <eitan@ascender.com>
# 
# This program is free software: you can redistribute it and/or modify it 
# under the terms of either or both of the following licenses:
# 
# 1) the GNU Lesser General Public License version 3, as published by the 
# Free Software Foundation; and/or
# 2) the GNU Lesser General Public License version 2.1, as published by 
# the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranties of 
# MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
# PURPOSE.  See the applicable version of the GNU Lesser General Public 
# License for more details.
# 
# You should have received a copy of both the GNU Lesser General Public 
# License version 3 and version 2.1 along with this program.  If not, see 
# <http://www.gnu.org/licenses/>

from _indicate import *
import ctypes

def server_dbus_name(gpointer):
    p = int(str(gpointer)[13:-1], 16)
    try:
        rv = ctypes.cast(p,ctypes.c_char_p).value
    except ctypes.ArgumentError:
        return None
    return rv

def indicator_id(gpointer):
    return int(str(gpointer)[13:-1],16)

