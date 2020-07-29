#makefile

solve: 	solve.o lao.o rlao.o blao.o nlao.o pvi.o blao_o.o graph.o stack.o statequeue.o track.o vi.o backup.o rtdp.o lrtdp.o frtdp.o pi.o top.o hdp.o qlearning.o ps.o tql.o rmax.o trmax.o fdp.o ips.o atvi.o math.o ftvi.o brtdp.o vpirtdp.o components.o setgoal.o cache_aware_vi.o cached_vi.o intqueue.o logger.o med_hash.o intheap.o test_fns.o
	icc -g -qopenmp -Wl,-stack_size,6000000 -o solve solve.o lao.o rlao.o blao.o nlao.o pvi.o blao_o.o graph.o stack.o statequeue.o track.o vi.c backup.c rtdp.o lrtdp.o frtdp.o pi.o top.o hdp.o math.o qlearning.o ps.o tql.o rmax.o trmax.o fdp.o ips.o atvi.o ftvi.o brtdp.o vpirtdp.o components.o setgoal.o cache_aware_vi.o cached_vi.o intqueue.o logger.o med_hash.o intheap.o test_fns.o -lm -pthread
#-lpthread

clean:
	rm -f *.o *.a *.il *.ti *\~ core core.* solve


solve.o: solve.c solve.h lao.h rlao.h blao.h nlao.o blao_o.h graph.h stack.h track.h vi.h rtdp.h lrtdp.h frtdp.h pi.h top.h atvi.o ftvi.o hdp.o ips.o math.o
	icc -g -c solve.c

backup.o: backup.c backup.h graph.h
	icc -g -c backup.c

lao.o: 	lao.c lao.h graph.h track.h
	icc -g -c lao.c

rlao.o: rlao.c rlao.h graph.h track.h
	icc -g -c rlao.c

blao.o: blao.c blao.h graph.h track.h
	icc -g -c blao.c -pthread
#-lpthread

nlao.o: nlao.c nlao.h graph.h track.h
	icc -g -c nlao.c -pthread
#-lpthread

pvi.o: pvi.c pvi.h graph.h track.h
	icc -g -c pvi.c

blao_o.o: blao_o.c blao_o.h graph.h track.h
	icc -g -c blao_o.c -pthread
#-lpthread

graph.o: graph.c graph.h lao.h rlao.h blao.h blao_o.h track.h
	 icc -g -c graph.c

stack.o: stack.c stack.h lrtdp.h
	 icc -g -c stack.c
 
statequeue.o: statequeue.c statequeue.h
	 icc -g -c statequeue.c
 
track.o: track.c track.h graph.h lao.h rlao.h blao.h blao_o.h
	 icc -g -c track.c

rtdp.o: rtdp.c rtdp.h graph.h
	icc -g -c rtdp.c

lrtdp.o: lrtdp.c lrtdp.h graph.h stack.h
	icc -g -c lrtdp.c

brtdp.o: brtdp.c brtdp.h graph.h
	icc -g -c brtdp.c

vpirtdp.o: vpirtdp.c vpirtdp.h brtdp.h graph.h
	icc -g -c vpirtdp.c

frtdp.o: frtdp.c frtdp.h graph.h stack.h
	icc -g -c frtdp.c

hdp.o: hdp.c hdp.h graph.h stack.h
	icc -g -c hdp.c

vi.o:	vi.c graph.h
	icc -g -c vi.c

top.o:	top.c graph.h
	icc -g -c top.c

atvi.o:	atvi.c graph.h
	icc -g -c atvi.c

ftvi.o:	ftvi.c graph.h
	gcc -g -c ftvi.c

components.o:	components.c graph.h
	icc -g -c components.c

setgoal.o: setgoal.c graph.h
	icc -g -c setgoal.c

qlearning.o:	qlearning.c graph.h
	icc -g -c qlearning.c

ps.o:	ps.c statequeue.c graph.h
	icc -g -c ps.c

tql.o:	tql.c graph.h
	icc -g -c tql.c

rmax.o:	rmax.c graph.h
	icc -g -c rmax.c

trmax.o:trmax.c graph.h vi.h
	icc -g -c trmax.c

fdp.o:fdp.c graph.h vi.h
	icc -g -c fdp.c

ips.o:ips.c graph.h vi.h
	icc -g -c ips.c

pi.o:  	pi.c graph.h
	icc -g -c pi.c

math.o:	math.c math.c
	icc -g -c math.c

cache_aware_vi.o:cache_aware_vi.c cache_aware_vi.h
	icc -g -c -qopenmp cache_aware_vi.c

cached_vi.o:cached_vi.c cached_vi.h
	icc -g -c -qopenmp cached_vi.c 

intqueue.o:intqueue.c intqueue.h 
	icc -g -c intqueue.c

logger.o:logger.c logger.h
	icc -g -c logger.c

med_hash.o:med_hash.c med_hash.h
	icc -g -c med_hash.c

intheap.o:intheap.c intheap.h
	icc -g -c intheap.c

test_fns.o:test_fns.c test_fns.h
	icc -g -c test_fns.c
