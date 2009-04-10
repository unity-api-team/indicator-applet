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
from time import time, ctime, strftime, localtime

def show_property_cb(listener, server, indicator, prop, propertydata):
    print "Indicator Property:       %s %d %s %s" % \
        (server, indicator, prop, propertydata)

def show_property_time_cb(listener, server, indicator, prop, propertydata):
    print "Indicator Property:       %s %d %s %s" % \
        (server, indicator, prop, 
         strftime("%I:%M", localtime(propertydata)))

def show_property_icon_cb(listener, server, indicator, prop, propertydata):
    print "Indicator Property:       %s %d %s %dx%d" % \
        (server, indicator, prop, 
         propertydata.get_width(), propertydata.get_height())

def show_property(listener, server, indicator, prop):
    if prop == "icon":
        listener.get_indicator_property_icon(server, indicator, 
                                             prop, show_property_icon_cb)
    elif prop == "time":
        listener.get_indicator_property_time(server, indicator, 
                                             prop, show_property_time_cb)
    else:
        listener.get_indicator_property(server, indicator, 
                                        prop, show_property_cb)

def get_properties(listener, server, indicator):
    # TODO: Not in libindicate API yet.
    return

def indicator_added(listener, server, indicator, typ):
    print "Indicator Added:          %s %d %s" % \
        (server, indicate.indicator_id(indicator), typ)

def indicator_removed(listener, server, indicator, typ):
    print "Indicator Removed:        %s %d %s" % \
        (server, indicate.indicator_id(indicator), typ)

def indicator_modified(listener, server, indicator, typ, prop):
    print "Indicator Modified:       %s %d %s %s" % \
        (server, indicate.indicator_id(indicator), typ, prop)
    show_property(listener, server, 
                  indicate.indicator_id(indicator), prop)

def type_cb(listener, server, value):
    print "Indicator Server Type:    %s %s" % \
        (server, value)

def desktop_cb(listener, server, value):
    print "Indicator Server Desktop: %s %s" % \
        (server, value)

def server_added(listener, server, typ):
    print "Indicator Server Added:   %s %s" % \
        (server.get_dbusname(), typ)
    listener.server_get_type(server, type_cb)
    listener.server_get_desktop(server, desktop_cb)

def server_removed(listener, server, typ):
    print "Indicator Server Removed: %s %s" % \
        (server, typ)

if __name__ == "__main__":
    listener = indicate.indicate_listener_ref_default()
    listener.connect("indicator-added", indicator_added)
    listener.connect("indicator-removed", indicator_removed)
    listener.connect("indicator-modified", indicator_modified)
    listener.connect("server-added", server_added)
    listener.connect("server-removed", server_removed)

    gtk.main()
