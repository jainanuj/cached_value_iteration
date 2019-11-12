#run all tests
#clust and 1M
cp ftvi.c.nocache ftvi.c
cp solve.c.1MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/sap_900x900.mdp -method ftvi >op_sap_900x900_nocache 2>sap_900_nocache_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_1000x1000.mdp -method ftvi >op_mcar_1000x1000_nocache 2>mcar_1000_nocache_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-100x100x10x10.mdp -method ftvi >op_dap-100x100x10x10_nocache 2>dap-100_nocache_perf
#clust and 4M
cp solve.c.4MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_2000x2000.mdp -method ftvi > op_mcar_2000x2000_nocache 2>mcar2000_nocache_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-200x100x10x10.mdp -method ftvi > op_dap-200x100x10x10_nocache 2>dap-200_nocache_perf

