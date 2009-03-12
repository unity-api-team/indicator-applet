#!/usr/bin/env python

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
