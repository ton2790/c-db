CC=gcc
CFLAGS=

db: main.o inputbuffer.o metacommand.o row.o table.o statement.o pager.o cursor.o btree.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

inputbuffer.o: inputbuffer.c
	$(CC) $(CFLAGS) -c -o $@ $<

metacommand.o: metacommand.c
	$(CC) $(CFLAGS) -c -o $@ $<

row.o: row.c
	$(CC) $(CFLAGS) -c -o $@ $<

table.o: table.c
	$(CC) $(CFLAGS) -c -o $@ $<

statement.o: statement.c
	$(CC) $(CFLAGS) -c -o $@ $<

pager.o: pager.c
	$(CC) $(CFLAGS) -c -o $@ $<

cursor.o: cursor.c
	$(CC) $(CFLAGS) -c -o $@ $<

btree.o: btree.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	-rm *.o ./db ./mydb.db
