import argtypes

argtypes.matcher.register('IndicateListenerServer', argtypes.CharArg())

argtypes.matcher.register('IndicateListenerServer*', argtypes.StringArg())
 
argtypes.matcher.register('IndicateListenerIndicator', argtypes.IntArg())

# !!!
argtypes.matcher.register('IndicateListenerIndicator*', argtypes.IntArg())
