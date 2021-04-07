#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = { "int", "char", "double", "float", "long" 
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct" };


operator_precedence operators[OPERATOR_CNT] = { //연산자 우선순위
	{"(", 0}, {")", 0}
	,{"->", 1}
	,{"*", 4}	,{"/", 3}	,{"%", 2}
	,{"+", 6}	,{"-", 5}
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};

void compare_tree(node* root1, node* root2, int* result) //std_root ans_root &result
{
	node* tmp;
	int cnt1, cnt2;

	if (root1 == NULL || root2 == NULL) { 
		*result = false;
		return;
	}

	if (!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")) { //root->name이 "< > <= >="과 같으면
		if (strcmp(root1->name, root2->name) != 0) { //root1->name 과 root2->name이 같지 않을 때

			if (!strncmp(root2->name, "<", 1)) //root2->name이 "<"로 시작한다면
				strncpy(root2->name, ">", 1); //root2->name 처음을 ">"로 복사해준다

			else if (!strncmp(root2->name, ">", 1)) //root2->name이 ">"로 시작한다면
				strncpy(root2->name, "<", 1); //root2->name 처음을 "<"로 복사해준다

			else if (!strncmp(root2->name, "<=", 2)) //root2->name이 "<="로 시작한다면
				strncpy(root2->name, ">=", 2); //root2->name 처음을 ">="로 복사해준다

			else if (!strncmp(root2->name, ">=", 2)) //root2->name이 ">="로 시작한다면
				strncpy(root2->name, "<=", 2); //root2->name 처음을 "<="로 복사해준다

			root2 = change_sibling(root2); //형제 노드 변경 후 부모노드(root2)를 리턴
		}
	}

	if (strcmp(root1->name, root2->name) != 0) { //root1->name과 root2->name이 일치하지 않다면 result는 false
		*result = false;
		return;
	}

	if ((root1->child_head != NULL && root2->child_head == NULL) //(root1의 자식노드가 존재하고 root2의 자식노드가 존재하지 않는다면) || (root1의 자식노드가 존재하지않고 root2의 자식노드가 존재한다면)
		|| (root1->child_head == NULL && root2->child_head != NULL)) { 
		*result = false;
		return;
	}

	else if (root1->child_head != NULL) { //root1의 자식노드가 존재한다면 (그러면 root2는 자식노드가 존재 ?)
		if (get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)) { //root1의 자식노드와 roo2의 자식노드의 형제 개수가 다르면 result는 false 
			*result = false;
			return;
		}

		if (!strcmp(root1->name, "==") || !strcmp(root1->name, "!=")) //root->name이 ==과 같거나 root1->name이 !=과 같으면
		{
			compare_tree(root1->child_head, root2->child_head, result); //자식노드로 재 비교

			if (*result == false)
			{
				*result = true;
				root2 = change_sibling(root2); //다른 형제 노드로 변경하고 부모노드(root2) 리턴 
				compare_tree(root1->child_head, root2->child_head, result); //바꾼 형제노드로 재 비교
			}
		}
		else if (!strcmp(root1->name, "+") || !strcmp(root1->name, "*") //root1->name이 "+ * | || & &&"과 같으면
			|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
			|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{
			if (get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)) { //root1의 자식노드와 roo2의 자식노드의 형제 개수가 다르면 result는 false 
				*result = false;
				return;
			}

			tmp = root2->child_head; //root2의 자식노드를 tmp가 가르킴

			while (tmp->prev != NULL) //tmp의 prev노드가 존재한다면
				tmp = tmp->prev; //가장 prev한 노드를 tmp가 가르킴

			while (tmp != NULL)
			{
				compare_tree(root1->child_head, tmp, result); //root1->child_head와 root2의 자식노드 중 가장 prev한 노드와 비교

				if (*result == true)
					break;
				else {
					if (tmp->next != NULL) //tmp의 next노드가 존재한다면 result는 true 
						*result = true;
					tmp = tmp->next; //최종 next 노드를 tmp가 가르킴
				}
			}
		}
		else {
			compare_tree(root1->child_head, root2->child_head, result); //root1->child_head 와 root2->child_head 비교
		}
	}


	if (root1->next != NULL) { //root1의 next 노드가 존재한다면

		if (get_sibling_cnt(root1) != get_sibling_cnt(root2)) { //root1와 roo2의 형제 개수가 다르면 result는 false 
			*result = false;
			return;
		}

		if (*result == true)
		{
			tmp = get_operator(root1); //root1이 가르키는 노드의 가장 prev한 노드의 부모 노드를 tmp가 가르킴

			if (!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*") //tmp->name이 "+ * | || & &&"와 같으면
				|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
				|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{
				tmp = root2; //root2를 tmp가 가르킴

				while (tmp->prev != NULL) //tmp(root2)의 이전 노드가 존재한다면 
					tmp = tmp->prev; //가장 prev한 노드를 tmp가 가르킴

				while (tmp != NULL)
				{
					compare_tree(root1->next, tmp, result); //root1과 tmp 재 비교

					if (*result == true) 
						break;
					else {
						if (tmp->next != NULL) //tmp의 다음 노드가 존재한다면
							*result = true;
						tmp = tmp->next; //가장 마지막의 노드를 tmp가 가르킴
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result); //tmp->name이 "+ * | || & &&"와 같지 않으면 root1->next와 root2->next 재비교
		}
	}
}

int make_tokens(char* str, char tokens[TOKEN_CNT][MINLEN]) //s_answer 랑 tokens 들어옴
{
	char* start, * end;
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char* op = "(),;><=!|&^/+-*\"";
	int row = 0;
	int i;
	int isPointer;
	int lcount, rcount;
	int p_str;

	clear_tokens(tokens); //0으로 채워넣음

	start = str;

	if (is_typeStatement(str) == 0) //gcc랑 datatype으로 시작하지 않으면
		return false;

	while (1)
	{
		if ((end = strpbrk(start, op)) == NULL) //start에서 op가 없으면 break , 있으면 start에서 가장 먼저 나온 op 위치 포인터를 end에 줌
			break;

		if (start == end) { //start가 end(op) 로 시작하면 (위에서 변경한 위치 포인터)

			if (!strncmp(start, "--", 2) || !strncmp(start, "++", 2)) { //start가 --로 시작하거나 start가 ++로 시작하면
				if (!strncmp(start, "++++", 4) || !strncmp(start, "----", 4)) //start가 ++++로 시작하거나 start가 ----로 시작하면 false
					return false;

				if (is_character(*ltrim(start + 2))) { //start+2의 좌측 공백 제거하고 , 문자열이면 (++ or --다음에 있는 문자 or 숫자)
					if (row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) //row>0이고 tokens[row][마지막]이 문자열이면
						return false;

					end = strpbrk(start + 2, op); //start+2에 op가 있으면 op 위치 포인터를 end에 줌 , 없으면 0 넣음
					if (end == NULL) //op없으면
						end = &str[strlen(str)]; //str의 마지막 위치를 end가 가르킴
					while (start < end) {
						if (*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) //연산자 앞에 공백이 있거나 tokens[row][strlen(tokens[row]-1)]가 숫자 or 문자이면 false
							return false;
						else if (*start != ' ') //(지금 가르키는게 공백이 아니면)연산자로 시작하면
							strncat(tokens[row], start, 1); //tokens[row]에 start의 1만큼을 이어 붙임
						start++; //다음으로 -> tokens[row]+2 이런식?	
					}
				}

				else if (row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) { //row==1 && tokens[0][strlen(tokens[0]) - 1]가 문자 (++ or -- 다음 character가 아닌 경우)
					if (strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL) //tokens[0]에 ++이 있거나 tokens[0]에 --가 있으면 false
						return false;

					memset(tmp, 0, sizeof(tmp)); //tmp를 0으로 채움
					strncpy(tmp, start, 2); //start의 사이즈2만큼 tmp에 복사 (++ or --)
					strcat(tokens[row - 1], tmp); //tokens[0]에 tmp를 이어 붙임
					start += 2; //++ or -- 다음으로 이동
					row--; 
				}
				else {
					memset(tmp, 0, sizeof(tmp)); //tmp를 0으로 채움
					strncpy(tmp, start, 2); //start의 ++ or --를 tmp에 복사
					strcat(tokens[row], tmp); //tokens[row] 뒤에 ++ or --를 이어 붙임
					start += 2; //++ or --로 이동
				}
			}

			else if (!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2) //++이나 --로 시작하는 것이 아닌 
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2)
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2)
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2) || !strncmp(start, "-=", 2)
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)) { //얘네들로 시작된다면

				strncpy(tokens[row], start, 2); //tokens[row]에 start의 2만큼 (위에 있는 op를 복사)
				start += 2; //op다음으로 이동
			}
			else if (!strncmp(start, "->", 2)) //->로 시작한다면
			{
				end = strpbrk(start + 2, op); //->다음에도 op가 나온다면 그 위치를 end 가 가르킴, 없으면 0 리턴

				if (end == NULL) //->다음에 op 안나오면
					end = &str[strlen(str)]; //str의 마지막 위치로 end이동

				while (start < end) { //다음 op 나오기 전까지
					if (*start != ' ') //start가 공백을 가르키지 않는다면
						strncat(tokens[row - 1], start, 1); //tokens[row-1]에 start의 1만큼 이어 붙임
					start++; //다음으로 이동
				}
				row--;
			}
			else if (*end == '&') //end가 &이면 
			{

				if (row == 0 || (strpbrk(tokens[row - 1], op) != NULL)) { //row==0 이거나 tokens[row-1]에 op가 있으면
					end = strpbrk(start + 1, op); //& 다음에도 op가 나오면 그 위치를 end가 가르킴, 없으면 null
					if (end == NULL) //& 다음에 op 안나오면
						end = &str[strlen(str)]; //맨 마지막 end가 가리킴

					strncat(tokens[row], start, 1); //tokens[row]에 start의 크기 1만큼 (&)을 이어붙임
					start++; //다음으로 넘어감

					while (start < end) { //다음 op 나오기 전까지
						if (*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') //연산자 앞에 공백이 있고 tokens[row][strlen(tokens[row]) - 1]이 &가 아니면 false
							return false;
						else if (*start != ' ') //연산자 앞에 공백이 있고 끝에가 &면
							strncat(tokens[row], start, 1); //tokens[row]에 start의 1만큼 이어 붙임
						start++; //다음으로 이동
					}
				}

				else { 
					strncpy(tokens[row], start, 1); //tokens[row]에 start의 1만큼 복사함
					start += 1; //다음으로 이동
				}

			}
			else if (*end == '*') //*로 시작할 경우
			{
				isPointer = 0;

				if (row > 0)
				{

					for (i = 0; i < DATATYPE_SIZE; i++) { //35
						if (strstr(tokens[row - 1], datatype[i]) != NULL) { //tokens[row - 1]에 datatype이 있으면 그 위치 리턴
							strcat(tokens[row - 1], "*"); //tokens[row - 1]에 *을 이어붙임
							start += 1; //다음으로 이동
							isPointer = 1; 
							break;
						}
					} 
					if (isPointer == 1) 
						continue;
					if (*(start + 1) != 0) //이동한 곳이 널이 아니면
						end = start + 1; //end가 가리킴


					if (row > 1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)) { //row > 1이고, tokens[row-2]랑 *이랑 같고, tokens[row-1] 문자열이 모두 *이면
						strncat(tokens[row - 1], start, end - start); //tokens[row-1]에 start의 (end-start)만큼 이어붙임 
						row--;
					}


					else if (is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1) { //tokens[row-1][strlen(tokens[row-1])-1] 이 문자열이면
						strncat(tokens[row], start, end - start); //tokens[row]에 start의 (end-start)만큼 이어붙임
					}


					else if (strpbrk(tokens[row - 1], op) != NULL) { //tokens[row-1] 에 op가 있다면
						strncat(tokens[row], start, end - start); //tokens[row]에 start의 (end-start)만큼 이어붙임

					}
					else
						strncat(tokens[row], start, end - start); //tokens[row]에 start의 (end-start)만큼 이어붙임

					start += (end - start); //end 다음으로 이동
				}

				else if (row == 0)
				{
					if ((end = strpbrk(start + 1, op)) == NULL) { //start+1부터 중에서 op가 없다면
						strncat(tokens[row], start, 1); //tokens[row]에 start를 1만큼 이어붙임
						start += 1; //다음으로 이동
					}
					else { //op가 있다면
						while (start < end) { //다음 op 나오기 전까지
							if (*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) //start 앞에 공백이 있고 tokens[row][strlen(tokens[row])-1]이 문자열이면 false
								return false;
							else if (*start != ' ') //start가 공백을 가르키지 않으면
								strncat(tokens[row], start, 1); //tokens[row]에 start의 1만큼 이어 붙임
							start++; //다음으로 넘어감
						}
						if (all_star(tokens[row])) //tokens[row]가 모두 *이면
							row--;

					}
				}
			}
			else if (*end == '(') //(으로 시작하는 경우
			{
				lcount = 0;
				rcount = 0;
				if (row > 0 && (strcmp(tokens[row - 1], "&") == 0 || strcmp(tokens[row - 1], "*") == 0)) { //row>0 이고 && ( tokens[row-1]이 &와 같거나 || tokens[row-1]이 *과 같으면)
					while (*(end + lcount + 1) == '(') // ( 만날 때마다 lcount 증가
						lcount++;
					start += lcount; //lcount만큼 이동

					end = strpbrk(start + 1, ")"); //start+1이 )이면 그 위치를 end에 넣음 , )이 아니면 NULL

					if (end == NULL)
						return false;
					else { //start + 1이 )이면
						while (*(end + rcount + 1) == ')') // ) 만날 때마다 rcount 증가
							rcount++;
						end += rcount; //end를 rcount만큼 증가 

						if (lcount != rcount) //괄호 짝 안맞으면 false
							return false;

						if ((row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1) { 
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); //tokens[row-1] 에 start+1의 (end-start-rcount-1)만큼 이어 붙임 -> 가장 마지막 여는 괄호와 가장 첫번째 닫는 괄호 사이의 문자열
							row--;
							start = end + 1; //start가 마지막 가리킴
						}
						else {
							strncat(tokens[row], start, 1); //tokens[row]에 start의 1만큼을 이어붙임
							start += 1; 
						}
					}

				}
				else {
					strncat(tokens[row], start, 1); //tokens[row]에 start의 1만큼 ( '(' )을 이어붙임
					start += 1;
				}

			}
			else if (*end == '\"') //\"로 시작하면
			{
				end = strpbrk(start + 1, "\""); //start+1부터 ~ \"가 있으면 그 위치 end가 가리킴, 없으면 NULL

				if (end == NULL)
					return false;

				else {
					strncat(tokens[row], start, end - start + 1); //tokens[row]에 start의 \" ~ \" 사이만큼을 이어붙임
					start = end + 1; 
				}

			}

			else {

				if (row > 0 && !strcmp(tokens[row - 1], "++")) //row>0 이고 tokens[row-1]이 ++이면
					return false;


				if (row > 0 && !strcmp(tokens[row - 1], "--")) //row>0 이고 tokens[row-1]이 --이면
					return false;

				strncat(tokens[row], start, 1); //tokens[row]에 start의 1만큼을 이어붙이고 다음으로 이동
				start += 1;


				if (!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")) { //tokens[row]가 - + -- ++ 중에서 같으면



					if (row == 0)
						row--;


					else if (!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) {

						if (strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL)
							row--;
					}
				}
			}
		}
		else { //start==end 아님 end는 가장 먼저 나온 op 가르킴
			if (all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) //tokens[row-1]이 전부 * / row>1 / tokens[row-2]의 마지막이 character가 아닐 때
				row--; //row감소

			if (all_star(tokens[row - 1]) && row == 1) // row==1 / tokens[row-1]이 전부 * 일 때
				row--; //row감소

			for (i = 0; i < end - start; i++) { //처음 ~ 첫 op 전까지
				if (i > 0 && *(start + i) == '.') { //i>0 이고 start+i 가 '.'이면
					strncat(tokens[row], start + i, 1); //tokens[row]에 . 이어붙임

					while (*(start + i + 1) == ' ' && i < end - start) //. 다음이 공백이고 i가 op나오기 전일 동안 i증가
						i++; 
				}
				else if (start[i] == ' ') { //start[i] 가 공백이면
					while (start[i] == ' ') //공백 만나면 i 증가하고 break
						i++;
					break;
				}
				else
					strncat(tokens[row], start + i, 1); //tokens[row]에 start + i의 1만큼 이어붙임
			}

			if (start[0] == ' ') { //start의 처음이 공백이면
				start += i; //i만큼 이동
				continue;
			}
			start += i; //i만큼 이동
		}

		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //tokens[row] 공백제거해서 tokens[row]에 복사

		if (row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) //row>0이고 tokens[row]의 마지막이 character이고
			&& (is_typeStatement(tokens[row - 1]) == 2 //tokens[row-1]이 "gcc"나 datatype으로 시작하고 tokens[row-1]의 마지막이 character이거나 . 일 때
				|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
				|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.')) {

			if (row > 1 && strcmp(tokens[row - 2], "(") == 0) //row > 1 이고 tokens[row-2]가 "("이면
			{
				if (strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1], "unsigned") != 0) //tokens[row-1]가 "struct"가 아니고 "unsigned"가 아니면 false
					return false;
			}
			else if (row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { //row==1 이고 tokens[row]의 마지막이 character이면
				if (strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) //tokens[0]가 "exturn"이 아니고 tokens[0]가 "unsigned"가 아니고 tokens[0]가 "gcc"나 datatype으로 시작하지 않으면 false
					return false;
			}
			else if (row > 1 && is_typeStatement(tokens[row - 1]) == 2) { //row > 1이고 tokens[row-1]이 "gcc"나 datatype으로 시작하면
				if (strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) //tokens[row-2]가 "unsigned"가 아니고 "extern"이 아니면 false
					return false;
			}

		}

		if ((row == 0 && !strcmp(tokens[row], "gcc"))) { //row==0 이고 tokens[row]가 gcc랑 같으면
			clear_tokens(tokens); //tokens 0으로 채우고
			strcpy(tokens[0], str); //tokens에 str복사
			return 1;
		}

		row++;
	} //while문 끝

	if (all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) //tokens[row-1]이 전부 *이고 row > 1이고 tokens[row-2]의 마지막이 character가 아니면
		row--; //row 감소
	if (all_star(tokens[row - 1]) && row == 1) //tokens[row-1]이 전부 *이고 row==1이면 
		row--; //row 감소

	for (i = 0; i < strlen(start); i++) //start의 길이만큼 
	{
		if (start[i] == ' ') //공백 만나면
		{
			while (start[i] == ' ') //공백 만나는 동안 i증가
				i++;
			if (start[0] == ' ') { //근데 start[0]이 공백이면
				start += i; //i만큼 이동
				i = 0;
			}
			else //맨 처음이 공백이 아니면 row 증가
				row++;

			i--;
		}
		else //공백 안만남
		{
			strncat(tokens[row], start + i, 1); //tokens[row]에 start+i의 1만큼 이어붙임
			if (start[i] == '.' && i < strlen(start)) { //start[i]가 . 이고 i가 start 길이보다 작으면
				while (start[i + 1] == ' ' && i < strlen(start)) //. 다음이 공백이고 i가 start 길이보다 작으면
					i++; //i증가

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //tokens[row] 공백 제거한 걸 tokens[row]에 복사함

		if (!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")) { //tokens[row]랑 "lpthread"랑 같고 row>0이고 tokens[row - 1]이 "-"과 같으면 (-lpthread)
			strcat(tokens[row - 1], tokens[row]); //tokens[row]를 tokens[row-1]에 복사하고
			memset(tokens[row], 0, sizeof(tokens[row])); //tokens[row]를 0으로 채움
			row--; //row감소
		}
		else if (row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) //row>0 이고 tokens[row]의 마지막이 character이고 tokens[row-1]이 "gcc"나 datatype으로 시작하고
			&& (is_typeStatement(tokens[row - 1]) == 2 
				|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) //tokens[row-1]의 마지막 문자가 character 거나 . 일때
				|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.')) {

			if (row > 1 && strcmp(tokens[row - 2], "(") == 0) //row>1이고 tokens[row-2]가 "("일 때
			{
				if (strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1], "unsigned") != 0) //tokens[row-1]가 "struct"가 아니고 "unsigned"가 아니면 false
					return false;
			}
			else if (row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { //row == 1 이고 tokens[row]의 마지막이 character일 때
				if (strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) //tokens[0]가 "exturn", "unsigned"가 아니고 tokens[0]가 "gcc"나 datatype으로 시작하지 않으면 false
					return false;
			}
			else if (row > 1 && is_typeStatement(tokens[row - 1]) == 2) { //row>1이고 tokens[row-1]이 "gcc"나 datatype으로 시작하면
				if (strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)  //tokens[row-2]가 "unsigned", "extern"이 아니면 false
					return false;
			}
		}
	} //for문 끝


	if (row > 0)
	{
		if (strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0) { //tokens[0]이 "#include"와 같거나 "inlucd"와 같거나 "struct"와 같으면
			clear_tokens(tokens); //토큰 0으로 채움
			strcpy(tokens[0], remove_extraspace(str)); //공백 없앤 str을 tokens[0]에 복사해서 넣음
		}
	}

	if (is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL) { //tokens가 "gcc"나 datatype으로 시작하거나 tokens[0]에 "extern"이 있으면
		for (i = 1; i < TOKEN_CNT; i++) { //TOKEN_CNT 50
			if (strcmp(tokens[i], "") == 0) //내용 없으면 break
				break;

			if (i != TOKEN_CNT - 1) //i가 마지막이 아닐 때
				strcat(tokens[0], " "); //tokens[0]에 공백을 이어붙임
			strcat(tokens[0], tokens[i]); //tokens[0]에 tokens[i]를 이어붙임 => tokens[0]에 공백과 tokens[i] 내용으로 다 합침
			memset(tokens[i], 0, sizeof(tokens[i])); //tokens[i]를 0으로 초기화
		}
	}

	while ((p_str = find_typeSpecifier(tokens)) != -1) { //i를 TOKEN_CNT 50번 돌리면서 tokens[i] 중에 datatype이 있고 tokens[i-1]이 "(" && tokens[i+1]이 ")" && tokens[i+2][0]이 (& * ) ( - +) 중에 하나인 i 를 리턴
		if (!reset_tokens(p_str, tokens))
			return false;
	}


	while ((p_str = find_typeSpecifier2(tokens)) != -1) { // //tokens[i]가 "struct"랑 같고 i+1이 50보다 작거나 같아야하고 tokens[i + 1][strlen(tokens[i + 1]) - 1])이 문자열인 i
		if (!reset_tokens(p_str, tokens))
			return false;
	}

	return true;
}

