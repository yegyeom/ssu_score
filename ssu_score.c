#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char cIDs[ARGNUM][FILELEN];
char iIDs[ARGNUM][FILELEN];

int mOption = false;
int eOption = false;
int tOption = false;
int iOption = false;

void ssu_score(int argc, char* argv[])
{
	char filename[FILELEN];
	char filename2[FILELEN];
	char saved_path[BUFLEN];
	int i,j;
	
	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-h")) { //입력 받은 인자들 중 -h가 있으면 print_usage 함수 호출
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN); //saved_path에 BUFLEN크기만큼 0을 채움
	if (argc >= 3 && strcmp(argv[1], "-i") != 0) { //argc가 3개 이상이고 첫번째인자가 -i가 아니면
		strcpy(stuDir, argv[1]); //argv[1]를 stuDir에 복사
		strcpy(ansDir, argv[2]); //argv[2]를 ansDir에 복사
	}

	if(!strcmp(argv[1], "-i")){ //argv[1]이 -i 이면 iOption 실행

		while(i < argc && argv[i][0] != '-'){ //i는 argc보다 작고 argv[i][0]이 -가 아닐 동안

			if(j >= ARGNUM) //인자를 최대로 받았을 때
				printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
			else 
				strcpy(iIDs[j], argv[i]); //argv[i] (입력받은 학번을) iIDs에 복사
			i++;
			j++;
		}

		do_iOption(iIDs);

		return;
	}

	if (!check_option(argc, argv)) //false면 종료 
		exit(1);
	
	getcwd(saved_path, BUFLEN); //현재 작업디렉토리의 경로를 BUFLEN만큼 saved_path에 복사 

	if (chdir(stuDir) < 0) { //stuDir로 디렉토리 변경
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}

	getcwd(stuDir, BUFLEN); //현재 작업디렉토리의 경로를 BUFLEN만큼 stuDir에 복사

	chdir(saved_path); //saved_path로 디렉토리 변경
	if (chdir(ansDir) < 0) { //ansDir로 디렉토리 변경
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN); //현재 작업디렉토리의 경로를 BUFLEN만큼 ansDir에 복사

	chdir(saved_path); //saved_path로 디렉토리 변경

	set_scoreTable(ansDir); //score table 설정
	set_idTable(stuDir); //stuDir의 id를 idtable에 입력

	sprintf(filename, "%s", "score_table.csv");
	if(mOption) //-m 옵션 실행
		do_mOption(filename);
	
	printf("grading student's test papers..\n");
	score_students(); //채점 결과 테이블 만들고 학생들 학번 모두 입력

	if(iOption) //-i 옵션 실행
		do_iOption(iIDs);

	return;
}

int check_option(int argc, char* argv[])
{
	int i, j;
	int c;

	while ((c = getopt(argc, argv, "e:mith")) != -1) //파라미터 처리 
	{
		switch (c) {
			case 'm': // -m : 채점 전에 원하는 문제의 점수 수정 score_table.csv 에 문제 점수 변경
			mOption = true;
			
			break;
		case 'e': // -e [DIRNAME] : DIRNAME/학번/문제번호_error.txt에 에러 메시지 출력
			eOption = true;
			strcpy(errorDir, optarg); //옵션 뒤에 별도의 파라미터가 오는 경우 이를 파싱한 결과 파라미터 값이 optarg에 저장 (여기선 DIRNAME) 그걸 errorDir에 복사 

			if (access(errorDir, F_OK) < 0) //errorDir이 존재하지 않으면 모드 0755로 만듦
				mkdir(errorDir, 0755);
			else { //errorDir이 존재한다면
				rmdirs(errorDir); //errorDir read
				mkdir(errorDir, 0755); //모드 755로 errorDir을 만듦
			}
			break;
		case 't': // -t [QNAMES] : QNAME을 문제 번호로 하는 문제는 컴파일시 -lpthread 옵션 추가
			tOption = true;
			i = optind; //다음번 처리될 옵션의 인덱스
			j = 0;

			while (i < argc && argv[i][0] != '-') { //i는 argc보다 작고 argv[i][0]이 '-'가 아닐 동안

				if (j >= ARGNUM) //인자를 최대로 받았을 때
					printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
				else //받을 수 있으면 argv[i]를 threadFiles[j]에 복사
					strcpy(threadFiles[j], argv[i]);
				i++;
				j++;
			}
			break;
		case 'i': // -i : 채점결과 파일이 있는 경우 해당 학생들의 틀린문제 파일 출력
			iOption = true;
			i = optind;
			j = 0;

			while(i < argc && argv[i][0] != '-'){ //i는 argc보다 작고 argv[i][0]이 -가 아닐 동안

				if(j >= ARGNUM) //인자를 최대로 받았을 때
					printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
				else 
					strcpy(iIDs[j], argv[i]); //argv[i] (입력받은 학번을) iIDs에 복사
				i++;
				j++;
			}
			break;
		case '?':
			printf("Unkown option %c\n", optopt);
			return false;
		}
	}

	return true;
}

