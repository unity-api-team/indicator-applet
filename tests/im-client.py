#!/usr/bin/env python
#
#Copyright 2009 Canonical Ltd.
#
#Authors:
#    Eitan Isaacson <eitan@ascender.com>
#
#This program is free software: you can redistribute it and/or modify it 
#under the terms of either or both of the following licenses:
#
#1) the GNU Lesser General Public License version 3, as published by the 
#Free Software Foundation; and/or
#2) the GNU Lesser General Public License version 2.1, as published by 
#the Free Software Foundation.
#
#This program is distributed in the hope that it will be useful, but 
#WITHOUT ANY WARRANTY; without even the implied warranties of 
#MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
#PURPOSE.  See the applicable version of the GNU Lesser General Public 
#License for more details.
#
#You should have received a copy of both the GNU Lesser General Public 
#License version 3 and version 2.1 along with this program.  If not, see 
#<http://www.gnu.org/licenses/>
#

import indicate
import gobject
import gtk
from time import time

PATHA = "/usr/share/icons/hicolor/16x16/apps/pidgin.png"
PATHB = "/usr/share/icons/hicolor/22x22/apps/pidgin.png"
lastpath = None

def timeout_cb(indicator):
    print "Modifying properties"
    global lastpath
    indicator.set_property_time("time", time())
    if lastpath == PATHA:
        lastpath = PATHB
    else:
        lastpath = PATHA

    pixbuf = gtk.gdk.pixbuf_new_from_file(lastpath)

    indicator.set_property_icon("icon", pixbuf)

    return True

def display(indicator):
    print "Ah, my indicator has been displayed"

def server_display(server):
    print "Ah, my server has been displayed"


if __name__ == "__main__":
    server = indicate.indicate_server_ref_default()
    server.set_type("message.im")
    server.set_desktop_file("/usr/share/applications/pidgin.desktop")
    server.connect("server-display", server_display)
    
    indicator = indicate.IndicatorMessage()
    indicator.set_property("subtype", "im")
    indicator.set_property("sender", "IM Client Test")
    indicator.set_property_time("time", time())
    indicator.show()

    indicator.connect("user-display", display)

    gobject.timeout_add_seconds(5, timeout_cb, indicator)

    gtk.main()
