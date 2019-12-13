#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>

char str[10000], token[100];
char ren_str[100];
int str_[10000] = {0};
char ch;
int wmcount = 0;
int k = 0;
int mid_line = 0;
int mid_if = 0;
int mid_for = 0;
int mid_do = 0;
int mid_while = 0;
int index1 = 0;
int str_no = 0;
int item();
void combine_state();
void state();
void function_state();

int strtonum(char *str){
	char *a;
	int find  = 0;
	int x = 0;
	for(a = str;*a!='\0';a++){
		if(*a == '-') {
			find = 1;
			continue;
		}
		x = 10*x + (*a - '0');
	}
	if(find == 1){
		return -x;
	}
	return x;
}
char *numtostr(int c) {
	int x = c;
	int a[1000];
	int i;
	char b[1000];
	int index = 0;
	do{
		a[index++] = x%10;
		x = x /10;
	}while(x!=0);
	for(i = 0,index-=1;index>=0;index--,i++){
		b[i] = a[index] + '0';
	}
	b[i] = '\0';
	char *tmp = (char*)malloc(sizeof(char));
	strcpy(tmp,b);
	return tmp;
}
char* ren_wmcount(){
	char * tmp = numtostr(mid_line);
    char * wmstr = (char*)malloc(sizeof(tmp)+3*sizeof(char));
    strcpy(wmstr,"wm");
    strcat(wmstr,tmp);
    free(tmp);
    wmcount++;
    return wmstr;
}
typedef struct{
    int op;
    int line;
    char n1[100];
    char n2[100];
    char n3[100];
    char n4[100];
}mid_code;
mid_code * code_list[20000] = {NULL};
void midstruct(int op,char*n1,char*n2,char* n3,char* n4){
    mid_code *wm = (mid_code*)malloc(sizeof(mid_code));
    wm->line = mid_line;
    wm->op = op;
    strcpy(wm->n1,n1);
    strcpy(wm->n2,n2);
    if(n3!=NULL){
        strcpy(wm->n3,n3);
    }
    if(n4!=NULL){
        strcpy(wm->n4,n4);
    }
    code_list[mid_line++] = wm;
}
const char* table[] = { "IDENFR","INTCON","CHARCON","STRCON","CONSTTK","INTTK","CHARTK","VOIDTK","MAINTK",
"IFTK","ELSETK","DOTK","WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK","PLUS","MINU","MULT","DIV","LSS","LEQ",
"GRE","GEQ","EQL","NEQ","ASSIGN","SEMICN","COMMA","LPARENT","RPARENT","LBRACK","RBRACK","LBRACE", "RBRACE"
};
const char* chars[] = { "const","int","char","void","main","if","else","do","while","for","scanf","printf","return" };
const char error[] = "abcdefghijklmno";
FILE* in, *out,*out2,*out_mid,*out_mips;
int line = 1;
int returned = 0;
typedef struct{
    char name[100];
    int isconst;
    int isint;
    int reg;//用了几号寄存器
	int no;//第几个
    struct var * next;
}var;

struct name_state {
	int style;
	char name[100];
	int var_num;
	int attr_num;
	var * attr_list;//传入的参数表
	var * var_list;//声明的参数表
	struct name_state* next;
};
struct name_state * now;
var * top_list = NULL;
var * main_list = NULL;
struct name_state *namelist,*p;

void clearToken() {
	int i = 0;
	for (; i < 100; i++) {
		token[i] = '\0';
	}
	index1 = 0;
}

void getch() {
	ch = str[k];
	k++;
}

void catToken() {
	token[index1++] = ch;
}

int isunderline(char c) {
	if (c == '_') {
		return 1;
	}
	else {
		return 0;
	}
}

void retract() {
	k--;
}

int reserver() {
	int i;
	for (i = 0; i <= 12; i++) {
		if (strcmp(token, chars[i]) == 0) {
			return i + 4;
		}
	}

	return 0;
}

int getsym()
{
	int resultValue;
	clearToken();
	getch();
	while (isspace(ch)) {
        if(ch == '\n' && str_[k]!=1){
            line++;
            str_[k] = 1;
		}
		getch();
	}
	if (isalpha(ch) || isunderline(ch))
	{
		while (isalpha(ch) || isdigit(ch) || isunderline(ch))
		{
			catToken();
			getch();
		}
		retract();
		resultValue = reserver();
	}
	else if (isdigit(ch))
	{
		while (isdigit(ch))
		{
			catToken();
			getch();
		}
		retract();
		resultValue = 1;
	}
	else if (ch == '\'')
	{
		getch();
        if(!isalnum(ch) && ch !='+' && ch!='-' && ch!='*' && ch != '/') {
            fprintf(out2,"%d %c\n",line,error[0]);
        }
        catToken();
        getch();
        if(ch != '\'') {
            fprintf(out2,"%d %c\n",line,error[0]);
        }
		resultValue = 2;
	}
	else if (ch == '\"')
	{
		getch();
		while (ch != '\"') {
            if(!(ch <=126 && ch>=32 && ch!= 34)) {
                fprintf(out2,"%d %c\n",line,error[0]);
            }
            if(ch == '\n') {
                fprintf(out2,"%d %c\n",line,error[0]);
                line++;
            }
			catToken();
			getch();
		}
		resultValue = 3;
	}
	else if (ch == '+') {
		catToken();
		resultValue = 17;
	}
	else if (ch == '-') {
		catToken();
		resultValue = 18;
	}
	else if (ch == '*') {
		catToken();
		resultValue = 19;
	}
	else if (ch == '/') {
		catToken();
		resultValue = 20;
	}
	else if (ch == '<') {
		catToken();
		getch();
		if (ch == '=') {
			resultValue = 22;
			catToken();
		}
		else {
			retract();
			resultValue = 21;
		}
	}
	else if (ch == '>') {
		catToken();
		getch();
		if (ch == '=') {
			catToken();
			resultValue = 24;
		}
		else {
			retract();
			resultValue = 23;
		}
	}
	else if (ch == '=') {
		catToken();
		getch();
		if (ch == '=') {
			catToken();
			resultValue = 25;
		}
		else {
			retract();
			resultValue = 27;
		}
	}
	else if (ch == '!') {
		catToken();
		getch();
		if (ch == '=') {
			catToken();
			resultValue = 26;
		}
		else {
			resultValue = -1;
		}
	}
	else if (ch == ';') {
		catToken();
		resultValue = 28;
	}
	else if (ch == ',') {
		catToken();
		resultValue = 29;
	}
	else if (ch == '(') {
		catToken();
		resultValue = 30;
	}
	else if (ch == ')') {
		catToken();
		resultValue = 31;
	}
	else if (ch == '[') {
		catToken();
		resultValue = 32;
	}
	else if (ch == ']') {
		catToken();
		resultValue = 33;
	}
	else if (ch == '{') {
		catToken();
		resultValue = 34;
	}
	else if (ch == '}') {
		catToken();
		resultValue = 35;
	}
	else {
		resultValue = -1;
	}
	return resultValue;
}

void unsigned_int() {
	int id;
	id = getsym();
	fprintf(out,"%s %s\n",table[id],token);
	fprintf(out, "<无符号整数>\n");
}

void integer() {
	int id;
	int i;
	i = k;
	id = getsym();
	if(id == 17 || id == 18) {
		fprintf(out,"%s %s\n",table[id],token);
	} else {
		k = i;
	}
	unsigned_int();
	fprintf(out, "<整数>\n");
}

void const_define(var ** llist) {
	int i;
	int id;
	int style;
	char n1[100]="const";
	char n2[100]={'\0'};
	char n3[100]={'\0'};
	char n4[100]={'\0'};
	id = getsym();
	style = id;
	fprintf(out,"%s %s\n",table[id],token);//输出"int/char"
	strcpy(n2,token);
	fprintf(out,"%s %s\n",table[getsym()],token);//输出标识符
    strcpy(n3,token);
    var * ll;
    var * mm;
    if(*llist == NULL){
       var *q = (var*)malloc(sizeof(var));
        q->isconst = 1;
        strcpy(q->name,token);
        q->isint = (id == 5)?1:0;
        q->next = NULL;
        *llist = q;
    } else{
        for(ll=*llist;ll!=NULL;ll=ll->next) {
            mm = ll;
            if(strcmp(ll->name,token) == 0) {
                fprintf(out2,"%d %c\n",line,error[1]);
            }
        }
        var *q = (var*)malloc(sizeof(var));
        q->isconst = 1;
        strcpy(q->name,token);
        q->isint = (id == 5)?1:0;
        q->next = NULL;
        mm->next= q;
    }

	fprintf(out,"%s %s\n",table[getsym()],token);//输出'='
    i = k;
    id = getsym();
    k = i;
	if(style == 6) {
        id = getsym();
		fprintf(out,"%s %s\n",table[id],token);
		char n[100] = {'\0'};
		n[0] = '\'';
		strcat(n,token);
		strcat(n,"\'");
		strcpy(n4,n);
		if(id != 2){
            fprintf(out2,"%d o\n",line);
            i = k;
            while(id != 28 && id != 29){
                i = k;
                id = getsym();
            }
            k = i;
		}
	} else if(style == 5){
	    if(id == 17 || id == 18 || id == 1 ){
            integer();
            char n[100] = {'\0'};
            if(id == 18){
				n[0] = '-';
			}
			strcat(n,token);
            strcpy(n4,n);
	    } else{
            fprintf(out2,"%d o\n",line);
            while(id != 28 && id != 29){
                i = k;
                id = getsym();
            }
            k = i;
	    }
	} else {
        fprintf(out,"%s %s\n",table[getsym()],token);
        fprintf(out2,"%d o\n",line);
	}
    midstruct(4,n1,n2,n3,n4);

	i = k;
	id = getsym();
	while(id == 29) {//判断逗号
		fprintf(out,"%s %s\n",table[id],token);
		fprintf(out,"%s %s\n",table[getsym()],token);
        if(style == 5){
            strcpy(n2,"int");
        }else{
            strcpy(n2,"char");
        }
        strcpy(n3,token);
        for(ll=*llist;ll!=NULL;ll=ll->next){
            mm = ll;
            if(strcmp(ll->name,token) == 0){
                fprintf(out2,"%d %c\n",line,error[1]);
            }
        }
		var *q = (var*)malloc(sizeof(var));
        q->isconst = 1;
        strcpy(q->name,token);
        q->isint = (style == 5)?1:0;
        q->next = NULL;
        mm->next = q;

		fprintf(out,"%s %s\n",table[getsym()],token);
		if(style == 6) {
			fprintf(out,"%s %s\n",table[getsym()],token);
			char n[100]= {'\0'};
			n[0] = '\'';
			strcat(n,token);
			strcat(n,"\'");
			strcpy(n4,n);
		} else if(style == 5){
			i = k;
			id = getsym();
			k = i;
			char n[100] = {'\0'};
			if(id == 18) {
				n[0]='-';
			}
			integer();
			strcat(n,token);
			strcpy(n4,n);
		} else {
            fprintf(out,"%s %s\n",table[getsym()],token);
            fprintf(out2,"%d o\n",line);
        }
        midstruct(4,n1,n2,n3,n4);
		i = k;
		id = getsym();
	}
	k = i;
	fprintf(out, "<常量定义>\n");
}

void const_state(var ** llist) {
	int i;
	i = k;
	int id;
	int style;
	id = getsym();
	style = id;
	while(id == 4) {
		fprintf(out,"%s %s\n",table[id],token);
		const_define(&(*llist));

		i = k;
		id = getsym();
		if(id != 28){
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}

		fprintf(out,"%s %s\n",table[id],token);//应该输出';'
		i = k;
		id = getsym();
	}
	k = i;
	fprintf(out, "<常量说明>\n");
}

void variable_define(var **llist) {
	int id;
	int i;
	int style;
	char n1[100]="var";
	char n2[100]={'\0'};
	char n3[100]={'\0'};
	char n4[100]={'\0'};

	i = k;
	id = getsym();
	style = id;
	fprintf(out,"%s %s\n",table[id],token);
	strcpy(n1,"var");
	strcpy(n2,token);
	//printf("%s %d\n",token,style);
	fprintf(out,"%s %s\n",table[getsym()],token);
    strcpy(n3,token);
    var * ll;
    var * mm;
    if(*llist == NULL){
       var *q = (var*)malloc(sizeof(var));
        q->isconst = 0;
        strcpy(q->name,token);
        q->isint = (id == 5)?1:0;
        q->next = NULL;
        *llist = q;
    } else{
        for(ll=*llist;ll!=NULL;ll=ll->next) {
            mm = ll;
            if(strcmp(ll->name,token) == 0) {
                fprintf(out2,"%d %c\n",line,error[1]);
            }
        }
        var *q = (var*)malloc(sizeof(var));
        q->isconst = 0;
        strcpy(q->name,token);
        q->isint = (id == 5)?1:0;
        q->next = NULL;
        mm->next= q;
    }

	i = k;
	id = getsym();
	if(id == 32) {
		fprintf(out,"%s %s\n",table[id],token);
		unsigned_int();
		strcpy(n4,token);
		i = k;
        id = getsym();
        if(id != 33){
            fprintf(out2,"%d m\n",line);
            k = i;
        }
		fprintf(out,"%s %s\n",table[id],token);
	} else {
		k = i;
	}
	midstruct(3,n1,n2,n3,n4);
	i = k;
	id = getsym();
	while(id == 29) {
        strcpy(n1,"var");
		fprintf(out,"%s %s\n",table[id],token);
		fprintf(out,"%s %s\n",table[getsym()],token);
		if(style == 6){
            strcpy(n2,"char");
		} else{
            strcpy(n2,"int");
		}
        strcpy(n3,token);
        for(ll=*llist;ll!=NULL;ll=ll->next) {
            mm = ll;
            if(strcmp(ll->name,token) == 0) {
                fprintf(out2,"%d %c\n",line,error[1]);
            }
        }
        var *q = (var*)malloc(sizeof(var));
        q->isconst = 0;
        strcpy(q->name,token);
        q->isint = (style == 5)?1:0;
        q->next = NULL;
        mm->next= q;

		i = k;//开始判断是不是数组
		id = getsym();
		n4[0] = '\0';
		if(id == 32) {
			fprintf(out,"%s %s\n",table[id],token);
			unsigned_int();
			strcpy(n4,token);
			i = k;
            id = getsym();
            if(id != 33){
                fprintf(out2,"%d m\n",line);
                k = i;
            }
			fprintf(out,"%s %s\n",table[id],token);
		} else {
			k = i;
		}
		midstruct(3,n1,n2,n3,n4);
		i = k;
		id = getsym();
	}
	k = i;
	fprintf(out, "<变量定义>\n");
}

