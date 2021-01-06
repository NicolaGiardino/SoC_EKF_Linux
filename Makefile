all:main

matrix.o: ./lib/matrix.c ./include/matrix.h
	gcc -c ./lib/matrix.c -g

SOC_EKF.o: ./lib/SOC_EKF.c ./include/SOC_EKF.h ./include/matrix.h
	gcc -c ./lib/SOC_EKF.c -g

libthreads.o: ./lib/libthreads.c ./include/libthreads.h
	gcc -c ./lib/libthreads.c -g

procedure.o: ./lib/procedure.c ./include/procedure.h ./include/SOC_EKF.h ./include/libthreads.h
	gcc -c ./lib/procedure.c -g

main.o: main.c ./include/procedure.h
	gcc -c main.c

main: main.o matrix.o SOC_EKF.o libthreads.o procedure.o
	gcc -ggdb -o main main.o matrix.o SOC_EKF.o libthreads.o procedure.o -lm -lpthread

clean:
	rm -f ./*.o

