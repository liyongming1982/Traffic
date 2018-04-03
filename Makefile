otherport.so: peartraffic.c
	gcc -Wall -nostartfiles -fpic -shared peartraffic.c -o peartraffic.so -ldl -D_GNU_SOURCE