node* make_tree(node* root, char(*tokens)[MINLEN], int* idx, int parentheses)
{
	node* cur = root;
	node* new;
	node* saved_operator;
	node* operator;
	int fstart;
	int i;

	while (1)
	{
		if (strcmp(tokens[*idx], "") == 0) //내용이 없으면 break
			break;

		if (!strcmp(tokens[*idx], ")")) //tokens[*idx]가 ")" 이면
			return get_root(cur); //최상위 노드 리턴

		else if (!strcmp(tokens[*idx], ",")) //tokens[*idx]가 ","이면
			return get_root(cur); //최상위 노드 리턴

		else if (!strcmp(tokens[*idx], "(")) //tokens[*idx]가 "("이면
		{

			if (*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0) { // *idx > 0 이고 tokens[*idx - 1]가 연산자가 아니고 tokens[*idx - 1]과 ,가 일치하지 않을 경우
				fstart = true; //fstart가 true

				while (1)
				{
					*idx += 1; //idx증가

					if (!strcmp(tokens[*idx], ")")) //tokens[*idx] 가 ")"와 같으면 break
						break;

					new = make_tree(NULL, tokens, idx, parentheses + 1); //make_tree

					if (new != NULL) { //new가 정상적으로 만들어졌다면
						if (fstart == true) {
							cur->child_head = new; //new가 자식노드 가리킴
							new->parent = cur; //cur가 부모노드 가리킴

							fstart = false;
						}
						else {
							cur->next = new; //new가 next 가리킴
							new->prev = cur; //cur가 prev 가리킴
						}

						cur = new; //cur 포인터 이동
					}

					if (!strcmp(tokens[*idx], ")")) //tokens[*idx]가 ")"와 같으면 break
						break;
				} //while문 끝
			}
			else { 
				*idx += 1; //한 칸 이동

				new = make_tree(NULL, tokens, idx, parentheses + 1); //make_tree

				if (cur == NULL) 
					cur = new;

				else if (!strcmp(new->name, cur->name)) { //new가 가리키는 name과 cur가 가리키는 name이 같다면
					if (!strcmp(new->name, "|") || !strcmp(new->name, "||") // (new->name 과 | 이 같거나) || (new->name과 ||) || (new->name과 &) || (new->&&이 같으면)
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))
					{
						cur = get_last_child(cur); //마지막 자식 노드 리턴

						if (new->child_head != NULL) { //child_head 가 NULL이 아니면 자식노드가  존재
							new = new->child_head; //자식노드를 new가 가리킴

							new->parent->child_head = NULL; //연결 끊음  
							new->parent = NULL; //연결 끊음
							new->prev = cur; 
							cur->next = new; //cur의 노드와 new노드 연결
						}
					}
					else if (!strcmp(new->name, "+") || !strcmp(new->name, "*")) //new->name이 "+"와 같거나 new->name이 "*"과 같으면
					{
						i = 0;

						while (1)
						{
							if (!strcmp(tokens[*idx + i], ""))
								break;

							if (is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0) //tokens[*idx + i]가 연산자이고 tokens[*idx + i]가 ")"와 일치하지 않는다면
								break;

							i++;
						}

						if (get_precedence(tokens[*idx + i]) < get_precedence(new->name)) //tokens[*idx + i]의 우선순위가 new->name의 우선순위보다 높으면
						{
							cur = get_last_child(cur); //마지막 자식 노드 리턴
							cur->next = new;
							new->prev = cur;
							cur = new; //cur가 new가 가리키는 값을 가짐
						}
						else
						{
							cur = get_last_child(cur);

							if (new->child_head != NULL) { ////child_head 가 NULL이 아니면 자식노드가  존재
								new = new->child_head; //자식노드를 new가 가리킴

								new->parent->child_head = NULL; //연결 끊음 (child_head null)
								new->parent = NULL; //(parent null)
								new->prev = cur;
								cur->next = new; //cur의 노드와 new 노드 연결
							}
						}
					}
					else {
						cur = get_last_child(cur); //마지막 자식 노드 리턴
						cur->next = new;
						new->prev = cur; //서로 연결
						cur = new; //cur가 new가 가리키는 값을 가짐
					}
				}

				else
				{
					cur = get_last_child(cur);  //마지막 자식 노드 리턴

					cur->next = new;
					new->prev = cur; //서로 연결

					cur = new; //cur가 new가 가리키는 값을 가짐
				}
			}
		}
		else if (is_operator(tokens[*idx])) //tokens[*idx]가 연산자면
		{
			if (!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
				|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&")
				|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*")) //tokens[*idx] 가 ||, &&, |, &, +, * 과 같으면
			{
				if (is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx])) //(cur->name 이 연산자) && (cur->name과 tokens[*idx]가 같으면)
					operator = cur; //operator가 cur가 가리키는 값을 가짐

				else
				{
					new = create_node(tokens[*idx], parentheses); //name 이랑 괄호만 있는 node를 new가 가리킴
					operator = get_most_high_precedence_node(cur, new); //prev, parent 끝까지한 가장 상위 노드를 가리킴

					if (operator->parent == NULL && operator->prev == NULL) { //최상위노드이면

						if (get_precedence(operator->name) < get_precedence(new->name)) { //operator->name의 우선순위가 new->name의 우선순위보다 높다면
							cur = insert_node(operator, new); //operator의 부모노드 new를 cur가 가르킴
						}

						else if (get_precedence(operator->name) > get_precedence(new->name)) //new->name의 우선순위가 operator->name의 우선순위보다 높다면
						{
							if (operator->child_head != NULL) { //operator의 자식노드가 존재한다면 
								operator = get_last_child(operator); //최종 자식노드를 구하여 operator가 가르킴
								cur = insert_node(operator, new); //operator의 부모노드 new를 cur가 가르킴
							}
						}
						else
						{
							operator = cur;

							while (1)
							{
								if (is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx])) //operator->name이 연산자이고 operator->name이 tokens[*idx]와 일치하면
									break;

								if (operator->prev != NULL) //operator에 이전노드가 존재한다면
									operator = operator->prev; //그 이전노드를 operator가 가르킴
								else
									break;
							}

							if (strcmp(operator->name, tokens[*idx]) != 0) //operator->name과 tokens[*idx]가 일치하지 않는다면
								operator = operator->parent; //operator의 부모노드를 operator가 가르킴

							if (operator != NULL) {
								if (!strcmp(operator->name, tokens[*idx])) //operator->name과 tokens[*idx]가 일치하면
									cur = operator; //cur도 operator를 가르킴
							}
						}
					}

					else //최상위 노드가 아니면
						cur = insert_node(operator, new); //operator의 부모노드인 new 노드를 cur가 가르킴
				}

			}
			else
			{
				new = create_node(tokens[*idx], parentheses); //tokens[*idx]가 name / parentheses가 parentheses 되는 노드 생성하고 new가 가르킴

				if (cur == NULL)
					cur = new;

				else
				{
					operator = get_most_high_precedence_node(cur, new); //가장 최상위(첫번째) 노드를 operator가 가르킴

					if (operator->parentheses > new->parentheses) //operator->parentheses의 계산이 new->parentheses보다 먼저면
						cur = insert_node(operator, new); //operator의 부모노드인 new노드를 cur가 가르킴

					else if (operator->parent == NULL && operator->prev == NULL) { //operator의 부모노드, 이전노드가 없을 때

						if (get_precedence(operator->name) > get_precedence(new->name)) //new->name의 우선순위가 operator->name보다 높을 때
						{
							if (operator->child_head != NULL) { //operator의 자식노드가 존재한다면

								operator = get_last_child(operator); //가장 마지막 자식노드를 operator가 가르킴
								cur = insert_node(operator, new); //가장 마지막 자식노드의 부모노드인 new를 cur가 가르킴
							}
						}

						else
							cur = insert_node(operator, new); //new 노드를 cur가 가르킴
					}

					else
						cur = insert_node(operator, new); //new 노드를 cur가 가르킴 
				}
			}
		}
		else
		{
			new = create_node(tokens[*idx], parentheses); //tokens[*idx]가 name , parentheses가 parentheses 되는 노드 생성하고 new가 가르킴

			if (cur == NULL)
				cur = new;

			else if (cur->child_head == NULL) { //자식노드가 더 이상 존재하지 않을 때
				cur->child_head = new; //자식노드를 만들어서 new가 가르킴
				new->parent = cur; //서로 연결

				cur = new; //new가 가르키는 노드의 값을 cur가 가짐
			}
			else {

				cur = get_last_child(cur); //가장 마지막 자식노드를 cur가 가르킴

				cur->next = new; 
				new->prev = cur; //서로연결

				cur = new; //new가 가르키는 값을 cur가 갖게됨
			}
		}

		*idx += 1;
	}

	return get_root(cur); //루트 리턴 후 끝
}

