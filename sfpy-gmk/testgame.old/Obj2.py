#See Obj1.py for explanation of functions.
from sfgame import *
import time
	
def start(self):
	self.registerEvent(GameEvent.Collision,self.onCollided)
	print("start2 running")
	while True:	
		self.velocity = Vector2(0.02,0.1)
		time.sleep(1)
		self.velocity = Vector2(-0.02,-0.1)
		time.sleep(1)
		
def onCollided(self,arg1):
	pass
	#print("obj2 collided!",args[0])
	
Obj2.onCollided = onCollided


	
Obj2.start = start