void do_iOption(char(*ids)[FILELEN]){ //옵션과 함께 입력한 학번 들어옴
	FILE* fp;
	char tmp1[BUFLEN];
	char tmp2[BUFLEN];
	char *qname[BUFLEN] = {NULL, };
	char *qscore[BUFLEN] = {NULL, };
	int i,k = 0;
	int count = 0;

	if((fp = fopen("score.csv", "r")) == NULL){ //채점결과테이블 오픈
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}
	
	fscanf(fp, "%s\n", tmp1);//파일 score.csv의 문자열 읽어서 tmp에 입력(첫번째 열)

	char *ptr = strtok(tmp1, ","); //콤마 기준으로 tmp1 자름
	
	while(ptr != NULL){ //자른 문자열이 나오지 않을 때까지 반복
		qname[i] = ptr; 
		i++;

		ptr = strtok(NULL, ","); 
	} //0행의 문제번호들과 sum을 qname배열에 입력

	while(fscanf(fp, "%s\n", tmp2) != EOF){ //다음 줄 문자열 tmp2에 입력
		count=0;
		char *p = strtok(tmp2, ","); //콤마 기준으로 tmp2 자름 (학번)
		
		if(!is_exist(ids, tmp2))//입력 받은 학번이 tmp2에 존재하지 않으면 while문 다시
			continue;
		
		printf("%s's wrong answer : ", tmp2);
		
		k=0;
		while(p != NULL){ //자른 문자열이 나오지 않을 때까지 반복
			qscore[k] = p;
			k++;

			p = strtok(NULL, ",");
		}	//학번 ~ 점수들을 qscore배열에 입력
	
		
		for(int j = 1 ; j < k  ; j++){
			if(!strcmp(qscore[j], "0")){ //qscore 배열 내용들 중 0이 있으면 그에 해당하는 문제번호 출력
				
				count++;
				if(count == 1)
					printf("%s", qname[j-1]);
				else 
					printf(",   %s", qname[j-1]);
			}

		}
		printf("\n");
	}
	fclose(fp);
}

void do_mOption(char* filename) {
	int num = sizeof(score_table) / sizeof(score_table[0]); 
	int fd, length;
	char qname[BUFLEN];
	char tmp[BUFLEN];
	char name[BUFLEN];
	char buf[BUFLEN];
	char modify[BUFLEN];
	char test[BUFLEN];
	double new;
	int i,j;
	off_t currpos;
	

	if ((fd = open(filename, O_RDWR)) < 0) { //점수테이블 오픈
		fprintf(stderr, "file open error for score_table.csv\n");
		exit(1);
	}

	if ((read(fd, buf, sizeof(score_table))) < 0) {
		fprintf(stderr, "file read error for score.csv\n");
		exit(1);
	}
	
	while (1) {
		printf("Input question's number to modify >> ");
		scanf("%s", qname); //점수 수정 원하는 문제 번호 입력

		if (!strcmp(qname, "no")) //입력받은 문자열이 no라면 while문 종료
			break;
		
		for (i = 0; i < num; i++) {
			if(score_table[i].score == 0){ //입력한 문제 번호가 없는 문제일 경우
				printf("error : the file doesn't exist\n");
				break;
			}

			memset(test, 0, sizeof(test));
			memcpy(test, score_table[i].qname, strlen(score_table[i].qname) - strlen(strchr(score_table[i].qname, '.'))); //score_table.qname에서 확장자명 제외한 길이만큼 test에 복사
			if(!strcmp(qname, test)){ //입력받은 qname과 test가 같다면
				lseek(fd, 0, SEEK_SET);
				printf("Current score : %.2lf", score_table[i].score); //현재 점수 출력
				
				for (j = 0 ; j < i; j++) { //i번째 문제 전에 있는 문제들의 길이를 구하고
					memset(tmp, 0, sizeof(tmp));
					sprintf(tmp, "%s,%.2f\n", score_table[j].qname, score_table[j].score);
					currpos = lseek(fd, strlen(tmp), SEEK_CUR); //그 길이만큼 오프셋 이동
				}

				lseek(fd, 0, SEEK_CUR);

				printf("\nNew score : ");
				scanf("%lf", &new); //새로운 점수 입력 받음

				memset(modify, 0, sizeof(modify));
				sprintf(modify, "%s,%.2f", score_table[i].qname, new); //새로 입력할 내용을 modify에 저장
				pwrite(fd, modify, strlen(modify), currpos); //파일의 currpos위치에 modify를 write해줌
				break;
			}
		
		}
	}
	close(fd);
}

