export sqlconnection
sqlconnection="test.db"
./execsql "create table if not exists emp(id int not null primary key, name varchar not null, address varchar not null)"
again="y"
while [[ $again == "y" ]]
do
	printf "ID      : "
	read id
	printf "Name    : "
	read name
	printf "Address : "
	read addr
	./execsql "insert into emp values ($id, '$name', '$addr')"
	printf "Fill it again? (y/n) "
	read again
done