void variable_state(var ** llist) {
	int id;
	int i;
	i = k;
	id = getsym();
	while(id == 5 || id == 6) {//int char
		getsym();
		id = getsym();
		if(id == 30) {
			break;
		}
		k = i;//将输出交给定义函数
		variable_define(&(*llist));
		i = k;
		id = getsym();
		if(id != 28) {
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}
		fprintf(out,"%s %s\n",table[id],token);
		i = k;//用于恢复到原来位置
		id = getsym();
	}
	k = i;
	fprintf(out, "<变量说明>\n");
}

void par_list(struct name_state ** qq) {
	int id;
	int i;
	i = k;
	id = getsym();
	var * p;
	if(id == 5 || id == 6) {
        mid_code *wm = (mid_code*)malloc(sizeof (mid_code));
        wm->line = mid_line;
        wm->op = 1;
        strcpy(wm->n1,"para");
		fprintf(out,"%s %s\n",table[id],token);
		strcpy(wm->n2,token);
		fprintf(out,"%s %s\n",table[getsym()],token);
        strcpy(wm->n3,token);
        strcpy(wm->n4,"\0");
        code_list[mid_line++] = wm;
		var *q = (var*)malloc(sizeof(var));
		q->isconst = 0;
		strcpy(q->name,token);
        q->isint = (id == 5)?1:0;
        q->next = NULL;
        (*qq)->attr_list = q;
        p = q;
		i = k;
		id = getsym();
		while(id == 29) {
			fprintf(out,"%s %s\n",table[id],token);
			mid_code *wm = (mid_code*)malloc(sizeof (mid_code));
            wm->line = mid_line;
            wm->op = 1;
			id = getsym();
			fprintf(out,"%s %s\n",table[id],token);
			strcpy(wm->n1,"para");
			strcpy(wm->n2,token);
			fprintf(out,"%s %s\n",table[getsym()],token);
            strcpy(wm->n3,token);
            strcpy(wm->n4,"\0");
            code_list[mid_line++] = wm;
			var *q = (var*)malloc(sizeof(var));
            q->isconst = 0;
            strcpy(q->name,token);
            q->isint = (id == 5)?1:0;
            q->next = NULL;
            p->next = q;
            p = q;
			i = k;
			id = getsym();
		}
		k = i;
	}
	 else {
		k = i;
	}
	var * c;
	c = (var*)malloc(sizeof(var));
    c ->isconst = 0;
    c->isint = 0;
    strcpy(c->name,(*qq)->name);
    c->next = NULL;
    (*qq)->var_list = c;
    var * v = c;
    var *x;
	for(x = (*qq)->attr_list;x != NULL;x=x->next) {
        var * b = (var*)malloc(sizeof(var));
        b ->isconst = x->isconst;
        b->isint = x->isint;
        strcpy(b->name,x->name);
        b->next = NULL;
        v->next = b;
        v =v->next;
	}
	fprintf(out, "<参数表>\n");
}

int expression() {
	int i;
	int id;
	char n1[100]={'\0'};
	char n2[100]={'\0'};
	char op[100] = {'\0'};
	char n3[100] = {'\0'};
	char* name = ren_wmcount();
	i = k;
	id = getsym();
	k = i;
	int style = 0;
	if(id == 17 || id ==18) {
        style = 1;
		fprintf(out,"%s %s\n",table[getsym()],token);
		if(id == 18) {
            op[0] = '-';
		}
	}
	style |= item();
	strcpy(n1,ren_str);
	if(op[0] == '-') {
        midstruct(5,op,"0",n1,name);
        strcpy(n1,name);
	}
	i = k;
	id = getsym();
	if(id != 17 && id != 18) {
		strcpy(ren_str,n1);
	}
	while(id == 17 || id == 18) {
        if(id == 17){
            op[0] = '+';
		}else{
            op[0] = '-';
		}
		fprintf(out,"%s %s\n",table[id],token);
		item();
        strcpy(n2,ren_str);
        midstruct(5,op,n1,n2,name);
        strcpy(n1,name);
        strcpy(ren_str,name);
		i = k;
		id = getsym();
		style = 1;
	}
	k = i;
	fprintf(out, "<表达式>\n");
	return style;
}

int factor() {
	int i;
	int id;
	int style;
	char n1[100];
	char n2[100];
	i = k;
	id = getsym();
	k = i;
	if(id == 17 || id == 18 || id == 1) {
		integer();
		char n[100]={'\0'};
		if(id == 17) {
		}else if(id == 18) {
            n[0] = '-';
		}
		strcat(n,token);
		strcpy(ren_str,n);
		return 1;
	} else if(id == 0) {
		struct name_state *q = namelist;
		int find;
		for(find = 0;q != NULL;q = q->next) {
			if(strcmp(q->name,token) == 0) {
				find = 1;
				function_state();
				if (q->style == 2) {
                    return 1;
				} else {
                    return 0;
				}
			}
		}
		if(find == 0) {
            i = k;
            id = getsym();//标识符
            id = getsym();//预读查看是否是括号
            k = i;
            if(id == 30) {
                fprintf(out2,"%d c\n",line);
                while(id != 31){
                    id = getsym();
                }
            } else {
                id = getsym();
                fprintf(out,"%s %s\n",table[id],token);
                strcpy(n1,token);
                if(1) {
                    var * l;
                    for(l = now->var_list;l != NULL;l=l->next) {
                        if(strcmp(l->name,token) == 0) {
                            break;
                        }
                    }
                    if(l == NULL) {
                        for(l = top_list;l!=NULL;l=l->next) {
                            if(strcmp(l->name,token) == 0){
                                break;
                            }
                        }
                        if(l == NULL){
                            fprintf(out2,"%d c\n",line);
                        }
                    }
                    i = k;
                    id = getsym();
                    k = i;
                    if(id == 32) {
                    	char *name = ren_wmcount();
                    	char n4[100] = {'\0'};
                        fprintf(out,"%s %s\n",table[getsym()],token);
                        style = expression();
                        if(style != 1){
                            fprintf(out2,"%d i\n",line);
                        }
                        i = k;
                        id = getsym();
                        if(id != 33){
                            fprintf(out2,"%d m\n",line);
                            k = i;
                        }
                        fprintf(out,"%s %s\n",table[id],token);
                        midstruct(16,name,"=",n1,ren_str);
                        strcpy(n1,name);
                    }
                    strcpy(ren_str,n1);
                    if(l->isint == 1) {
                        return 1;
                    } else {
                        return 0;
                    }
                }
            }
		}
	} else if(id == 30) {
		fprintf(out,"%s %s\n",table[getsym()],token);
		expression();
		i = k;
        id = getsym();
        if(id != 31){
            fprintf(out2,"%d l\n",line);
            k = i;
        }
		fprintf(out,"%s %s\n",table[id],token);
		return 1;
	} else if(id == 2) {
		fprintf(out,"%s %s\n",table[getsym()],token);
		char n2[100] = {'\0'};
		n2[0]='\'';
		strcat(n2,token);
		strcat(n2,"\'");
		strcpy(ren_str,n2);
		return 0;
	}
	fprintf(out, "<因子>\n");
}

int item() {
	int i;
	int id;
	int style;
	char op[100];
	char n1[100]={'\0'};
	char n2[100]={'\0'};
	char n3[100]={'\0'};
	char n4[100]={'\0'};
	char *name = ren_wmcount();
	style = factor();
	strcpy(n1,ren_str);
	i = k;
	id = getsym();
	if(id != 19 && id != 20){
		strcpy(ren_str,n1);
	}
	while(id == 19 || id == 20) {
        if(id == 19){
        	strcpy(op,"*");
        } else{
            strcpy(op,"/");
        }
		fprintf(out,"%s %s\n",table[id],token);
		factor();
		strcpy(n2,ren_str);
		midstruct(5,op,n1,n2,name);
		strcpy(n1,name);
		strcpy(ren_str,name);
		i = k;
		id = getsym();
		style = 1;
	}
	k = i;
	fprintf(out, "<项>\n");
	return style;
}

void assign_state() {
	int i;
	int id;
	int style;
	char n1[100] = {'\0'};
	char n2[100] = {'\0'};
	char n3[100] = {'\0'};
	char n4[100] = {'\0'};
	fprintf(out,"%s %s\n",table[getsym()],token);
	strcpy(n1,token);
	var *l;
	for(l = now->var_list;l != NULL;l=l->next) {
        if(strcmp(l->name,token)==0) {
            break;
        }
	}
	if(l == NULL) {
        for(l = top_list;l!=NULL;l=l->next){
            if(strcmp(l->name,token) == 0){
                break;
            }
        }
        if(l == NULL){
            fprintf(out2,"%d c\n",line);
        }
	}
	if( l!=NULL && l->isconst == 1) {
        fprintf(out2,"%d j\n",line);
    }
	i = k;
	id = getsym();
	if(id == 27) {
		fprintf(out,"%s %s\n",table[id],token);
		strcpy(n2,"null");
		expression();
		strcpy(n3,"=");
		strcpy(n4,ren_str);
	} else if(id == 32) {
		fprintf(out,"%s %s\n",table[id],token);
		style = expression();
		strcpy(n2,ren_str);
		if(style != 1){
            fprintf(out2,"%d i\n",line);
		}
		fprintf(out,"%s %s\n",table[getsym()],token);// ]
		fprintf(out,"%s %s\n",table[getsym()],token);// =
		expression();
		strcpy(n3,"=");
		strcpy(n4,ren_str);
	}
	midstruct(6,n1,n2,n3,n4);
	fprintf(out, "<赋值语句>\n");
}

void condition() {
	int i;
	int id;
	int style;
	char n1[100] = {'\0'};
	char n2[100] = {'\0'};
	char n3[100] = {'\0'};
	char n4[100] = {'\0'};
	style = expression();
	strcpy(n1,ren_str);
	if(style != 1){
        fprintf(out2,"%d f\n",line);
	}
	i = k;
	id = getsym();
	if(id <= 26 && id >= 21) {
		fprintf(out,"%s %s\n",table[id],token);
		strcpy(n2,token);
		style = expression();
		strcpy(n3,ren_str);
		if(style != 1) {
            fprintf(out2,"%d f\n",line);
		}
	} else {
		k = i;
	}
	midstruct(10,n1,n2,n3,n4);
	fprintf(out, "<条件>\n");
}

void step() {
	unsigned_int();
	fprintf(out, "<步长>\n");
}

void value_list(char * char_list) {
	int i;
	int id;
	char n1[100] = {'\0'};
	char n2[100] = {'\0'};
	char n3[100] = {'\0'};
	char n4[100] = {'\0'};
	char *list[10]={NULL};
	int x = 0;
	strcpy(n1,"push");
	i = k;
	id = getsym();
	k = i;
	struct name_state *l;
	for(l = namelist;l != NULL;l = l->next) {
        if(strcmp(l->name,char_list) == 0) {
            break;
        }
	}
	if(id == 31) {
		fprintf(out, "<值参数表>\n");
		if(l->attr_list != NULL) {
            fprintf(out2,"%d d\n",line);
		}
		return;
	} else {
	    int style;
	    var * c = l->attr_list;
		style = expression();
		list[x] = (char*)malloc(500*sizeof(char));
		strcpy(list[x++],ren_str);//表达式赋值
		if(c->isint != style) {
            fprintf(out2,"%d e\n",line);
		}
		i = k;
		id = getsym();
		c = c->next;
		while(id == 29) {
            if(c == NULL) {
                fprintf(out2,"%d d\n",line);
            }
			fprintf(out,"%s %s\n",table[id],token);
			style = expression();
			list[x] = (char*)malloc(500*sizeof(char));
			strcpy(list[x++],ren_str);//表达式赋值
            if(c->isint != style) {
                fprintf(out2,"%d e\n",line);
            }
			i = k;
			id = getsym();
			if(c!=NULL) {
                c = c->next;
			}
		}
		k = i;
		if(c != NULL){
            fprintf(out2,"%d d\n",line);
        }
	}
	for(i=0;i<x;i++){
		midstruct(7,n1,list[i],n3,n4);
	}
	fprintf(out, "<值参数表>\n");
}