node* change_sibling(node* parent) //형제 노드로 변경
{
	node* tmp;

	tmp = parent->child_head; //입력받은 노드의 자식노드를 tmp가 가르킴

	parent->child_head = parent->child_head->next; 
	parent->child_head->parent = parent;
	parent->child_head->prev = NULL;

	parent->child_head->next = tmp;
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;
	parent->child_head->next->parent = NULL;

	return parent;
}

node* create_node(char* name, int parentheses)
{
	node* new;

	new = (node*)malloc(sizeof(node)); //노드 만듦
	new->name = (char*)malloc(sizeof(char) * (strlen(name) + 1)); //동적할당
	strcpy(new->name, name); //인자로 받은 name을 new->name에 복사

	new->parentheses = parentheses; //인자로 받은 parentheses를 new->parenthese에 넣음
	new->parent = NULL;
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;
	//나머지 NULL
	return new;
}

int get_precedence(char* op) //연산자 우선 순위 구함
{
	int i;

	for (i = 2; i < OPERATOR_CNT; i++) { //24번
		if (!strcmp(operators[i].operator, op))
			return operators[i].precedence;
	}
	return false;
}

int is_operator(char* op) //연산자 확인 함수
{
	int i;

	for (i = 0; i < OPERATOR_CNT; i++) //24
	{
		if (operators[i].operator == NULL)
			break;
		if (!strcmp(operators[i].operator, op)) { //operators[i].operator이랑 op랑 같으면 return true
			return true;
		}
	}

	return false;
}

