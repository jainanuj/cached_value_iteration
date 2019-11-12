#run all tests
cp ftvi.c.cache ftvi.c
#clust and 1M
cp cache_aware_vi.c.Clust.Anneal cache_aware_vi.c
cp solve.c.1MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/sap_900x900.mdp -method ftvi >op_sap_900x900_clust_anneal 2>sap_900_clust_ann_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_1000x1000.mdp -method ftvi >op_mcar_1000x1000_clust_anneal 2>mcar_1000_clust_ann_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-100x100x10x10.mdp -method ftvi >op_dap-100x100x10x10_clust_anneal 2>dap-100_clust_ann_perf
#clust and 4M
cp solve.c.4MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_2000x2000.mdp -method ftvi > op_mcar_2000x2000_clust_anneal 2>mcar2000_clust_ann_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-200x100x10x10.mdp -method ftvi > op_dap-200x100x10x10_clust_anneal 2>dap-200_clust_ann_perf

#clustnoann and 1M
cp cache_aware_vi.c.clust.noAnneal cache_aware_vi.c
cp solve.c.1MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/sap_900x900.mdp -method ftvi >op_sap_900x900_clust_noanneal 2>sap_900_clust_noann_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_1000x1000.mdp -method ftvi >op_mcar_1000x1000_clust_noanneal 2>mcar_1000_clust_noann_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-100x100x10x10.mdp -method ftvi >op_dap-100x100x10x10_clust_noanneal 2>dap-100_clust_noann_perf
#clust and 4M
cp solve.c.4MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_2000x2000.mdp -method ftvi > op_mcar_2000x2000_clust_noanneal 2>mcar2000_clust_noann_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-200x100x10x10.mdp -method ftvi > op_dap-200x100x10x10_clust_noanneal 2>dap-200_clust_noann_perf

#noclust and 1M
cp cache_aware_vi.c.noClust.noAnneal cache_aware_vi.c
cp solve.c.1MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/sap_900x900.mdp -method ftvi > op_sap_900x900_noclust 2>sap_900_noclust_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_1000x1000.mdp -method ftvi > op_mcar_1000x1000_noclust 2>mcar_1000_noclust_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-100x100x10x10.mdp -method ftvi > op_dap-100x100x10x10_noclust 2>dap-100_noclust_perf
#noclust and 4M
cp solve.c.4MInitial solve.c
make clean
make
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/mcar_2000x2000.mdp -method ftvi >op_mcar_2000x2000_noclust 2>mcar_2000_noclust_perf
perf stat -e task-clock,cycles,instructions,cache-references,cache-misses ./solve -p ../MDPs/evaluated/dap-200x100x10x10.mdp -method ftvi >op_dap-200x100x10x10_noclust 2>dap-200_noclust_perf
