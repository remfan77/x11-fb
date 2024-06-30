SRC=x11-fb
All:
	gcc $(SRC).c -lX11 -lpthread -o $(SRC)