void print(node* cur) //출력함수
{
	if (cur->child_head != NULL) { //cur의 자식노드가 존재한다면 출력
		print(cur->child_head); 
		printf("\n");
	}

	if (cur->next != NULL) { //cur의 다음노드가 존재한다면 출력
		print(cur->next);
		printf("\t");
	}
	printf("%s", cur->name); //cur가 가르키는 노드의 name 출력
}

node* get_operator(node* cur)
{
	if (cur == NULL)
		return cur;

	if (cur->prev != NULL) //cur이 가르키는 노드가 prev한 노드가 존재한다면
		while (cur->prev != NULL)
			cur = cur->prev; //가장 prev한 노드를 cur가 가르킴

	return cur->parent; //cur가 가르키는 노드의 부모노드 리턴
}

node* get_root(node* cur) 
{
	if (cur == NULL)
		return cur;

	while (cur->prev != NULL) //가장 prev한 노드로 이동
		cur = cur->prev;

	if (cur->parent != NULL) //최종 부모 노드 구함
		cur = get_root(cur->parent);

	return cur;
}

node* get_high_precedence_node(node* cur, node* new)
{
	if (is_operator(cur->name)) //cur->name 연산자 검사
		if (get_precedence(cur->name) < get_precedence(new->name)) //cur->name의 우선순위가 new->name의 우선순위보다 높으면 return cur
			return cur;

	if (cur->prev != NULL) { //cur가 가리키는 노드의 이전 노드가 있으면
		while (cur->prev != NULL) {
			cur = cur->prev; //cur가 이전 노드를 가리킴

			return get_high_precedence_node(cur, new); //제일 첫번째 노드
		}
		//제일 첫번째 노드로 가서

		if (cur->parent != NULL) //cur가 가리키는 노드의 부모노드가 있으면
			return get_high_precedence_node(cur->parent, new); //최상 부모 노드
	}

	if (cur->parent == NULL) //제일 첫번째 노드, 부모노드
		return cur;
}

