from sfgame import *
import time

def f(self,v):
	print("entity py function: " +  str(v))

def run():
	print("running test a")
	Entity.f = f
	
	time.sleep(1)
	
	print("adding a e...")
	e = Entity(200,100)
	game.add(e)
	print("added a e")
