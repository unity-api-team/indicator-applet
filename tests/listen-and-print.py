#!/usr/bin/env python

import indicate
import gobject
import gtk
from time import time, ctime

def show_property_cb(listener, server, indicator, prop, propertydata):
    print "Indicator Property:       %s %d %s %s" % \
        (indicate.server_dbus_name(server), indicate.indicator_id(indicator),
         prop, propertydata)

def show_property_time_cb(listener, server, indicator, prop, propertydata):
    print "Indicator Property:       %s %d %s %s" % \
        (indicate.server_dbus_name(server), indicate.indicator_id(indicator),
         prop, time.strftime("%I:%M", time.localtime(propertydata)))

def show_property_icon_cb(listener, server, indicator, prop, propertydata):
    print "Indicator Property:       %s %d %s %dx%d" % \
        (indicate.server_dbus_name(server), indicate.indicator_id(indicator),
         prop, propertydata.get_width, propertydata.get_height)

def show_property(listener, server, indicator, prop):
    print 'SHOW'
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
        (indicate.server_dbus_name(server), 
         indicate.indicator_id(indicator), typ)

def indicator_removed(listener, server, indicator, typ):
    print "Indicator Removed:        %s %d %s" % \
        (indicate.server_dbus_name(server), 
         indicate.indicator_id(indicator), typ)

def indicator_modified(listener, server, indicator, typ, prop):
    print "Indicator Modified:       %s %d %s %s" % \
        (indicate.server_dbus_name(server), 
         indicate.indicator_id(indicator), typ, prop)
    show_property(listener, indicate.server_dbus_name(server), 
                  indicate.indicator_id(indicator), prop)

def type_cb(listener, server, value):
    print "Indicator Server Type:    %s %s" % \
        (indicate.server_dbus_name(server), value)

def desktop_cb(listener, server, value):
    print "Indicator Server Desktop: %s %s" % \
        (indicate.server_dbus_name(server), value)

def server_added(listener, server, typ):
    print "Indicator Server Added:   %s %s" % \
        (indicate.server_dbus_name(server), typ)
    listener.server_get_type(indicate.server_dbus_name(server), type_cb)
    listener.server_get_desktop(indicate.server_dbus_name(server), desktop_cb)

def server_removed(listener, server, typ):
    print "Indicator Server Removed: %s %s" % \
        (indicate.server_dbus_name(server), typ)

if __name__ == "__main__":
    listener = indicate.indicate_listener_ref_default()
    listener.connect("indicator-added", indicator_added)
    listener.connect("indicator-removed", indicator_removed)
    listener.connect("indicator-modified", indicator_modified)
    listener.connect("server-added", server_added)
    listener.connect("server-removed", server_removed)

    gtk.main()
