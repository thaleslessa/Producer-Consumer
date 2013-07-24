consumerProducer: consumerProducer.o
	gcc consumerProducer.o -o consumerProducer -lpthread
	rm consumerProducer.o

consumerProducer.o: consumerProducer.c
	gcc -c consumerProducer.c
