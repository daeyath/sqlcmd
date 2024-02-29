const char appname[]="SQLCMD";
const char copyright[]="Copyright (C) 2023 HIDAYAT";
const char ver[]="0.1.8";
const char license_version[]="GPL Version 3";
const char copylinks[]="https://www.gnu.org/licenses/";

sqlite3 *db;
char *zErrMsg=0;
bool conn;

bool connectdb(const char *filename){
	conn=false;
	if(filename[0]!='-'){
		conn=sqlite3_open(filename, &db)==SQLITE_OK;
		if(!conn){
			fprintf(stderr, "Error: %s '%s'\n", sqlite3_errmsg(db), filename);
		}
	}else{
		fprintf(stderr, "File name not accepted: %s\n",filename);
	}
	return conn;
}

bool closedb(){
	bool closed = sqlite3_close(db)==SQLITE_OK || !conn;
	if(!closed) printf("The database can not be closed yet\n");
	return closed;
}

int exec(const char *str){
	int state=0;
	char **azResult;
	int nRow, nColumn;
	int query=sqlite3_get_table(db, str, &azResult, &nRow, &nColumn, &zErrMsg);
	if(query==SQLITE_OK){
		if(nRow==1){
			int l=0, k=0;
			for(int i=0; i<nColumn; i++){
				l=strlen(azResult[i]);
				if(l>k)k=l;
			}
			for(int i=0; i<nColumn; i++){
				printf("%-*s = %s\n",k,azResult[i],azResult[nColumn+i]);
			}
		}else if(nRow>1){
			int lenCol[nColumn];
			for(int i=0; i<nColumn; i++)lenCol[i]=0;
			int z=-1;
			for(int i=0; i<nRow+1; i++){
				for(int j=0; j<nColumn; j++){
					++z;
					int l=azResult[z]==NULL?6:strlen(azResult[z]);
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
			int stmtc=0;
			for(int i=0; i<strlen(str); i++){
				if(str[i]==';')stmtc++;
				if(stmtc>1)break;
			}
			if(stmtc<2){
				const char ins[]="insert",
					upd[]="update",
					del[]="delete";
				char cmd[6]; int j=0; char b;
				for(int i=0; i<strlen(str); i++){
					b=tolower(str[i]);
					if(b!=32){
						if(b==ins[j]||b==upd[j]||b==del[j]){
							cmd[j]=b;j++;
							if(j==6){cmd[j]='\0';break;}
						}else break;
					}
				}
				if(strcmp(cmd,"insert")==0
					||strcmp(cmd,"update")==0
					||strcmp(cmd,"delete")==0){
					int changes=sqlite3_changes(db);
					if(changes>0){
						cmd[0]=toupper(cmd[0]);
						printf("%s %i\n", cmd, changes);
					}
				}
			}else printf("ok\n");
		}
	}else{
		fprintf(stderr,"SQL Error: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
		state=5;
	}
	sqlite3_free_table(azResult);
	return state;
}
