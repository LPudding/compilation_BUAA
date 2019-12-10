#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>

const long long int sp_top=0x7fffeffc ;//0x7fffeffc
char token[1000],beforeid[1000],beforetoken[1000]; //use getsym to get a word
char nexttoken[10][1000];
int nextid[10],beforetype,infunc,symbol_recent=0,sp_reserve,wyzsymboli=0,maini=0; //infunc记录函数类型
char ch,a0[100],a1[100],a2[100],a3[100],int2str[20]; //get char from file
int id,tablei=0,layer=0,beforeline,sp_recent_top=0x7fffeffc ; //kind记录表达式类型
int sp_recent=0x7fffeffc ,parai=0,funcparai=0; //kind记录表达式类型
int line=1,symboli=0,returnflag=0,fouri=0,ti=0,ifi=0,whilei=0,doi=0,fori=0,wyzi=0,wyzstringi=0;
FILE *fp1,*fp2;

struct fourexpression {
	char a[100];
	char b[100];
	char c[100];
	char d[100];
} four[2000];

struct parameter {
	char a[1000];
	int type;
	struct parameter *next;
} ;

struct symboltable {
	char a[1000];
	int type;
	int dimension;
	long long int addr;
	int layers;
	long long int inaddr;
	struct parameter *para;
} table[1000];

struct symboltable_run {
	char a[100];
	int dimension;
	long long int addr;
	int layers;
} symbol[1000];

struct retsymbol{
	long long int addr;
	int layers;
};

void sentencelist();
int expression();
void sentence();
void printerror(char s);
void printmips(char op[100],char a[100],char b[100],char c[100]);


char* itoa(int num,char* str,int radix) {
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
	unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
	int i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。

	//获取要转换的整数的绝对值
	if(radix==10&&num<0) { //要转换成十进制数并且是负数
		unum=(unsigned)-num;//将num的绝对值赋给unum
		str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
	} else unum=(unsigned)num; //若是num为正，直接赋值给unum

	//转换部分，注意转换后是逆序的
	do {
		str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
		unum/=radix;//unum去掉最后一位

	} while(unum);//直至unum为0退出循环

	str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。

	//将顺序调整过来
	if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
	else k=0;//不是负数，全部都要调整

	char temp;//临时变量，交换两个值时用到
	for(j=k; j<=(i-1)/2; j++) { //头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
		temp=str[j];//头部赋值给临时变量
		str[j]=str[i-1+k-j];//尾部赋值给头部
		str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
	}

	return str;//返回转换后的字符串
}

void init() {
	int i,j;
	for(i=0; i<1000; i++) {
		token[i]='\0';
	}
	for(i=0; i<10; i++) {
		for(j=0; j<1000; j++) {
			nexttoken[i][j]='\0';
		}
	}
}

void clearToken() {
	int i=strlen(token);
	for(; i>=0; i--) {
		token[i]='\0';
	}
}

void clearnexttoken() {
	int i,j=0;
	for(; j<10; j++) {
		i=strlen(nexttoken[j]);
		for(i=i-1; i>=0; i--) {
			nexttoken[j][i]='\0';
		}
	}
}

void catToken() { //token get ch
	int i=0;
	for(; i<strlen(token); i++) {
		if(token[i]=='\0') {
			break;
		}
	}
	token[i]=ch;
}

int isrelation() {
	if(id>=21&&id<=26) {
		return 1;
	} else {
		return 0;
	}
}

int isunderline(char c) {
	if(c=='_') {
		return 1;
	} else {
		return 0;
	}
}

void getch() {
	ch=fgetc(fp1);
	if(ch=='\n') {
		line++;
	}
}

int char2int(char a[1000]) {
	int i=0,r=0;
	while(a[i]!='\0') {
		if(isdigit(a[i])) {
			r=r*10+a[i]-'0';
		}
		i++;
	}
	return r;
}

int char2integer(char a[20]) {
	return (int) (a[1]);
}

void addpara(struct parameter** phead,char a[1000],int type) {
	struct parameter *head,*node,*n;
	head=*phead;
	if(head==NULL) {
		head=(struct parameter*)malloc(sizeof(struct parameter));
		strcpy(head->a,a);
		head->type=type;
		head->next=NULL;
		*phead=head;
	} else {
		node=head;
		while(node->next!=NULL) {
			node=node->next;
		}
		n=(struct parameter*)malloc(sizeof(struct parameter));
		strcpy(n->a,a);
		n->type=type;
		node->next=n;
		n->next=NULL;
		*phead=head;
	}
}

int findpara(struct parameter* head,char a[1000]) {
	struct parameter *node;
	node=head;
	if(head==NULL) {
		return 0;
	}
	while(1) {
		if(strcmp(node->a,a)==0) {
			return 1;
		} else {
			if(node->next==NULL) {
				break;
			} else {
				node=node->next;
			}
		}
	}
	return 0;
}

int inspectsymbol (char a[1000],int layers) { //1对，0错
	int i;
	for(i=0; i<tablei; i++) {
		if(strcmp(table[i].a,a)==0&&layers==table[i].layers) {
			printerror('b');
			return 0;
		}
	}
	return 1;
}

void addsymbol(char a[1000],int type,int dimension,long long int addr,int layers) {
	char c[1000];
	int i,flag=0;
	if(inspectsymbol(a,layers)) {
		strcpy(table[tablei].a,a);
		table[tablei].addr=addr;
		table[tablei].dimension=dimension;
		table[tablei].type=type;
		table[tablei].layers=layers;
		if(a[0]=='w'&&a[1]=='y'&&a[2]=='z'&&isdigit(a[3])){
			strcpy(c,a+3);
			for(i=0;i<strlen(c);i++){
				if(!isdigit(c[i])){
					flag=1;
					break;
				}
			}
			if(flag==0){
				wyzi=char2int(c)+1;
			}
		}
		tablei++;
	}
}

void addsymbol_run(char a[100],int dimension) {
	funcparai+=dimension;
    sp_recent-=4*dimension;
    strcpy(symbol[symboli].a,a);
    if(layer>0){
    	symbol[symboli].addr=sp_recent-sp_recent_top;
	}else{
		symbol[symboli].addr=sp_recent;
	}
    symbol[symboli].dimension=dimension;
    symbol[symboli].layers=layer;
    symboli++;
	/*strcpy(symbol[symboli].a,a);
	if(a[0]=='w'&&a[1]=='y'&&a[2]=='z'&&isdigit(a[3])){
		wyzsymboli++;
		symbol[symboli].addr=-wyzsymboli*4;
		symbol[symboli].dimension=1;
		symbol[symboli].layers=-1;
	}else{
	    sp_recent-=4*dimension;
	    funcparai+=dimension;
		if(layer>0) {
			symbol[symboli].addr=sp_recent-sp_recent_top;
		} else {
			symbol[symboli].addr=sp_recent;
		}
		symbol[symboli].dimension=dimension;
		symbol[symboli].layers=layer;
	}
	symboli++;*/
	//printf("%d %s %d %lld %d\n",funcparai,a,symbol[symboli-1].layers,symbol[symboli-1].addr,symbol[symboli-1].dimension);
}

void updatesymbol() {
	int i;
	for(i=tablei-1; i>=0; i--) {
		if(table[i].layers==layer) {
			tablei--;
		} else if(table[i].layers<layer) {
			break;
		}
	}
	layer--;
}

void updatesymbol_run() {
	int i;
	for(i=symboli-1; i>=0; i--) {
		if(symbol[i].layers==layer) {
			symboli--;
			sp_recent+=4;
		} else if(symbol[i].layers==-1){
			continue;
		} else if(symbol[i].layers<layer) {
			break;
		}
	}
	sp_recent+=12;
	layer--;
}

int findsymbol(char a[1000]) {
	int i;
	for(i=0; i<tablei; i++) {
		if(strcmp(table[i].a,a)==0) {
			return i;
		}
	}
	printerror('c');

	return -1;
}

struct retsymbol findsymbol_run(char a[100]) {
	int i;
	struct retsymbol p;
	for(i=symboli-1; i>=0; i--) {
		if(strcmp(symbol[i].a,a)==0) {
			p.addr=symbol[i].addr;
			p.layers=symbol[i].layers;
			return p;
		}
	}
	//printf("%s\n",a);
	addsymbol_run(a,1);
	p.addr=symbol[symboli-1].addr;
	p.layers=symbol[symboli-1].layers;
	return p;
	printerror('c');
}

void updatetable() {
	int i;
	for(i=tablei-1; i>=0; i--) {
		if(table[i].layers==layer) {
			tablei--;
		} else if(table[i].layers<layer) {
			break;
		}
	}
	layer--;
}

void addfour() {
	strcpy(four[fouri].a,a0);
	strcpy(four[fouri].b,a1);
	strcpy(four[fouri].c,a2);
	strcpy(four[fouri].d,a3);
	printf("%d ",fouri);
	printf("%s ",four[fouri].a);
	printf("%s ",four[fouri].b);
	printf("%s ",four[fouri].c);
	printf("%s\n",four[fouri].d);
	fouri++;
}

void retract() { //pointer rollback
	if(ch=='\n') {
		line--;
	}
	fseek(fp1,-1,SEEK_CUR);
}

int reserver() { //get identifier
	if(strcmp(token,"const")==0) {
		return 4;
	} else if(strcmp(token,"int")==0) {
		return 5;
	} else if(strcmp(token,"char")==0) {
		return 6;
	} else if(strcmp(token,"void")==0) {
		return 7;
	} else if(strcmp(token,"main")==0) {
		return 8;
	} else if(strcmp(token,"if")==0) {
		return 9;
	} else if(strcmp(token,"else")==0) {
		return 10;
	} else if(strcmp(token,"do")==0) {
		return 11;
	} else if(strcmp(token,"while")==0) {
		return 12;
	} else if(strcmp(token,"for")==0) {
		return 13;
	} else if(strcmp(token,"scanf")==0) {
		return 14;
	} else if(strcmp(token,"printf")==0) {
		return 15;
	} else if(strcmp(token,"return")==0) {
		return 16;
	} else {
		return 0;
	}
}

