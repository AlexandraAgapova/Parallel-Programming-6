CXX = pgc++
CXXFLAGS = -Minfo=all -std=c++17
BOOST_LIBS = -lboost_program_options

# Путь к CUDA
CUDA_INCLUDE = -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/cuda/12.3/targets/x86_64-linux/include

all: gpu multicore host

# GPU
gpu: main.cpp
	$(CXX) -acc $(CXXFLAGS) -o main_gpu main.cpp $(BOOST_LIBS)

# Multicore CPU
multicore: main.cpp
	$(CXX) -acc=multicore $(CXXFLAGS) -o main_multicore main.cpp $(BOOST_LIBS) $(CUDA_INCLUDE)

# Onecore CPU
host: main.cpp
	$(CXX) -acc=host $(CXXFLAGS) -o main_host main.cpp $(BOOST_LIBS) $(CUDA_INCLUDE)

clean:
	rm -f main_gpu main_multicore main_host