int is_exist(char(*src)[FILELEN], char* target){
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM) //i가 최대인자 수보다 크면 false
			return false;
		else if(!strcmp(src[i], "")) //내용이 없다면 false
			return false;
		else if(!strcmp(src[i++], target)) //ids와 target가  같다면 true
			return true;
	}
	return false;
}

void set_scoreTable(char* ansDir) //답안 디렉토리가 들어왔을 때 score table을 설정
{
	char filename[FILELEN];

	sprintf(filename, "%s", "score_table.csv"); //점수테이블을 filename 에 저장

	if (access(filename, F_OK) == 0) //filename 파일이 존재하면
		read_scoreTable(filename); 
	else { //filename 파일 존재하지 않으면
		make_scoreTable(ansDir); 
		write_scoreTable(filename);  
	}

}

void read_scoreTable(char* path) 
{
	FILE* fp;
	char filename[FILELEN];
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;
	sprintf(filename, "%s", path);

	if ((fp = fopen(path, "r")) == NULL) { //점수 테이블을 읽기 전용으로 오픈
		fprintf(stderr, "file open error for %s\n", path);
		return;
	}

	while (fscanf(fp, "%[^,],%s\n", qname, score) != EOF) { //점수테이블을 콤마 단위로 읽어서 qname과 score변수에 입력
		strcpy(score_table[idx].qname, qname); //qname을 score_table[idx].qname에 복사
		score_table[idx++].score = atof(score); //score을 실수로 변환하여 score_table[idx++].score에 넣음
	}

	fclose(fp);
}

void make_scoreTable(char* ansDir) //점수테이블(문제번호, 점수)이 없을 경우 만드는 함수
{
	int type, num;
	double score, bscore, pscore;
	struct dirent* dirp, * c_dirp;
	DIR* dp, * c_dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type(); //점수 입력 타입 결정

	if (num == 1) //1. input blank question and program question's score. ex) 0.5 1  
	{
		printf("Input value of blank question : "); //빈칸 채우기 문제의 점수 입력하고 bscore에 저장
		scanf("%lf", &bscore);
		printf("Input value of program question : "); //프로그램 문제의 점수 입력하고 pscore에 저장
		scanf("%lf", &pscore);
	}
	
	if ((dp = opendir(ansDir)) == NULL) { //ansDir 디렉토리 오픈하고 이를 dp가 가르킴
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}

	while ((dirp = readdir(dp)) != NULL) //ansDir 디렉토리 read, 정보가 리턴되어 dirp로
	{
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //dirp->d_name이 "." 거나 ".."이면 다시 while문
			continue;
		
		if((type = get_file_type(dirp->d_name)) < 0) //점수 입력 타입이 올바르지 않으면 다시 while문
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name); //dirp->d_name을 score_table[idx++].qname으로 복사
	} 

	closedir(dp);
	sort_scoreTable(idx); //scoretable sorting

	for (i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname); //점수테이블에 있는 문제들의 타입 (TEXTFILE or CFILE)을 리턴받음

		if (num == 1) //1. input blank question and program question's score. ex) 0.5 1 
		{
			if (type == TEXTFILE) //빈칸 문제면
				score = bscore; //bscore에 입력 받았던 값을 score로
			else if (type == CFILE) //프로그램 문제면
				score = pscore; //pscore에 입력 받았던 값을 score로
		}
		else if (num == 2) //2. input all question's score. ex) Input value of 1-1: 0.1
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score); //문제들 점수 입력
		}

		score_table[i].score = score; //입력 받은 점수들을 점수 테이블의 점수란에 저장
	} 
}