void getsym() { //get word
	int resultValue;
	int flag=0;
	beforeline=line;
	clearToken();
	getch();
	while(isspace(ch)) {
		getch();  /*读取字符，跳过空格、换行和Tab*/
	}
	if(isalpha(ch)||isunderline(ch)) {                            /*判断当前字符是否是一个字母*/
		while(isalpha(ch)||isdigit(ch)||isunderline(ch)) {             /*将字符拼接成字符串*/
			catToken();
			getch();
		}
		retract();                             /*指针后退一个字符*/
		resultValue = reserver();              /*resultValue是查找保留字的返回值*/
	} else if(isdigit(ch)) {                         /*判断当前字符是否是一个数字*/
		while(isdigit(ch)) {                      /*将字符拼接成整数*/
			catToken();
			getch();
		}
		retract();
		resultValue=1;
	} else if(ch=='\'') {                         /*判断当前字符是否是一个字符*/
		getch();
		if(isdigit(ch)||isalpha(ch)||ch=='+'||ch=='-'||ch=='*'||ch=='/'||isunderline(ch)) {
			catToken();
			resultValue=2;
		} else {
			resultValue=2;
			//fprintf(fp2,"%d a\n",line);
		}
		getch();
	} else if(ch=='\"') {                         /*判断当前字符是否是一个字符串*/
		getch();
		while(ch!='\"') {
			if(ch>=32&&ch<=126&&ch!=34) {
				catToken();
			} else {
				flag=1;
			}
			getch();
		}
		if(flag==0) {
			resultValue=3;
		} else {
			resultValue=3;
			fprintf(fp2,"%d a\n",line);
		}
	} else if(ch=='+') {
		catToken();
		resultValue=17;
	} else if(ch=='-') {
		catToken();
		resultValue=18;
	} else if(ch=='*') {
		catToken();
		resultValue=19;
	} else if(ch=='/') {
		catToken();
		resultValue=20;
	} else if(ch=='<') {
		catToken();
		getch();
		if(ch=='=') {
			resultValue=22;
			catToken();
		} else {
			retract();
			resultValue=21;
		}
	} else if(ch=='>') {
		catToken();
		getch();
		if(ch=='=') {
			catToken();
			resultValue=24;
		} else {
			retract();
			resultValue=23;
		}
	} else if(ch=='=') {
		catToken();
		getch();
		if(ch=='=') {
			catToken();
			resultValue=25;
		} else {
			retract();
			resultValue=27;
		}
	} else if(ch=='!') {
		catToken();
		getch();
		if(ch=='=') {
			catToken();
			resultValue=26;
		} else {
			retract();
			resultValue=-1;
		}
	} else if(ch==';') {
		catToken();
		resultValue=28;
	} else if(ch==',') {
		catToken();
		resultValue=29;
	} else if(ch=='(') {
		catToken();
		resultValue=30;
	} else if(ch==')') {
		catToken();
		resultValue=31;
	} else if(ch=='[') {
		catToken();
		resultValue=32;
	} else if(ch==']') {
		catToken();
		resultValue=33;
	} else if(ch=='{') {
		catToken();
		resultValue=34;
	} else if(ch=='}') {
		catToken();
		resultValue=35;
	} else {
		resultValue=-1;
	}
	id=resultValue;
	if(id==-1) {
		fprintf(fp2,"%d a\n",line);
	}
}

void printerror(char s) {
	    /*if(s=='a'){
	        fprintf(fp2,"%d a",line);
	    }else if(s=='b'){
	        fprintf(fp2,"%d b",line);
	    }else if(s=='c'){
	        fprintf(fp2,"%d c",line);
	    }else if(s=='d'){
	        fprintf(fp2,"%d d",line);
	    }else if(s=='e'){
	        fprintf(fp2,"%d e",line);
	    }else if(s=='f'){
	        fprintf(fp2,"%d f",line);
	    }else if(s=='g'){
	        fprintf(fp2,"%d g",line);
	    }else if(s=='h'){
	        fprintf(fp2,"%d h",line);
	    }else if(s=='i'){
	        fprintf(fp2,"%d i",line);
	    }else if(s=='j'){
	        fprintf(fp2,"%d j",line);
	    }else if(s=='k'){
	        fprintf(fp2,"%d k",beforeline);
	    }else if(s=='l'){
	        fprintf(fp2,"%d l",beforeline);
	    }else if(s=='m'){
	        fprintf(fp2,"%d m",beforeline);
	    }else if(s=='n'){
	        fprintf(fp2,"%d n",beforeline);
	    }else if(s=='o'){
	        fprintf(fp2,"%d o",line);
	    }
	    fprintf(fp2,"\n");*/ 
}

void print(int i) {
	        /*if(i==0){
	            fprintf(fp2,"IDENFR %s",token);
	        }else if(i==1){
	            fprintf(fp2,"INTCON %s",token);
	        }else if(i==2){
	            fprintf(fp2,"CHARCON %s",token);
	        }else if(i==3){
	            fprintf(fp2,"STRCON %s",token);
	        }else if(i==4){
	            fprintf(fp2,"CONSTTK %s",token);
	        }else if(i==5){
	            fprintf(fp2,"INTTK %s",token);
	        }else if(i==6){
	            fprintf(fp2,"CHARTK %s",token);
	        }else if(i==7){
	            fprintf(fp2,"VOIDTK %s",token);
	        }else if(i==8){
	            fprintf(fp2,"MAINTK %s",token);
	        }else if(i==9){
	            fprintf(fp2,"IFTK %s",token);
	        }else if(i==10){
	            fprintf(fp2,"ELSETK %s",token);
	        }else if(i==11){
	            fprintf(fp2,"DOTK %s",token);
	        }else if(i==12){
	            fprintf(fp2,"WHILETK %s",token);
	        }else if(i==13){
	            fprintf(fp2,"FORTK %s",token);
	        }else if(i==14){
	            fprintf(fp2,"SCANFTK %s",token);
	        }else if(i==15){
	            fprintf(fp2,"PRINTFTK %s",token);
	        }else if(i==16){
	            fprintf(fp2,"RETURNTK %s",token);
	        }else if(i==17){
	            fprintf(fp2,"PLUS %s",token);
	        }else if(i==18){
	            fprintf(fp2,"MINU %s",token);
	        }else if(i==19){
	            fprintf(fp2,"MULT %s",token);
	        }else if(i==20){
	            fprintf(fp2,"DIV %s",token);
	        }else if(i==21){
	            fprintf(fp2,"LSS %s",token);
	        }else if(i==22){
	            fprintf(fp2,"LEQ %s",token);
	        }else if(i==23){
	            fprintf(fp2,"GRE %s",token);
	        }else if(i==24){
	            fprintf(fp2,"GEQ %s",token);
	        }else if(i==25){
	            fprintf(fp2,"EQL %s",token);
	        }else if(i==26){
	            fprintf(fp2,"NEQ %s",token);
	        }else if(i==27){
	            fprintf(fp2,"ASSIGN %s",token);
	        }else if(i==28){
	            fprintf(fp2,"SEMICN %s",token);
	        }else if(i==29){
	            fprintf(fp2,"COMMA %s",token);
	        }else if(i==30){
	            fprintf(fp2,"LPARENT %s",token);
	        }else if(i==31){
	            fprintf(fp2,"RPARENT %s",token);
	        }else if(i==32){
	            fprintf(fp2,"LBRACK %s",token);
	        }else if(i==33){
	            fprintf(fp2,"RBRACK %s",token);
	        }else if(i==34){
	            fprintf(fp2,"LBRACE %s",token);
	        }else if(i==35){
	            fprintf(fp2,"RBRACE %s",token);
	        }
	        fprintf(fp2,"\n");
	        do{
	            getch();
	            if(isspace(ch)==0&&ch!=EOF){
	                fseek(fp1,-1,SEEK_CUR);
	                break;
	            }
	        }while(ch!=EOF);*/
}

void preprint(int i,char s[1000]) {
	/*if(i==0){
	    fprintf(fp2,"IDENFR %s",s);
	}else if(i==1){
	    fprintf(fp2,"INTCON %s",s);
	}else if(i==2){
	    fprintf(fp2,"CHARCON %s",s);
	}else if(i==3){
	    fprintf(fp2,"STRCON %s",s);
	}else if(i==4){
	    fprintf(fp2,"CONSTTK %s",s);
	}else if(i==5){
	    fprintf(fp2,"INTTK %s",s);
	}else if(i==6){
	    fprintf(fp2,"CHARTK %s",s);
	}else if(i==7){
	    fprintf(fp2,"VOIDTK %s",s);
	}else if(i==8){
	    fprintf(fp2,"MAINTK %s",s);
	}else if(i==9){
	    fprintf(fp2,"IFTK %s",s);
	}else if(i==10){
	    fprintf(fp2,"ELSETK %s",s);
	}else if(i==11){
	    fprintf(fp2,"DOTK %s",s);
	}else if(i==12){
	    fprintf(fp2,"WHILETK %s",s);
	}else if(i==13){
	    fprintf(fp2,"FORTK %s",s);
	}else if(i==14){
	    fprintf(fp2,"SCANFTK %s",s);
	}else if(i==15){
	    fprintf(fp2,"PRINTFTK %s",s);
	}else if(i==16){
	    fprintf(fp2,"RETURNTK %s",s);
	}else if(i==17){
	    fprintf(fp2,"PLUS %s",s);
	}else if(i==18){
	    fprintf(fp2,"MINU %s",s);
	}else if(i==19){
	    fprintf(fp2,"MULT %s",s);
	}else if(i==20){
	    fprintf(fp2,"DIV %s",s);
	}else if(i==21){
	    fprintf(fp2,"LSS %s",s);
	}else if(i==22){
	    fprintf(fp2,"LEQ %s",s);
	}else if(i==23){
	    fprintf(fp2,"GRE %s",s);
	}else if(i==24){
	    fprintf(fp2,"GEQ %s",s);
	}else if(i==25){
	    fprintf(fp2,"EQL %s",s);
	}else if(i==26){
	    fprintf(fp2,"NEQ %s",s);
	}else if(i==27){
	    fprintf(fp2,"ASSIGN %s",s);
	}else if(i==28){
	    fprintf(fp2,"SEMICN %s",s);
	}else if(i==29){
	    fprintf(fp2,"COMMA %s",s);
	}else if(i==30){
	    fprintf(fp2,"LPARENT %s",s);
	}else if(i==31){
	    fprintf(fp2,"RPARENT %s",s);
	}else if(i==32){
	    fprintf(fp2,"LBRACK %s",s);
	}else if(i==33){
	    fprintf(fp2,"RBRACK %s",s);
	}else if(i==34){
	    fprintf(fp2,"LBRACE %s",s);
	}else if(i==35){
	    fprintf(fp2,"RBRACE %s",s);
	}
	fprintf(fp2,"\n");
	do{
	    getch();
	    if(isspace(ch)==0&&ch!=EOF){
	        fseek(fp1,-1,SEEK_CUR);
	        break;
	    }
	}while(ch!=EOF);*/
}

void uinteger() { //调用时已得到sym
	if(id==1) {
		if(token[0]!='0'||(token[0]=='0'&&strlen(token)==1)) {
			print(id);
			//fprintf(fp2,"<无符号整数>\n");
		}
	}
}

void integer() { //调用时已得到sym,赋给a3
	strcpy(a0,"+");
	if(id==17||id==18) {
		print(id);
		strcpy(a0,token);
		getsym();
		uinteger();
		//fprintf(fp2,"<整数>\n");
	} else if(id==1) {
		uinteger();
		//fprintf(fp2,"<整数>\n");
	}
	if(strcmp(a0,"-")==0) {
		strcpy(a1,"0");
		strcpy(a2,token);
		itoa(wyzi,int2str,10);
		strcpy(a3,"wyz");
		strcat(a3,int2str);
		wyzi++;
		addfour();
	} else {
		strcpy(a3,token);
	}
}

int ischar() { //调用时已得到sym
	if(strcmp(token,"+")==0||strcmp(token,"-")==0||strcmp(token,"*")==0||
	        strcmp(token,"/")==0||isunderline(token[0])||((isalpha(token[0])||isdigit(token[0]))&&strlen(token)==1)) {
		return 1;
	} else {
		return 0;
	}
}

