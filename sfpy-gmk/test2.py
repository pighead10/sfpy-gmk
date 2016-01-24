from sfgame import *
import time

def run():
	e1 = Entity(500,100)
	game.add(e1)

	time.sleep(2)

	e3 = Entity(500,200)
	game.add(e3)
	
	e1.f(5)