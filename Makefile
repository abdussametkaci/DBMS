all: program

program : program.c database kaydet
	gcc -o program program.c

database: database.c 
	gcc -o database database.c

kaydet : kaydet.c 
	gcc -o kaydet kaydet.c