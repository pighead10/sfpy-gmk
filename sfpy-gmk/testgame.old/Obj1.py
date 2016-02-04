from sfgame import *
import time
	
def changeAmount(self,amount):
	self.amount = self.amount + amount

Obj1.changeAmount = changeAmount

def init(self): #Optional constructor. Most of constructor generated automatically.
				#Should only be used for complex initialisation. 
				#TODO: make custom properties auto generate in constructor.
	self.amount = 100

Obj1.init = init

def onCollide(self,other):
	if self.debounce:
		self.debounce = False
		print(self.name + " collided with " + other.name)
		self.velocity = Vector2(2,0)
		time.sleep(1)
		self.debounce = True
	
Obj1.onCollide = onCollide
	
def start(self): #Run when object is added to game.
	self.registerEvent(GameEvent.Collision,self.onCollide)
	self.debounce = True
	self.velocity = Vector2(0,0.1)
	print("start running")
	while True:
		print("ob iteration")
		time.sleep(1)
	
	
Obj1.start = start
