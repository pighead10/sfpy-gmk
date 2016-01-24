from sfgame import *
import time

def run():
	print("running test b")
	print("adding b e1...")
	e1 = Entity(500,100)
	game.add(e1)
	print("added b e1")
	
	time.sleep(2)
	
	print("adding b e3...")
	e3 = Entity(500,200)
	game.add(e3)
	print("added b e3") #appears that python deletes object when function is done, causing error on C++ side
	
	e1.f(5)
	
	time.sleep(2)
	e3.x = 300
	print("changing x!")
	print(e3.x)
	time.sleep(2)
	e3.x = 200
	print(e3.x)