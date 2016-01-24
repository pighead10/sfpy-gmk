from sfgame import *
import time

def f(self,v):
	print("entity py function: " +  str(v))

def run():
	Entity.f = f
	
	time.sleep(1)
	
	e = Entity(200,100)
	game.add(e)
