from sfgame import *
import time
	
a1 = Obj1()
a2 = Obj2()

player = Player()

game.add(player)
game.add(a1)
game.add(a2)

a1.position = Vector2(500,200)
a2.position = Vector2(500,400)
player.position = Vector2(200,200)