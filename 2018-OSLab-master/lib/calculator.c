
#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

typedef struct node {
    double date;
    char opt;
    struct node* pre;
}* pnode, node;

typedef struct stack {
    pnode bottom;   //指向底部
    pnode top;  //指向头部
}* pstack, stack;

pstack create_stack(void) {
    pstack s = (pstack)malloc(sizeof(stack));
    s->bottom = s->top = (pnode)malloc(sizeof(node));
    return s;
}

void push(pstack s, char date) {
    pnode t = (pnode)malloc(sizeof(node));
    t->pre = s->top;
    t->date = date;
    s->top = t;
}

void pop(pstack s) {
    if (s->top == s->bottom) {
        printf("不能弹出，此时栈为空\n");
        return;
    }
    pnode t = s->top;
    s->top = s->top->pre;
    free(t);
}

int empty(pstack s) {
    if (s->top == s->bottom) {
        return 1;
    }
    else {
        return 0;
    }
}

int top(pstack s) {
    return s->top->date;
}

void push_num(pstack s, int date) {
    pnode t = (pnode)malloc(sizeof(node));
    t->pre = s->top;
    t->date = date;
    s->top = t;
}

PUBLIC int Calculator(char *str) {
    pstack s_opt = create_stack();
    pstack s_num = create_stack();
    char suffix[100];
    int opt[100];
    memset(opt, 0, sizeof(opt));
    opt['+'] = opt['-'] = 1;
    opt['*'] = opt['/'] = 2;
    opt['('] = opt[')'] = 0;
    int len = strlen(str), p = 0, flag = 1;
    //printf("%d\n",len);
    for (int i = 0; i < len; i++) {
        if (str[i] <= '9' && str[i] >= '0') {
            suffix[p++] = str[i];
            if (i+1 < len && str[i+1] <= '9' && str[i+1] >= '0');
            else    suffix[p++] = ' ';
            flag = 0;
            continue;
        }
        else {
            flag++;
        }
        if (str[i] == '(') {
            push(s_opt, str[i]);
        }
        else if (str[i] == ')') {
            while(top(s_opt) != '(') {
                //printf("%c\n", top(s_opt));
                suffix[p++] = top(s_opt);
                pop(s_opt);
            }
            pop(s_opt);
            //printf("1");
        }
        else if (empty(s_opt) && str[i] != ' ') {
            push(s_opt, str[i]);
        }
        else {
            while(!empty(s_opt) && opt[str[i]] <= opt[top(s_opt)]) {
                suffix[p++] = top(s_opt);
                pop(s_opt);
            }
            push(s_opt, str[i]);
        }
        if (flag >= 1) {
            //    suffix[p++] = ' ';
        }
    }
    while(!empty(s_opt)) {
        suffix[p++] = top(s_opt);
        pop(s_opt);
    }
    for (int i = 0; i < p-1; i++) {
        printf("%c",suffix[i]);
    }
    printf("\n");
    pstack number_stack = create_stack();
    flag = 0;
    double sum = 0, ans = 0;
    for (int i = 0; i < p; i++) {
        if (suffix[i] <= '9' && suffix[i] >= '0') {
            sum = sum*10 + (suffix[i]-'0');
            flag = 1;
        }
        else {
            if (flag == 1) {
                push(number_stack, sum);
                sum = 0;
                flag = 0;
            }
            if (suffix[i] == '+' || suffix[i] == '-' || suffix[i] == '*' || suffix[i] == '/') {
                double t1 = number_stack->top->date;
                pop(number_stack);
                if (suffix[i] == '+') {
                    number_stack->top->date += t1;
                }
                else if (suffix[i] == '-') {
                    number_stack->top->date -= t1;
                }
                else if (suffix[i] == '*') {
                    number_stack->top->date *= t1;
                }
                else if (suffix[i] == '/') {
                    number_stack->top->date /= t1;
                }
            }
        }
    }
    printf("%lf",number_stack->top->date);
    return 0;
}