void write_scoreTable(char* filename) //점수테이블 인자로 받음
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]); //score_table의 항목 갯수

	if ((fd = creat(filename, 0666)) < 0) { //모드 666으로 filename creat함
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for (i = 0; i < num; i++) //score_table 개수만큼 돌면서
	{
		if (score_table[i].score == 0) //점수 0이면 break
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score); //score_table의 문제번호,점수 를 tmp로 저장
		write(fd, tmp, strlen(tmp)); //fd(점수테이블)에 tmp(문제번호, 점수) write
	}

	close(fd);
}


void set_idTable(char* stuDir) //학생들이 제출한 답안 디렉토리 인자로 받음
{
	struct stat statbuf;
	struct dirent* dirp;
	DIR* dp;
	char tmp[BUFLEN];
	int num = 0;

	if ((dp = opendir(stuDir)) == NULL) { //stuDir 디렉토리 오픈
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}
	
	while ((dirp = readdir(dp)) != NULL) { //stuDir 디렉토리 read, 정보가 리턴되어 dirp로
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //dirp->d_name이 "." 거나 ".."이면 다시 while문
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); //stuDir/dirp->dname tmp로 저장하고 출력
		stat(tmp, &statbuf); //tmp정보가 stat구조체를 통해 리턴됨

		if (S_ISDIR(statbuf.st_mode)) //tmp(stuDir/dirp->d_name)가 디렉토리이면
			strcpy(id_table[num++], dirp->d_name); //dirp->d_name을 id_table[num++]에 복사
		else
			continue;
	}

	sort_idTable(num); //id table 정렬
}

void sort_idTable(int size) //id table 정렬
{
	int i, j;
	char tmp[10];

	for (i = 0; i < size - 1; i++) { //항목 갯수만큼 돌면서
		for (j = 0; j < size - 1 - i; j++) {
			if (strcmp(id_table[j], id_table[j + 1]) > 0) { //id_table[j+1]가 id_table[i]보다 작다면 
				strcpy(tmp, id_table[j]); //id_table[i]를 tmp에 복사
				strcpy(id_table[j], id_table[j + 1]); //id_table[j+1]을 id_table[j]로 복사
				strcpy(id_table[j + 1], tmp); //tmp를 id_table[j+1]로 복사
			} 
		}
	}//순서 올바르게 정렬
}

void sort_scoreTable(int size) //score table 정렬
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for (i = 0; i < size - 1; i++) { //항목 갯수만큼 돌면서
		for (j = 0; j < size - 1 - i; j++) {

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j + 1].qname, &num2_1, &num2_2);
			//score_table의 qname이 -.기준으로 상수로 전환됨

			if ((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))) { //num1_1 >= num2_1 %% num1_2 > num2_2 이면

				memcpy(&tmp, &score_table[j], sizeof(score_table[0])); //score_table[j]가 가르키는 곳부터 score_table[0]크기만큼의 내용을 tmp가 가르키는 곳에 복사함
				memcpy(&score_table[j], &score_table[j + 1], sizeof(score_table[0])); //score_table[j+1]가 가르키는 곳부터 score_table[0]크기만큼의 내용을 score_table[j]가 가르키는 곳에 복사
				memcpy(&score_table[j + 1], &tmp, sizeof(score_table[0])); //tmp가 가르키는 곳부터 score_table[0]크기만큼의 내용을 score_table[j+1]가 가르키는 곳에 복사
			}
		}
	}
}

void get_qname_number(char* qname, int* num1, int* num2)
{
	char* p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); //qname에 있는 문자열을 dup으로 복사하는데 크기는 qname만큼
	*num1 = atoi(strtok(dup, "-.")); //dup을 -.을 기준으로 자른 문자열을 상수로 전환

	p = strtok(NULL, "-."); //더 이상 나오지 않을 때까지 -.로 자름
	if (p == NULL)
		*num2 = 0;
	else //상수로 전환
		*num2 = atoi(p);
}

