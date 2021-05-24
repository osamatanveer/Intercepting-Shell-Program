numOfBytes=4096
mode=2

executable: isp.c
	gcc -o executable isp.c
	gcc -o outC consumer.c
	gcc -o outP producer.c

run:
	./executable $(numOfBytes) $(mode)

clean:
	rm executable outC outP
	
