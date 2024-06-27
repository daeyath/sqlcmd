#include <iostream>
#include <map>
#include <fstream>
#include <sqlite3.h>

using namespace std;

const char appname[]="SQLCMD";
const char copyright[]="Copyright (C) 2023 HIDAYAT";
const char ver[]="0.1.10";
const char license_version[]="GPL Version 3";
const char copylinks[]="https://www.gnu.org/licenses/";

class query {
		string lastsql;
		const int tlm=6;
		char *term;
		void fixterm(string&);
		void setterm(const char*);
		void help();
		virtual void show_object(const char*)=0;
		virtual void exec(const char*)=0;
		virtual void xp(const char*)=0;
		virtual bool disconnected()=0;
		int runinternal(string&);
		int execfile(const char*);
	public:
		void serve();
		virtual void connect(const char*)=0;
		virtual bool connected()=0;
		query();
		virtual ~query();
};

class sqlite: public query {
		sqlite3 *db;
		bool conn=false;
		void show_object(const char*);
		bool disconnected();
		void disconnect();
		void exec(const char*);
		void xp(const char*);
		static int xpresult(void*, int, char**, char**);
	public:
		void connect(const char*);
		bool connected();
};

void sqlite::xp(const char *exp){
	char stmt[7+strlen(exp)];
	strcpy(stmt,"SELECT ");
	strcat(stmt,exp);
	int res=sqlite3_exec(db, stmt, xpresult, 0, 0);
}

int sqlite::xpresult(void *NotUsed, int argc, char **argv, char **colname){
	printf("%s\n", argv[0]);
}

void sqlite::connect(const char *f){
	if(conn){
		puts("Please close the current database");
	}else{
		bool val=false;
		for(int i=0; i<strlen(f); i++){
			val=(f[i]>=97 && 122>=f[i]) 
				|| (f[i]>=65 && f[i]<=90)
				|| f[i]==58 || f[i]==46;
			if(!val)break;
		}
		if(val){
			conn=sqlite3_open(f, &db)==SQLITE_OK;
			if(!conn)
				fprintf(stderr, "Error: %s '%s'\n", sqlite3_errmsg(db), f);
		}else{
			fprintf(stderr, "File name not accepted: %s\n",f);
		}
	}
}

bool sqlite::connected(){return conn;}

void sqlite::disconnect(){
	bool closed = sqlite3_close(db)==SQLITE_OK;
	if(closed || !conn){
		conn=false;
	}else{
		puts("The database can not be closed yet");
		conn=true;
	}
}

bool sqlite::disconnected(){
	disconnect();
	return !conn;
}