node* get_most_high_precedence_node(node* cur, node* new)
{
	node* operator = get_high_precedence_node(cur, new); //prev, parent 최상 노드를 operator가 가리킴
	node* saved_operator = operator; //saved_operator도 opertator가 가리키는 곳을 가리킴

	while (1)
	{
		if (saved_operator->parent == NULL)
			break;

		if (saved_operator->prev != NULL) //NULL이 아니면 노드 다시 찾기
			operator = get_high_precedence_node(saved_operator->prev, new);

		else if (saved_operator->parent != NULL) //NULL이 아니면 노드 다시 찾기
			operator = get_high_precedence_node(saved_operator->parent, new);

		saved_operator = operator; //재설정
	}

	return saved_operator;
}

node* insert_node(node* old, node* new)
{
	if (old->prev != NULL) { //old가 가리키는 노드에 이전 노드가 있다면
		new->prev = old->prev; //new도 같은 이전 노드 가리킴
		old->prev->next = new; // 이전노드가 new 가리킴
		old->prev = NULL; //old랑 이전노드 연결 끊고
	}

	new->child_head = old; 
	old->parent = new; // old랑 new랑 부모자식 연결

	return new; //old->prew != NULL 이라면 (old 와 이전노드에 삽입된 new 노드 return) 아니면 new와 old 부모자식 연결하고 new return
}