void cdefinite() { //调用时已得到sym,多读一个sym
	//printf("%d\n",id);
	if(id==5) {
		print(id);
		strcpy(a1,token);
		getsym();
		if(id==0) {
			print(id);
			addsymbol(token,5,1,(long long int)malloc(sizeof(int)),layer);
			strcpy(a2,token);
			getsym();
		}
		if(id==27) {
			print(id);
			getsym();
		}
		if(id==17||id==18) {
			print(id);
			strcpy(a3,token);
			getsym();
			uinteger();
			strcat(a3,token);
			getsym();
			//fprintf(fp2,"<整数>\n");
		} else if(id==1) {
			uinteger();
			strcpy(a3,token);
			getsym();
			//fprintf(fp2,"<整数>\n");
		} else {
			printerror('o');
			getsym();
		}
		addfour();
		while(id==29) {
			print(id);
			getsym();
			if(id==0) {
				print(id);
				strcpy(a2,token);
				addsymbol(token,5,1,(long long int)malloc(sizeof(int)),layer);
				getsym();
			}
			if(id==27) {
				print(id);
				getsym();
			}
			if(id==17||id==18) {
				print(id);
				strcpy(a3,token);
				getsym();
				uinteger();
				strcat(a3,token);
				getsym();
				//fprintf(fp2,"<整数>\n");
			} else if(id==1) {
				uinteger();
				strcpy(a3,token);
				getsym();
				//fprintf(fp2,"<整数>\n");
			} else {
				printerror('o');
				getsym();
			}
			addfour();
		}
		//fprintf(fp2,"<常量定义>\n");
	} else if(id==6) {
		print(id);
		strcpy(a1,token);
		getsym();
		if(id==0) {
			print(id);
			addsymbol(token,6,1,(long long int)malloc(sizeof(char)),layer);
			strcpy(a2,token);
			getsym();
		}
		if(id==27) {
			print(id);
			getsym();
			if(id==2) {
				if(ischar()) {
					print(id);
					strcpy(a3,token);
				}
			} else {
				printerror('o');
			}
		}
		addfour();
		getsym();
		while(id==29) {
			print(id);
			getsym();
			if(id==0) {
				print(id);
				addsymbol(token,6,1,(long long int)malloc(sizeof(char)),layer);
				strcpy(a2,token);
				//printf("%s\n",a2);
				getsym();
			}
			if(id==27) {
				print(id);
				getsym();
				if(id==2) {
					if(ischar()) {
						print(id);
						strcpy(a3,token);
						//printf("%s\n",a3);
					}
				} else {
					printerror('o');
				}
			}
			getsym();
			addfour();
		}
		//fprintf(fp2,"<常量定义>\n");
	}
}

void cdeclare() { //调用时已得到sym,多读一个sym
	int flag=0;
	if(id==4) {
		strcpy(a0,token);
		do {
			print(id);
			getsym();
			cdefinite();
			if(id==28) {
				flag=1;
				print(id);
			} else {
				printerror('k');
			}
			getsym();
		} while(id==4);
		if(flag==1) {
			//fprintf(fp2,"<常量说明>\n");
		}
	}
}

void vdefinite() { //调用时已得到sym,多读一个sym
	int d;
	while(id==29) {
		print(id);
		getsym();
		if(id==0) {
			print(id);
			strcpy(a1,token);
			strcpy(beforeid,token);
			getsym();
			if(id==32) {
				print(id);
				if(a0[strlen(a0)-1]!=']')
					strcat(a0,"[]");
				getsym();
				if(id==1) {
					print(id);
					if(token[0]!='0') {
						//fprintf(fp2,"<无符号整数>\n");
						d=char2int(token);
						strcpy(a2,token);
						if(beforetype==1) {
							addsymbol(beforeid,3,d,(long long int)malloc(sizeof(int)*d),layer);
						} else {
							addsymbol(beforeid,4,d,(long long int)malloc(sizeof(char)*d),layer);
						}
					} else {
						printerror('i');
					}
					getsym();
				}
				if(id==33) {
					print(id);
					getsym();
				} else {
					printerror('m');
				}
				strcpy(a3,"");
				addfour();
				if(a0[strlen(a0)-1]==']'){
					a0[strlen(a0)-2]='\0';
				}
			} else {
				if(beforetype==1) {
					addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(int)),layer);
				} else {
					addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(char)),layer);
				}
				strcpy(a2,"");
				strcpy(a3,"");
				addfour();
			}
		}
	}
	//fprintf(fp2,"<变量定义>\n");
}

void vdeclare() { //调用时已得到sym,多读一个sym
	int d;
	if(id==32) {
		print(id);
		if(a0[strlen(a0)-1]!=']')
			strcat(a0,"[]");
		getsym();
		if(id==1) {
			print(id);
			if(token[0]!='0') {
				//fprintf(fp2,"<无符号整数>\n");
				d=char2int(token);
				strcpy(a2,token);
				if(beforetype==1) {
					addsymbol(beforeid,3,d,(long long int)malloc(sizeof(int)*d),layer);
				} else {
					addsymbol(beforeid,4,d,(long long int)malloc(sizeof(char)*d),layer);
				}
			} else {
				printerror('i');
			}
			getsym();
		}
		if(id==33) {
			print(id);
			getsym();
		} else {
			printerror('m');
		}
		strcpy(a3,"");
		addfour();
		if(a0[strlen(a0)-1]==']'){
			a0[strlen(a0)-2]='\0';
		}
	} else {
		if(beforetype==1) {
			addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(int)),layer);
		} else {
			addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(char)),layer);
		}
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
	}
	vdefinite();
	if(id==28) {
		print(id);
		getsym();
	} else {
		printerror('k');
	}
	if(id==5||id==6) {
		beforetype=id-4;
		strcpy(nexttoken[0],token);
		nextid[0]=id;
		strcpy(a0,token);
		getsym();
	}
	if(id==0) {
		strcpy(beforeid,token);
		strcpy(nexttoken[1],token);
		nextid[1]=id;
		strcpy(a1,token);
		getsym();
	}
	while((nextid[0]==5||nextid[0]==6)&&nextid[1]==0&&(id==29||id==28||id==32)) {
		preprint(nextid[0],nexttoken[0]);
		preprint(nextid[1],nexttoken[1]);
		clearnexttoken();
		if(id==32) {
			print(id);
			if(a0[strlen(a0)-1]!=']')
				strcat(a0,"[]");
			getsym();
			if(id==1) {
				print(id);
				if(token[0]!='0') {
					//fprintf(fp2,"<无符号整数>\n");
					d=char2int(token);
					strcpy(a2,token);
					if(beforetype==1) {
						addsymbol(beforeid,3,d,(long long int)malloc(sizeof(int)*d),layer);
					} else {
						addsymbol(beforeid,4,d,(long long int)malloc(sizeof(char)*d),layer);
					}
				} else {
					printerror('i');
				}
				getsym();
			}
			if(id==33) {
				print(id);
				getsym();
			} else {
				printerror('m');
			}
			strcpy(a3,"");
			addfour();
			if(a0[strlen(a0)-1]==']'){
				a0[strlen(a0)-2]='\0';
			}
		} else {
			if(beforetype==1) {
				addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(int)),layer);
			} else {
				addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(char)),layer);
			}
			strcpy(a2,"");
			strcpy(a3,"");
			addfour();
		}
		vdefinite();
		if(id==28) {
			print(id);
			getsym();
		} else {
			printerror('k');
		}
		if(id==5||id==6) {
			beforetype=id-4;
			strcpy(nexttoken[0],token);
			nextid[0]=id;
			strcpy(a0,token);
			getsym();
		}
		if(id==0) {
			strcpy(beforeid,token);
			strcpy(nexttoken[1],token);
			nextid[1]=id;
			strcpy(a1,token);
			getsym();
		} else {
			clearnexttoken();
			//error;
		}

	}
	//fprintf(fp2,"<变量说明>\n");
}

void normalvdefinite() { //调用时已得到sym,多读一个sym
	int d;
	if(id==5||id==6) {
		print(id);
		beforetype=id-4;
		strcpy(a0,token);
		getsym();
	}
	if(id==0) {
		print(id);
		strcpy(beforeid,token);
		strcpy(a1,token);
		getsym();
	}
	if(id==32) {
		print(id);
		if(a0[strlen(a0)-1]!=']')
			strcat(a0,"[]");
		getsym();
		if(id==1) {
			print(id);
			if(token[0]!='0') {
				//fprintf(fp2,"<无符号整数>\n");
				d=char2int(token);
				strcpy(a2,token);
				if(beforetype==1) {
					addsymbol(beforeid,3,d,(long long int)malloc(sizeof(int)*d),layer);
				} else {
					addsymbol(beforeid,4,d,(long long int)malloc(sizeof(char)*d),layer);
				}
			} else {
				printerror('i');
			}
			getsym();
		}
		if(id==33) {
			print(id);
			getsym();
		} else {
			printerror('m');
		}
		strcpy(a3,"");
		addfour();
		if(a0[strlen(a0)-1]==']'){
			a0[strlen(a0)-2]='\0';
		}
	} else {
		if(beforetype==1) {
			addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(int)),layer);
		} else {
			addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(char)),layer);
		}
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
	}
	while(id==29) {
		print(id);
		getsym();
		if(id==0) {
			print(id);
			strcpy(a1,token);
			strcpy(beforeid,token);
			getsym();
			if(id==32) {
				print(id);
				if(a0[strlen(a0)-1]!=']')
					strcat(a0,"[]");
				getsym();
				if(id==1) {
					print(id);
					if(token[0]!='0') {
						//fprintf(fp2,"<无符号整数>\n");
						d=char2int(token);
						strcpy(a2,token);
						if(beforetype==1) {
							addsymbol(beforeid,3,d,(long long int)malloc(sizeof(int)*d),layer);
						} else {
							addsymbol(beforeid,4,d,(long long int)malloc(sizeof(char)*d),layer);
						}
					} else {
						printerror('i');
					}
					getsym();
				}
				if(id==33) {
					print(id);
					getsym();
				} else {
					printerror('m');
				}
				strcpy(a3,"");
				addfour();
				if(a0[strlen(a0)-1]==']'){
					a0[strlen(a0)-2]='\0';
				}
			} else {
				if(beforetype==1) {
					addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(int)),layer);
				} else {
					addsymbol(beforeid,beforetype,1,(long long int)malloc(sizeof(char)),layer);
				}
				strcpy(a2,"");
				strcpy(a3,"");
				addfour();
			}
		}
	}
	//fprintf(fp2,"<变量定义>\n");
}

void normalvdeclare() { //调用时已得到sym,多读一个sym
	int flag=0;
	while(id==5||id==6) {
		normalvdefinite();
		flag=1;
		if(id==28) {
			print(id);
			getsym();
		} else {
			printerror('k');
		}
	}
	if(flag==1) {
		//fprintf(fp2,"<变量说明>\n");
	}
}