int get_create_type() //score table 만들 때 점수 입력 타입
{
	int num;

	while (1)
	{
		printf("score_table.csv file doesn't exist!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n"); //빈칸문제 프로그램문제 점수 입력
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n"); //하위 문제 점수
		printf("select type >> ");
		scanf("%d", &num);

		if (num != 1 && num != 2) //올바른 번호 입력이 아닐 경우
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students()
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]); //size는 id table 항목 갯수

	if ((fd = creat("score.csv", 0666)) < 0) { //채점 결과 테이블을 0666모드로 만듦
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd); //채점 결과 테이블 1행에 문제번호, sum 써놓음

	for (num = 0; num < size; num++)
	{
		if (!strcmp(id_table[num], "")) //내용이 없다면 break
			break;

		sprintf(tmp, "%s,", id_table[num]); //tmp에 id_table[num]을 저장
		write(fd, tmp, strlen(tmp)); //score.csv에 tmp(id_table[num])을 write 
		//학생들의 학번을 채점 결과 테이블에 입력 
		score += score_student(fd, id_table[num]); //학생들 점수 구해서 다 더함
	}

	printf("Total average : %.2f\n", score / num); //학생들 평균 출력

	close(fd);
}

double score_student(int fd, char* id)  //채점결과테이블과 id_table에 있는 학번을 인자로 받음
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //size는 score_table 항목 갯수

	for (i = 0; i < size; i++)
	{
		if (score_table[i].score == 0) //0점 있으면 break
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); //tmp에 stuDir/id(학번)/score_tabe[i].qname(문제번호)로 저장

		if (access(tmp, F_OK) < 0) //tmp 파일 존재하지 않으면 false
			result = false;
		else
		{
			if ((type = get_file_type(score_table[i].qname)) < 0) //문제별 type 확인
				continue;

			if (type == TEXTFILE) //빈칸 문제면 
				result = score_blank(id, score_table[i].qname); //false or true
			else if (type == CFILE) //프로그램 문제면
				result = score_program(id, score_table[i].qname); //false or true
		}

		if (result == false)
			write(fd, "0,", 2); //fd가 가르키는 곳(채점 결과 테이블)에 2만큼 "0" write
		else {
			if (result == true) {
				score += score_table[i].score; //점수 더함
				sprintf(tmp, "%.2f,", score_table[i].score); //tmp에 점수 저장
			}
			else if (result < 0) { //warning이면 warning 수 만큼 감점 (하나 당 -0.1점)
				score = score + score_table[i].score + result;
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp)); //fd가 가르키는 곳(채점 결과 테이블)에 tmp길이만큼 tmp write
		}
	}

	printf("%s is finished.. score : %.2f\n", id, score); //학번과 점수 출력

	sprintf(tmp, "%.2f\n", score); //score를 tmp로 저장
	write(fd, tmp, strlen(tmp)); //fd에 tmp길이만큼 tmp write

	return score;
}

void write_first_row(int fd) //fd는 채점결과테이블
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //table 항목 갯수

	write(fd, ",", 1); //score.csv(채점 결과 테이블) 에 1만큼 , write

	for (i = 0; i < size; i++) {
		if (score_table[i].score == 0) //score table에 0점이 있다면 break
			break;

		sprintf(tmp, "%s,", score_table[i].qname); //tmp에 score_table[i].qname(문제번호)을 다 저장
		write(fd, tmp, strlen(tmp)); //score.csv(채점결과테이블)에 tmp(score_table[i].qname)를 write  
	}
	write(fd, "sum\n", 4); //score.csv에 4만큼 sum\n write
}

char* get_answer(int fd, char* result) //학생의 답안을 버퍼에 넣어서 리턴
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN); //result에 buflen만큼 0을 채움
	while (read(fd, &c, 1) > 0) //fd로부터 1만큼 읽어서 c에 입력
	{
		if (c == ':') // 콜론 : 만나면 break
			break;

		result[idx++] = c; //버퍼(result)에 fd의 내용 넣음
	}
	if (result[strlen(result) - 1] == '\n') //개행문자를 널문자로 바꿔줌
		result[strlen(result) - 1] = '\0';

	return result; //result는 fd의 내용을 채운 것 (콜론 나오면 끝, 개행문자는 널로 바꿈)
}

