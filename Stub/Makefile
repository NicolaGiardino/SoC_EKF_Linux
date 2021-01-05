all:main


main.o: ./main.c ./include/procedure_stub.h
	gcc -c ./main.c -g
	
matrix.o: ../lib/matrix.c ../include/matrix.h
	gcc -c ../lib/matrix.c -g

SOC_EKF.o: ../lib/SOC_EKF.c ../include/SOC_EKF.h ../include/matrix.h
	gcc -c ../lib/SOC_EKF.c -g

libthreads.o: ../lib/libthreads.c ../include/libthreads.h
	gcc -c ../lib/libthreads.c -g

procedure_stub.o: ./lib/procedure_stub.c ./include/procedure_stub.h ../include/SOC_EKF.h ../include/libthreads.h
	gcc -c ./lib/procedure_stub.c -g

main: main.o matrix.o SOC_EKF.o libthreads.o procedure_stub.o
	gcc -ggdb -Wall -o main main.o matrix.o SOC_EKF.o libthreads.o procedure_stub.o -lm -lpthread

clean:
	rm -f ./*.o

