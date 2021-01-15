all:main

matrix.o: ../lib/matrix.c ../include/matrix.h
	gcc -Wall -Wextra -c ../lib/matrix.c -g

SOC_EKF.o: ../lib/SOC_EKF.c ../include/SOC_EKF.h ../include/matrix.h
	gcc -Wall -Wextra -c ../lib/SOC_EKF.c -g

libthreads.o: ../lib/libthreads.c ../include/libthreads.h
	gcc -Wall -Wextra -c ../lib/libthreads.c -g

procedure_stub.o: ./lib/procedure_stub.c ./include/procedure_stub.h ../include/SOC_EKF.h 
	gcc -Wall -Wextra -c ./lib/procedure_stub.c -g

main.o: main.c ./include/procedure_stub.h 
	gcc -Wall -Wextra -c main.c

main: main.o matrix.o SOC_EKF.o libthreads.o procedure_stub.o
	gcc -ggdb -o main main.o matrix.o SOC_EKF.o libthreads.o procedure_stub.o -lm -lpthread

clean:
	rm -f *.o

