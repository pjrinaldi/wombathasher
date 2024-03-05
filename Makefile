blake3:
	cd blake3
	make blake3
	cd ..

wombathasher:
	g++ -O3 -static -o wombathasher wombathasher.cpp -lpthread -L./blake3 -lblake3
