from my_module import *

class Derived(MyClass):
	def __init__(self):
		MyClass.__init__(self)
		self.foo()
		
d = Derived()