void sqlite::exec(const char *str){
	char *zErrMsg=0;
	char **azResult;
	int nRow, nColumn;
	int query=sqlite3_get_table(
		db, 
		str, 
		&azResult, 
		&nRow, 
		&nColumn, 
		&zErrMsg
	);
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
			}else puts("ok");
		}
	}else{
		fprintf(stderr,"SQL Error: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_free_table(azResult);
}

void query::help(){
	puts(":c dbfile -> connect to database file");
	puts(":x file -> execute sql file");
	puts(":l -> show license");
	puts(":print text -> show a text");
	puts(":q -> exit and close database");
	puts(":r command -> run shell command");
	puts(":s object -> show objects, ex: :s table");
	puts(":v term char -> set char for terminator");
	puts(":w file -> write last typing to a file");
}

void sqlite::show_object(const char *objname){
	char str[]="select name as \"%s\" from sqlite_schema where type='%s'";
	char tmp[]=" and name not like 'sqlite_%'";
	int len=0;
	len+=strlen(str)-1;
	len+=strlen(tmp)-1;
	len+=(2*(strlen(objname)-1));
	char sql[len];
	sprintf(sql, str, objname, objname);
	strcat(sql, tmp);
	exec(sql);
}

void query::setterm(const char *f){
	if(f!=0){
		const int len=strlen(f);
		if(len<=tlm){
			strcpy(term,f);
		}else
			printf("Limited to %i\n",tlm);
	}else printf("terminating with \"%s\" for executing query\n",term);
}

void query::fixterm(string &s){
	if(strcmp(term,";")!=0) s.erase(
		s.find(term), strlen(term)
	);
}

int query::runinternal(string &s){
	const char *cmd=s.c_str();
	int value=0, j=0;
	while(cmd[j]!=32 && cmd[j]!=0)j++;
	const char *param=0;
	if(cmd[j]!=0)param=cmd+j+1;
	if(strncmp(cmd,":c",j)==0){
		if(param!=NULL)
			if(disconnected())
				connect(param);
	}else if(strncmp(cmd,":x",j)==0){
		if(param!=NULL){
			const char *path=param;
			value=execfile(path);
		}
	}else if(strncmp(cmd,":xp",j)==0){
		if(param!=0) xp(param);
	}else if(strncmp(cmd,":print",j)==0){
		if(param!=NULL)
			printf("%s\n",param);
	}else if(strncmp(cmd,":q",j)==0){
		value=1;
	}else if(strncmp(cmd,":r",j)==0){
		system(param);
	}else if(strncmp(cmd,":v",j)==0){
		if(param!=NULL){
			if(strncmp(param,"term",4)==0){
				const char *dlm=0;
				if(param[4]!=0)dlm=param+5;
				setterm(dlm);
			}
		}
	}else if(strncmp(cmd,":s",j)==0){
		if(param!=NULL)show_object(param);
	}else if(strncmp(cmd,":w",j)==0){
		if(param!=NULL){
			FILE *f=fopen(param,"w");
			if(f!=NULL){
				if(!lastsql.empty())
					fprintf(f,"%s",lastsql.c_str());
				fclose(f);
			}else{
				fprintf(stderr,"File can't wrote.\n");
			}
		}
	}else if(strncmp(cmd,":l",j)==0){
		printf("%s %s\n",appname,copyright);
		printf("See %s at %s\n",license_version, copylinks);
	}else if(strncmp(cmd,":h",j)==0){
		help();
	}else{
		printf("'%s' command not found\n", cmd);
	}
	return value;
}

int query::execfile(const char *fname){
	int state=0;
	ifstream sqlfile(fname);
	if(sqlfile){
		string sql, line;
		struct paramstr {
			bool empty(){
				return d.empty() && !tag;
			}
			void data(string d){
				if(!d.empty())this->d = d;
				tag = d.empty();
			}
			const char *data(){
				return d.c_str();
			}
			private:
				string d;
				int tag=0;
		} *pr;
		map<string,paramstr> param;
		while(getline(sqlfile, line)){
			string::size_type start;
			while((start=line.find("${"))!=string::npos){
				string::size_type end=line.find("}");
				if(start<end){
					string name=line.substr(start+2,end-start-2);
					pr=&param[name];
					if(pr->empty()){
						cout<<name<<": "; string result;
						getline(cin, result);
						pr->data(result);
					}
					line.replace(start, end-start+1, pr->data());
				}else break;
			}
			if(line[0]==':'){
				state=runinternal(line);
			}else{
				sql+=line+"\n";
				if(line.find(term)!=string::npos){
					fixterm(sql);
					exec(sql.c_str());
					lastsql=sql;
					sql.clear();
				}
			}
		}
		if(!sql.empty()){
			exec(sql.c_str());
			lastsql=sql;
		}
		sqlfile.close();
	}else{
		printf("File '%s' not found.\n", fname);
	}
	return state;
}

void query::serve(){
	string sql, str;
	int newline=1, cmd=0;
	serv:
	printf(newline?"sql> ":"   | ");
	getline(cin, str);
	if(str[0]==':' && newline){
		cmd=runinternal(str);
		if(cmd==1)
			if(!disconnected())
				cmd=0;
	}else{
		sql+=str+"\n";
		newline=str.find(term)!=string::npos;
		if(newline){
			fixterm(sql);
			exec(sql.c_str());
			lastsql=sql;
			sql.clear();
		}
	}
	if(cmd==0)goto serv;
}

query::query(){
	term=new char[tlm];
	setterm(";");
}

query::~query(){
	free(term);
}

int main(int argc, char *argv[]){
	int state=1;
	printf("%s Version %s\n",appname,ver);
	const char *connection; char *tmp=0;
	query *dbx = 0;
	{
		dbx = new sqlite();
		connection=argv[1];
		if(!connection){
			tmp=new char[8];
			strcpy(tmp,":memory:");
			connection=tmp;
		}
	}
	if(dbx){
		dbx->connect(connection);
		if(dbx->connected()){
			puts("Type :h for help");
			dbx->serve();
			puts("See u!");
			state=0;
		}
		delete dbx; 
	}
	if(tmp)free(tmp);
	return state;
}