void function_state() {
    int i;
    int id;
	char n1[100] = {'\0'};
	char n2[100] = {'\0'};
	char n3[100] = {'\0'};
	char n4[100] = {'\0'};
	fprintf(out,"%s %s\n",table[getsym()],token);//标识符
	char name[100];
	strcpy(name,token);
	fprintf(out,"%s %s\n",table[getsym()],token);// （
	value_list(name);
	strcpy(n1,"call");
	strcpy(n2,name);
	midstruct(8,n1,n2,n3,n4);
	strcpy(n1,ren_wmcount());
	strcpy(n2,"ret");
	n3[0]='\0';
	n4[0]='\0';
	midstruct(17,n1,n2,n3,n4);
	strcpy(ren_str,n1);
	i = k;
    id = getsym();
    if(id != 31) {
        fprintf(out2,"%d l\n",line);
        k = i;
    }
	fprintf(out,"%s %s\n",table[id],token);// ）
	fprintf(out, "<有返回值函数调用语句>\n");
}

void function_void_state() {
    int i;
    int id;
    char n1[100] = {'\0'};
	char n2[100] = {'\0'};
	char n3[100] = {'\0'};
	char n4[100] = {'\0'};
	fprintf(out,"%s %s\n",table[getsym()],token);//标识符
	char name[100];
	strcpy(name,token);
	fprintf(out,"%s %s\n",table[getsym()],token);// （
	value_list(name);

	strcpy(n1,"call");
	strcpy(n2,name);
	midstruct(8,n1,n2,n3,n4);
	strcpy(ren_str,"RET");
	i = k;
    id = getsym();
    if(id != 31){
        fprintf(out2,"%d l\n",line);
        k = i;
    }
	fprintf(out,"%s %s\n",table[id],token);// ）
	fprintf(out, "<无返回值函数调用语句>\n");
}

void state_read() {
	int i;
	int id;
	char n1[100]={'\0'};
	char n2[100]={'\0'};
	char n3[100]={'\0'};
	char n4[100]={'\0'};
	strcpy(n1,"scanf");
	fprintf(out,"%s %s\n",table[getsym()],token);// scanf
	fprintf(out,"%s %s\n",table[getsym()],token);// (
	fprintf(out,"%s %s\n",table[getsym()],token);// 标识符
	strcpy(n2,token);
	midstruct(14,n1,n2,n3,n4);
	i = k;
	id = getsym();
	while(id == 29) {
		fprintf(out,"%s %s\n",table[id],token);//,
		fprintf(out,"%s %s\n",table[getsym()],token);//标识符
		strcpy(n2,token);
		midstruct(14,n1,n2,n3,n4);
		i = k;
		id = getsym();
	}
	k = i;
	i = k;
        id = getsym();
        if(id != 31){
            fprintf(out2,"%d l\n",line);
            k = i;
        }
	fprintf(out,"%s %s\n",table[id],token);//)
	fprintf(out, "<读语句>\n");
}

void state_write() {
	int i;
	int id;
	char n1[100]={'\0'};
	char n2[100]={'\0'};
	char n3[100]={'\0'};
	char n4[100]={'\0'};
	fprintf(out,"%s %s\n",table[getsym()],token);//printf
	fprintf(out,"%s %s\n",table[getsym()],token);//(
	strcpy(n1,"printf");
	i = k;
	id = getsym();
	k = i;
	if(id == 3) {
		fprintf(out,"%s %s\n",table[getsym()],token);
		fprintf(out,"<字符串>\n");
		int aa;
		int in;
		for(aa=0,in=0;aa<strlen(token);aa++){
			if(token[aa]!='\\'){
				n2[in++] = token[aa];
			}else{
				n2[in++] = '\\';
				n2[in++] = '\\';
			}
		}
		//strcpy(n2,token);
		i = k;
		id = getsym();
		k = i;
		strcpy(n4,numtostr(str_no++));
		if(id == 29) {
			fprintf(out,"%s %s\n",table[getsym()],token);
			expression();
			strcpy(n3,ren_str);
		}
	} else {
		expression();
		strcpy(n3,ren_str);
	}
	i = k;
        id = getsym();
        if(id != 31){
            fprintf(out2,"%d l\n",line);
            k = i;
        }
	fprintf(out,"%s %s\n",table[id],token);// )
	midstruct(13,n1,n2,n3,n4);
	fprintf(out, "<写语句>\n");
}

void state_return() {
    returned = 1;
	int i;
	int id;
	char n1[100] = {'\0'};
	char n2[100] = {'\0'};
	char n3[100] = {'\0'};
	char n4[100] = {'\0'};
	strcpy(n1,"ret");
	fprintf(out,"%s %s\n",table[getsym()],token);//return
	i = k;
	id = getsym();
	k = i;
	int style;
	if(id == 30) {
        if(now->style == 0) {
            fprintf(out2,"%d g\n",line);
        }
		fprintf(out,"%s %s\n",table[getsym()],token);// (
		style = expression();
		strcpy(n2,ren_str);
        if(now->style!=0 && now->style!=style+1 ) {
            fprintf(out2,"%d h\n",line);
        }
        i = k;
        id = getsym();
        if(id != 31){
            fprintf(out2,"%d l\n",line);
            k = i;
        }
		fprintf(out,"%s %s\n",table[id],token);// )
	}
	midstruct(9,n1,n2,n3,n4);
	fprintf(out, "<返回语句>\n");
}

struct name_state* head_state() {
    int style;
    int id;
    id = getsym();
	fprintf(out,"%s %s\n",table[id],token);//int char
	style = (id == 5) ? 2 : 1;
	mid_code *wm = (mid_code*)malloc(sizeof (mid_code));
	wm->line = mid_line;
	wm->op = 0;
	strcpy(wm->n1,token);
	fprintf(out,"%s %s\n",table[getsym()],token);//标识符
	strcpy(wm->n2,token);
	strcpy(wm->n3,"\0");
	strcpy(wm->n4,"\0");
	wm->line = mid_line;
	code_list[mid_line++] = wm;
	if(namelist == NULL) {
		namelist = (struct name_state*)malloc(sizeof(struct name_state));
		strcpy(namelist->name,token);
		namelist->style = style;
		namelist->next = NULL;
		namelist->attr_list = NULL;
		namelist->var_list = NULL;
		p = namelist;
		return namelist;
	} else {
		struct name_state *q;
		q = (struct name_state*)malloc(sizeof(struct name_state));
		q->attr_list = NULL;
		q->var_list = NULL;
		strcpy(q->name,token);
		q->style = style;
		q->next = NULL;
		p->next = q;
		p = p->next;
		return q;
	}
	fprintf(out, "<声明头部>\n");
}

void function_void() {
    int i;
    int id;
    returned = 0;
    mid_code *wm = (mid_code*)malloc(sizeof (mid_code));
	wm->line = mid_line;
	wm->op = 0;
	fprintf(out,"%s %s\n",table[getsym()],token);//void
	strcpy(wm->n1,token);
	fprintf(out,"%s %s\n",table[getsym()],token);//标识符
	strcpy(wm->n2,token);
	strcpy(wm->n3,"\0");
	strcpy(wm->n4,"\0");
	code_list[mid_line++] = wm;
	struct name_state * qq;
	if(namelist == NULL) {
		namelist = (struct name_state*)malloc(sizeof(struct name_state));
		strcpy(namelist->name,token);
		namelist->style = 0;
		namelist->next = NULL;
		namelist->attr_list = NULL;
		namelist->var_list = NULL;
		p = namelist;
		qq = namelist;
	} else {
		struct name_state *q;
		q = (struct name_state*)malloc(sizeof(struct name_state));
		qq = q;
		strcpy(q->name,token);
		q->style = 0;
		q->next = NULL;
		q->attr_list = NULL;
		q->var_list = NULL;
		p->next = q;
		p = p->next;
	}
	fprintf(out,"%s %s\n",table[getsym()],token);//  (
	par_list(&qq);
	now = qq;
	i = k;
    id = getsym();
    if(id != 31){
        fprintf(out2,"%d l\n",line);
        k = i;
    }
	fprintf(out,"%s %s\n",table[id],token);//  )
	fprintf(out,"%s %s\n",table[getsym()],token);// {
	combine_state(&(qq->var_list));
	fprintf(out,"%s %s\n",table[getsym()],token);// }
	midstruct(9,"ret","\0","\0","\0");
	fprintf(out, "<无返回值函数定义>\n");
}

void function_main() {
	char n1[100] = "void";
	char n2[100] = "main";
	char n3[100] = {'\0'};
	char n4[100] = {'\0'};
	midstruct(0,n1,n2,n3,n4);
	fprintf(out,"%s %s\n",table[getsym()],token);//void
	fprintf(out,"%s %s\n",table[getsym()],token);//main
	fprintf(out,"%s %s\n",table[getsym()],token);//(
	fprintf(out,"%s %s\n",table[getsym()],token);//)
	fprintf(out,"%s %s\n",table[getsym()],token);//{
    var *c = (var *)malloc(sizeof(var));
    strcpy(c->name,"main");
    c->isint = 0;
    c->isconst = 0;
    c->next = NULL;
    main_list = c;
	struct name_state * p = (struct name_state*)malloc(sizeof(struct name_state));
	p->var_list = main_list;
	p->attr_list = NULL;
	strcpy(p->name,"main");
	p->style = 0;
	p->next = namelist;
	namelist = p;
    now = p;
	combine_state(&main_list);
	fprintf(out,"%s %s\n",table[getsym()],token);//}
	fprintf(out, "<主函数>\n");
}

void condition_state() {
	int i;
	int id;
	char * name = ren_wmcount(mid_line);
	fprintf(out,"%s %s\n",table[getsym()],token);//  if
	char n1[100]={'\0'};
	char n2[100]={'\0'};
	char n3[100]={'\0'};
	char n4[100]={'\0'};
	strcpy(n1,"if_state_");
	strcat(n1,numtostr(mid_if));
	midstruct(-1,n1,n2,n3,n4);//表达是if
	//midstruct();
	fprintf(out,"%s %s\n",table[getsym()],token);// (
	condition();
	strcpy(n1,"bz");
	char ss[100];
	char ss2[100];
	strcpy(ss2,"else_");
	strcat(ss2,numtostr(mid_if++));
	strcpy(n2,ss2);
	strcpy(ss,"end_if_");
	strcat(ss,numtostr(mid_if++));
	midstruct(11,n1,n2,n3,n4);
	i = k;
    id = getsym();
    if(id != 31){
        fprintf(out2,"%d l\n",line);
        k = i;
    }
	fprintf(out,"%s %s\n",table[id],token);// )
	state();
	strcpy(n1,"goto");
	strcpy(n2,ss);
	midstruct(11,n1,n2,n3,n4);//goto
	n2[0] = '\0';
	strcat(ss2,":");
	midstruct(12,ss2,n2,n3,n4);
	i = k;
	id = getsym();
	if(id == 10) {
		fprintf(out,"%s %s\n",table[id],token);
		state();
	} else {
		k = i;
	}
	strcat(ss,":");
	strcpy(n1,ss);
	n2[0]='\0';
	midstruct(12,n1,n2,n3,n4);
	fprintf(out, "<条件语句>\n");
}