node* get_last_child(node* cur) //최종 자식 노드 리턴
{
	if (cur->child_head != NULL)
		cur = cur->child_head;

	while (cur->next != NULL)
		cur = cur->next;

	return cur;
}

int get_sibling_cnt(node* cur)
{
	int i = 0;

	while (cur->prev != NULL)
		cur = cur->prev; //prev 끝까지 감

	while (cur->next != NULL) {
		cur = cur->next; //next 끝까지 가면서 i 증가
		i++;
	}

	return i; //i가 총 형제 개수
}

void free_node(node* cur)
{
	if (cur->child_head != NULL)
		free_node(cur->child_head);

	if (cur->next != NULL)
		free_node(cur->next);

	if (cur != NULL) {
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL;
		free(cur);
	}
}


int is_character(char c) //문자열 확인
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char* str)
{
	char* start;
	char str2[BUFLEN] = { 0 };
	char tmp[BUFLEN] = { 0 };
	char tmp2[BUFLEN] = { 0 };
	int i;

	start = str;
	strncpy(str2, str, strlen(str)); //str에서 str2로 str의 길이만큼 복사
	remove_space(str2); //공백제거

	while (start[0] == ' ') //공백 아닌 걸 만나면 멈춤
		start += 1;

	if (strstr(str2, "gcc") != NULL) //str2에서 gcc와 일치하는 부분이 있으면
	{
		strncpy(tmp2, start, strlen("gcc")); //start에서 tmp2로 gcc의 길이만큼 복사
		if (strcmp(tmp2, "gcc") != 0) //tmp2가 "gcc"와 같지 않으면 return 0
			return 0;
		else //tmp2가 "gcc"와 같으면(start도 gcc로 시작하면) return 2
			return 2;
	}

	for (i = 0; i < DATATYPE_SIZE; i++) //DATATYPE_SIZE 35
	{
		if (strstr(str2, datatype[i]) != NULL) //str2에서 datatype[i]와 일치하는 부분이 있으면
		{
			strncpy(tmp, str2, strlen(datatype[i])); //str2에서 tmp로 datatype[i]의 길이만큼 복사 (data형 길이만큼만)
			strncpy(tmp2, start, strlen(datatype[i])); //start에서 tmp2로 datatype[i]의 길이만큼 복사

			if (strcmp(tmp, datatype[i]) == 0) //tmp가 datatype[i]랑 같으면
				if (strcmp(tmp, tmp2) != 0) //tmp랑 datatype으로 시작하지 않으면 return 0 
					return 0;
				else //시작하면 return 2
					return 2;
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) 
{
	int i, j; //i를 TOKEN_CNT 50번 돌리면서 tokens[i] 중에 datatype이 있고 tokens[i-1]이 "(" && tokens[i+1]이 ")" && tokens[i+2][0]이 (& * ) ( - +) 중에 하나인 i 를 리턴

	for (i = 0; i < TOKEN_CNT; i++) //TOKEN_CNT 50
	{
		for (j = 0; j < DATATYPE_SIZE; j++) //DATATYPE_SIZE 35
		{
			if (strstr(tokens[i], datatype[j]) != NULL && i > 0) //tokens[i]에 datatype[i]가 있고 i>0이면
			{
				if (!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") //tokens[i-1]이 "("과 같고 tokens[i+1]이 ")"과 같고 tokens[i+2][0]이 (& * ) ( - +)이면 return i
					&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*'
						|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '('
						|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+'
						|| is_character(tokens[i + 2][0])))
					return i;
			}
		}
	}
	return -1;
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN])
{
	int i, j;


	for (i = 0; i < TOKEN_CNT; i++) //50
	{
		for (j = 0; j < DATATYPE_SIZE; j++) //35
		{
			if (!strcmp(tokens[i], "struct") && (i + 1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1])) //tokens[i]가 "struct"랑 같고 i+1이 50보다 작거나 같아야하고 tokens[i + 1][strlen(tokens[i + 1]) - 1])이 문자열인 i
				return i;
		}
	}
	return -1;
}