void palist() { //调用时已得到sym,多读一个sym,参数表
	int index=tablei-1; //符号表函数下标
	layer++;
	strcpy(a0,"para");
	strcpy(a3,"");
	if(id==5) {
		print(id);
		strcpy(a1,token);
		getsym();
		if(id==0) {
			print(id);
			strcpy(a2,token);
			addsymbol(token,1,1,(long long int)malloc(sizeof(int)),layer);
			addpara(&(table[index].para),token,1);
			getsym();
		}
		addfour();
	} else if(id==6) {
		print(id);
		strcpy(a1,token);
		getsym();
		if(id==0) {
			print(id);
			strcpy(a2,token);
			addsymbol(token,2,1,(long long int)malloc(sizeof(char)),layer);
			addpara(&(table[index].para),token,2);
			getsym();
		}
		addfour();
	}
	while(id==29) {
		print(id);
		getsym();
		if(id==5) {
			print(id);
			strcpy(a1,token);
			getsym();
			if(id==0) {
				print(id);
				strcpy(a2,token);
				addsymbol(token,1,1,(long long int)malloc(sizeof(int)),layer);
				addpara(&(table[index].para),token,1);
				getsym();
			}
			addfour();
		} else if(id==6) {
			print(id);
			strcpy(a1,token);
			getsym();
			if(id==0) {
				print(id);
				strcpy(a2,token);
				addsymbol(token,2,1,(long long int)malloc(sizeof(char)),layer);
				addpara(&(table[index].para),token,2);
				getsym();
			}
			addfour();
		}
	}
	layer--;
	//fprintf(fp2,"<参数表>\n");
}

void valuepalist(int i) { //调用时已得到sym,多读一个sym，值参数表
	int kind;
	struct parameter *node;
	node=table[i].para;
	if(id!=31) {
		kind=expression();
		if(node==NULL) {
			printerror('d');
		} else {
			if(kind!=node->type) {
				printerror('e');
			}
			node=node->next;
		}
		strcpy(a0,"push");
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
	}
	while(id==29) {
		print(id);
		getsym();
		kind=expression();
		if(node==NULL) {
			printerror('d');
		} else {
			if(kind!=node->type) {
				printerror('e');
			}
			node=node->next;
		}
		strcpy(a0,"push");
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
	}
	if(node!=NULL) {
		printerror('d');
	}
	//fprintf(fp2,"<值参数表>\n");
}

int factor() {  //调用时已得到sym,多读一个sym,因子,结果赋给a3
	int i,kind=2,flag;
	char s[100];
	if(id==0) {
		print(id);
		strcpy(a3,token);
		i=findsymbol(token);
		if(i!=-1) {
			kind=2-table[i].type%2;
		}
		getsym();
		if(id==32) {
			print(id);
			strcpy(s,a3);
			getsym();
			flag=expression();
			if(flag!=1) {
				printerror('i');
			}
			strcpy(a0,"[]");
			strcpy(a2,s);
			itoa(wyzi,int2str,10);
			strcpy(a3,"wyz");
			strcat(a3,int2str);
			wyzi++;
			addfour();

			if(id==33) {
				print(id);
				getsym();
			} else {
				printerror('m');
			}
		} else if(id==30) { //有返回值函数调用语句
			print(id);

			strcpy(s,a3);
			getsym();
			strcpy(a0,"startfunc");
			strcpy(a1,"");
			strcpy(a2,"");
			strcpy(a3,"");
			addfour();
			valuepalist(i);
			if(id==31) {
				print(id);
				//fprintf(fp2,"<有返回值函数调用语句>\n");
				getsym();
			} else {
				printerror('l');
			}

			strcpy(a0,"call");
			strcpy(a1,s);
			strcpy(a2,"");
			strcpy(a3,"");
			addfour();

			strcpy(a0,"=");
			strcpy(a1,"RET");
			itoa(wyzi,int2str,10);
			strcpy(a2,"wyz");
			strcat(a2,int2str);
			strcat(a3,"");
			wyzi++;
			addfour();
			strcpy(a3,a2);
		}
		//fprintf(fp2,"<因子>\n");
	} else if(id==30) {
		print(id);
		getsym();
		expression();
		strcpy(a3,a1);
		if(id==31) {
			print(id);
			getsym();
		} else {
			printerror('l');
		}
		kind=1;
		//fprintf(fp2,"<因子>\n");
	} else if(id==17||id==18||id==1) {
		integer();
		getsym();
		kind=1;
		//fprintf(fp2,"<因子>\n");
	} else if(id==2) {
		if(ischar()) {
			print(id);
		}
		strcpy(a0,"=");
		strcpy(a1,"\'");
		strcat(a1,token);
		strcat(a1,"\'");
		itoa(wyzi,int2str,10);
		strcpy(a2,"wyz");
		strcat(a2,int2str);
		strcpy(a3,"");
		wyzi++;
		addfour();
		strcpy(a3,a2);
		getsym();
		kind=2;
		//fprintf(fp2,"<因子>\n");
	}
	return kind;
}

int term() {  //调用时已得到sym,多读一个sym,项,结果赋给a3
	int kind,i;
	char s[1000],s1[100];
	kind=factor();
	while(id==19||id==20) {
		print(id);
		strcpy(s,token);
		strcpy(s1,a3);
		getsym();
		factor();
		kind=1;
		strcpy(a0,s);
		strcpy(a1,s1);
		strcpy(a2,a3);
		itoa(wyzi,int2str,10);
		strcpy(a3,"wyz");
		strcat(a3,int2str);
		wyzi++;
		//printf("%s\n",a1);
		addfour();
	}
	return kind;
	//fprintf(fp2,"<项>\n");
}

int expression() {  //调用时已得到sym,多读一个sym,表达式,结果赋给a1
	int kind,flag=0,i=-1;
	char s[100],s1[100];
	if(id==17||id==18) {
		print(id);
		strcpy(s,token);
		getsym();
		flag=1;
	}
	kind=term();
	if(flag==1) {
		kind=1;
		strcpy(a0,s);
		strcpy(a1,"0");
		strcpy(a2,a3);
		itoa(wyzi,int2str,10);
		strcpy(a3,"wyz");
		strcat(a3,int2str);
		i=wyzi;
		wyzi++;
		addfour();
	}
	while(id==17||id==18) {
		print(id);
		strcpy(s,token);
		if(i==-1) {
			strcpy(s1,a3);
		} else {
			itoa(i,int2str,10);
			strcpy(s1,"wyz");
			strcat(s1,int2str);
		}
		getsym();
		term();
		kind=1;
		strcpy(a0,s);
		strcpy(a1,s1);
		strcpy(a2,a3);
		itoa(wyzi,int2str,10);
		strcpy(a3,"wyz");
		strcat(a3,int2str);
		i=wyzi;
		wyzi++;
		addfour();
	}
	strcpy(a1,a3);
	return kind;
	//fprintf(fp2,"<表达式>\n");
}

void condition() { //调用时已得到sym,多读一个sym，条件
	int kind;
	char s[100],s1[100];
	kind=expression();
	if(kind==2) {
		printerror('f');
	}
	strcpy(s,a1);
	if(isrelation()) {
		print(id);
		strcpy(s1,token);
		getsym();
		kind=expression();
		if(kind==2) {
			printerror('f');
		}
		strcpy(a0,s1);
		strcpy(a2,a1);
		strcpy(a1,s);
		strcpy(a3,"");
		addfour();
	} else {
		strcpy(a0,"!=");
		strcpy(a1,s);
		strcpy(a2,"0");
		strcpy(a3,"");
		addfour();
	}
	//fprintf(fp2,"<条件>\n");
}

void ifsentence() { //调用时已得到sym,多读一个sym
	int i;
	print(id);

	strcpy(a0,"label");
	strcpy(a1,"if");
	itoa(ifi,int2str,10);
	strcat(a1,int2str);
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();

	getsym();
	if(id==30) {
		print(id);
		getsym();
	}
	condition();
	if(id==31) {
		print(id);
		getsym();
	} else {
		printerror('l');
	}


	itoa(ifi,int2str,10);
	strcpy(a0,"BZ");
	strcpy(a1,"endif");
	strcat(a1,int2str);
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();

	i=ifi;
	ifi++;
	sentence();
	ifi--;
	itoa(i,int2str,10);

	strcpy(a0,"GOTO");
	strcpy(a1,"endelse");
	strcat(a1,int2str);
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();

	strcpy(a0,"label");
	strcpy(a1,"endif");
	strcat(a1,int2str);
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();

	if(id==10) {
		print(id);
		getsym();
		ifi++;
		sentence();
		ifi--;
	}

	itoa(i,int2str,10);
	strcpy(a0,"label");
	strcpy(a1,"endelse");
	strcat(a1,int2str);
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();
	ifi++;
	//fprintf(fp2,"<条件语句>\n");
}

void step() { //调用时已得到sym
	uinteger();
	//fprintf(fp2,"<步长>\n");
}

void circlesentence() { //调用时已得到sym,多读一个sym
	int i,j;
	struct fourexpression four_reserve;
	print(id);
	if(id==11) {

		strcpy(a0,"label");
		strcpy(a1,"do");
		itoa(doi,int2str,10);
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		getsym();

		j=doi;
		doi++;
		sentence();
		doi--;
		itoa(j,int2str,10);

		if(id==12) {
			print(id);
			getsym();
		} else {
			printerror('n');
		}
		if(id==30) {
			print(id);
			getsym();
			condition();
		}
		if(id==31) {
			print(id);
			getsym();
		} else {
			printerror('l');
		}

		itoa(j,int2str,10);
		strcpy(a0,"BNZ");
		strcpy(a1,"do");
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
		doi++;

	} else if(id==12) {

		strcpy(a0,"label");
		strcpy(a1,"while");
		itoa(whilei,int2str,10);
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		getsym();
		if(id==30) {
			print(id);
			getsym();
		}
		condition();
		if(id==31) {
			print(id);
			getsym();
		} else {
			printerror('l');
		}

		itoa(whilei,int2str,10);
		strcpy(a0,"BZ");
		strcpy(a1,"endwhile");
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		j=whilei;
		whilei++;
		sentence();
		whilei--;
		itoa(j,int2str,10);

		strcpy(a0,"GOTO");
		strcpy(a1,"while");
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		strcpy(a0,"label");
		strcpy(a1,"endwhile");
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		whilei++;

	} else if(id==13) {
		getsym();
		if(id==30) {
			print(id);
			getsym();
		}
		if(id==0) {
			print(id);
			i=findsymbol(token);
			if(i==-1) {
				printerror('c');
			} else if(table[i].type==5||table[i].type==6) {
				printerror('j');
			}
			strcpy(nexttoken[0],token);
			getsym();
		}
		if(id==27) {
			print(id);
			getsym();
		}
		expression();

		strcpy(a0,"=");
		strcpy(a2,nexttoken[0]);
		strcpy(a3,"");
		addfour();
		clearnexttoken();

		if(id==28) {
			print(id);
			getsym();
		} else {
			printerror('k');
		}

		strcpy(a0,"label");
		strcpy(a1,"for");
		itoa(fori,int2str,10);
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		condition();

		itoa(fori,int2str,10);
		strcpy(a0,"BZ");
		strcpy(a1,"endfor");
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		if(id==28) {
			print(id);
			getsym();
		} else {
			printerror('k');
		}
		if(id==0) {
			print(id);
			i=findsymbol(token);
			if(i==-1) {
				printerror('c');
			} else if(table[i].type==5||table[i].type==6) {
				printerror('j');
			}
			strcpy(nexttoken[0],token);
			getsym();
		}
		if(id==27) {
			print(id);
			getsym();
		}
		if(id==0) {
			print(id);
			i=findsymbol(token);
			if(i==-1) {
				printerror('c');
			}
			strcpy(nexttoken[1],token);
			getsym();
		}
		if(id==17||id==18) {
			print(id);
			strcpy(nexttoken[2],token);
			getsym();
		}
		step();
		strcpy(nexttoken[3],token);
		getsym();
		if(id==31) {
			print(id);
			getsym();
		}

		strcpy(four_reserve.a,nexttoken[2]);
		strcpy(four_reserve.b,nexttoken[1]);
		strcpy(four_reserve.c,nexttoken[3]);
		strcpy(four_reserve.d,nexttoken[0]);
		clearnexttoken();

		j=fori;
		fori++;
		sentence();
		fori--;
		itoa(j,int2str,10);

		strcpy(a0,four_reserve.a);
		strcpy(a1,four_reserve.b);
		strcpy(a2,four_reserve.c);
		strcpy(a3,four_reserve.d);
		addfour();

		strcpy(a0,"GOTO");
		strcpy(a1,"for");
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		strcpy(a0,"label");
		strcpy(a1,"endfor");
		strcat(a1,int2str);
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();

		fori++;
	}
	//fprintf(fp2,"<循环语句>\n");
}

