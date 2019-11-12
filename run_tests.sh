#run all tests
#clust and 1M
cp cache_aware_vi.c.Clust.Anneal cache_aware_vi.c
cp solve.c.1MInitial solve.c
make clean
make
./solve -p ../MDPs/evaluated/sap_900x900.mdp -method ftvi >op_sap_900x900_clust_anneal
./solve -p ../MDPs/evaluated/mcar_1000x1000.mdp -method ftvi >op_mcar_1000x1000_clust_anneal
./solve -p ../MDPs/evaluated/dap-100x100x10x10.mdp -method ftvi >op_dap-100x100x10x10_clust_anneal
#clust and 4M
cp solve.c.4MInitial solve.c
make clean
make
./solve -p ../MDPs/evaluated/mcar_2000x2000.mdp -method ftvi > op_mcar_2000x2000_clust_anneal
./solve -p ../MDPs/evaluated/dap-200x100x10x10.mdp -method ftvi > op_dap-200x100x10x10_clust_anneal

#clustnoann 1M
cp cache_aware_vi.c.clust.noAnneal cache_aware_vi.c
cp solve.c.1MInitial solve.c
make clean
make
./solve -p ../MDPs/evaluated/sap_900x900.mdp -method ftvi >op_sap_900x900_clust_noanneal
./solve -p ../MDPs/evaluated/mcar_1000x1000.mdp -method ftvi >op_mcar_1000x1000_clust_noanneal
./solve -p ../MDPs/evaluated/dap-100x100x10x10.mdp -method ftvi >op_dap-100x100x10x10_clust_noanneal

#clustnoann and 4M
cp solve.c.4MInitial solve.c
make clean
make
./solve -p ../MDPs/evaluated/mcar_2000x2000.mdp -method ftvi > op_mcar_2000x2000_clust_noanneal
./solve -p ../MDPs/evaluated/dap-200x100x10x10.mdp -method ftvi > op_dap-200x100x10x10_clust_noanneal

#noclust and 1M
cp cache_aware_vi.c.noClust.noAnneal cache_aware_vi.c
cp solve.c.1MInitial solve.c
make clean
make
./solve -p ../MDPs/evaluated/sap_900x900.mdp -method ftvi > op_sap_900x900_noclust
./solve -p ../MDPs/evaluated/mcar_1000x1000.mdp -method ftvi > op_mcar_1000x1000_noclust
./solve -p ../MDPs/evaluated/dap-100x100x10x10.mdp -method ftvi > op_dap-100x100x10x10_noclust
#noclust and 4M
cp solve.c.4MInitial solve.c
make clean
make
./solve -p ../MDPs/evaluated/mcar_2000x2000.mdp -method ftvi >op_mcar_2000x2000_noclust
./solve -p ../MDPs/evaluated/dap-200x100x10x10.mdp -method ftvi >op_dap-200x100x10x10_noclust