int all_star(char* str) //문자열이 전부 *인지 확인
{
	int i;
	int length = strlen(str);

	if (length == 0)
		return 0;

	for (i = 0; i < length; i++)
		if (str[i] != '*')
			return 0;
	return 1;

}

int all_character(char* str) //입력받은 문자열 모두 is_character 확인
{
	int i;

	for (i = 0; i < strlen(str); i++)
		if (is_character(str[i]))
			return 1;
	return 0;

}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN])
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if (start > -1) { 
		if (!strcmp(tokens[start], "struct")) { //tokens[start]가 "struct"이면 => datatype이 struct
			strcat(tokens[start], " "); //tokens[start]에 공백을 이어붙임
			strcat(tokens[start], tokens[start + 1]); //tokens[start]에 tokens[start+1]을 이어붙임

			for (i = start + 1; i < TOKEN_CNT - 1; i++) { //start+1 부터 49까지 돌리면서
				strcpy(tokens[i], tokens[i + 1]); //tokens[i+1] 을 tokens[i]에 복사함
				memset(tokens[i + 1], 0, sizeof(tokens[0])); //tokens[i+1]을 0으로 초기화
			}
		}
		//tokens[0]에 tokens내용 다 옮겨서 합치고 원래 tokens는 0으로 초기화
		else if (!strcmp(tokens[start], "unsigned") && strcmp(tokens[start + 1], ")") != 0) { //tokens[start]가 "unsigned"이고 tokens[start + 1]이 ")"가 아니면
			strcat(tokens[start], " "); //tokens[start]에 공백을 이어붙임
			strcat(tokens[start], tokens[start + 1]); //tokens[start]에 tokens[start + 1]을 이어붙임
			strcat(tokens[start], tokens[start + 2]); //tokens[start]에 tokens[start + 2]을 이어붙임

			for (i = start + 1; i < TOKEN_CNT - 1; i++) { //start+1 부터 49까지
				strcpy(tokens[i], tokens[i + 1]); //tokens[i+1] 을 tokens[i]에 복사함
				memset(tokens[i + 1], 0, sizeof(tokens[0])); //tokens[i+1]을 0으로 초기화
			}
		}

		j = start + 1;  
		while (!strcmp(tokens[j], ")")) { //tokens[j] 가 ")"일 동안
			rcount++; //rcount 증가
			if (j == TOKEN_CNT) //j == 50이면 break
				break;
			j++; //j 증가
		}
		// (datatype) => datatype이 start
		j = start - 1;
		while (!strcmp(tokens[j], "(")) { //tokens[j]가 "("일 동안
			lcount++; //lcount 증가
			if (j == 0) //j==0이면 break
				break; 
			j--; //j감소
		}
		if ((j != 0 && is_character(tokens[j][strlen(tokens[j]) - 1])) || j == 0) 
			lcount = rcount;

		if (lcount != rcount) //괄호 안맞으면 false
			return false;

		if ((start - lcount) > 0 && !strcmp(tokens[start - lcount - 1], "sizeof")) { //start 앞에 ( 하나있고 "sizeof"가 있으면 true
			return true;
		}

		else if ((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start + 1], ")")) { //tokens[start]가 "unsigned"거나 "struct"이고 tokens[start+1]이 ")"이면
			strcat(tokens[start - lcount], tokens[start]); //tokens[start]를 tokens[start - lcount]에 이어붙임
			strcat(tokens[start - lcount], tokens[start + 1]); //tokenas[start+1]를 tokens[start - lcount]에 이어붙임
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]); //tokens[start + rcount]를 tokens[start - lcount + 1]에 복사

			for (int i = start - lcount + 1; i < TOKEN_CNT - lcount - rcount; i++) { // ) 뒤에 있던 내용들 땡겨주고 뒤에 있던 내용들 초기화
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));
			}


		}
		else {
			if (tokens[start + 2][0] == '(') { //tokens[start+2][0] 이 ( 이면
				j = start + 2; // 그 부분이 j
				while (!strcmp(tokens[j], "(")) { //tokens[j]가 "("일 동안
					sub_lcount++;  //sub_lcout, j 증가
					j++;
				}
				if (!strcmp(tokens[j + 1], ")")) { //tokens[j+1]이 ")"이면
					j = j + 1; //한 칸 이동
					while (!strcmp(tokens[j], ")")) { //tokens[j]가 ")"일 동안
						sub_rcount++; //sub_rcount, j 증가
						j++;
					}
				}
				else
					return false;

				if (sub_lcount != sub_rcount) //괄호 짝 다르면 false
					return false;

				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]); //tokens[start + 2 + sub_lcount] 를 tokens[start + 2]로 복사
				for (int i = start + 3; i < TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0])); //초기화

			}
			strcat(tokens[start - lcount], tokens[start]); //tokens[start]를 tokens[start - lcount]에 이어붙임
			strcat(tokens[start - lcount], tokens[start + 1]); //tokens[start + 1]을 tokens[start - lcount]에 이어붙임
			strcat(tokens[start - lcount], tokens[start + rcount + 1]); //tokens[start + rcount + 1]을 tokens[start - lcount]에 이어붙임

			for (int i = start - lcount + 1; i < TOKEN_CNT - lcount - rcount - 1; i++) { //뒤에 있는 내용들 땡겨주고 뒤에 있던 내용들 초기화
				strcpy(tokens[i], tokens[i + lcount + rcount + 1]);
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0]));

			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN]) //토큰 0으로 clear
{
	int i;

	for (i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));
}

