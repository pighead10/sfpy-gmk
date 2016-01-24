from threading import Thread
from sfgame import *
import testa
import testb

print("execute order 66")
#testa.run()

print("creating threads...")
t1 = Thread(target=testa.run)
print("t1 created")
t2 = Thread(target=testb.run)
print("t2 created")


t1.start()
print("t1 started")
t2.start()
print("t2 started")

t1.join()
print("joined t1") #do I need these?
t2.join()
print("joined t2")

print("execute order 67")