int score_blank(char* id, char* filename) //인자로 id와 score_table[i].qname을 인자로 받음 => 학번, 문제번호
{
	char tokens[TOKEN_CNT][MINLEN];
	node* std_root = NULL, * ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	memset(qname, 0, sizeof(qname)); //qname에 0을 sizeof(qname)만큼 채움
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //filename(문제번호)을 길이만큼 복사해서 확장자 제외하고 qname에 넣음

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename); //tmp에 stuDir/id/filename(문제번호 확장자 O)로 저장
	fd_std = open(tmp, O_RDONLY); //tmp를 읽기 전용으로 오픈 -> 파일 디스크립터 fd_std
	strcpy(s_answer, get_answer(fd_std, s_answer)); //fd_std의 내용으로 채운 버퍼(result)를 s_answer에 복사 (근데 fd_std에 콜론 나오면 콜론 전까지만, 개행문자는 널문자로 바뀜)

	if (!strcmp(s_answer, "")) { //내용이 없으면 false
		close(fd_std);
		return false;
	}

	if (!check_brackets(s_answer)) { // 괄호 짝 안맞으면 close
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer))); //s_answer의 좌측, 우측 공백 제거

	if (s_answer[strlen(s_answer) - 1] == ';') { //s_answer의 맨마지막이 세미콜론(;)이면 
		has_semicolon = true; //true로 바뀜
		s_answer[strlen(s_answer) - 1] = '\0'; //맨 마지막에 널문자 넣어줌
	}

	if (!make_tokens(s_answer, tokens)) { // 답안인 문자열을 token으로 분리
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0); //루트(std_root) 리턴 후 끝

	sprintf(tmp, "%s/%s", ansDir, filename); //andDir/qname/filename으로 tmp에 저장
	fd_ans = open(tmp, O_RDONLY); //tmp 읽기 전용으로 오픈하고 fd_ans가 가르킴

	while (1)
	{
		ans_root = NULL;
		result = true;

		for (idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx])); //tokens[idx]를 0으로 초기화

		strcpy(a_answer, get_answer(fd_ans, a_answer)); //fd_ans의 내용으로 채워진 a_answer를 a_answer에 복사

		if (!strcmp(a_answer, "")) //a_answer에 내용이 없다면 break
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer))); //공백 제거한 a_answer을 a_answer에 복사

		if (has_semicolon == false) {
			if (a_answer[strlen(a_answer) - 1] == ';') //a_anwer의 마지막에 세미콜론 삽입
				continue;
		}

		else if (has_semicolon == true)
		{
			if (a_answer[strlen(a_answer) - 1] != ';') //문자열의 마지막이 세미콜론이라면
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0'; //널문자 넣어줌
		}

		if (!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0); //트리 만들고 루트(ans_root) 리턴

		compare_tree(std_root, ans_root, &result); //std_root와 ans_root 비교

		if (result == true) {
			close(fd_std);
			close(fd_ans);
			//free node해줌
			if (std_root != NULL)
				free_node(std_root);
			if (ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}

	close(fd_std);
	close(fd_ans);
	//free node해줌
	if (std_root != NULL)
		free_node(std_root);
	if (ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char* id, char* filename)
{
	double compile;
	int result;

	compile = compile_program(id, filename); //프로그램 컴파일

	if (compile == ERROR || compile == false) //컴파일 후 에러 났으면 false
		return false;

	result = execute_program(id, filename); //프로그램 실행 result는 true or false

	if (!result)
		return false;

	if (compile < 0) //false면
		return compile;

	return true;
}

int is_thread(char* qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]); //threadFiles의 갯수

	for (i = 0; i < size; i++) {
		if (!strcmp(threadFiles[i], qname)) //threadFiles[i]랑 qname이랑 같으면 true
			return true;
	}
	return false;
}

