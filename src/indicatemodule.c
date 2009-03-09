#include <pygobject.h>
 
void pyindicate_register_classes (PyObject *d); 
extern PyMethodDef pyindicate_functions[];

DL_EXPORT(void)
init_indicate(void)
{
		PyObject *m, *d;
		
		init_pygobject ();
		
		m = Py_InitModule ("_indicate", pyindicate_functions);
		d = PyModule_GetDict (m);
		
		pyindicate_register_classes (d);
		
		if (PyErr_Occurred ()) {
				Py_FatalError ("can't initialise module indicate");
		}
}