char* rtrim(char* _str) //우측 공백, 문자열 제거
{
	char tmp[BUFLEN];
	char* end;

	strcpy(tmp, _str);
	end = tmp + strlen(tmp) - 1;
	while (end != _str && isspace(*end))
		--end;

	*(end + 1) = '\0';
	_str = tmp;
	return _str;
}

char* ltrim(char* _str) //좌측 공백, 문자열 제거
{
	char* start = _str;

	while (*start != '\0' && isspace(*start))
		++start;
	_str = start;
	return _str;
}

char* remove_extraspace(char* str)
{
	int i;
	char* str2 = (char*)malloc(sizeof(char) * BUFLEN); //BUFLEN 1024
	char* start, * end;
	char temp[BUFLEN] = "";
	int position;

	if (strstr(str, "include<") != NULL) { //str에 "include<" 가 있으면
		start = str;
		end = strpbrk(str, "<"); //str에서 "<" 나오는 곳을 end가 가리킴
		position = end - start; //position은 처음 ~ < 전까지의 길이

		strncat(temp, str, position); //temp에 str의 position만큼 이어붙임
		strncat(temp, " ", 1); // temp에 공백 이어붙임
		strncat(temp, str + position, strlen(str) - position + 1); //temp에 str + position의 위치에서 str의길이 - position + 1 만큼 씀

		str = temp; //str이 temp 가리킴
	}

	for (i = 0; i < strlen(str); i++) //str의 길이만큼
	{
		if (str[i] == ' ') //공백 만나면
		{
			if (i == 0 && str[0] == ' ') //맨 처음이 공백이면
				while (str[i + 1] == ' ') //str[i+1]가 공백이 아닐 때까지 i를 증가
					i++;
			else { //맨 처음이 공백이 아니면
				if (i > 0 && str[i - 1] != ' ') //i-1번째가 공백이 아니면
					str2[strlen(str2)] = str[i]; //그 값을 str2에 넣음
				while (str[i + 1] == ' ') //str[i+1]가 공백이 아닐 때까지 i를 증가
					i++;
			}
		}
		else
			str2[strlen(str2)] = str[i]; //공백이 아니면 그 값을 str2에 넣음
	}

	return str2; //str 중에서 공백 아닌 값을 넣은 str2를 리턴
}



void remove_space(char* str)
{
	char* i = str;
	char* j = str;

	while (*j != 0) //j가 널문자를 가리키지 않을 동안
	{
		*i = *j++; //j가 가르켰던 걸 i가 가리킴
		if (*i != ' ') //i가 가리키는 게 공백이 아니라면 i를 증가
			i++;
	}//i가 공백을 만나면 j가 가리키고 있는 내용을 i가 가리키는 곳으로 땡겨서 공백 없앰
	*i = 0; //널문자 가리킴
}

int check_brackets(char* str)
{
	char* start = str;
	int lcount = 0, rcount = 0;

	while (1) {
		if ((start = strpbrk(start, "()")) != NULL) { //start중에서 처음으로 ()과 일치하는 문자 가르킴
			if (*(start) == '(') //(가 먼저 나오면 lcount 증가
				lcount++;
			else //)가 먼저 나오면 rcount 증가
				rcount++;

			start += 1;
		}
		else
			break;
	}

	if (lcount != rcount)
		return 0;
	else
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]) //토큰 개수 셈
{
	int i;

	for (i = 0; i < TOKEN_CNT; i++)
		if (!strcmp(tokens[i], ""))
			break;

	return i;
}