void loop_state() {
	int i;
	int id;
	id = getsym();
	if(id == 12) {
		char n1[100]={'\0'};
		char n2[100]={'\0'};
		char n3[100]={'\0'};
		char n4[100]={'\0'};
		char name[100]={'\0'};
		char label[100]={'\0'};
		strcpy(n1,"while_state");
		strcat(n1,numtostr(mid_while));
		strcpy(label,n1);
		strcat(n1,":");
		midstruct(12,n1,n2,n3,"1");//代表while语句
		fprintf(out,"%s %s\n",table[id],token);
		fprintf(out,"%s %s\n",table[getsym()],token);// (
		condition();
		strcpy(n1,"bz");
		strcpy(n2,"end_while_");
		strcat(n2,numtostr(mid_while++));//mid号码保持一致
		strcpy(name,n2);
		strcat(name,":");
		midstruct(11,n1,n2,n3,n4);//跳转语句
		i = k;
        id = getsym();
        if(id != 31){
            fprintf(out2,"%d l\n",line);
            k = i;
        }
		fprintf(out,"%s %s\n",table[id],token);// )
		state();
		strcpy(n1,"goto");
		strcpy(n2,label);
		midstruct(11,n1,n2,n3,n4);
		strcpy(n1,name);
		n2[0]='\0';
		midstruct(12,n1,n2,n3,"0");//end_while
	} else if(id == 11) {
		char n1[100]={'\0'};
		char n2[100]={'\0'};
		char n3[100]={'\0'};
		char n4[100]={'\0'};
		char name[100]={'\0'};
		fprintf(out,"%s %s\n",table[id],token);//do
		strcpy(n1,"do_");
		strcat(n1,numtostr(mid_do++));//do_while 语句
		strcpy(name,n1);
		strcat(n1,":");
		midstruct(12,n1,n2,n3,"1");
		state();
		i = k;
		id = getsym();
        if(id != 12){
            fprintf(out2,"%d n\n",line);
            k = i;
        }
		fprintf(out,"%s %s\n",table[id],token);//while
		fprintf(out,"%s %s\n",table[getsym()],token);// (
		condition();
		strcpy(n1,"bnz");
		strcpy(n2,name);
		midstruct(11,n1,n2,n3,n4);//跳转到头
		char end[10]={'\0'};
		strcpy(end,"end_");
		strcat(end,name);
		strcat(end,":");
		midstruct(12,end,n3,n3,"0");
		i = k;
        id = getsym();
        if(id != 31){
            fprintf(out2,"%d l\n",line);
            k = i;
        }
		fprintf(out,"%s %s\n",table[id],token);// )
	} else if(id == 13) {
		char n1[100]={'\0'};
		char n2[100]={'\0'};
		char n3[100]={'\0'};
		char n4[100]={'\0'};
		char name[100]={'\0'};
		char label[100]={'\0'};
		strcpy(n1,"for_state");
		midstruct(-1,n1,n2,n3,n4);
		fprintf(out,"%s %s\n",table[id],token);
		fprintf(out,"%s %s\n",table[getsym()],token);// (
		fprintf(out,"%s %s\n",table[getsym()],token);//标识符
		strcpy(n1,token);
		strcpy(n2,"null");
		var * l;
		for(l = now->var_list;l != NULL;l=l->next) {
            if(strcmp(l->name,token) == 0) {
                break;
            }
		}
		if(l == NULL) {
                fprintf(out2,"%d c\n",line);
        }
		fprintf(out,"%s %s\n",table[getsym()],token);// =
		expression();
		strcpy(n4,ren_str);
		strcpy(n3,"=");
		midstruct(6,n1,n2,n3,n4);
		i = k;
		id = getsym();
		if(id != 28){
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}
		fprintf(out,"%s %s\n",table[id],token);//;
		strcpy(n1,"for_label_");
		strcat(n1,numtostr(mid_for));
		strcpy(label,n1);
		n2[0]='\0';
		n3[0]='\0';
		n4[0]='\0';
		strcat(n1,":");
		midstruct(12,n1,n2,n3,"1");
		condition();
		strcpy(n1,"bz");
		strcpy(n2,"end_for_");
		n3[0]='\0';
		n4[0]='\0';
		strcat(n2,numtostr(mid_for++));
		strcpy(name,n2);
		midstruct(11,n1,n2,n3,n4);//跳转语句
		i = k;
		id = getsym();
		if(id != 28) {
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}
		fprintf(out,"%s %s\n",table[id],token);//;
		fprintf(out,"%s %s\n",table[getsym()],token);//标识符
		char nn1[100] = {'\0'};
		strcpy(nn1,token);
		for( l = now->var_list;l != NULL;l=l->next) {
            if(strcmp(l->name,token) == 0) {
                break;
            }
		}
		if(l == NULL) {
                fprintf(out2,"%d c\n",line);
        }
		fprintf(out,"%s %s\n",table[getsym()],token);// =
		fprintf(out,"%s %s\n",table[getsym()],token);//标识符
		for(l = now->var_list;l != NULL;l=l->next) {
            if(strcmp(l->name,token) == 0) {
                break;
            }
		}
		if(l == NULL) {
                fprintf(out2,"%d c\n",line);
        }
		fprintf(out,"%s %s\n",table[getsym()],token);// +/-
		char nn2[100]={'\0'};
		strcpy(nn2,token);
		step();
		char nn3[100]={'\0'};
		strcpy(nn3,token);
		i = k;
        id = getsym();
        if(id != 31){
            fprintf(out2,"%d l\n",line);
            k = i;
        }
		fprintf(out,"%s %s\n",table[id],token);// )
		state();
		midstruct(15,nn1,"=",nn2,nn3);
		strcpy(n1,"goto");
		strcpy(n2,label);
		midstruct(11,n1,n2,n3,n4); //goto到顶端
		strcpy(n1,name);
		strcat(n1,":");
		n2[0]='\0';
		n3[0]='\0';
		n4[0]='\0';
		midstruct(12,n1,n2,n3,"0"); //end标签
	} else {
		printf("loop_state: not for/do/while\n");
		return;
	}
	fprintf(out, "<循环语句>\n");
}

void state_list() {
	int i;
	int id;
	i = k;
	id = getsym();
	while(id == 9 ||(id <= 16 && id >= 11) || id == 0 || id == 28 || id == 34) {
		//printf("%d %s\n",id,token);
		k = i;
		state();
		i = k;
		id = getsym();
	}
	k = i;
	fprintf(out, "<语句列>\n");
	//printf("4444!!!4\n");
}

void state() {
	int i;
	int id;
	i = k;
	id = getsym();
	k = i;
	//printf("%d %s\n",id,token);
	if(id == 9) {
		condition_state();
	} else if(id <= 13 && id >= 11) {
		loop_state();
	} else if(id == 34) {
	    fprintf(out,"%s %s\n",table[getsym()],token);
		state_list();
		fprintf(out,"%s %s\n",table[getsym()],token);
	} else if(id == 0) {
		struct name_state *q = namelist;
		int find;
		for(find = 0;q != NULL;q = q->next) {
			if(strcmp(q->name,token) == 0) {
				find = 1;
				if(q->style == 0) {
					function_void_state();
				} else {
					function_state();
				}
				break;
			}
		}
		if(find == 0) {
            i=k;
            id = getsym();
            k=i;
            if(id == 30) {
                fprintf(out2,"%d %c\n",line,error[2]);
                i = k;
                while(id != 31) {
                    id = getsym();
                }
            } else {
                assign_state();
            }
		}
		i = k;
		id = getsym();
		fprintf(out,"%s %s\n",table[id],token); //;
		if(id != 28){
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}
	} else if(id == 14) {
		state_read();
		i = k;
		id = getsym();
		if(id != 28){
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}
		fprintf(out,"%s %s\n",table[id],token);//;
	} else if(id == 15) {
		state_write();
		i = k;
		id = getsym();
		if(id != 28){
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}
		fprintf(out,"%s %s\n",table[id],token);//;
	} else if(id == 16) {
		state_return();
		i = k;
		id = getsym();
		if(id != 28){
            fprintf(out2,"%d k\n",line-1);
            k = i;
		}
		fprintf(out,"%s %s\n",table[id],token);//;
	}else if(id == 28) {
		fprintf(out,"%s %s\n",table[getsym()],token);
	} else {  //空
		fprintf(out, "<语句>\n");
		return ;
	}
	fprintf(out, "<语句>\n");
}

void combine_state(var ** llist) {
	int id;
	int i;
	i = k;
	id = getsym();
	k = i;
	if(id == 4) {
		const_state(&(*llist));
	}
	i = k;
	id = getsym();
	k = i;
	//printf("%s\n",token);
	if(id == 5 || id == 6) {
		variable_state(&(*llist));
	}
	state_list();
	fprintf(out, "<复合语句>\n");
	//printf("4444");
}

void function() {
    int i;
    int id;
    struct name_state *qq;
    returned = 0;
	qq = head_state();

	fprintf(out,"%s %s\n",table[getsym()],token);// (
	par_list(&qq);
	now = qq;
	i = k;
    id = getsym();
    if(id != 31){
        fprintf(out2,"%d l\n",line);
        k = i;
    }
	fprintf(out,"%s %s\n",table[id],token);// )
	fprintf(out,"%s %s\n",table[getsym()],token);// {

	combine_state(&(qq->var_list));
	fprintf(out,"%s %s\n",table[getsym()],token);// }
	if(returned == 0){
        fprintf(out2,"%d h\n",line);
	}
	fprintf(out, "<有返回值函数定义>\n");
}

void project() {
	int i;
	i = k;
	int id;
	id = getsym();
	k = i;
	if (id == 4) {
		//printf("enter const_state\n");
		const_state(&top_list);
	}//常量定义
	i = k;
	id = getsym();
	k = i;
	if(id == 5 || id == 6) {
		i = k;
		getsym();
		getsym();
		id = getsym();
		k = i;
		if(id != 30) {
			variable_state(&top_list);
		}
	}//变量定义
    var * cc = top_list;
	i = k;
	id = getsym();
	while(id == 5 || id == 6 || id == 7) {
		k = i;
		if(id == 7) {
			i = k;
			getsym();
			id = getsym();
			k = i;
			if(id != 8) {
				function_void();
			} else {
				break;
			}
		} else {
			function();
		}
		i = k;
		id = getsym();
	}
	k = i;
	function_main();
	fprintf(out, "<程序>");
}

typedef struct{
	char name[100];
	int type;
	int space;
	int no;
	int is_const;
}Array;
Array *array_list[100]={NULL};
int array_num=0;
typedef struct {
	char name[100];
	int type;// 0->char 1->int
	char value[100];
}const_;
int const_num = 0;
const_ *const_list[100]={NULL};
//------------------------mips_gen-------------------------
int register_pool[22] = {0};// a t s  1为可用，0为不可用
void data_gen() {
	fprintf(out_mips,".data\n");
	fprintf(out_mips,"enter:.asciiz \"\\n\"\n");
	mid_code *a;
	int index;
	int switch_0 = 0;
	for(index = 0;index < mid_line;index++) {
		a = code_list[index];
		if(a->op == 13) {
			if(a->n2[0] != '\0') {
				char name[100];
				strcpy(name,"str_");
				strcat(name,a->n4);
				fprintf(out_mips,"%s : .asciiz \"%s\"\n",name,a->n2);
			}
		}
		if(a->op == 3) {
			if(a->n4[0]!='\0' && switch_0 == 0) {
				Array *p = (Array*)malloc(sizeof(Array));
				strcpy(p->name,a->n3);
				p->space = strtonum(a->n4);
				p->no = array_num;
				if(strcmp(a->n2,"int")==0){
					p->type = 1;
				}else {
					p->type = 0;
				}
				p->is_const = 0;
				array_list[array_num++] = p;
			} else if(switch_0 == 0){
				Array *p = (Array*)malloc(sizeof(Array));
				strcpy(p->name,a->n3);
				p->space = 1;
				p->no = array_num;
				if(strcmp(a->n2,"int")==0){
					p->type = 1;
				}else{
					p->type = 0;
				}
				p->is_const = 0;
				array_list[array_num++] = p;
			}
			//fprintf(out_mips,"%s : .word 0 : %d\n",p->name,p->space);
		}
		if(a->op == 0) {
			switch_0 = 1;
		}
		if(a->op == 4 && switch_0 == 0) {
			if(a->op == 4){
				Array *p = (Array*)malloc(sizeof(Array));
				strcpy(p->name,a->n3);
				p->no = array_num;
				if(strcmp(a->n2,"int")==0){
					p->type = 1;
					p->space = strtonum(a->n4);
				}else{
					p->type = 0;
					p->space = a->n4[1];
				}
				p->is_const = 1;
				array_list[array_num++] = p;
			}
		}
	}
	Array * p;
	for(index = 0;index<array_num;index++) {
		p = array_list[index];
		if(p->is_const == 1) {
			fprintf(out_mips,"%s : .word %d\n",p->name,p->space);
		} else if(p->space != 0) {
			fprintf(out_mips,"%s : .word 0 : %d\n",p->name,p->space);
		} else {
			fprintf(out_mips,"%s : .word 0\n",p->name);
		}
	}
}
char * space_list[10000] = {NULL};//存储函数块栈空间的所有变量
int space_type[10000];
char *a1_a3[3]={NULL};
int a1_a3_type[3];
int find_space(int shift_sum,char *name) {
	int i;
	for(i = 2;i < shift_sum;i++) {
		if(space_list[i]==NULL){
			continue;
		}
		if(strcmp(name,space_list[i]) == 0) {
			return i*4;
		}
	}
	return -1;//表示没找到
}
int find_attr(char *name) {
	int i;
	for(i = 0;i < 3;i++) {
		if(a1_a3[i]==NULL){
			return -1;
		}
		if(strcmp(name,a1_a3[i]) == 0){
			return i+1;
		}
	}
	return -1;
}//找值参数表
char *loc_reg[5]={NULL};//t3 t4 t5 t6 t7
int loc_reg_type[5];
int loc_reg_use[5] = {0,0,0,0,0};

char *global_reg[8] = {NULL};//s0->s8
int global_reg_type[8];
int global_reg_use[8]={0,0,0,0,0,0,0,0};
int global_can = 0;//标识是否可以用

char *lw_global_reg[8]={NULL};
int lw_global_reg_type[8];