double compile_program(char* id, char* filename) 
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname)); //qname을 0으로 채움
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //filename을 길이만큼 복사해서 qname에 넣음 근데 길이가 filename (처음 ~ . 나올 때까지) => 확장자명빼고

	isthread = is_thread(qname); //qname이랑 threadFile이랑 같으면 true 아니면 false

	sprintf(tmp_f, "%s/%s", ansDir, filename); //tmp_f에 ansDir/filename 저장 (확장자명 O)
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname); //tmp_e에 ansDir/qname.exe 저장 (확장자명 X)

	if (tOption && isthread) //t option 있으면
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //gcc -o tmp_e tmp_f -lpthread로 command에 저장
	else //t option 없으면
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //gcc -o tmp_e tmp_f로 command에 저장

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname); //ansDir/qname_error.txt으로 tmp_e에 저장
	fd = creat(tmp_e, 0666); //tmp_e 파일을 666모드로 만듦

	redirection(command, fd, STDERR); //command 실행하고 STDERR error문구를 fd가 가리키는 tmp_e에 출력 
	size = lseek(fd, 0, SEEK_END); //tmp_e의 파일 사이즈
	close(fd);
	unlink(tmp_e); //tmp_e 파일 삭제

	if (size > 0) //아직 남아 있으면 false
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); //tmp_f에 stuDir/id/filename 저장 (확장자명 O)
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname); //tmp_e에 stuDir/id/qname 저장 (확장자명 X)

	if (tOption && isthread) //t option
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //gcc -o tmp_e tmp_s -lpthread로 command에 저장
	else //t option 없으면
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //gcc -o tmp_e tmp_f로 command에 저장

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname); //stuDir/id/qname_error.txt로 tmp_f에 저장
	fd = creat(tmp_f, 0666); //tmp_f 파일을 666모드로 만듦

	redirection(command, fd, STDERR); //command 실행하고 STDERR error문구를 fd가 가리키는 tmp_f에 출력
	size = lseek(fd, 0, SEEK_END); //tmp_f의 파일 사이즈
	close(fd);

	if (size > 0) {
		if (eOption) //e option
		{
			sprintf(tmp_e, "%s/%s", errorDir, id); //errorDir/id로 tmp_e에 저장
			if (access(tmp_e, F_OK) < 0) //tmp_e 파일이 존재하지 않는다면 모드 755로 mkdir
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname); //errorDir/id/qname_error.txt로 tmp_e에 저장
			rename(tmp_f, tmp_e); //tmp_f에서 tmp_e로 이름 변경

			result = check_error_warning(tmp_e); //error warning 체크
		}
		else { //e option 없을 때
			result = check_error_warning(tmp_f); //error warning 체크
			unlink(tmp_f); //tmp_f 파일 삭제
		}

		return result;
	}

	unlink(tmp_f); //tmp_f 파일 삭제
	return true;
}

double check_error_warning(char* filename)
{
	FILE* fp;
	char tmp[BUFLEN];
	double warning = 0;

	if ((fp = fopen(filename, "r")) == NULL) { //filename 오픈 , 파일디스크립터 fp
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while (fscanf(fp, "%s", tmp) > 0) { //tmp를 입력받아서 fp에 입력함
		if (!strcmp(tmp, "error:")) //tmp랑 "error:" 랑 같으면 에러 리턴
			return ERROR; //0
		else if (!strcmp(tmp, "warning:")) //tmp랑 "warning:" 이랑 같으면 
			warning += WARNING; //warning에 숫자 WARNING(-0.1) 넣음 
	}

	return warning;
}

int execute_program(char* id, char* filename)
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname)); //qname을 0으로 채움
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //filename을 길이만큼 복사해서 qname에 넣음 근데 길이가 filename (처음 ~ . 나올 때까지) => 확장자명 빼고

	sprintf(ans_fname, "%s/%s.stdout", ansDir,qname); // ansDir/qname.stdout로 ans_fname에 저장
	fd = creat(ans_fname, 0666); //ans_fname파일 모드 666으로 만듦

	sprintf(tmp, "%s/%s.exe", ansDir, qname); // ansDir/qname.exe로 tmp에 저장
	redirection(tmp, fd, STDOUT); // tmp실행하고 표준출력하려던 걸 fd가 참조하는 곳(ans_fname)에 씀 
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname); // stuDir/id/qname.stdout로 std_fname에 저장
	fd = creat(std_fname, 0666); //std_fname파일 모드 666으로 만듦

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); //stuDir/id/qname.stdexe & 로 tmp에 저장

	start = time(NULL);
	redirection(tmp, fd, STDOUT); //tmp실행하고 표준출력하려던 걸 fd가 참조하는 곳 (std_fname)에 씀

	sprintf(tmp, "%s.stdexe", qname); //qname.stdexe로 tmp에 저장
	while ((pid = inBackground(tmp)) > 0) { //프로세스번호가 0보다 크면
		end = time(NULL);

		if (difftime(end, start) > OVER) { //실행시간이 5초 초과면
			kill(pid, SIGKILL); //프로세스 무조건 종료
			close(fd);
			return false;
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname); //stuDir/id/qname.stdout , ansDir/qname.stdout 같으면 true 다르면 false
}

