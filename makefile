CXX:=g++

objs:=test.o libcryptopp.a

test:$(objs)
	$(CXX) -g -o test.out $(objs)
test.o:merkletree.h test.cpp
	$(CXX) -g -c test.cpp -o test.o
.PHONY:clean

clean:
	rm -f *.o *.out