void readsentence() { //调用时已得到sym,多读一个sym
	int i;
	print(id);

	strcpy(a0,"read");
	strcpy(a3,"");

	getsym();
	if(id==30) {
		print(id);
		getsym();
		if(id==0) {
			print(id);
			i=findsymbol(token);
			if(i==-1) {
				printerror('c');
			}
			if(table[i].type==1) {
				strcpy(a2,"int");
			} else if(table[i].type==2) {
				strcpy(a2,"char");
			}
			strcpy(a1,token);
			getsym();
		}
		addfour();
		while(id==29) {
			print(id);
			getsym();
			if(id==0) {
				print(id);
				i=findsymbol(token);
				if(i==-1) {
					printerror('c');
				}
				if(table[i].type==1) {
					strcpy(a2,"int");
				} else if(table[i].type==2) {
					strcpy(a2,"char");
				}
				strcpy(a1,token);
				getsym();
			}
			addfour();
		}
		if(id==31) {
			print(id);
			//fprintf(fp2,"<读语句>\n");
			getsym();
		} else {
			printerror('l');
		}
	}
}

void printsentence() { //调用时已得到sym,多读一个sym
	int flag=0;
	print(id);
	getsym();
	if(id==30) {
		print(id);
		getsym();
		if(id==3) {
			print(id);
			//fprintf(fp2,"<字符串>\n");
			strcpy(a0,"print");
			strcpy(a1,"\"");
			strcat(a1,token);
			strcat(a1,"\"");
			strcpy(a2,"string");
			strcpy(a3,"");
			addfour();

			getsym();
			if(id==29) {
				print(id);
				getsym();
				flag=expression();
				strcpy(a0,"print");
				if(flag==1)
					strcpy(a2,"int");
				else
					strcpy(a2,"char");
				strcpy(a3,"");
				addfour();
			}
			strcpy(a0,"endprint");
			strcpy(a1,"");
			strcpy(a2,"");
			strcpy(a3,"");
			addfour();
		}
		if(id==0||id==1||id==2||id==17||id==18||id==30) {
			flag=expression(); //以后要改的bug
			strcpy(a0,"print");
			if(flag==1)
				strcpy(a2,"int");
			else
				strcpy(a2,"char");
			strcpy(a3,"");
			addfour();
			strcpy(a0,"endprint");
			strcpy(a1,"");
			strcpy(a2,"");
			strcpy(a3,"");
			addfour();
		}
		if(id==31) {
			print(id);
			//fprintf(fp2,"<写语句>\n");
			getsym();
		} else {
			printerror('l');
		}
	}
}

void returnsentence() { //调用时已得到sym,多读一个sym
	int flag=0,kind;
	//printf("!!!!!"); 
	print(id);
	strcpy(a0,"ret");
	getsym();
	if(id==30) {
		flag=1;
		print(id);
		getsym();
		kind=expression();
		if(id==31) {
			print(id);
			getsym();
		} else {
			printerror('l');
		}
		strcpy(a0,"ret");
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
		if(infunc==3) {
			printerror('g');
		} else if(infunc!=kind) {
			printerror('h');
		}
	} else {
		strcpy(a0,"ret");
		strcpy(a1,"");
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
	}
	returnflag=1;
	if(flag==0&&infunc==1) {
		printerror('h');
	}
	//fprintf(fp2,"<返回语句>\n");
}

void assignment() { //调用时已得到sym,多读一个sym,从=或[开始操作
	int kind;
	char s[100];
	strcpy(s,beforetoken);
	if(id==27) {
		print(id);
		strcpy(a0,"=");
		getsym();
		expression();

		strcpy(a0,"=");
		strcpy(a2,s);
		strcpy(a3,"");
		addfour();
	} else if(id==32) {
		print(id);
		getsym();
		kind=expression();
		strcpy(nexttoken[1],a1);
		if(kind==2) {
			printerror('i');
		}
		if(id==33) {
			print(id);
			getsym();
		} else {
			printerror('m');
		}
		if(id==27) {
			print(id);
			getsym();
			expression();
			strcpy(nexttoken[2],a1);
		}
		strcpy(a0,"[]=");
		strcpy(a1,nexttoken[1]);
		strcpy(a2,nexttoken[2]);
		strcpy(a3,beforetoken);   //a[5]=t1 转换成 []= 5 t1 a
		addfour();
	}
	//fprintf(fp2,"<赋值语句>\n");
}

void sentence() { //调用时已得到sym,多读一个sym,语句
	int i;
	char s[100];
	//printf("%s\n",token);
	if(id==9) {
		ifsentence();
		//fprintf(fp2,"<语句>\n");
	} else if(id==11||id==12||id==13) {
		circlesentence();
		//fprintf(fp2,"<语句>\n");
	} else if(id==34) {
		print(id);
		getsym();
		sentencelist();
		if(id==35) {
			print(id);
			//fprintf(fp2,"<语句>\n");
			getsym();
		}
	} else if(id==14) {
		readsentence();
		if(id==28) {
			print(id);
			//fprintf(fp2,"<语句>\n");
			getsym();
		} else {
			printerror('k');
		}
	} else if(id==15) {
		printsentence();
		if(id==28) {
			print(id);
			//fprintf(fp2,"<语句>\n");
			getsym();
		} else {
			printerror('k');
		}
	} else if(id==16) {
		returnsentence();
		if(id==28) {
			print(id);
			//fprintf(fp2,"<语句>\n");
			getsym();
		} else {
			printerror('k');
		}
	} else if(id==28) {
		print(id);
		//fprintf(fp2,"<语句>\n");
		getsym();
	} else if(id==0) {
		print(id);
		strcpy(beforetoken,token);
		i=findsymbol(token);
		if(i==-1) {
			printerror('c');
		}
		getsym();
		if(id==30) {
			strcpy(s,beforetoken);
			print(id);
			if(table[i].type==7||table[i].type==8) {
				strcpy(a0,"startfunc");
				strcpy(a1,"");
				strcpy(a2,"");
				strcpy(a3,"");
				addfour();
				getsym();
				valuepalist(i);
				if(id==31) {
					print(id);
					//fprintf(fp2,"<有返回值函数调用语句>\n"); //要改
					getsym();
				} else {
					printerror('l');
				}
				if(id==28) {
					print(id);
					//fprintf(fp2,"<语句>\n");
					getsym();
				} else {
					printerror('k');
				}
			}
			if(table[i].type==9) {
				strcpy(a0,"startfunc");
				strcpy(a1,"");
				strcpy(a2,"");
				strcpy(a3,"");
				addfour();
				getsym();
				valuepalist(i);
				if(id==31) {
					print(id);
					//fprintf(fp2,"<无返回值函数调用语句>\n"); //要改
					getsym();
				} else {
					printerror('l');
				}
				if(id==28) {
					print(id);
					//fprintf(fp2,"<语句>\n");
					getsym();
				} else {
					printerror('k');
				}
			}
			strcpy(a0,"call");
			strcpy(a1,s);
			strcpy(a2,"");
			strcpy(a3,"");
			addfour();
		} else if(id==27||id==32) {
			if(table[i].type==5||table[i].type==6) {
				printerror('j');
			}
			assignment();
			if(id==28) {
				print(id);
				//fprintf(fp2,"<语句>\n");
				getsym();
			} else {
				printerror('k');
			}
		}
	} else {
		printerror('k');
	}
}

void sentencelist() { //调用时已得到sym,多读一个sym
	while(id==9||(id>10&&id<17)||id==0||id==28||id==34) {
		sentence();
	}
	//fprintf(fp2,"<语句列>\n");
}

void complexsentence() { //调用时已得到sym,多读一个sym
	cdeclare();
	normalvdeclare();
	sentencelist();
	//fprintf(fp2,"<复合语句>\n");
}

void rfunction() { //调用时已得到sym,多读一个sym,声音头部已操作完
	returnflag=0;
	print(id);
	if(beforetype==1) {
		addsymbol(beforeid,7,1,(long long int)malloc(sizeof(int)),layer);
		infunc=1;
	} else {
		addsymbol(beforeid,8,1,(long long int)malloc(sizeof(char)),layer);
		infunc=2;
	}
	strcat(a1,"()");
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();
	getsym();
	palist();
	if(id==31) {
		print(id);
		getsym();
	} else {
		printerror('l');
	}
	if(id==34) {
		print(id);
		getsym();
		layer++;
	}
	complexsentence();
	if(id==35) {
		print(id);
		updatetable();
		//fprintf(fp2,"<有返回值函数定义>\n");
		getsym();
	}
	infunc=0;
	if(returnflag==0) {
		printerror('h');
	}
	strcpy(a0,"end");
	strcpy(a1,"");
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();
}

void vfunction() { //调用时已得到sym,多读一个sym,此时sym为标识符
	returnflag=0;
	print(id);
	strcpy(a1,token);
	addsymbol(token,9,1,(long long int)malloc(sizeof(void)),layer);
	infunc=3;
	getsym();
	strcat(a1,"()");
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();
	if(id==30) {
		print(id);
		getsym();
	}
	palist();
	if(id==31) {
		print(id);
		getsym();
	} else {
		printerror('l');
	}
	if(id==34) {
		print(id);
		getsym();
		layer++;
	}
	complexsentence();
	if(id==35) {
		print(id);
		updatetable();
		//fprintf(fp2,"<无返回值函数定义>\n");
		getsym();
	}
	infunc=0;
	if(returnflag==0) {
		strcpy(a0,"ret");
		strcpy(a1,"");
		strcpy(a2,"");
		strcpy(a3,"");
		addfour();
	}

	strcpy(a0,"end");
	strcpy(a1,"");
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();
}