int find_loc_reg(char *name){
	int i;
	for(i=0;i<5;i++){
		if(loc_reg[i]==NULL){
			continue;
		}
		if(strcmp(name,loc_reg[i])==0){
			return i+3;
		}
	}
	return -1;
}
int find_global_reg(char *name){
	int i;
	for(i = 0;i < 8 && global_can > 0;i++){
		if(global_reg[i]==NULL){
			continue;
		}
		if(strcmp(global_reg[i],name)==0){
			return i;
		}
	}
	return -1;
}
void sw_global_reg(int shift) {
	int i,x;
	for(i = 0;i < 8;i++) {
		if(global_reg[i]==NULL){
			global_reg_use[i]=0;
			continue;
		}
		x = find_space(shift,global_reg[i]);
		if(x == -1) {
			continue;
		}else{
            fprintf(out_mips,"sw $s%d,-%d($sp)\n",i,x);
		}
		lw_global_reg[i] = global_reg[i];
		lw_global_reg_type[i] = global_reg_type[i];
		global_reg[i]=NULL;
		global_reg_use[i] = 0;
	}
}
void lw_global(int shift) {
	int i,x;
	for(i = 0;i < 8;i++){
		if(lw_global_reg[i]==NULL){
			continue;
		}
		x = find_space(shift,lw_global_reg[i]);
		if(x == -1){
			continue;
		}
		fprintf(out_mips,"lw $s%d,-%d($sp)\n",i,x);
		global_reg[i] = lw_global_reg[i];
		global_reg_type[i] = lw_global_reg_type[i];
	}
}
int done_code = 0;//表示已经分析到多少句了
int can(int shift,char *name){
	int x,i;
	x = find_space(shift,name);
	i = find_attr(name);
	if(x!= -1 && (name[0]!='w'|| name[0]!='m') && i == -1){
		if(x/4 + 1 < shift){
			if(space_list[x/4+1]==NULL){
				return 1;
			}
			if(strcmp(name,space_list[x/4+1])==0){
				return 0;
			}
		}
		return 1;
	}
	return 0;
}
int find_name_list(char *name,char *name_list[],int number){
	int i;
	for(i = 0;i <= number;i++){
		if(name_list[i]==NULL){
			continue;
		}
		if(strcmp(name_list[i],name)==0){
			return i;
		}
	}
	return -1;
}
void alloc_global(int shift,int index){
	int i,j,num=0;
	int number=-1;
	char *name_list[500]={NULL};
	int name_time[500]={0};
	int visit[500] = {0};
	mid_code *a;
	int ret;
	if(index < done_code){
		return;
	}
	for(i = index;i<mid_line;i++) {
		a = code_list[i];
		if(a->op == 12){
			if(a->n4[0]=='1' && i > done_code) {
				num++;
				while(num != 0) {
					i++;
					a = code_list[i];
					if(a->op == 12){
						if(a->n4[0]=='1'){
							num++;
						}
						if(a->n4[0]=='0'){
							num--;
						}
					}
					if(a->op == 5) {
						if(can(shift,a->n2)){
							ret=find_name_list(a->n2,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n2;
								name_time[number] +=(num-1)*30+1;
							}
						}
						if(can(shift,a->n3)){
							ret=find_name_list(a->n3,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n3;
								name_time[number] +=(num-1)*30+1;
							}
						}
						if(ret= can(shift,a->n4)){
							ret=find_name_list(a->n4,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n4;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 6) {
						if(can(shift,a->n2)){
							ret=find_name_list(a->n2,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n2;
								name_time[number] +=(num-1)*30+1;
							}
						}
						if(can(shift,a->n4)){
							ret=find_name_list(a->n4,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n4;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 7) {
						if(can(shift,a->n2)){
							ret=find_name_list(a->n2,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n2;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 9) {
						if(can(shift,a->n2) ){
							ret=find_name_list(a->n2,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n2;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 10){
						if(can(shift,a->n1)){
							ret=find_name_list(a->n1,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n1;
								name_time[number] +=(num-1)*30+1;
							}
						}
						if(can(shift,a->n3)){
							ret=find_name_list(a->n3,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n3;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 13) {
						if(can(shift,a->n3)){
							ret=find_name_list(a->n3,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n3;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 14){
						if(can(shift,a->n2)){
							ret=find_name_list(a->n2,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n2;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 15){
						if(can(shift,a->n1)){
							ret=find_name_list(a->n1,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n1;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
					if(a->op == 16){
						if(can(shift,a->n4)){
							ret=find_name_list(a->n4,name_list,number);
							if(ret != -1){
								name_time[ret]+=(num-1)*30+1;
							} else {
								number++;
								name_list[number] = a->n4;
								name_time[number] +=(num-1)*30+1;
							}
						}
					}
				}
				done_code = i;
				break;
			}
		}
	}
	for(i = 0;i < 8 && i <= number;i++) {
		int max = 0,max_index;
		for(j = 0;j <= number;j++) {
			if(visit[j]==1){
				continue;
			}
			if(name_time[j]>max){
				max = name_time[j];
				max_index = j;
			}
		}
		global_reg_use[i] = 1;
		global_reg[i] = name_list[max_index];
		visit[max_index] = 1;
		int x,j;
		x = find_space(shift,global_reg[i]);
		global_reg_type[i] = space_type[x/4];
        fprintf(out_mips,"lw $s%d,-%d($sp)",i,x);
		fprintf(out_mips,"#%s\n",global_reg[i]);
	}
}
int alloc_global_reg(int shift,char *name){
	int i,x;
	return -1;//方法作废，算法有问题
	x = find_space(shift,name);
	if(x!= -1 && (name[0]!='w'|| name[0]!='m') && global_can){
		for(i=0;i < 8;i++){
			if(global_reg_use[i]==0){
				global_reg_use[i] = 1;
				global_reg[i] = name;
				global_reg_type[i] = space_type[x/4];
				fprintf(out_mips,"lw $s%d,-%d($sp)\n",i,x);
				return i;
			}
		}
	}
	return -1;
}
void calculate(char* op,char * a1,char *a2,char * ret,int a1_isconst,int a2_isconst) {
	// a1不能是立即数
	if(strcmp(op,"+")==0) {
        if(a1_isconst == 1 && a2_isconst == 1){
            fprintf(out_mips,"li $t0,%s\n",a1);
            fprintf(out_mips,"addi %s,$t0,%s\n",ret,a2);
        }else if(a1_isconst == 0 && a2_isconst == 1){
            fprintf(out_mips,"addi %s,%s,%s\n",ret,a1,a2);
        }else if(a1_isconst == 1 && a2_isconst == 0){
            fprintf(out_mips,"addi %s,%s,%s\n",ret,a2,a1);
        }else{
            fprintf(out_mips,"add %s,%s,%s\n",ret,a1,a2);
        }
	}else if(strcmp(op,"-")==0){
        if(a1_isconst == 1 && a2_isconst == 1){
            fprintf(out_mips,"li $t0,%s\n",a1);
            fprintf(out_mips,"subi %s,$t0,%s\n",ret,a2);
        }else if(a1_isconst == 0 && a2_isconst == 1){
            fprintf(out_mips,"subi %s,%s,%s\n",ret,a1,a2);
        }else if(a1_isconst == 1 && a2_isconst == 0){
            fprintf(out_mips,"li $t0,%s\n",a1);
            fprintf(out_mips,"sub %s,$t0,%s\n",ret,a2);
        }else{
            fprintf(out_mips,"sub %s,%s,%s\n",ret,a1,a2);
        }
	}else if(strcmp(op,"*")==0){
        char reg1[50] ={'\0'};
        char reg2[50] = {'\0'};
        strcpy(reg1,a1);
        strcpy(reg2,a2);
        if(a1_isconst == 1){
            fprintf(out_mips,"li $t0,%s\n",a1);
            strcpy(reg1,"$t0");
        }
        if(a2_isconst == 1){
            fprintf(out_mips,"li $t1,%s\n",a2);
            strcpy(reg2,"$t1");
        }
        fprintf(out_mips,"mul %s,%s,%s\n",ret,reg1,reg2);
	}else {
	    char reg1[50] ={'\0'};
        char reg2[50] = {'\0'};
        strcpy(reg1,a1);
        strcpy(reg2,a2);
        if(a1_isconst == 1){
            fprintf(out_mips,"li $t0,%s\n",a1);
            strcpy(reg1,"$t0");
        }
        if(a2_isconst == 1){
            fprintf(out_mips,"li $t1,%s\n",a2);
            strcpy(reg2,"$t1");
        }
		fprintf(out_mips,"div %s,%s\n",reg1,reg2);
		fprintf(out_mips,"mflo %s\n",ret);
	}
}

typedef struct {
	char name[100];
	int sp;
	int shift;
	char *space_list[100];
	int space_type[100];
	struct block *next;
	struct block *front;
}block;

typedef struct{
	char name[50];
	int type;
}func_ret;
func_ret *ret_list[100];
int ret_num=0;
block *block_head = NULL,*block_now = NULL;

void text_gen() {
	fprintf(out_mips,".text\n");
	fprintf(out_mips,"j main\n");
	int index;
	int sp = 0x7fffeff0;
	mid_code *a;
	int ren_type;
	char *func_list[50]={NULL};
	int func_for[50]={0};
	int for_num = 0;
	for(index = 0;index < mid_line;index++) {
		a = code_list[index];
		if(a->op == 0) {
			func_list[for_num++] = a->n2;
			int ww;
			for(ww = 0;ww < 3;ww++){
				a1_a3[ww] = NULL;
			}
			func_ret *pp =(func_ret*)malloc(sizeof(func_ret));
			strcpy(pp->name,a->n2);
			if(strcmp(a->n1,"int")==0) {
				pp->type = 1;
			} else{
				pp->type = 0;
			}
			ret_list[ret_num++] = pp;

			char fun_name[100]={'\0'};
			fprintf(out_mips,"%s:\n",a->n2);
			strcpy(fun_name,a->n2);
			index++;
			if(index >= mid_line){
				break;
			}
			a = code_list[index];
			//取出函数记录信息,这个表里面没有main函数
			//a0-a3用于函数参数传递，参数表首先找这几个，如果参数还要多就放在内存里面
			int shift = 2;
			int attr_num = 0;
			while(a->op == 1) {
				attr_num++;
				if(attr_num <= 3) {
					a1_a3[attr_num-1] = a->n3;
					if(strcmp(a->n2,"int")==0) {
						a1_a3_type[attr_num-1] = 1;
					} else {
						a1_a3_type[attr_num-1] = 0;
					}
				//	fprintf();//push x,目前都放在内存里
				}
				shift++;
				space_list[shift-1]=a->n3;//第一个空间给ra,第一次shift为3
				if(strcmp(a->n2,"int")==0){
					space_type[shift-1] = 1;
				}else {
					space_type[shift-1] = 0;
				}
				index++;
				if(index == mid_line){
					return ;
				}
				a = code_list[index];
			}
			while(a->op == 4) {
				shift++;
				space_list[shift-1]=a->n3;
				if(strcmp(a->n2,"int")==0){
					space_type[shift-1] = 1;
				}else {
					space_type[shift-1] = 0;
				}
				fprintf(out_mips,"li $t0,%s\n",a->n4);
				fprintf(out_mips,"sw $t0,-%d($sp)\n",4*(shift - 1));
				index++;
				if(index == mid_line){
					return ;
				}
				a = code_list[index];
			}
			while(a->op == 3) {
				if(a->n4[0] == '\0') {
					shift++;
					space_list[shift-1]=a->n3;
					if(strcmp(a->n2,"int")==0){
						space_type[shift-1] = 1;
					}else {
						space_type[shift-1] = 0;
					}
				} else {
					int x;
					for(x=0;x<strtonum(a->n4);x++,shift++) {
						space_list[shift]=a->n3;
						if(strcmp(a->n2,"int")==0){
							space_type[shift] = 1;
						}else {
							space_type[shift] = 0;
						}
					}
				}
				index++;
				if(index == mid_line){
					return ;
				}
				a = code_list[index];
			}//变量记录
			//fprintf(out_mips,"addiu $sp,$sp,-%d\n",shift);
			//sp = sp - 4*shift;//栈指针移动
			int v0 = 1;//代表v0寄存器为空 new: 新的方法只读v1l
			int attr = 0;
			while(a->op != 0) {
				while(a->op == 5) { //calculate
					char reg1[50],reg2[50];
					strcpy(reg1,"$t0");
					strcpy(reg2,"$t1");
					int n2_isconst = 0;
					if(!isalpha(a->n2[0]) && a->n2[0]!='_') {
						//fprintf(out_mips,"li $t0,%s\n",a->n2);
                        strcpy(reg1,a->n2);
                        n2_isconst = 1;
					} else {
							int i,find = 0;
							i = find_global_reg(a->n2);
							if(i != -1) {
								find = 1;
								char reg[5]="$s";
								strcat(reg,numtostr(i));
								strcpy(reg1,reg);
							} else {
								i = alloc_global_reg(shift,a->n2);
								if(i != -1){
									find = 1;
									char reg[5] = "$s";
									strcat(reg,numtostr(i));
									strcpy(reg1,reg);
								}
							}
							i = find_loc_reg(a->n2);
							if(i != -1 && find == 0) {
								find = 1;
								char reg[5]="$t";
								strcat(reg,numtostr(i));
								strcpy(reg1,reg);
								if(strcmp(a->n2,a->n4)!=0){
									loc_reg_use[i-3]=0;
									loc_reg[i-3]=NULL;
								}
							}
							if(find == 0){
								i = find_attr(a->n2);
								if(i != -1){
									char reg[5]="$a";
									strcat(reg,numtostr(i));
									strcpy(reg1,reg);
								} else {
									i = find_space(shift,a->n2);
									if(i != -1){
										fprintf(out_mips,"lw $t0,-%d($sp)\n",i);
									} else {
										if(strcmp(a->n2,"RETS")!=0){
											fprintf(out_mips,"lw $t0,%s\n",a->n2);//全局变量
										}else{
											if(v0 == 1) {
												fprintf(out_mips,"move $t0,$v0\n");
											}else {
												fprintf(out_mips,"move $t0,$v1\n");
											}
										}

									}
								}
							}
					}
					int n3_isconst = 0;
					if(!isalpha(a->n3[0]) && a->n3[0]!='_') {
						//fprintf(out_mips,"li $t1,%s#1111\n",a->n3);
                        strcpy(reg2,a->n3);
                        n3_isconst = 1;
					} else {
							int i,find = 0;
							 //栈里面找
							i = find_global_reg(a->n3);
							if(i != -1){
								find = 1;
								char reg[5]="$s";
								strcat(reg,numtostr(i));
								strcpy(reg2,reg);
							} else {
								i = alloc_global_reg(shift,a->n3);
								if(i != -1){
									find = 1;
									char reg[5] = "$s";
									strcat(reg,numtostr(i));
									strcpy(reg2,reg);
								}
							}
							i = find_loc_reg(a->n3);

							if(i != -1 && find == 0) {
								find = 1;
								char reg[5]="$t";
								strcat(reg,numtostr(i));
								strcpy(reg2,reg);
								if(strcmp(a->n3,a->n4)!=0){
									loc_reg_use[i-3]=0;
									loc_reg[i-3]=NULL;
								}
							}
							if(find == 0){
								int is_int;
								i = find_attr(a->n3);

								if(i != -1){
									char reg[5]="$a";
									strcat(reg,numtostr(i));
									strcpy(reg2,reg);
								} else {
									i = find_space(shift,a->n3);

									if(i != -1) {
										fprintf(out_mips,"lw $t1,-%d($sp)\n",i);
									} else {
										if(strcmp(a->n3,"RETS")!=0){
											fprintf(out_mips,"lw $t1,%s\n",a->n3);//全局变量
										}else {
											if(v0 == 1) {
												fprintf(out_mips,"move $t1,$v0\n");
												v0 = 0;
											}else {
												fprintf(out_mips,"move $t1,$v1\n");
											}
										}
									}
								}
							}

					}
					int i,find =0;
					//改成在局部寄存器里找
					i = find_global_reg(a->n4);
							if(i != -1){
								find = 1;
								char reg[5]="$s";
								strcat(reg,numtostr(i));
								calculate(a->n1,reg1,reg2,reg,n2_isconst,n3_isconst);
							} else {
								i = alloc_global_reg(shift,a->n3);
								if(i != -1){
									find = 1;
									char reg[5] = "$s";
									strcat(reg,numtostr(i));
									calculate(a->n1,reg1,reg2,reg,n2_isconst,n3_isconst);
								}
							}
					i = find_loc_reg(a->n4);
					//calculate(a->n1,"$t0","$t1","$t2");
					if(i == -1 && find == 0) {
						for(i = 0;i < 5;i++) {
							if(loc_reg_use[i] == 0 && (a->n4[0]=='w' && a->n4[1]=='m')) {
								loc_reg_use[i] = 1;
								loc_reg_type[i] = 1;
								loc_reg[i] = a->n4;
								char reg[5]="$t";
								strcat(reg,numtostr(i+3));
								calculate(a->n1,reg1,reg2,reg,n2_isconst,n3_isconst);
								break;
							}
						}
						if(i == 5) {
							i = find_attr(a->n4);
							if(i != -1){
								char reg[5]="$a";
								strcat(reg,numtostr(i));
								calculate(a->n1,reg1,reg2,reg,n2_isconst,n3_isconst);

							} else{
								calculate(a->n1,reg1,reg2,"$t2",n2_isconst,n3_isconst);
								i = find_space(shift,a->n4);
								if(i == -1){
                                    if(a->n4[0]!='w'||a->n4[1]!='m'){
                                        fprintf(out_mips,"sw $t2,%s\n",a->n4);
                                    }else{
                                        shift++;
                                        space_list[shift-1] = a->n4;
                                        space_type[shift-1] = 1;
                                        fprintf(out_mips,"sw $t2,-%d($sp)\n",4*(shift-1));
                                    }

								}else{
									space_type[shift-1]=1;
									fprintf(out_mips,"sw $t2,-%d($sp)\n",i);
								}
							}
						}
					} else if(find == 0){
						char reg[5]="$t";
						strcat(reg,numtostr(i));
						calculate(a->n1,reg1,reg2,reg,n2_isconst,n3_isconst);
						loc_reg_type[i-3]=1;
					}
					//printf("%s %s %s %s\n",a->n1,a->n2,a->n3,a->n4);
					index++;
					if(index >= mid_line){
						return;
					}
					a = code_list[index];
				}
				if(a->op == 13) {//printf
					fprintf(out_mips,"#printf\n");
					if(a->n2[0]!='\0') {
						fprintf(out_mips,"la $a0,str_%s\n",a->n4);
						fprintf(out_mips,"li $v0,4\n");
						fprintf(out_mips,"syscall\n");
					}
					if(a->n3[0]!='\0') {
						if(!isalpha(a->n3[0]) && a->n3[0] != '_') {
							fprintf(out_mips,"li $a0,%s\n",a->n3);
							if(isalnum(a->n3[0])) {
								fprintf(out_mips,"li $v0,1\n");
							} else {
								fprintf(out_mips,"li $v0,11\n");
							}
							fprintf(out_mips,"syscall\n");
						} else {
							int i;
							int find = 0;
							int is_int;
							if(global_can > 0) {
								i = find_global_reg(a->n3);
								if(i!=-1) {
									find = 1;
									fprintf(out_mips,"move $a0,$s%d\n",i);
									if(global_reg_type[i]==1){
										fprintf(out_mips,"li $v0,1\n");
									}else{
										fprintf(out_mips,"li $v0,11\n");
									}
									fprintf(out_mips,"syscall\n");
								}
							}

							i = find_loc_reg(a->n3);
							if(i != -1) {
								find = 1;
								fprintf(out_mips,"move $a0,$t%d\n",i);
								if(loc_reg_type[i-3] == 1){
									fprintf(out_mips,"li $v0,1\n");
								}else {
									fprintf(out_mips,"li $v0,11\n");
								}
								fprintf(out_mips,"syscall\n");
								loc_reg_use[i-3] = 0;
								loc_reg[i-3]=NULL;
							}

							for(i = 0;i < 3 && find == 0;i++) {
								if(a1_a3[i] == NULL){
									break;
								}
								if(strcmp(a->n3,a1_a3[i])==0) {
									find = 1;
									fprintf(out_mips,"move $a0,$a%d\n",i+1);
									if(a1_a3_type[i] == 1){
										fprintf(out_mips,"li $v0,1\n");
									}else {
										fprintf(out_mips,"li $v0,11\n");
									}
									fprintf(out_mips,"syscall\n");
									break;
								}
							}//变量寄存里面找

							for(i = 2;i < shift && find == 0;i++) {
                                if(space_list[i] == NULL){
                                    continue;
                                }
								if(strcmp(space_list[i],a->n3)==0) {
									//printf("yyyyyyy\n");
									find = 1;
									fprintf(out_mips,"lw $a0,-%d($sp)\n",4*i);
									if(space_type[i] == 1){
										fprintf(out_mips,"li $v0,1\n");
									}else{
										fprintf(out_mips,"li $v0,11\n");
									}
									fprintf(out_mips,"syscall\n");
									break;
								}
							}//栈里面找
							if(find == 0 && strcmp(a->n3,"RET") == 0) {
								if(v0 == 1) {
									fprintf(out_mips,"move $a0,$v0\n");
									v0 = 0;
								}else {
									fprintf(out_mips,"move $a0,$v1\n");
								}
								if(ren_type == 1) {
									fprintf(out_mips,"li $v0,1\n");
								}else{
									fprintf(out_mips,"li $v0,11\n");
								}
								fprintf(out_mips,"syscall\n");
								find = 1;
							}
							if(find == 0) {
								fprintf(out_mips,"lw $a0,%s\n",a->n3);
								for(i = 0;i < array_num;i++) {
									if(strcmp(a->n3,array_list[i]->name) == 0){
										if(array_list[i]->type == 1){
											fprintf(out_mips,"li $v0,1\n");
										} else{
											fprintf(out_mips,"li $v0,11\n");
										}
										fprintf(out_mips,"syscall\n");
										break;
									}
								}
							}//全局里面找 常量变量
						}
					}
					fprintf(out_mips,"la $a0,enter\n");
					fprintf(out_mips,"li $v0,4\n");
					fprintf(out_mips,"syscall\n");
				}
				if(a->op == 12){//label
					int i;
					if(a->n4[0]=='1') {
						func_for[for_num-1] = 1;
						global_can++;
						//printf("%s %s %d\n",a->n1,a->n4,global_can);
						alloc_global(shift,index);
					}
					fprintf(out_mips,"%s\n",a->n1);//标签
					if(a->n4[0]=='0'){
						global_can--;
						if(global_can == 0){
							sw_global_reg(shift);
							for(i = 0;i < 8;i++) {
								global_reg[i] =NULL;
							}
						}
					}
				}
				if(a->op == 14) { //scanf
					fprintf(out_mips,"#scanf %s\n",a->n2);
					int i;
					int find=0;
					if(global_can > 0) {
						i = find_global_reg(a->n2);
						if(i != -1){
							find = 1;
							if(global_reg_type[i]==1){
								fprintf(out_mips,"li $v0,5\n");
							} else{
								fprintf(out_mips,"li $v0,12\n");
							}
							fprintf(out_mips,"syscall\n");
							fprintf(out_mips,"move $s%d,$v0\n",i);
						} else{
							i = alloc_global_reg(shift,a->n2);
							if(i != -1) {
								find = 1;
								if(global_reg_type[i]==1){
								fprintf(out_mips,"li $v0,5\n");
								} else {
									fprintf(out_mips,"li $v0,12\n");
								}
								fprintf(out_mips,"syscall\n");
								fprintf(out_mips,"move $s%d,$v0\n",i);
							}
						}
					}
					i = find_attr(a->n2);
					if(i != -1 && find == 0) {
						if(a1_a3_type[i-1]==1){
							fprintf(out_mips,"li $v0,5\n");
						}else {
							fprintf(out_mips,"li $v0,12\n");
						}
						fprintf(out_mips,"syscall\n");
						fprintf(out_mips,"move $a%d,$v0\n",i);

					} else if(find == 0) {
							i = find_space(shift,a->n2);
							if(i != -1){
								int type = space_type[i/4];
							if(type == 1) {
								fprintf(out_mips,"li $v0,5\n");
							} else {
								fprintf(out_mips,"li $v0,12\n");
							}
							fprintf(out_mips,"syscall\n");
							fprintf(out_mips,"sw $v0,-%d($sp)\n",i);
						} else {
							int i;
							for(i = 0;i < array_num;i++){
								if(strcmp(array_list[i]->name,a->n2)==0){
									if(array_list[i]->type == 1){
										fprintf(out_mips,"li $v0,5\n");
									}else{
										fprintf(out_mips,"li $v0,12\n");
									}
									fprintf(out_mips,"syscall\n");
									fprintf(out_mips,"sw $v0,%s\n",array_list[i]->name);
									break;
								}
							}
						}
					}
				}

				if(a->op == 9) {//return
					//printf("\n");
					if(a->n2[0]=='\0') {
						if(strcmp(fun_name,"main")==0) {
							fprintf(out_mips,"li $v0,10\n");
							fprintf(out_mips,"syscall\n");
						} else {
							fprintf(out_mips,"jr $ra\n");
						}
						index++;
						if(index == mid_line){
							break;
						}
						a = code_list[index];
						continue;
					}
					if(!isalpha(a->n2[0]) && a->n2[0]!= '_') {
						fprintf(out_mips,"li $v1,%s\n",a->n2);
						v0 = 1;
					} else {
						int find = 0;
						int i;
							int is_int;
							i = find_global_reg(a->n2);
							if(i != -1){
								find = 1;
								fprintf(out_mips,"move $v1,$s%d\n",i);
							} else {
								i = alloc_global_reg(shift,a->n2);
								if(i != -1){
									find = 1;
									fprintf(out_mips,"move $v1,$s%d\n",i);
								}
							}
							i = find_loc_reg(a->n2);
							if(i != -1 && find == 0) {
								find = 1;
								fprintf(out_mips,"move $v1,$t%d\n",i);
								loc_reg_use[i-3]=0;
								loc_reg[i-3]=NULL;
							}
							for(i = 0;i < 3 && find == 0;i++) {
								if(a1_a3[i] == NULL){
									break;
								}
								if(strcmp(a->n2,a1_a3[i])==0) {
									find = 1;
									fprintf(out_mips,"move $v1,$a%d\n",i+1);
									break;
								}
							}//变量寄存里面找
							for(i = 2;i < shift && find == 0;i++) {
								if(strcmp(space_list[i],a->n2)==0){
									find = 1;
									if(v0 == 0){
										fprintf(out_mips,"lw $v0,-%d($sp)\n",4*i);
										v0 = 1;
									}else{
										fprintf(out_mips,"lw $v1,-%d($sp)\n",4*i);
									}
									break;
								}
							}//栈里面找

							if(find == 0){
								fprintf(out_mips,"lw $v1,%s\n",a->n2);
								v0 = 1;
							}//全局里面找 常量变量
					}
					fprintf(out_mips,"jr $ra\n");
				}//返回语句
				if(a->op == 6) {//assign
					//printf("%s %s\n",a->n3,a->n4);
					fprintf(out_mips,"#%s [%s] = %s\n",a->n1,a->n2,a->n4);
					//printf("%s %s %s %s\n",a->n1,a->n2,a->n3,a->n4);
					char reg1[5],reg2[5];
					strcpy(reg2,"$t1");
					if(!isalpha(a->n4[0]) && a->n4[0]!='_') {
						fprintf(out_mips,"li $t1,%s#2222\n",a->n4);
					} else {
						int i,find = 0;
						int is_int;
						i = find_global_reg(a->n4);
						if(i != -1){
							find = 1;
							char reg[5]="$s";
							strcat(reg,numtostr(i));
							strcpy(reg2,reg);
						} else {
							i = alloc_global_reg(shift,a->n4);
							if(i != -1){
								find = 1;
								char reg[5]="$s";
								strcat(reg,numtostr(i));
								strcpy(reg2,reg);
							}
						}
						i = find_loc_reg(a->n4);
						if(i != -1 && find == 0) {
							find = 1;
							char reg[5]="$t";
							strcat(reg,numtostr(i));
							strcpy(reg2,reg);
							loc_reg_use[i-3]=0;
							loc_reg[i-3]=NULL;
						}
						if(find == 0){
							i = find_attr(a->n4);
							if(i != -1) {
								char reg[5]="$a";
								strcat(reg,numtostr(i));
								strcpy(reg2,reg);
							} else {
								i = find_space(shift,a->n4); //栈里面找
								if(i != -1) {
									fprintf(out_mips,"lw $t1,-%d($sp)\n",i);
								} else {
									if(strcmp(a->n4,"RET")!=0) {
										fprintf(out_mips,"lw $t1,%s\n",a->n4);//全局变量
									}else {
										if(v0 == 1) {
											fprintf(out_mips,"move $t1,$v0\n");
											v0 = 0;
										}else {
											fprintf(out_mips,"move $t1,$v1\n");
										}
									}
								}
							}
						}
					}
					if(strcmp(a->n2,"null") == 0) {
						int i,find =0;
						int is_int;
						i = find_global_reg(a->n1);
						if(i != -1){
							find = 1;
							fprintf(out_mips,"move $s%d,%s\n",i,reg2);
						}else{
							i = alloc_global_reg(shift,a->n1);
							if(i != -1){
								find = 1;
								fprintf(out_mips,"move $s%d,%s\n",i,reg2);
							}
						}
						i = find_loc_reg(a->n1);
						if(i!=-1 && find == 0) {
							fprintf(out_mips,"move $t%d,%s\n",i,reg2);
						}else if(find == 0){
							i = find_attr(a->n1);
							if(i != -1) {
								fprintf(out_mips,"move $a%d,%s\n",i,reg2);
							} else {
								i = find_space(shift,a->n1); //栈里面找
								if(i != -1) {
									fprintf(out_mips,"sw %s,-%d($sp)\n",reg2,i);
								} else {
									fprintf(out_mips,"sw %s,%s\n",reg2,a->n1);//全局变量
								}
							}
						}
					} else {
						int i;
						if(!isalpha(a->n2[0]) && a->n2[0]!='_'){
							int s = strtonum(a->n2)*4;
							fprintf(out_mips,"li $t2,%d\n",s);
							i = find_space(shift,a->n1);
							if(i == -1) {
								fprintf(out_mips,"sw %s,%s($t2)\n",reg2,a->n1);
							}else{
								fprintf(out_mips,"sw %s,-%d($sp)\n",reg2,s+i);
							}
						}else{
							int i,find = 0;
							int is_int;
							i = find_global_reg(a->n2);
							if(i != -1){
								find = 1;
								fprintf(out_mips,"move $t0,$s%d\n",i);
							}else{
								i = alloc_global_reg(shift,a->n2);
								if(i != -1){
									find = 1;
									fprintf(out_mips,"move $t0,$s%d\n",i);
								}
							}
							i = find_loc_reg(a->n2);
							if(i != -1){
								find = 1;
								fprintf(out_mips,"move $t0,$t%d\n",i);
								loc_reg_use[i-3] = 0;
								loc_reg[i-3]=NULL;
							}
							i = find_attr(a->n2);
							if(i != -1 && find == 0) {
								fprintf(out_mips,"move $t0,$a%d\n",i);
							} else if(find == 0){
								i = find_space(shift,a->n2); //栈里面找
								if(i != -1) {
									fprintf(out_mips,"lw $t0,-%d($sp)\n",i);
								} else {
									if(strcmp(a->n2,"RETS")!=0) {
										fprintf(out_mips,"lw $t0,%s\n",a->n2);//全局变量
									}else {
										fprintf(out_mips,"move $t0,$v1\n");
									}
								}
							}
							i = find_space(shift,a->n1);
							fprintf(out_mips,"sll $t0,$t0,2\n");
							if(i == -1){
								fprintf(out_mips,"sw %s,%s($t0)\n",reg2,a->n1);
							}else {
								fprintf(out_mips,"addiu $t0,$t0,%d\n",i);
								fprintf(out_mips,"subu $t0,$sp,$t0\n");
								fprintf(out_mips,"sw %s,0($t0)\n",reg2);
							}
						}
					}
					//printf("%s %s %s %s\n",a->n1,a->n2,a->n3,a->n4);
				}
				if(a->op == 16){//get value from array
					fprintf(out_mips,"#  %s= %s [%s]~~~~~~~~~~~~~~~~~~~~~~~\n",a->n1,a->n3,a->n4);
					int i;
					int type;
					for(i = 0;i < array_num;i++) {
						if(strcmp(a->n3,array_list[i]->name) == 0){
							if(array_list[i]->type == 1){
								type = 1;
							} else{
								type = 0;
							}
							break;
						}
					}
					i = find_space(shift,a->n3);
					if(i != -1){
						type = space_type[i/4];
					}//找类型

					if(!isalpha(a->n4[0]) && a->n4[0] != '_') {
						int s = strtonum(a->n4)*4;
						int x;
						for(i =0;i < 5;i++) {
							if(loc_reg_use[i] == 0){
								loc_reg_use[i] = 1;
								loc_reg[i] = a->n1;
								loc_reg_type[i] = type;
								x = i;
								break;
							}
						}
						i = find_space(shift,a->n3);
						if(i == -1){
							fprintf(out_mips,"la $t0,%s\n",a->n3);
							fprintf(out_mips,"addiu $t0,$t0,%d\n",s);
							fprintf(out_mips,"lw $t%d,0($t0)\n",x+3);
						}else {
							fprintf(out_mips,"lw $t%d,-%d($sp)\n",x+3,i+s);
						}
						if(i == 5){
							shift++;
							space_list[shift-1] = a->n1;
							fprintf(out_mips,"sw $t0,-%d($sp)\n",4*(shift-1));
							space_type[shift-1] = type;
						}
					} else {
						int i;
						int is_int;
						int find = 0;
						char reg1[5];
						strcpy(reg1,"$t1");
						i = find_global_reg(a->n4);
						if(i != -1){
							find = 1;
							char reg[5] = "$s";
							strcat(reg,numtostr(i));
							strcpy(reg1,reg);
						}
						i = find_loc_reg(a->n4);
						if(i != -1 && find == 0) {
							find = 1;
							char reg[5] = "$t";
							strcat(reg,numtostr(i));
							strcpy(reg1,reg);
							if(strcmp(a->n1,a->n4)!=0){
								loc_reg[i-3] = NULL;
								loc_reg_use[i-3] = 0;
							}
						}
						if(find == 0){
							i = find_attr(a->n4);
							if(i != -1) {
								char reg[5]="$a";
								strcat(reg,numtostr(i));
								strcpy(reg1,reg);
							} else {
								i = find_space(shift,a->n4); //栈里面找
								if(i != -1) {
									fprintf(out_mips,"lw $t1,-%d($sp)\n",i);
								} else {
									if(strcmp(a->n4,"RETS")!=0) {
										fprintf(out_mips,"lw $t1,%s\n",a->n4);//全局变量
									}else {
										fprintf(out_mips,"move $t1,$v1\n");
									}
								}
							}
						}//偏移量找完

						i = find_loc_reg(a->n1);
						if(i == -1) {
							int x;
							for(i = 0;i<5;i++) {
								if(loc_reg_use[i] == 0) {
									loc_reg[i] = a->n1;
									loc_reg_type[i] = type;
									loc_reg_use[i] = 1;
									fprintf(out_mips,"sll $t1,%s,2\n",reg1);
									x = find_space(shift,a->n3);
									if(x != -1) {
										fprintf(out_mips,"addiu $t1,$t1,%d\n",x);
										fprintf(out_mips,"subu $t1,$sp,$t1\n");
										fprintf(out_mips,"lw $t0,0($t1)\n");
										fprintf(out_mips,"move $t%d,$t0\n",i+3);
									} else {
										fprintf(out_mips,"la $t0,%s\n",a->n3);
										fprintf(out_mips,"addu $t0,$t0,$t1\n");
										fprintf(out_mips,"lw $t0,0($t0)\n");
										fprintf(out_mips,"move $t%d,$t0\n",i+3);
									}
									break;
								}
							}
							if(i==5){
								i = find_space(shift,a->n1);
								if(i == -1) {
									shift++;
									space_list[shift-1] = a->n1;
									space_type[shift-1] = type;
									fprintf(out_mips,"sll $t1,%s,2\n",reg1);
									i = find_space(shift,a->n3);
									if(i != -1) {
										fprintf(out_mips,"addiu $t1,$t1,%d\n",i);
										fprintf(out_mips,"subu $t1,$sp,$t1\n");
										fprintf(out_mips,"lw $t0,0($t1)\n");
											fprintf(out_mips,"sw $t0,-%d($sp)\n",4*(shift-1));
									} else {
										fprintf(out_mips,"la $t0,%s\n",a->n3);
										fprintf(out_mips,"addu $t0,$t0,$t1\n");
										fprintf(out_mips,"lw $t0,0($t0)\n");
										fprintf(out_mips,"sw $t0,-%d($sp)\n",4*(shift-1));
									}
								} else {
									int aa;
									aa = i;
									i = find_space(shift,a->n3);
									space_type[i/4] = type;
									fprintf(out_mips,"sll $t1,%s,2\n",reg1);
									if(i != -1) {
										fprintf(out_mips,"addiu $t1,$t1,%d\n",i);
										fprintf(out_mips,"subu $t1,$sp,$t1\n");
										fprintf(out_mips,"lw $t0,0($t1)\n");
										fprintf(out_mips,"sw $t0,-%d($sp)\n",aa);
									}else{
										fprintf(out_mips,"la $t0,%s\n",a->n3);
										fprintf(out_mips,"addu $t0,$t0,$t1\n");
										fprintf(out_mips,"lw $t0,0($t0)\n");
										fprintf(out_mips,"sw $t0,-%d($sp)\n",aa);
									}
								}
							}
						} else {
							int x;
							x = find_space(shift,a->n3);
							loc_reg_type[i-3] = type;
							fprintf(out_mips,"sll $t1,%s,2\n",reg1);
							if(x != -1) {
								fprintf(out_mips,"addiu $t1,$t1,%d\n",x);
								fprintf(out_mips,"subu $t1,$sp,$t1\n");
								fprintf(out_mips,"lw $t0,0($t1)\n");
								fprintf(out_mips,"move $t%d,$t0\n",i);
							} else {
								fprintf(out_mips,"la $t0,%s\n",a->n3);
								fprintf(out_mips,"addu $t0,$t0,$t1\n");
								fprintf(out_mips,"lw $t0,0($t0)\n");
								fprintf(out_mips,"move $t%d,$t0\n",i);
							}
						}
							//fprintf(out_mips,"sw $t0,-%d($sp)\n",4*(shift-1));
					}
				}
				if(a->op == 10) {//condition and jump 10 and 11
					int value;// 1:condition true 0:false
					char reg1[5],reg2[5];
					strcpy(reg1,"$t0");
					strcpy(reg2,"$t1");
					if(!isalpha(a->n1[0]) && a->n1[0]!='_') {
						fprintf(out_mips,"li $t0,%s\n",a->n1);
					} else{
						int i,find=0;
						i = find_global_reg(a->n1);
						if(i != -1){
							find = 1;
							char reg[5] = "$s";
							strcat(reg,numtostr(i));
							strcpy(reg1,reg);
						}else{
							i = alloc_global_reg(shift,a->n1);
							if(i != -1){
								find = 1;
								char reg[5] = "$s";
								strcat(reg,numtostr(i));
								strcpy(reg1,reg);
							}
						}
						i = find_loc_reg(a->n1);
						if(i!=-1 && find == 0){
							find = 1;
							char reg[5]="$t";
							strcat(reg,numtostr(i));
							strcpy(reg1,reg);
							if(strcmp(a->n1,a->n3)!=0){
								loc_reg[i-3] = NULL;
								loc_reg_use[i-3] = 0;
							}
						}
						if(find == 0){
							i = find_attr(a->n1);
							if(i != -1) {
								char reg[5]="$a";
								strcat(reg,numtostr(i));
								strcpy(reg1,reg);
							} else {
								i = find_space(shift,a->n1);
								if(i != -1) {
									fprintf(out_mips,"lw $t0,-%d($sp)\n",i);
								} else {
									if(strcmp(a->n1,"RETS")!=0){
										fprintf(out_mips,"lw $t0,%s\n",a->n1);//全局变量
									}else {
										if(v0 == 1) {
											fprintf(out_mips,"move $t0,$v0\n");
											v0 = 1;
										}else {
											fprintf(out_mips,"move $t0,$v1\n");
										}
									}
								}
							}
						}
					}

					if(a->n2[0] == '\0') {
						// a->n1 == 0 false
						index++;
						a = code_list[index];//jump
						if(strcmp(a->n1,"bz")==0){// 不满足跳转
							fprintf(out_mips,"beq %s,$0,%s\n",reg1,a->n2);
						}else if(strcmp(a->n1,"bnz")==0){//满足跳转
							fprintf(out_mips,"bne %s,$0,%s\n",reg1,a->n2);
						}
					} else {
						if(!isalpha(a->n3[0]) && a->n3[0]!='_') {
							fprintf(out_mips,"li $t1,%s#33333\n",a->n3);
						} else {
							int i,find = 0;
							i = find_global_reg(a->n3);
							if(i != -1){
								find = 1;
								char reg[5]="$s";
								strcat(reg,numtostr(i));
								strcpy(reg2,reg);
							} else{
								i = alloc_global_reg(shift,a->n3);
								if(i != -1){
									find = 1;
									char reg[5]="$s";
									strcat(reg,numtostr(i));
									strcpy(reg2,reg);
								}
							}
							i = find_loc_reg(a->n3);
							if(i != -1 && find == 0){
								find = 1;
								char reg[5]="$t";
								strcat(reg,numtostr(i));
								strcpy(reg2,reg);
								loc_reg[i-3] = NULL;
								loc_reg_use[i-3] = 0;
							}
							if(find == 0) {
								i = find_attr(a->n3);
								if(i != -1) {
									char reg[5]="$a";
									strcat(reg,numtostr(i));
									strcpy(reg2,reg);
								} else {
									i = find_space(shift,a->n3);
									if(i != -1){
										fprintf(out_mips,"lw $t1,-%d($sp)\n",i);
									} else {
										if(strcmp(a->n3,"RETS")!=0){
											fprintf(out_mips,"lw $t1,%s\n",a->n3);//全局变量
										}else{
											if(v0 == 1) {
												fprintf(out_mips,"move $t1,$v0\n");
												v0 = 0;
											}else {
												fprintf(out_mips,"move $t1,$v1\n");
											}
										}

									}
								}
							}
						}
						char op[20]={'\0'};
						strcpy(op,a->n2);
						index++;
						a = code_list[index];
						if(strcmp(op,"==")==0){
							if(strcmp(a->n1,"bz")==0){//不满足跳转
								fprintf(out_mips,"bne %s,%s,%s\n",reg1,reg2,a->n2);
							} else {//满足跳转
								fprintf(out_mips,"beq %s,%s,%s\n",reg1,reg2,a->n2);
							}
						} else if(strcmp(op,"!=")==0){
							if(strcmp(a->n1,"bz")==0){//不满足跳转
								fprintf(out_mips,"beq %s,%s,%s\n",reg1,reg2,a->n2);
							} else {//满足跳转
								fprintf(out_mips,"bne %s,%s,%s\n",reg1,reg2,a->n2);
							}
						} else if(strcmp(op,"<")==0){
							if(strcmp(a->n1,"bz")==0){//不满足跳转
								fprintf(out_mips,"bge %s,%s,%s\n",reg1,reg2,a->n2);
							} else {//满足跳转
								fprintf(out_mips,"blt %s,%s,%s\n",reg1,reg2,a->n2);
							}
						} else if(strcmp(op,"<=")==0){
							if(strcmp(a->n1,"bz")==0){//不满足跳转
								fprintf(out_mips,"bgt %s,%s,%s\n",reg1,reg2,a->n2);
							} else {//满足跳转
								fprintf(out_mips,"ble %s,%s,%s\n",reg1,reg2,a->n2);
							}
						} else if(strcmp(op,">")==0){
							if(strcmp(a->n1,"bz")==0){//不满足跳转
								fprintf(out_mips,"ble %s,%s,%s\n",reg1,reg2,a->n2);
							} else {//满足跳转
								fprintf(out_mips,"bgt %s,%s,%s\n",reg1,reg2,a->n2);
							}
						} else if(strcmp(op,">=")==0){
							if(strcmp(a->n1,"bz")==0){//不满足跳转
								fprintf(out_mips,"blt %s,%s,%s\n",reg1,reg2,a->n2);
							} else {//满足跳转
								fprintf(out_mips,"bge %s,%s,%s\n",reg1,reg2,a->n2);
							}
						}
					}
					//printf("%s %s %s %s\n",a->n1,a->n2,a->n3,a->n4);
				}
				if(a->op == 15) { //步长
						int i,find = 0;
						int ss;
						i = find_global_reg(a->n1);
						if(i != -1) {
							find = 1;
							fprintf(out_mips,"move $t0,$s%d\n",i);
						} else {
							i = alloc_global_reg(shift,a->n1);
							if(i != -1){
								find = 1;
								fprintf(out_mips,"move $t0,$s%d\n",i);
							}
						}
						i = find_attr(a->n1);//a1_a3
						if(i != -1 && find == 0) {
							fprintf(out_mips,"move $t0,$a%d\n",i);
						} else if(find == 0){
							i = find_space(shift,a->n1);
							if(i != -1) {
								fprintf(out_mips,"lw $t0,-%d($sp)\n",i);
							} else {
								fprintf(out_mips,"lw $t0,%s\n",a->n1);//全局变量
							}
						}
						if(strcmp(a->n3,"+")==0) {
							fprintf(out_mips,"addiu $t0,$t0,%s\n",a->n4);
						} else if(strcmp(a->n3,"-")==0) {
							fprintf(out_mips,"subiu $t0,$t0,%s\n",a->n4);
						}
						find = 0;
						i = find_global_reg(a->n1);
						if(i != -1) {
							find = 1;
							fprintf(out_mips,"move $s%d,$t0\n",i);
						} else {
							i = alloc_global_reg(shift,a->n1);
							if(i != -1){
								find = 1;
								fprintf(out_mips,"move $s%d,$t0\n",i);
							}
						}
						i = find_attr(a->n1);//a1_a3
						if(i != -1 && find == 0) {
							fprintf(out_mips,"move $a%d,$t0\n",i);
						} else if(find == 0){
							i = find_space(shift,a->n1);
							if(i != -1) {
								fprintf(out_mips,"sw $t0,-%d($sp)\n",i);
							} else {
								fprintf(out_mips,"sw $t0,%s\n",a->n1);//全局变量
							}
						}
				}
				if(a->op == 11) {
					if(strcmp(a->n1,"goto")==0){
						fprintf(out_mips,"j %s\n",a->n2);
					}
				}
				int dis;
				if(a->op == 7) {//函数调用
					attr = 1;
					int c = 5;
					int num = 0;
					int i,x;
					for(i = 0;i < 3;i++) {
						if(a1_a3[i] == NULL) {
							break;
						}
						x = find_space(shift,a1_a3[i]);
						if(x == -1) {
							printf("call fun error!!!\n");
						} else {
							fprintf(out_mips,"sw $a%d,-%d($sp)\n",i+1,x);
						}
					}
					while(a->op == 7) {
						if(!isalpha(a->n2[0]) && a->n2[0]!='_') {
							if(num < 3) {
								num++;
								fprintf(out_mips,"li $a%d,%s\n",num,a->n2);
								index++;
								a = code_list[index];
								continue;
							} else {
								fprintf(out_mips,"li $t0,%s\n",a->n2);
							}
						} else {
							int find = 0;
							i = find_global_reg(a->n2);
							if(i != -1) {
								find = 1;
								if(num < 3){
									num++;
									fprintf(out_mips,"move $a%d,$s%d\n",num,i);
									index++;
									a = code_list[index];
									continue;
								} else{
									fprintf(out_mips,"sw $s%d,-%d($sp)\n",i,4*(shift+5+c));
									c++;
									index++;
									a = code_list[index];
									continue;
								}

							} else{
								i = alloc_global_reg(shift,a->n2);
								if(i != -1){
									find = 1;
									if(num < 3){
										num++;
										fprintf(out_mips,"move $a%d,$s%d\n",num,i);
										index++;
										a = code_list[index];
										continue;
									} else{
										fprintf(out_mips,"sw $s%d,-%d($sp)\n",i,4*(shift+5+c));
										c++;
										index++;
										a = code_list[index];
										continue;
									}
								}
							}
							i = find_loc_reg(a->n2);
							if(i != -1 && find == 0) {
								find = 1;
								if(num < 3){
									num++;
									fprintf(out_mips,"move $a%d,$t%d\n",num,i);
								}else {
									fprintf(out_mips,"sw $t%d,-%d($sp)\n",i,4*(shift+5+c));
									c++;
								}
								loc_reg[i-3] = NULL;
								loc_reg_use[i-3] = 0;
								index++;
								a = code_list[index];
								continue;
							}
							if(find == 0) {
								i = find_space(shift,a->n2);//栈里面找
								if(i != -1) {
									fprintf(out_mips,"lw $t0,-%d($sp)\n",i);
								} else {
									if(strcmp(a->n2,"RETS")!=0){
										fprintf(out_mips,"lw $t0,%s\n",a->n2);//全局变量
									}else {
										if(num < 3) {
											num++;
											fprintf(out_mips,"move $a%d,$v1\n",num);
										} else {
											fprintf(out_mips,"sw $v1,-%d($sp)\n",4*(shift+5+c));
											c++;
										}
										index++;
										a = code_list[index];
										continue;
									}
								}
							}
						}//把push的数存在t0里面
						if(num < 3) {
							num++;
							fprintf(out_mips,"move $a%d,$t0\n",num);
						} else {
							fprintf(out_mips,"sw $t0,-%d($sp)\n",4*(shift+5+c));
							c++;
						}
						index++;
						a = code_list[index];
					}
					index--;
					a = code_list[index];
				}
				if(a->op == 8) {
					func_for[for_num-1] = 1;//存在函数调用还是压栈
					int x,i;

					func_ret *pp;
					for(x=0;x<ret_num;x++) {
						pp = ret_list[x];
						if(strcmp(pp->name,a->n2)==0){
							ren_type = pp->type;
							break;
						}
					}
					for(i = 0;i < 3 && attr == 0;i++) {
						if(a1_a3[i] == NULL) {
							break;
						}
						x = find_space(shift,a1_a3[i]);
						if(x == -1) {
							printf("call fun error!!!\n");
						} else {
							fprintf(out_mips,"sw $a%d,-%d($sp)\n",i+1,x);
						}
					}
					int flag;
					for(i =0;i < for_num;i++){
						if(strcmp(func_list[i],a->n2) == 0){
							flag = func_for[i];
							break;
						}
					}
					if(flag == 1){
						sw_global_reg(shift);//将全局寄存器内部的存入到栈中
					}
					for(i = 0;i < 5;i++) {
						if(loc_reg_use[i] == 1) {
							space_list[shift+i] = loc_reg[i];
							space_type[shift+i] = loc_reg_type[i];
							fprintf(out_mips,"sw $t%d,-%d($sp)\n",i+3,4*(shift+i));
						}
					}
					shift = shift + 5;
					fprintf(out_mips,"sw $sp,-%d($sp)\n",4*(shift));
					fprintf(out_mips,"sw $ra,-%d($sp)\n",4*(shift+1));
					fprintf(out_mips,"subiu $sp,$sp,%d\n",4*(shift));
					fprintf(out_mips,"jal %s\n",a->n2);
					fprintf(out_mips,"lw $ra,-%d($sp)\n",4*(1));
					fprintf(out_mips,"lw $sp,-%d($sp)\n",4*(0));

					if(flag == 1){
						lw_global(shift);
					}
					for(i = 0;i < 5;i++) {
						if(loc_reg_use[i] == 1) {
							fprintf(out_mips,"lw $t%d,-%d($sp)\n",i+3,4*(shift-5+i));
						}
					}
					for(i = 0;i < 3;i++) {
						if(a1_a3[i] == NULL) {
							break;
						}
						x = find_space(shift,a1_a3[i]);
						if(x == -1) {
							printf("call fun error!!!\n");
						} else {
							fprintf(out_mips,"lw $a%d,-%d($sp)\n",i+1,x);
						}
					}
				}
				if(a->op == 17){//取出函数返回的数据
					//printf("%s\n",a->n1);
					int i;
					for(i=0;i < 5;i++) {
						if(loc_reg_use[i] == 0){
							loc_reg_use[i] = 1;
							loc_reg_type[i] = ren_type;
							loc_reg[i] = a->n1;
							fprintf(out_mips,"move $t%d,$v1\n",i+3);
							break;
						}
					}
					if(i == 5){
						shift++;
						space_list[shift-1] = a->n1;
						if(ren_type == 1){
							space_type[shift-1] = 1;
						}else{
							space_type[shift-1] = 0;
						}
						fprintf(out_mips,"sw $v1,-%d($sp)\n",4*(shift-1));
					}
					//printf("%s\n",a->n1);
				}
				index++;
				if(index == mid_line){
					break;
				}
				a = code_list[index];
			}//其他语句
			index--;
			//fprintf(out_mips,"addiu $sp,$sp,%d\n",4*shift);
			if(strcmp(fun_name,"main")!=0){
				//fprintf(out_mips,"lw $sp,0($sp)\n");
				fprintf(out_mips,"jr $ra\n");
			}
		}
	}
}
//------------------------mips_end-------------------------

int main() {
	namelist = NULL;
	in = fopen("testfile.txt", "r");
	out = fopen("output.txt", "w");
	out2 = fopen("error.txt","w");
	out_mid = fopen("midcode.txt","w");
	out_mips = fopen("mips.txt","w");
	char c;
	int len = 0;
	while ((c = fgetc(in)) != EOF) {
		str[len] = c;
		len++;
	}
	char n1[100]={'\0'};
	char n2[100]={'\0'};
	char n3[100]={'\0'};
	char n4[100]={'\0'};
	project();

	//return 0;
	mid_code *a,*b;
	int count = 0;
	for(count = 0;count < mid_line;count++) {
		a = code_list[count];
		if(a->op == -2){
			continue;
		}
		if(a->op == 5) {
			b = code_list[count+1];
			if(b->op == 6){
				if(strcmp(a->n4,b->n4) == 0 && strcmp(b->n2,"null") == 0){
					strcpy(a->n4,b->n1);
					b->op = -2;
				}
			}
		}
			fprintf(out_mid," %d",a->op);
		if(a -> n1!=NULL){
			fprintf(out_mid," %s",a->n1);
		}
		if(a -> n2!=NULL){
			fprintf(out_mid," %s",a->n2);
		}
		if(a -> n3!=NULL){
			fprintf(out_mid," %s",a->n3);
		}
		if(a -> n4!=NULL){
			fprintf(out_mid," %s",a->n4);
		}
		fprintf(out_mid,"\n");
	}
	//return 0;
	data_gen();
	text_gen();
	return 0;
}

