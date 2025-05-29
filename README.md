# Parallel-Programming-6

```
pgc++ -acc -Minfo=all -std=c++17 -o main main.cpp -lboost_program_options

```

```
pgc++ -acc=multicore -Minfo=all -std=c++17 -o main main.cpp -lboost_program_options -I/opt/nvidia/hpc_sdk/Linux_x86_6
4/23.11/cuda/12.3/targets/x86_64-linux/include
```
```
pgc++ -acc=host -Minfo=all -std=c++17 -o main main.cpp -lboost_program_options -I/opt/nvidia/hpc_sdk/Linux_x86_6
4/23.11/cuda/12.3/targets/x86_64-linux/include
```
```
./main --size 512 --tol 1e-6 --max_iter 1000000
```