void mainfunction() { //调用时已得到sym,此时sym为main
	print(id);
	strcpy(a1,token);
	infunc=3;
	getsym();
	strcat(a1,"()");
	strcpy(a2,"");
	strcpy(a3,"");
	addfour();
	if(id==30) {
		print(id);
		getsym();
	}
	if(id==31) {
		print(id);
		getsym();
	} else {
		printerror('l');
	}
	if(id==34) {
		print(id);
		getsym();
		layer++;
	}
	complexsentence();
	if(id==35) {
		print(id);
		updatetable();
		//fprintf(fp2,"<主函数>\n");
	}
	infunc=0;
}

void program() {
	getsym();
	if(id==4) {
		cdeclare();
	}
	if(id==5||id==6) {
		print(id);
		strcpy(a0,token);
		if(id==5) {
			beforetype=1; //int 1
		} else {
			beforetype=2; //char 2
		}
		getsym();
		if(id==0) {
			print(id);
			strcpy(a1,token);
			strcpy(beforeid,token);
			strcpy(nexttoken[0],token);
			getsym();
			if(id==30) {
				clearnexttoken();
				//fprintf(fp2,"<声明头部>\n");
				rfunction();
			} else if(id==32||id==29||id==28) {
				clearnexttoken();
				vdeclare();
			}
		}
	}
	if(id==30&&strlen(nexttoken[0])!=0) {
		preprint(nextid[0],nexttoken[0]);
		preprint(nextid[1],nexttoken[1]);
		strcpy(a0,nexttoken[0]);
		strcpy(a1,nexttoken[1]);
		if(nextid[0]==5) {
			beforetype=1;
		} else {
			beforetype=2;
		}
		strcpy(beforeid,nexttoken[1]);
		clearnexttoken();
		//fprintf(fp2,"<声明头部>\n");
		rfunction();
	}
	while(id==5||id==6||id==7) {
		if(id==5||id==6) {
			print(id);
			strcpy(a0,token);
			beforetype=id-4;
			getsym();
			if(id==0) {
				print(id);
				strcpy(a1,token);
				strcpy(beforeid,token);
				//fprintf(fp2,"<声明头部>\n");
				getsym();
				if(id==30) {
					rfunction();
				}
			}
		} else if(id==7) {
			print(id);
			strcpy(a0,token);
			beforetype=9;
			getsym();
			if(id==8) {
				mainfunction();
				break;
			} else if(id==0) {
				vfunction();
			}
		}
	}
	if(ch!=EOF) {
		ch=EOF;
		//error;
	} else {
		//fprintf(fp2,"<程序>");
	}
}

void addprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int lay;
	struct retsymbol p1,p2,p3;
	int v;
	p1=findsymbol_run(d);
	addr1=p1.addr;
	if(b[0]=='\'') {
		if(c[0]=='\'') {
			v=char2integer(c);
			itoa(v,int2str,10);
			printmips("add","$t1","$0",int2str);
			v=char2integer(b);
			itoa(v,int2str,10);
			printmips("add","$t0","$t1",int2str);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else if(isdigit(c[0])||c[0]=='-') {
			printmips("add","$t1","$0",c);
			v=char2integer(b);
			itoa(v,int2str,10);
			printmips("add","$t0","$t1",int2str);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else {
			p2=findsymbol_run(c);
			addr2=p2.addr;
			itoa(addr2,int2str,10);
			if(p2.layers==-1){
                printmips("lw","$t1",int2str,"$s6");
			}else{
                if(addr2>0) {
                    itoa(addr2-sp_top,int2str,10);
                    printmips("lw","$t1",int2str,"$s7");
                } else {
                    printmips("lw","$t1",int2str,"$sp");
                }
			}
			v=char2integer(b);
			itoa(v,int2str,10);
			printmips("add","$t0","$t1",int2str);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		}
	} else if(isdigit(b[0])||b[0]=='-') {
		if(c[0]=='\'') {
			v=char2integer(c);
			itoa(v,int2str,10);
			printmips("add","$t1","$0",int2str);
			printmips("add","$t0","$t1",b);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else if(isdigit(c[0])||c[0]=='-') {
			printmips("add","$t1","$0",c);
			printmips("add","$t0","$t1",b);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else {
			p2=findsymbol_run(c);
			addr2=p2.addr;
			itoa(addr2,int2str,10);
			if(p2.layers==-1){
                printmips("lw","$t1",int2str,"$s6");
			}else{
                if(addr2>0) {
                    itoa(addr2-sp_top,int2str,10);
                    printmips("lw","$t1",int2str,"$s7");
                } else {
                    printmips("lw","$t1",int2str,"$sp");
                }
			}
			printmips("add","$t0","$t1",b);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		}
	} else {
		p2=findsymbol_run(b);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t1",int2str,"$s6");
        }else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t1",int2str,"$s7");
            } else {
                printmips("lw","$t1",int2str,"$sp");
            }
        }
		if(c[0]=='\'') {
			v=char2integer(c);
			itoa(v,int2str,10);
			printmips("add","$t0","$t1",int2str);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else if(isdigit(c[0])||(c[0]=='-')) {
			printmips("add","$t0","$t1",c);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else {
			p3=findsymbol_run(c);
			addr3=p3.addr;
			itoa(addr3,int2str,10);
			if(p3.layers==-1){
                printmips("lw","$t2",int2str,"$s6");
			}else{
                if(addr3>0) {
                    itoa(addr3-sp_top,int2str,10);
                    printmips("lw","$t2",int2str,"$s7");
                } else {
                    printmips("lw","$t2",int2str,"$sp");
                }
			}
			printmips("add","$t0","$t1","$t2");
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		}
	}
}

void subprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	p1=findsymbol_run(d);
	addr1=p1.addr;
	if(b[0]=='\'') {
		if(c[0]=='\'') {
			v=char2integer(b);
			itoa(v,int2str,10);
			printmips("add","$t1","$0",int2str);
			v=char2integer(c);
			itoa(v,int2str,10);
			strcpy(s,"-");
			strcat(s,int2str);
			printmips("add","$t0","$t1",s);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else if(isdigit(c[0])||c[0]=='-') {
			v=char2integer(b);
			itoa(v,int2str,10);
			printmips("add","$t1","$0",int2str);
			if(c[0]=='-') {
				strcpy(s,c+1);
			} else {
				strcpy(s,"-");
				strcat(s,c);
			}
			printmips("add","$t0","$t1",s);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else {
			v=char2integer(b);
			itoa(v,int2str,10);
			printmips("add","$t1","$0",int2str);
			p2=findsymbol_run(c);
			addr2=p2.addr;
			itoa(addr2,int2str,10);
			if(p2.layers==-1){
                printmips("lw","$t2",int2str,"$s6");
			}else{
                if(addr2>0) {
                    itoa(addr2-sp_top,int2str,10);
                    printmips("lw","$t2",int2str,"$s7");
                } else {
                    printmips("lw","$t2",int2str,"$sp");
                }
			}
			printmips("sub","$t0","$t1","$t2");
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		}
	} else if(isdigit(b[0])||b[0]=='-') {
		if(c[0]=='\'') {
			printmips("add","$t1","$0",b);
			v=char2integer(c);
			itoa(v,int2str,10);
			strcpy(s,"-");
			strcat(s,int2str);
			printmips("add","$t0","$t1",s);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else if(isdigit(c[0])||c[0]=='-') {
			printmips("add","$t1","$0",b);
			if(c[0]=='-') {
				strcpy(s,c+1);
			} else {
				strcpy(s,"-");
				strcat(s,c);
			}
			printmips("add","$t0","$t1",s);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else {
			printmips("add","$t1","$0",b);
			p2=findsymbol_run(c);
			addr2=p2.addr;
			itoa(addr2,int2str,10);
			if(p2.layers==-1){
                printmips("lw","$t2",int2str,"$s6");
			}else{
                if(addr2>0) {
                    itoa(addr2-sp_top,int2str,10);
                    printmips("lw","$t2",int2str,"$s7");
                } else {
                    printmips("lw","$t2",int2str,"$sp");
                }
			}
			printmips("sub","$t0","$t1","$t2");
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		}
	} else {
		p2=findsymbol_run(b);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t1",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t1",int2str,"$s7");
            } else {
                printmips("lw","$t1",int2str,"$sp");
            }
		}
		if(c[0]=='\'') {
			v=char2integer(c);
			itoa(v,int2str,10);
			strcpy(s,"-");
			strcat(s,int2str);
			printmips("add","$t0","$t1",s);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else if(isdigit(c[0])||c[0]=='-') {
			if(c[0]=='-') {
				strcpy(s,c+1);
			} else {
				strcpy(s,"-");
				strcat(s,c);
			}
			printmips("add","$t0","$t1",s);
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else {
			p3=findsymbol_run(c);
			addr3=p3.addr;
			itoa(addr3,int2str,10);
			if(p3.layers==-1){
                printmips("lw","$t2",int2str,"$s6");
			}else{
                if(addr3>0) {
                    itoa(addr3-sp_top,int2str,10);
                    printmips("lw","$t2",int2str,"$s7");
                } else {
                    printmips("lw","$t2",int2str,"$sp");
                }
			}
			printmips("sub","$t0","$t1","$t2");
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		}
	}
}

void mulprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	p1=findsymbol_run(d);
	addr1=p1.addr;
	if(b[0]=='\'') {
		v=char2integer(b);
		itoa(v,int2str,10);
		printmips("add","$t1","$0",int2str);
	} else if(isdigit(b[0])||b[0]=='-') {
		printmips("add","$t1","$0",b);
	} else {
		p2=findsymbol_run(b);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t1",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t1",int2str,"$s7");
            } else {
                printmips("lw","$t1",int2str,"$sp");
            }
		}
	}
	if(c[0]=='\'') {
		v=char2integer(c);
		itoa(v,int2str,10);
		printmips("add","$t2","$0",int2str);
	} else if(isdigit(c[0])||c[0]=='-') {
		printmips("add","$t2","$0",c);
	} else {
		p2=findsymbol_run(c);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t2",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t2",int2str,"$s7");
            } else {
                printmips("lw","$t2",int2str,"$sp");
            }
		}
	}
	printmips("mul","$t0","$t1","$t2");
	itoa(addr1,int2str,10);
	if(p1.layers==-1){
        printmips("sw","$t0",int2str,"$s6");
	}else{
        if(addr1>0) {
            itoa(addr1-sp_top,int2str,10);
            printmips("sw","$t0",int2str,"$s7");
        } else {
            printmips("sw","$t0",int2str,"$sp");
        }
	}
}

void divprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	p1=findsymbol_run(d);
	addr1=p1.addr;
	if(b[0]=='\'') {
		v=char2integer(b);
		itoa(v,int2str,10);
		printmips("add","$t1","$0",int2str);
	} else if(isdigit(b[0])||b[0]=='-') {
		printmips("add","$t1","$0",b);
	} else {
		p2=findsymbol_run(b);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t1",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t1",int2str,"$s7");
            } else {
                printmips("lw","$t1",int2str,"$sp");
            }
		}
	}
	if(c[0]=='\'') {
		v=char2integer(c);
		itoa(v,int2str,10);
		printmips("add","$t2","$0",int2str);
	} else if(isdigit(c[0])||c[0]=='-') {
		printmips("add","$t2","$0",c);
	} else {
		p2=findsymbol_run(c);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t2",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t2",int2str,"$s7");
            } else {
                printmips("lw","$t2",int2str,"$sp");
            }
		}
	}
	printmips("div","$t1","$t2","");
	printmips("mflo","$t0","","");
	itoa(addr1,int2str,10);
	if(p1.layers==-1){
        printmips("sw","$t0",int2str,"$s6");
	}else{
        if(addr1>0) {
            itoa(addr1-sp_top,int2str,10);
            printmips("sw","$t0",int2str,"$s7");
        } else {
            printmips("sw","$t0",int2str,"$sp");
        }
	}
}

void equalprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	p1=findsymbol_run(c);
	addr1=p1.addr;
	//fprintf(fp2,"%lld\n",addr1);
	if(b[0]=='\'') {
		v=char2integer(b);
		itoa(v,int2str,10);
		printmips("add","$t0","$0",int2str);
		itoa(addr1,int2str,10);
		if(p1.layers==-1){
            printmips("sw","$t0",int2str,"$s6");
        }else{
            if(addr1>0) {
                itoa(addr1-sp_top,int2str,10);
                printmips("sw","$t0",int2str,"$s7");
            } else {
                printmips("sw","$t0",int2str,"$sp");
            }
        }
	} else if(isdigit(b[0])||b[0]=='-') {
		printmips("add","$t0","$0",b);
		itoa(addr1,int2str,10);
		if(p1.layers==-1){
            printmips("sw","$t0",int2str,"$s6");
        }else{
            if(addr1>0) {
                itoa(addr1-sp_top,int2str,10);
                printmips("sw","$t0",int2str,"$s7");
            } else {
                printmips("sw","$t0",int2str,"$sp");
            }
        }
	} else if(strcmp(b,"RET")==0) {
		itoa(addr1,int2str,10);
		if(p1.layers==-1){
            printmips("sw","$v1",int2str,"$s6");
        }else{
            if(addr1>0) {
                itoa(addr1-sp_top,int2str,10);
                printmips("sw","$v1",int2str,"$s7");
            } else {
                printmips("sw","$v1",int2str,"$sp");
            }
        }
	} else {
		p2=findsymbol_run(b);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t0",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t0",int2str,"$s7");
            } else {
                printmips("lw","$t0",int2str,"$sp");
            }
		}
		itoa(addr1,int2str,10);
		if(p1.layers==-1){
            printmips("sw","$t0",int2str,"$s6");
        }else{
            if(addr1>0) {
                itoa(addr1-sp_top,int2str,10);
                printmips("sw","$t0",int2str,"$s7");
            } else {
                printmips("sw","$t0",int2str,"$sp");
            }
        }
	}
}

void arrayequalprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	p1=findsymbol_run(d);
	addr1=p1.addr;
	//fprintf(fp2,"%lld\n",addr1);
	if(c[0]=='\'') {
		v=char2integer(c);
		itoa(v,int2str,10);
		printmips("add","$t0","$0",int2str);
	} else if(isdigit(c[0])||c[0]=='-') {
		printmips("add","$t0","$0",c);
	} else {
		p2=findsymbol_run(c);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t0",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t0",int2str,"$s7");
            } else {
                printmips("lw","$t0",int2str,"$sp");
            }
		}

	}
	if(isdigit(b[0])) {
		v=char2int(b);
		addr1+=v*4;
		itoa(addr1,int2str,10);
		if(p1.layers==-1){
            printmips("sw","$t0",int2str,"$s6");
        }else{
            if(addr1>0) {
                itoa(addr1-sp_top,int2str,10);
                printmips("sw","$t0",int2str,"$s7");
            } else {
                printmips("sw","$t0",int2str,"$sp");
            }
        }
	} else {
        p2=findsymbol_run(b);
        addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t1",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t1",int2str,"$s7");
            } else {
                printmips("lw","$t1",int2str,"$sp");
            }
		}
		printmips("mul","$t1","$t1","4");
		itoa(addr1,int2str,10);
		printmips("add","$t1","$t1",int2str);
		if(addr1>0)
			printmips("sw","$t0","0","$t1");
		else {
			printmips("add","$t1","$t1","$sp");
			printmips("sw","$t0","0","$t1");
		}
	}
}

void arrayprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	p1=findsymbol_run(c);
	addr1=p1.addr;
	//fprintf(fp2,"%lld\n",addr1);
	if(isdigit(b[0])) {
		v=char2int(b);
		addr1+=v*4;
		itoa(addr1,int2str,10);
		if(p1.layers==-1){
            printmips("lw","$t0",int2str,"$s6");
		}else{
            if(addr1>0){
                itoa(addr1-sp_top,int2str,10);
                printmips("lw","$t0",int2str,"$s7");
            }else
                printmips("lw","$t0",int2str,"$sp");
		}
	} else {
		p2=findsymbol_run(b);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t1",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t1",int2str,"$s7");
            } else {
                printmips("lw","$t1",int2str,"$sp");
            }
		}
		printmips("mul","$t1","$t1","4");
		itoa(addr1,int2str,10);
		printmips("add","$t1","$t1",int2str);
		if(addr1>0)
			printmips("lw","$t0","0","$t1");
		else {
			printmips("add","$t1","$t1","$sp");
			printmips("lw","$t0","0","$t1");
		}
	}
	p3=findsymbol_run(d);
	addr3=p3.addr;
	itoa(addr3,int2str,10);
	if(p3.layers==-1){
        printmips("sw","$t0",int2str,"$s6");
	}else{
        if(addr3>0) {
            itoa(addr3-sp_top,int2str,10);
            printmips("sw","$t0",int2str,"$s7");
        } else {
            printmips("sw","$t0",int2str,"$sp");
        }
	}

}

void relationprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	if(isdigit(b[0])||b[0]=='-') {
		printmips("add","$t1","$0",b);
	} else {
		p2=findsymbol_run(b);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		//printf("%lld\n",addr2);
		if(p2.layers==-1){
            printmips("lw","$t1",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t1",int2str,"$s7");
            } else {
                printmips("lw","$t1",int2str,"$sp");
            }
		}
	}
	if(isdigit(c[0])||c[0]=='-') {
		printmips("add","$t2","$0",c);
	} else {
		p2=findsymbol_run(c);
		addr2=p2.addr;
		itoa(addr2,int2str,10);
		if(p2.layers==-1){
            printmips("lw","$t2",int2str,"$s6");
		}else{
            if(addr2>0) {
                itoa(addr2-sp_top,int2str,10);
                printmips("lw","$t2",int2str,"$s7");
            } else {
                printmips("lw","$t2",int2str,"$sp");
            }
		}
	}
	if(strcmp(a,"<")==0) {
		printmips("slt","$t0","$t1","$t2");
	} else if(strcmp(a,"<=")==0) {
		printmips("sle","$t0","$t1","$t2");
	} else if(strcmp(a,">")==0) {
		printmips("sgt","$t0","$t1","$t2");
	} else if(strcmp(a,">=")==0) {
		printmips("sge","$t0","$t1","$t2");
	} else if(strcmp(a,"==")==0) {
		printmips("seq","$t0","$t1","$t2");
	} else if(strcmp(a,"!=")==0) {
		printmips("sne","$t0","$t1","$t2");
	}

}

void printprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	struct retsymbol p1,p2,p3;
    if(strcmp(c,"int")==0){
    	p1=findsymbol_run(b);
    	addr1=p1.addr;
    	itoa(addr1,int2str,10);
    	if(p1.layers==-1){
            printmips("lw","$a0",int2str,"$s6");
    	}else{
            if(addr1>0){
                printmips("lw","$a0",int2str,"$0");
            }else{
                printmips("lw","$a0",int2str,"$sp");
            }
    	}
        printmips("add","$v0","$0","1");
    }else if(strcmp(c,"char")==0){
        p1=findsymbol_run(b);
    	addr1=p1.addr;
        itoa(addr1,int2str,10);
        if(p1.layers==-1){
            printmips("lw","$a0",int2str,"$s6");
    	}else{
            if(addr1>0){
                printmips("lw","$a0",int2str,"$0");
            }else{
                printmips("lw","$a0",int2str,"$sp");
            }
    	}
        printmips("add","$v0","$0","11");
    }else{
    	printmips("la","$a0",b,"");
    	printmips("add","$v0","$0","4");
	}
    printmips("syscall","","","");
}

void readprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	p1=findsymbol_run(b);
	addr1=p1.addr;
	//printf("%s %lld\n",b,addr1);
	if(strcmp(c,"int")==0) {
		printmips("add","$v0","$0","5");
	} else if(strcmp(c,"char")==0) {
		printmips("add","$v0","$0","12");
	}
	printmips("syscall","","","");
	itoa(addr1,int2str,10);
	if(p1.layers==-1){
        printmips("sw","$v0",int2str,"$s6");
	}else{
        if(addr1>0) {
            itoa(addr1-sp_top,int2str,10);
            printmips("sw","$v0",int2str,"$s7");
        } else {
            printmips("sw","$v0",int2str,"$sp");
        }
	}
}

void retprocess(char a[100],char b[100],char c[100],char d[100]) {
	long long int addr1,addr2,addr3;
	int v;
	char s[100];
	struct retsymbol p1,p2,p3;
	if(maini==1){
		printmips("j","end","","");
	}else{
		if(strlen(b)>0) {
			if(b[0]=='\'') {
				v=char2integer(c);
				itoa(v,int2str,10);
				printmips("add","$v1","$0",int2str);
			} else if(isdigit(b[0])||b[0]=='-') {
				printmips("add","$v1","$0",b);
			} else {
				p2=findsymbol_run(b);
				addr2=p2.addr;
				itoa(addr2,int2str,10);
				if(p2.layers==-1){
	                printmips("lw","$v1",int2str,"$s6");
				}else{
	                if(addr2>0) {
	                    itoa(addr2-sp_top,int2str,10);
	                    printmips("lw","$v1",int2str,"$s7");
	                } else {
	                    printmips("lw","$v1",int2str,"$sp");
	                }
				}
			}
		}
		printmips("lw","$ra","-8","$sp");
		printmips("lw","$sp","-12","$sp");
		printmips("jr","$ra","","");
	}
	
}