pid_t inBackground(char* name) // .stdexe 파일
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;

	memset(tmp, 0, sizeof(tmp)); //tmp 0으로 채움
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); //background.txt 파일 읽기쓰기,모드0666으로 만듦

	sprintf(command, "ps | grep %s", name); //ps | grep 은 백그라운드로 실행되는 프로그램을 쉽게 캐치하고자 사용
	redirection(command, fd, STDOUT); //ps | grep 실행, 실행결과(표준출력하려는 걸) fd에 씀

	lseek(fd, 0, SEEK_SET); //offset 맨처음에 위치
	read(fd, tmp, sizeof(tmp)); //fd가 가르키는 txt파일을 tmp사이즈만큼 읽어서 tmp에 넣어놓음

	if (!strcmp(tmp, "")) { //tmp가 비었다면
		unlink("background.txt"); //파일삭제, 연결계수를 1 줄임
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " ")); //tmp문자열을 공백을 기준으로 자름 그걸 상수로 변환해서 pid에 넣음
	close(fd);

	unlink("background.txt"); //파일 삭제
	return pid; //ps | grep 후 출력 된 프로세스 번호 리턴
}

int compare_resultfile(char* file1, char* file2) //stuDir/id/qname.stdout , ansDir/qname.stdout (답안, 정답 실행결과 들어옴)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;
	//읽기 전용으로 파일 오픈 각각 fd1, fd2가 가르킴
	fd1 = open(file1, O_RDONLY); //학생 답 실행 결과
	fd2 = open(file2, O_RDONLY); //답안 실행 결과

	while (1)
	{ //size 1씩 비교함
		while ((len1 = read(fd1, &c1, 1)) > 0) { //fd1이 가르키는 곳을 1만큼 읽고 c1에 저장 , 읽은 바이트 수는 len1
			if (c1 == ' ') //공백이면 다시
				continue;
			else
				break;
		}
		while ((len2 = read(fd2, &c2, 1)) > 0) { //fd2가 가리키는 곳을 1만큼 읽고 c2에 저장 , 읽은 바이트 수는 len2
			if (c2 == ' ') //공백이면 다시
				continue;
			else
				break;
		}

		if (len1 == 0 && len2 == 0) //두 파일 모두 read후 파일의 끝에 도달한 경우 break
			break;
		//c1, c2 모두 소문자로
		to_lower_case(&c1);
		to_lower_case(&c2);

		if (c1 != c2) { //학생답이랑 답안이랑 다르면 return false
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;
}

void redirection(char* command, int new, int old)
{
	int saved;

	saved = dup(old); //old를 saved도 복사해서 가짐
	dup2(new, old); //old의 fd를 new의 fd로 바꿈

	system(command); //command실행

	dup2(saved, old); //saved가 참조하는 곳을 old도 참조함
	close(saved);
}

int get_file_type(char* filename) //파일의 타입 (c or txt)를 리턴해줌
{
	char* extension = strrchr(filename, '.'); //filename의 끝에서부터 .이 나온 첫번째 문자열 포인터를 리턴

	if (!strcmp(extension, ".txt")) //찾은 첫번째 .가 .txt면 TEXTFILE을 리턴
		return TEXTFILE;
	else if (!strcmp(extension, ".c")) //찾은 첫번째 .가 .c면 CFILE을 리턴
		return CFILE;
	else
		return -1;
}

void rmdirs(const char* path) //디렉토리, 디렉토리 아래의 파일들 모두 삭제
{
	struct dirent* dirp;
	struct stat statbuf;
	DIR* dp;
	char tmp[257];

	if ((dp = opendir(path)) == NULL) //인자로 들어온 디렉토리 오픈
		return;

	while ((dirp = readdir(dp)) != NULL) //오픈한 디렉토리 read
	{
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))//dirp->d_name이 "." 이거나 dirp->d_name이 ".."이면 반복문 다시
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name); ////path/dirp->d_name 으로 tmp 에 저장

		if (lstat(tmp, &statbuf) == -1) //심볼릭 링크의 파일을 tmp로 넘김
			continue;

		if (S_ISDIR(statbuf.st_mode)) //디렉토리이면 디렉토리 삭제 , 디렉토리가 아니면 unlink해준다
			rmdirs(tmp);
		else
			unlink(tmp);
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char* c) //소문자로 바뀜
{
	if (*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage() //h옵션 받았을 때 사용법 출력
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify score before scoring\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -i                print ID's wrong questions\n");
	printf(" -h                print usage\n");
}

