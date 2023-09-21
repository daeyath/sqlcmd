#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <sqlite3.h>

using namespace std;

sqlite3 *db;
char *zErrMsg=0;
int rc;
bool colexists;

static int result(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	if(!colexists){
		for(i=0; i<argc; i++){
			printf("%s\t", azColName[i]);
		}
		colexists=true;
	}
	printf("\n");
	for(i=0; i<argc; i++){
		printf("%s\t", argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

void cmd(){
	const int length=1024;
	char sqlstr[length];
	while(true){
		printf("sql> ");
		fgets(sqlstr,length,stdin);
		if(strncmp("exit",sqlstr,4)==0){
			break;
		}else{
			colexists=false;
			rc=sqlite3_exec(db,sqlstr,result,0,&zErrMsg);
			if(rc!=SQLITE_OK){
				fprintf(stderr,"SQL Error: %s",zErrMsg);
				sqlite3_free(zErrMsg);
			}
			printf("\n");
		}
	}
}

int main(int argc, char *argv[])
{
	char dbname[]="test.db";
	rc=sqlite3_open(dbname, &db);
	if(rc){
		fprintf(stderr, "Tidak dapat membuka database %s\n", sqlite3_errmsg(db));
		return(0);
	}else{
		fprintf(stderr, "Database terkoneksi\n");
		cmd();
	}
	sqlite3_close(db);
}