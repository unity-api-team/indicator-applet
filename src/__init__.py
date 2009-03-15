from _indicate import *
import ctypes

def server_dbus_name(gpointer):
    p = int(str(gpointer)[13:-1],16)
    try:
        rv = ctypes.cast(p,ctypes.c_char_p).value
    except ctypes.ArgumentError:
        return None
    return rv

def indicator_id(gpointer):
    return int(str(gpointer)[13:-1],16)
