let l=20
let a=10
let s=10
for p in $(seq 10000 10000 100000)
do
	echo $p >> res_random
	for x in $(seq 1 1 10)
	do
		cd mdps
		./lranMDPgen $p $a $s $l
		cd ..
		./solve -p mdps/mdp.txt -method vi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method lao -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method lrtdp -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method tvi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method ftvi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method brtdp -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
	done
done
for l in $(seq 20 20 100)
do
	echo $l >> res_random
	for x in $(seq 1 1 10)
	do
		cd mdps
		./lranMDPgen 50400 $a $s $l
		cd ..
		./solve -p mdps/mdp.txt -method vi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method lao -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method lrtdp -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method tvi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method ftvi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method brtdp -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
	done
done
for l in $(seq 200 100 1000)
do
	echo $l >> res_random
	for x in $(seq 1 1 10)
	do
		cd mdps
		./lranMDPgen 50400 $a $s $l
		cd ..
		./solve -p mdps/mdp.txt -method vi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method lao -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method lrtdp -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method tvi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method ftvi -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
		./solve -p mdps/mdp.txt -method brtdp -initial 0 -hdiscount 1.0 > tmp.log
		t=$(cat tmp.log|grep "Converge"|cut -f 3 -d ' ')
		echo $t >> res_random
	done
done

