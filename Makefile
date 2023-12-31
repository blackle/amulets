all : recurse amulet

llama.cpp/k_quants.o :
	$(MAKE) -C llama.cpp k_quants.o

llama.cpp/ggml.o :
	$(MAKE) -C llama.cpp ggml.o

llama.cpp/llama.o :
	$(MAKE) -C llama.cpp llama.o

llama.cpp/common.o :
	$(MAKE) -C llama.cpp common.o

recurse : recurse.cpp llama.cpp/ggml.o llama.cpp/llama.o llama.cpp/common.o llama.cpp/k_quants.o
	g++ -o $@ $^ -O3 -g -std=c++14 -I ./llama.cpp -pthread -march=native -mtune=native

amulet : amulet.cpp lib/SHA256/SHA256.cpp llama.cpp/ggml.o llama.cpp/llama.o llama.cpp/common.o llama.cpp/k_quants.o
	g++ -o $@ $^ -O3 -g -std=c++14 -I ./llama.cpp -I ./lib/SHA256 -I ./lib/moodycamel -pthread -march=native -mtune=native