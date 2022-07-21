all : main.c
	gcc -o main.exe ./main.c `pkgconf --libs libcurl libcjson tidy`
	./main.exe