void printmips(char op[100],char a[100],char b[100],char c[100]) {
	if(strcmp(op,"lw")==0||strcmp(op,"sw")==0) {
		fprintf(fp2,"%s %s,%s(%s)\n",op,a,b,c);
	} else if(strcmp(op,"add")==0) {
		fprintf(fp2,"%s %s,%s,%s\n",op,a,b,c);
	} else if(strcmp(op,"sub")==0) {
		fprintf(fp2,"%s %s,%s,%s\n",op,a,b,c);
	} else if(strcmp(op,"mul")==0) {
		fprintf(fp2,"%s %s,%s,%s\n",op,a,b,c);
	} else if(strcmp(op,"div")==0) {
		fprintf(fp2,"%s %s,%s\n",op,a,b);
	} else if(strcmp(op,"slt")==0||strcmp(op,"sle")==0||strcmp(op,"sgt")==0||strcmp(op,"sge")==0||strcmp(op,"seq")==0||strcmp(op,"sne")==0) {
		fprintf(fp2,"%s %s,%s,%s\n",op,a,b,c);
	} else if(strcmp(op,"mflo")==0) {
		fprintf(fp2,"%s %s\n",op,a);
	} else if(strcmp(op,"jal")==0||strcmp(op,"j")==0||strcmp(op,"jr")==0) {
		fprintf(fp2,"%s %s\n",op,a);
	} else if(strcmp(op,"label")==0) {
		fprintf(fp2,"%s:\n",a);
	} else if(strcmp(op,"syscall")==0) {
		fprintf(fp2,"%s\n",op);
	} else if(strcmp(op,"la")==0) {
		fprintf(fp2,"%s %s,%s\n",op,a,b);
	} else if(strcmp(op,"bgtz")==0||strcmp(op,"blez")==0) {
		fprintf(fp2,"%s %s,%s\n",op,a,b);
	} else{
		printf("!!!");
	}
}

void data() {
	int i,j,len,first,last,k,t,flag=0;
	char a[100],b[100],c[100],d[100];
	char s[100];
	fprintf(fp2,".data\n");
	fprintf(fp2,"endprint: .asciiz \"\\n\"\n");
	for(i=0; i<fouri; i++) {
		strcpy(a,four[i].a);
        strcpy(b,four[i].b);
        strcpy(c,four[i].c);
        strcpy(d,four[i].d);
        if(strcmp(a,"print")==0 && strcmp(c,"string")==0){
            for(j=0;j<strlen(b);j++){
                if(b[j]=='\\'||b[j]=='\''){
                    for(k=0;k<j;k++){
                        s[k]=b[k];
                    }
                    s[k]='\\';
                    for(k++;k<=strlen(b);k++){
                        s[k]=b[k-1];
                        //printf("%c\n",b[k-1]);
                    }
                    s[k]='\0';
                    //printf("%d %s\n",j,s);
                    strcpy(b,s);
                    j++;
                }
            }
            //printf("%s\n",b);
            fprintf(fp2,"wyzstring%d: .asciiz %s\n",wyzstringi,b);
            itoa(wyzstringi,int2str,10);
            strcpy(b,"wyzstring");
            strcat(b,int2str);
            strcpy(four[i].b,b);
            wyzstringi++;
        }
	}
}

void text() {
	int i,v,pushi[1000],flag=0,funcparareserve[1000],start=0,j,k;
	long long int addr1,addr2,addr3;
	int addr,dimension;
	char a[100],b[100],c[100],d[100],givepara[100][100][100],s[100];
	struct retsymbol p1,p2,p3;
	fprintf(fp2,".text\n");
	itoa(2147479548,int2str,10);//2147479548
	printmips("add","$s7","$0",int2str);
	//itoa(2147000000,int2str,10);//2147479548
	//printmips("add","$s6","$0",int2str);
	for(i=0; i<fouri; i++) {
		strcpy(a,four[i].a);
		strcpy(b,four[i].b);
		strcpy(c,four[i].c);
		strcpy(d,four[i].d);
		//fprintf(fp2,"%s %s %s %s\n",a,b,c,d);
		if(strcmp(a,"const")==0&&strcmp(b,"char")==0) {
			addsymbol_run(c,1);
			v=(int) (d[0]);
			itoa(v,int2str,10);
			printmips("add","$t0","$0",int2str);
			p1=findsymbol_run(c);
			addr1=p1.addr;
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else if(strcmp(a,"const")==0&&strcmp(b,"int")==0) {
			addsymbol_run(c,1);
			printmips("add","$t0","$0",d);
			p1=findsymbol_run(c);
			addr1=p1.addr;
			itoa(addr1,int2str,10);
			if(p1.layers==-1){
                printmips("sw","$t0",int2str,"$s6");
			}else{
                if(addr1>0) {
                    itoa(addr1-sp_top,int2str,10);
                    printmips("sw","$t0",int2str,"$s7");
                } else {
                    printmips("sw","$t0",int2str,"$sp");
                }
			}
		} else {
			if(flag==0) {
				fprintf(fp2,"j main\n");
				flag++;
			}
			if(b[strlen(b)-1]==')'&&b[strlen(b)-2]=='(') {
				b[strlen(b)-2]='\0';
				funcparai=0;
				//addsymbol_run(b,1);
				printmips("label",b,"","");
				sp_recent_top=sp_recent;
				if(strcmp(b,"main")==0) {
					maini=1;
					sp_recent-=4;
					itoa(sp_recent_top,int2str,10);
					printmips("add","$sp","$0",int2str);//可优化
					itoa(sp_recent,int2str,10);
					printmips("sw","$sp",int2str,"$0");
					sp_recent-=8;
				} else {
					sp_recent-=8;
					itoa(sp_recent,int2str,10);
					printmips("sw","$ra","-8","$sp");
					sp_recent-=4;
				}
				parai=0;
				layer++;
			} else if(strcmp(a,"int")==0||strcmp(a,"char")==0) {
				addsymbol_run(b,1);
			} else if(strcmp(a,"int[]")==0||strcmp(a,"char[]")==0) {
				dimension=char2int(c);
				addsymbol_run(b,dimension);
			} else if(strcmp(a,"=")==0) {
				equalprocess(a,b,c,d);
			} else if(strcmp(a,"call")==0) {
				//printf("%d\n",pushi[start]);
				funcparareserve[start]=funcparai;
				for(j=1;j<=pushi[start];j++){
					strcpy(s,givepara[start][j]);
					//printf("%d %s\n",j,s);
					if(s[0]=='\'') {
						v=char2integer(s);
						itoa(v,int2str,10);
						printmips("add","$t0","$0",int2str);
						itoa(-j*4+funcparareserve[start]*-4-24*start,int2str,10);
						printmips("sw","$t0",int2str,"$sp");
					} else if(isdigit(s[0])||s[0]=='-') {
						printmips("add","$t0","$0",s);
						itoa(-j*4+funcparareserve[start]*-4-24*start,int2str,10);
						printmips("sw","$t0",int2str,"$sp");
					} else {
						p1=findsymbol_run(s);
						addr1=p1.addr;
						itoa(addr1,int2str,10);
						if(p1.layers==-1){
	                        printmips("lw","$t0",int2str,"$s6");
						}else{
	                        if(addr1>0) {
	                            itoa(addr1-sp_top,int2str,10);
	                            printmips("lw","$t0",int2str,"$s7");
	                        } else {
	                            printmips("lw","$t0",int2str,"$sp");
	                        }
						}
						itoa(-j*4+funcparareserve[start]*-4-24*start,int2str,10);
						printmips("sw","$t0",int2str,"$sp");
					}
				}
				itoa(funcparareserve[start]*-4-24*start+12,int2str,10);
				printmips("add","$t9","$sp","$0");
				printmips("add","$sp","$sp",int2str);
				printmips("sw","$sp","-4","$sp");
				printmips("sw","$t9","-12","$sp");
				printmips("jal",b,"","");
				start--;
			} else if(strcmp(a,"push")==0) {
				//printf("%d %d\n",start,pushi[start]);
				pushi[start]++;
				strcpy(givepara[start][pushi[start]],b);
				/*if(b[0]=='\'') {
					v=char2integer(b);
					itoa(v,int2str,10);
					printmips("add","$t0","$0",int2str);
					itoa(-pushi[start]*4+funcparareserve[start]*-4-24*start,int2str,10);
					printmips("sw","$t0",int2str,"$sp");
				} else if(isdigit(b[0])||b[0]=='-') {
					printmips("add","$t0","$0",b);
					itoa(-pushi[start]*4+funcparareserve[start]*-4-24*start,int2str,10);
					printmips("sw","$t0",int2str,"$sp");
				} else {
					p1=findsymbol_run(b);
					addr1=p1.addr;
					itoa(addr1,int2str,10);
					if(p1.layers==-1){
                        printmips("lw","$t0",int2str,"$s6");
					}else{
                        if(addr1>0) {
                            itoa(addr1-sp_top,int2str,10);
                            printmips("lw","$t0",int2str,"$s7");
                        } else {
                            printmips("lw","$t0",int2str,"$sp");
                        }
					}
					itoa(-pushi[start]*4+funcparareserve[start]*-4-24*start,int2str,10);
					printmips("sw","$t0",int2str,"$sp");
				}*/
			} else if(strcmp(a,"startfunc")==0){
				start++;
				pushi[start]=0;
				//funcparareserve[start]=funcparai;
			} else if(strcmp(a,"+")==0) {
				//fprintf(fp2,"%s %s %s %s\n",a,b,c,d);
				addprocess(a,b,c,d);
			} else if(strcmp(a,"-")==0) {
				subprocess(a,b,c,d);
			} else if(strcmp(a,"*")==0) {
				mulprocess(a,b,c,d);
			} else if(strcmp(a,"/")==0) {
				//fprintf(fp2,"%s %s %s %s\n",a,b,c,d);
				divprocess(a,b,c,d);
			} else if(strcmp(a,"read")==0) {
				readprocess(a,b,c,d);
			} else if(strcmp(a,"print")==0) {
				printprocess(a,b,c,d);
			} else if(strcmp(a,"endprint")==0) {
				printmips("la","$a0","endprint","");
				printmips("add","$v0","$0","4");
				printmips("syscall","","","");
			} else if(strcmp(a,"ret")==0) {
				retprocess(a,b,c,d);
			} else if(strcmp(a,"end")==0) {
				printmips("lw","$ra","-8","$sp");
				printmips("lw","$sp","-12","$sp");
				printmips("jr","$ra","","");
				updatesymbol_run();
			} else if(strcmp(a,"para")==0) {
				addsymbol_run(c,1);
				//fprintf(fp2,"%lld\n",symbol[symboli-1].addr);
			} else if(strcmp(a,"[]=")==0) {
				arrayequalprocess(a,b,c,d);
			} else if(strcmp(a,"[]")==0) {
				arrayprocess(a,b,c,d);
			} else if(strcmp(a,"<")==0||strcmp(a,"<=")==0||strcmp(a,">")==0||strcmp(a,">=")==0||strcmp(a,"==")==0||strcmp(a,"!=")==0) {
				relationprocess(a,b,c,d);
			} else if(strcmp(a,"label")==0) {
				printmips("label",b,"","");
			} else if(strcmp(a,"GOTO")==0) {
				printmips("j",b,"","");
			} else if(strcmp(a,"BNZ")==0) {
				printmips("bgtz","$t0",b,"");
			} else if(strcmp(a,"BZ")==0) {
				printmips("blez","$t0",b,"");
			}
		}
	}
}



void mips() {
	layer=0;
	data();
	text();
	printmips("label","end","","");
}

int main() {
	int i=0;
	fp1=fopen("testfile.txt","r");
	fp2=fopen("mips.txt","w");
	init();
	do {
		program();
	} while(ch!=EOF);

	mips();
	fclose(fp1);
	fclose(fp2);

	return 0;
}

