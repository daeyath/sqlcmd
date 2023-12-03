sqlite3 *db;
char *zErrMsg=0;

bool connectdb(const char *filename){
	int error=sqlite3_open(filename, &db);
	if(error)
		fprintf(stderr, "Error: %s '%s'\n", sqlite3_errmsg(db), filename);
	return !error;
}

bool closedb(){
	bool closed = sqlite3_close(db)==SQLITE_OK;
	if(!closed) printf("The database can not be closed yet\n");
	return closed;
}

int exec(const char *str){
	int state=0;
	char **azResult;
	int nRow, nColumn;
	int query=sqlite3_get_table(db, str, &azResult, &nRow, &nColumn, &zErrMsg);
	if(query!=SQLITE_OK){
		fprintf(stderr,"SQL Error: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
		state=5;
	}else if(nRow>0){
		int lenCol[nColumn];
		for(int i=0; i<nColumn; i++)lenCol[i]=0;
		int z=-1;
		for(int i=0; i<nRow+1; i++){
			for(int j=0; j<nColumn; j++){
				++z;
				int l=azResult[z]==NULL?0:strlen(azResult[z]);
				if(lenCol[j]<l)lenCol[j]=l;
			}
		}	
		z=-1;
		for(int i=0; i<nRow+1; i++){
			for(int j=0; j<nColumn; j++){
				++z;
				printf("%-*s", lenCol[j], azResult[z]);
				if(j<nColumn-1)printf(" ");
			}
			printf("\n");
			if(i==0){
				for(int j=0; j<nColumn; j++){
					for(int k=0; k<lenCol[j]; k++)printf("=");
					if(j<nColumn-1)printf(" ");
				}
				printf("\n");
			}
		}
		printf("(%i rows)\n", nRow);
	}else{
		int changes=sqlite3_changes(db);
		if(changes>0)
			printf("%i row changes\n", changes);
	}
	sqlite3_free_table(azResult);
	return state;
}