/* Run sql command from shell script,
read & write to dbfile */
#include <iostream>
#include <sqlite3.h>
#include "database.cxx"
int main(int argc, char *argv[])
{
	int state=0;
	const char sqlconstr[]="sqlconnection";
	char *dbfile=getenv(sqlconstr);
	if(dbfile!=NULL){
		if(connectdb(dbfile)){
			if(argv[1])
				state=exec(argv[1]);
			else
				state=2;
			if(!closedb()) state=4;
		}else state=3;
	}else{
		printf("%s\n",copyright);
		printf("Usage:\n\nexport %s\n%s=\"a file.db\"\n%s \"sql command\"\n",sqlconstr,sqlconstr,argv[0]);
		state=1;
	}
	return state;
}