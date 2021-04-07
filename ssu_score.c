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
		if (!strcmp(argv[i], "-h")) { //�Է� ���� ���ڵ� �� -h�� ������ print_usage �Լ� ȣ��
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN); //saved_path�� BUFLENũ�⸸ŭ 0�� ä��
	if (argc >= 3 && strcmp(argv[1], "-i") != 0) { //argc�� 3�� �̻��̰� ù��°���ڰ� -i�� �ƴϸ�
		strcpy(stuDir, argv[1]); //argv[1]�� stuDir�� ����
		strcpy(ansDir, argv[2]); //argv[2]�� ansDir�� ����
	}

	if(!strcmp(argv[1], "-i")){ //argv[1]�� -i �̸� iOption ����

		while(i < argc && argv[i][0] != '-'){ //i�� argc���� �۰� argv[i][0]�� -�� �ƴ� ����

			if(j >= ARGNUM) //���ڸ� �ִ�� �޾��� ��
				printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
			else 
				strcpy(iIDs[j], argv[i]); //argv[i] (�Է¹��� �й���) iIDs�� ����
			i++;
			j++;
		}

		do_iOption(iIDs);

		return;
	}

	if (!check_option(argc, argv)) //false�� ���� 
		exit(1);
	
	getcwd(saved_path, BUFLEN); //���� �۾����丮�� ��θ� BUFLEN��ŭ saved_path�� ���� 

	if (chdir(stuDir) < 0) { //stuDir�� ���丮 ����
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}

	getcwd(stuDir, BUFLEN); //���� �۾����丮�� ��θ� BUFLEN��ŭ stuDir�� ����

	chdir(saved_path); //saved_path�� ���丮 ����
	if (chdir(ansDir) < 0) { //ansDir�� ���丮 ����
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN); //���� �۾����丮�� ��θ� BUFLEN��ŭ ansDir�� ����

	chdir(saved_path); //saved_path�� ���丮 ����

	set_scoreTable(ansDir); //score table ����
	set_idTable(stuDir); //stuDir�� id�� idtable�� �Է�

	sprintf(filename, "%s", "score_table.csv");
	if(mOption) //-m �ɼ� ����
		do_mOption(filename);
	
	printf("grading student's test papers..\n");
	score_students(); //ä�� ��� ���̺� ����� �л��� �й� ��� �Է�

	if(iOption) //-i �ɼ� ����
		do_iOption(iIDs);

	return;
}

int check_option(int argc, char* argv[])
{
	int i, j;
	int c;

	while ((c = getopt(argc, argv, "e:mith")) != -1) //�Ķ���� ó�� 
	{
		switch (c) {
			case 'm': // -m : ä�� ���� ���ϴ� ������ ���� ���� score_table.csv �� ���� ���� ����
			mOption = true;
			
			break;
		case 'e': // -e [DIRNAME] : DIRNAME/�й�/������ȣ_error.txt�� ���� �޽��� ���
			eOption = true;
			strcpy(errorDir, optarg); //�ɼ� �ڿ� ������ �Ķ���Ͱ� ���� ��� �̸� �Ľ��� ��� �Ķ���� ���� optarg�� ���� (���⼱ DIRNAME) �װ� errorDir�� ���� 

			if (access(errorDir, F_OK) < 0) //errorDir�� �������� ������ ��� 0755�� ����
				mkdir(errorDir, 0755);
			else { //errorDir�� �����Ѵٸ�
				rmdirs(errorDir); //errorDir read
				mkdir(errorDir, 0755); //��� 755�� errorDir�� ����
			}
			break;
		case 't': // -t [QNAMES] : QNAME�� ���� ��ȣ�� �ϴ� ������ �����Ͻ� -lpthread �ɼ� �߰�
			tOption = true;
			i = optind; //������ ó���� �ɼ��� �ε���
			j = 0;

			while (i < argc && argv[i][0] != '-') { //i�� argc���� �۰� argv[i][0]�� '-'�� �ƴ� ����

				if (j >= ARGNUM) //���ڸ� �ִ�� �޾��� ��
					printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
				else //���� �� ������ argv[i]�� threadFiles[j]�� ����
					strcpy(threadFiles[j], argv[i]);
				i++;
				j++;
			}
			break;
		case 'i': // -i : ä����� ������ �ִ� ��� �ش� �л����� Ʋ������ ���� ���
			iOption = true;
			i = optind;
			j = 0;

			while(i < argc && argv[i][0] != '-'){ //i�� argc���� �۰� argv[i][0]�� -�� �ƴ� ����

				if(j >= ARGNUM) //���ڸ� �ִ�� �޾��� ��
					printf("Maximum Number of Argument Exceeded. :: %s\n", argv[i]);
				else 
					strcpy(iIDs[j], argv[i]); //argv[i] (�Է¹��� �й���) iIDs�� ����
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

void do_iOption(char(*ids)[FILELEN]){ //�ɼǰ� �Բ� �Է��� �й� ����
	FILE* fp;
	char tmp1[BUFLEN];
	char tmp2[BUFLEN];
	char *qname[BUFLEN] = {NULL, };
	char *qscore[BUFLEN] = {NULL, };
	int i,k = 0;
	int count = 0;

	if((fp = fopen("score.csv", "r")) == NULL){ //ä��������̺� ����
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}
	
	fscanf(fp, "%s\n", tmp1);//���� score.csv�� ���ڿ� �о tmp�� �Է�(ù��° ��)

	char *ptr = strtok(tmp1, ","); //�޸� �������� tmp1 �ڸ�
	
	while(ptr != NULL){ //�ڸ� ���ڿ��� ������ ���� ������ �ݺ�
		qname[i] = ptr; 
		i++;

		ptr = strtok(NULL, ","); 
	} //0���� ������ȣ��� sum�� qname�迭�� �Է�

	while(fscanf(fp, "%s\n", tmp2) != EOF){ //���� �� ���ڿ� tmp2�� �Է�
		count=0;
		char *p = strtok(tmp2, ","); //�޸� �������� tmp2 �ڸ� (�й�)
		
		if(!is_exist(ids, tmp2))//�Է� ���� �й��� tmp2�� �������� ������ while�� �ٽ�
			continue;
		
		printf("%s's wrong answer : ", tmp2);
		
		k=0;
		while(p != NULL){ //�ڸ� ���ڿ��� ������ ���� ������ �ݺ�
			qscore[k] = p;
			k++;

			p = strtok(NULL, ",");
		}	//�й� ~ �������� qscore�迭�� �Է�
	
		
		for(int j = 1 ; j < k  ; j++){
			if(!strcmp(qscore[j], "0")){ //qscore �迭 ����� �� 0�� ������ �׿� �ش��ϴ� ������ȣ ���
				
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
	

	if ((fd = open(filename, O_RDWR)) < 0) { //�������̺� ����
		fprintf(stderr, "file open error for score_table.csv\n");
		exit(1);
	}

	if ((read(fd, buf, sizeof(score_table))) < 0) {
		fprintf(stderr, "file read error for score.csv\n");
		exit(1);
	}
	
	while (1) {
		printf("Input question's number to modify >> ");
		scanf("%s", qname); //���� ���� ���ϴ� ���� ��ȣ �Է�

		if (!strcmp(qname, "no")) //�Է¹��� ���ڿ��� no��� while�� ����
			break;
		
		for (i = 0; i < num; i++) {
			if(score_table[i].score == 0){ //�Է��� ���� ��ȣ�� ���� ������ ���
				printf("error : the file doesn't exist\n");
				break;
			}

			memset(test, 0, sizeof(test));
			memcpy(test, score_table[i].qname, strlen(score_table[i].qname) - strlen(strchr(score_table[i].qname, '.'))); //score_table.qname���� Ȯ���ڸ� ������ ���̸�ŭ test�� ����
			if(!strcmp(qname, test)){ //�Է¹��� qname�� test�� ���ٸ�
				lseek(fd, 0, SEEK_SET);
				printf("Current score : %.2lf", score_table[i].score); //���� ���� ���
				
				for (j = 0 ; j < i; j++) { //i��° ���� ���� �ִ� �������� ���̸� ���ϰ�
					memset(tmp, 0, sizeof(tmp));
					sprintf(tmp, "%s,%.2f\n", score_table[j].qname, score_table[j].score);
					currpos = lseek(fd, strlen(tmp), SEEK_CUR); //�� ���̸�ŭ ������ �̵�
				}

				lseek(fd, 0, SEEK_CUR);

				printf("\nNew score : ");
				scanf("%lf", &new); //���ο� ���� �Է� ����

				memset(modify, 0, sizeof(modify));
				sprintf(modify, "%s,%.2f", score_table[i].qname, new); //���� �Է��� ������ modify�� ����
				pwrite(fd, modify, strlen(modify), currpos); //������ currpos��ġ�� modify�� write����
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
		if(i >= ARGNUM) //i�� �ִ����� ������ ũ�� false
			return false;
		else if(!strcmp(src[i], "")) //������ ���ٸ� false
			return false;
		else if(!strcmp(src[i++], target)) //ids�� target��  ���ٸ� true
			return true;
	}
	return false;
}

void set_scoreTable(char* ansDir) //��� ���丮�� ������ �� score table�� ����
{
	char filename[FILELEN];

	sprintf(filename, "%s", "score_table.csv"); //�������̺��� filename �� ����

	if (access(filename, F_OK) == 0) //filename ������ �����ϸ�
		read_scoreTable(filename); 
	else { //filename ���� �������� ������
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

	if ((fp = fopen(path, "r")) == NULL) { //���� ���̺��� �б� �������� ����
		fprintf(stderr, "file open error for %s\n", path);
		return;
	}

	while (fscanf(fp, "%[^,],%s\n", qname, score) != EOF) { //�������̺��� �޸� ������ �о qname�� score������ �Է�
		strcpy(score_table[idx].qname, qname); //qname�� score_table[idx].qname�� ����
		score_table[idx++].score = atof(score); //score�� �Ǽ��� ��ȯ�Ͽ� score_table[idx++].score�� ����
	}

	fclose(fp);
}

void make_scoreTable(char* ansDir) //�������̺�(������ȣ, ����)�� ���� ��� ����� �Լ�
{
	int type, num;
	double score, bscore, pscore;
	struct dirent* dirp, * c_dirp;
	DIR* dp, * c_dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type(); //���� �Է� Ÿ�� ����

	if (num == 1) //1. input blank question and program question's score. ex) 0.5 1  
	{
		printf("Input value of blank question : "); //��ĭ ä��� ������ ���� �Է��ϰ� bscore�� ����
		scanf("%lf", &bscore);
		printf("Input value of program question : "); //���α׷� ������ ���� �Է��ϰ� pscore�� ����
		scanf("%lf", &pscore);
	}
	
	if ((dp = opendir(ansDir)) == NULL) { //ansDir ���丮 �����ϰ� �̸� dp�� ����Ŵ
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}

	while ((dirp = readdir(dp)) != NULL) //ansDir ���丮 read, ������ ���ϵǾ� dirp��
	{
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //dirp->d_name�� "." �ų� ".."�̸� �ٽ� while��
			continue;
		
		if((type = get_file_type(dirp->d_name)) < 0) //���� �Է� Ÿ���� �ùٸ��� ������ �ٽ� while��
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name); //dirp->d_name�� score_table[idx++].qname���� ����
	} 

	closedir(dp);
	sort_scoreTable(idx); //scoretable sorting

	for (i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname); //�������̺� �ִ� �������� Ÿ�� (TEXTFILE or CFILE)�� ���Ϲ���

		if (num == 1) //1. input blank question and program question's score. ex) 0.5 1 
		{
			if (type == TEXTFILE) //��ĭ ������
				score = bscore; //bscore�� �Է� �޾Ҵ� ���� score��
			else if (type == CFILE) //���α׷� ������
				score = pscore; //pscore�� �Է� �޾Ҵ� ���� score��
		}
		else if (num == 2) //2. input all question's score. ex) Input value of 1-1: 0.1
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score); //������ ���� �Է�
		}

		score_table[i].score = score; //�Է� ���� �������� ���� ���̺��� �������� ����
	} 
}

void write_scoreTable(char* filename) //�������̺� ���ڷ� ����
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]); //score_table�� �׸� ����

	if ((fd = creat(filename, 0666)) < 0) { //��� 666���� filename creat��
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for (i = 0; i < num; i++) //score_table ������ŭ ���鼭
	{
		if (score_table[i].score == 0) //���� 0�̸� break
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score); //score_table�� ������ȣ,���� �� tmp�� ����
		write(fd, tmp, strlen(tmp)); //fd(�������̺�)�� tmp(������ȣ, ����) write
	}

	close(fd);
}


void set_idTable(char* stuDir) //�л����� ������ ��� ���丮 ���ڷ� ����
{
	struct stat statbuf;
	struct dirent* dirp;
	DIR* dp;
	char tmp[BUFLEN];
	int num = 0;

	if ((dp = opendir(stuDir)) == NULL) { //stuDir ���丮 ����
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}
	
	while ((dirp = readdir(dp)) != NULL) { //stuDir ���丮 read, ������ ���ϵǾ� dirp��
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //dirp->d_name�� "." �ų� ".."�̸� �ٽ� while��
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); //stuDir/dirp->dname tmp�� �����ϰ� ���
		stat(tmp, &statbuf); //tmp������ stat����ü�� ���� ���ϵ�

		if (S_ISDIR(statbuf.st_mode)) //tmp(stuDir/dirp->d_name)�� ���丮�̸�
			strcpy(id_table[num++], dirp->d_name); //dirp->d_name�� id_table[num++]�� ����
		else
			continue;
	}

	sort_idTable(num); //id table ����
}

void sort_idTable(int size) //id table ����
{
	int i, j;
	char tmp[10];

	for (i = 0; i < size - 1; i++) { //�׸� ������ŭ ���鼭
		for (j = 0; j < size - 1 - i; j++) {
			if (strcmp(id_table[j], id_table[j + 1]) > 0) { //id_table[j+1]�� id_table[i]���� �۴ٸ� 
				strcpy(tmp, id_table[j]); //id_table[i]�� tmp�� ����
				strcpy(id_table[j], id_table[j + 1]); //id_table[j+1]�� id_table[j]�� ����
				strcpy(id_table[j + 1], tmp); //tmp�� id_table[j+1]�� ����
			} 
		}
	}//���� �ùٸ��� ����
}

void sort_scoreTable(int size) //score table ����
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for (i = 0; i < size - 1; i++) { //�׸� ������ŭ ���鼭
		for (j = 0; j < size - 1 - i; j++) {

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j + 1].qname, &num2_1, &num2_2);
			//score_table�� qname�� -.�������� ����� ��ȯ��

			if ((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))) { //num1_1 >= num2_1 %% num1_2 > num2_2 �̸�

				memcpy(&tmp, &score_table[j], sizeof(score_table[0])); //score_table[j]�� ����Ű�� ������ score_table[0]ũ�⸸ŭ�� ������ tmp�� ����Ű�� ���� ������
				memcpy(&score_table[j], &score_table[j + 1], sizeof(score_table[0])); //score_table[j+1]�� ����Ű�� ������ score_table[0]ũ�⸸ŭ�� ������ score_table[j]�� ����Ű�� ���� ����
				memcpy(&score_table[j + 1], &tmp, sizeof(score_table[0])); //tmp�� ����Ű�� ������ score_table[0]ũ�⸸ŭ�� ������ score_table[j+1]�� ����Ű�� ���� ����
			}
		}
	}
}

void get_qname_number(char* qname, int* num1, int* num2)
{
	char* p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); //qname�� �ִ� ���ڿ��� dup���� �����ϴµ� ũ��� qname��ŭ
	*num1 = atoi(strtok(dup, "-.")); //dup�� -.�� �������� �ڸ� ���ڿ��� ����� ��ȯ

	p = strtok(NULL, "-."); //�� �̻� ������ ���� ������ -.�� �ڸ�
	if (p == NULL)
		*num2 = 0;
	else //����� ��ȯ
		*num2 = atoi(p);
}

int get_create_type() //score table ���� �� ���� �Է� Ÿ��
{
	int num;

	while (1)
	{
		printf("score_table.csv file doesn't exist!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n"); //��ĭ���� ���α׷����� ���� �Է�
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n"); //���� ���� ����
		printf("select type >> ");
		scanf("%d", &num);

		if (num != 1 && num != 2) //�ùٸ� ��ȣ �Է��� �ƴ� ���
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
	int size = sizeof(id_table) / sizeof(id_table[0]); //size�� id table �׸� ����

	if ((fd = creat("score.csv", 0666)) < 0) { //ä�� ��� ���̺��� 0666���� ����
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd); //ä�� ��� ���̺� 1�࿡ ������ȣ, sum �����

	for (num = 0; num < size; num++)
	{
		if (!strcmp(id_table[num], "")) //������ ���ٸ� break
			break;

		sprintf(tmp, "%s,", id_table[num]); //tmp�� id_table[num]�� ����
		write(fd, tmp, strlen(tmp)); //score.csv�� tmp(id_table[num])�� write 
		//�л����� �й��� ä�� ��� ���̺� �Է� 
		score += score_student(fd, id_table[num]); //�л��� ���� ���ؼ� �� ����
	}

	printf("Total average : %.2f\n", score / num); //�л��� ��� ���

	close(fd);
}

double score_student(int fd, char* id)  //ä��������̺�� id_table�� �ִ� �й��� ���ڷ� ����
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //size�� score_table �׸� ����

	for (i = 0; i < size; i++)
	{
		if (score_table[i].score == 0) //0�� ������ break
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); //tmp�� stuDir/id(�й�)/score_tabe[i].qname(������ȣ)�� ����

		if (access(tmp, F_OK) < 0) //tmp ���� �������� ������ false
			result = false;
		else
		{
			if ((type = get_file_type(score_table[i].qname)) < 0) //������ type Ȯ��
				continue;

			if (type == TEXTFILE) //��ĭ ������ 
				result = score_blank(id, score_table[i].qname); //false or true
			else if (type == CFILE) //���α׷� ������
				result = score_program(id, score_table[i].qname); //false or true
		}

		if (result == false)
			write(fd, "0,", 2); //fd�� ����Ű�� ��(ä�� ��� ���̺�)�� 2��ŭ "0" write
		else {
			if (result == true) {
				score += score_table[i].score; //���� ����
				sprintf(tmp, "%.2f,", score_table[i].score); //tmp�� ���� ����
			}
			else if (result < 0) { //warning�̸� warning �� ��ŭ ���� (�ϳ� �� -0.1��)
				score = score + score_table[i].score + result;
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp)); //fd�� ����Ű�� ��(ä�� ��� ���̺�)�� tmp���̸�ŭ tmp write
		}
	}

	printf("%s is finished.. score : %.2f\n", id, score); //�й��� ���� ���

	sprintf(tmp, "%.2f\n", score); //score�� tmp�� ����
	write(fd, tmp, strlen(tmp)); //fd�� tmp���̸�ŭ tmp write

	return score;
}

void write_first_row(int fd) //fd�� ä��������̺�
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //table �׸� ����

	write(fd, ",", 1); //score.csv(ä�� ��� ���̺�) �� 1��ŭ , write

	for (i = 0; i < size; i++) {
		if (score_table[i].score == 0) //score table�� 0���� �ִٸ� break
			break;

		sprintf(tmp, "%s,", score_table[i].qname); //tmp�� score_table[i].qname(������ȣ)�� �� ����
		write(fd, tmp, strlen(tmp)); //score.csv(ä��������̺�)�� tmp(score_table[i].qname)�� write  
	}
	write(fd, "sum\n", 4); //score.csv�� 4��ŭ sum\n write
}

char* get_answer(int fd, char* result) //�л��� ����� ���ۿ� �־ ����
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN); //result�� buflen��ŭ 0�� ä��
	while (read(fd, &c, 1) > 0) //fd�κ��� 1��ŭ �о c�� �Է�
	{
		if (c == ':') // �ݷ� : ������ break
			break;

		result[idx++] = c; //����(result)�� fd�� ���� ����
	}
	if (result[strlen(result) - 1] == '\n') //���๮�ڸ� �ι��ڷ� �ٲ���
		result[strlen(result) - 1] = '\0';

	return result; //result�� fd�� ������ ä�� �� (�ݷ� ������ ��, ���๮�ڴ� �η� �ٲ�)
}

int score_blank(char* id, char* filename) //���ڷ� id�� score_table[i].qname�� ���ڷ� ���� => �й�, ������ȣ
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

	memset(qname, 0, sizeof(qname)); //qname�� 0�� sizeof(qname)��ŭ ä��
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //filename(������ȣ)�� ���̸�ŭ �����ؼ� Ȯ���� �����ϰ� qname�� ����

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename); //tmp�� stuDir/id/filename(������ȣ Ȯ���� O)�� ����
	fd_std = open(tmp, O_RDONLY); //tmp�� �б� �������� ���� -> ���� ��ũ���� fd_std
	strcpy(s_answer, get_answer(fd_std, s_answer)); //fd_std�� �������� ä�� ����(result)�� s_answer�� ���� (�ٵ� fd_std�� �ݷ� ������ �ݷ� ��������, ���๮�ڴ� �ι��ڷ� �ٲ�)

	if (!strcmp(s_answer, "")) { //������ ������ false
		close(fd_std);
		return false;
	}

	if (!check_brackets(s_answer)) { // ��ȣ ¦ �ȸ����� close
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer))); //s_answer�� ����, ���� ���� ����

	if (s_answer[strlen(s_answer) - 1] == ';') { //s_answer�� �Ǹ������� �����ݷ�(;)�̸� 
		has_semicolon = true; //true�� �ٲ�
		s_answer[strlen(s_answer) - 1] = '\0'; //�� �������� �ι��� �־���
	}

	if (!make_tokens(s_answer, tokens)) { // ����� ���ڿ��� token���� �и�
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0); //��Ʈ(std_root) ���� �� ��

	sprintf(tmp, "%s/%s", ansDir, filename); //andDir/qname/filename���� tmp�� ����
	fd_ans = open(tmp, O_RDONLY); //tmp �б� �������� �����ϰ� fd_ans�� ����Ŵ

	while (1)
	{
		ans_root = NULL;
		result = true;

		for (idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx])); //tokens[idx]�� 0���� �ʱ�ȭ

		strcpy(a_answer, get_answer(fd_ans, a_answer)); //fd_ans�� �������� ä���� a_answer�� a_answer�� ����

		if (!strcmp(a_answer, "")) //a_answer�� ������ ���ٸ� break
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer))); //���� ������ a_answer�� a_answer�� ����

		if (has_semicolon == false) {
			if (a_answer[strlen(a_answer) - 1] == ';') //a_anwer�� �������� �����ݷ� ����
				continue;
		}

		else if (has_semicolon == true)
		{
			if (a_answer[strlen(a_answer) - 1] != ';') //���ڿ��� �������� �����ݷ��̶��
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0'; //�ι��� �־���
		}

		if (!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0); //Ʈ�� ����� ��Ʈ(ans_root) ����

		compare_tree(std_root, ans_root, &result); //std_root�� ans_root ��

		if (result == true) {
			close(fd_std);
			close(fd_ans);
			//free node����
			if (std_root != NULL)
				free_node(std_root);
			if (ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}

	close(fd_std);
	close(fd_ans);
	//free node����
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

	compile = compile_program(id, filename); //���α׷� ������

	if (compile == ERROR || compile == false) //������ �� ���� ������ false
		return false;

	result = execute_program(id, filename); //���α׷� ���� result�� true or false

	if (!result)
		return false;

	if (compile < 0) //false��
		return compile;

	return true;
}

int is_thread(char* qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]); //threadFiles�� ����

	for (i = 0; i < size; i++) {
		if (!strcmp(threadFiles[i], qname)) //threadFiles[i]�� qname�̶� ������ true
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

	memset(qname, 0, sizeof(qname)); //qname�� 0���� ä��
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //filename�� ���̸�ŭ �����ؼ� qname�� ���� �ٵ� ���̰� filename (ó�� ~ . ���� ������) => Ȯ���ڸ���

	isthread = is_thread(qname); //qname�̶� threadFile�̶� ������ true �ƴϸ� false

	sprintf(tmp_f, "%s/%s", ansDir, filename); //tmp_f�� ansDir/filename ���� (Ȯ���ڸ� O)
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname); //tmp_e�� ansDir/qname.exe ���� (Ȯ���ڸ� X)

	if (tOption && isthread) //t option ������
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //gcc -o tmp_e tmp_f -lpthread�� command�� ����
	else //t option ������
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //gcc -o tmp_e tmp_f�� command�� ����

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname); //ansDir/qname_error.txt���� tmp_e�� ����
	fd = creat(tmp_e, 0666); //tmp_e ������ 666���� ����

	redirection(command, fd, STDERR); //command �����ϰ� STDERR error������ fd�� ����Ű�� tmp_e�� ��� 
	size = lseek(fd, 0, SEEK_END); //tmp_e�� ���� ������
	close(fd);
	unlink(tmp_e); //tmp_e ���� ����

	if (size > 0) //���� ���� ������ false
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); //tmp_f�� stuDir/id/filename ���� (Ȯ���ڸ� O)
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname); //tmp_e�� stuDir/id/qname ���� (Ȯ���ڸ� X)

	if (tOption && isthread) //t option
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //gcc -o tmp_e tmp_s -lpthread�� command�� ����
	else //t option ������
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //gcc -o tmp_e tmp_f�� command�� ����

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname); //stuDir/id/qname_error.txt�� tmp_f�� ����
	fd = creat(tmp_f, 0666); //tmp_f ������ 666���� ����

	redirection(command, fd, STDERR); //command �����ϰ� STDERR error������ fd�� ����Ű�� tmp_f�� ���
	size = lseek(fd, 0, SEEK_END); //tmp_f�� ���� ������
	close(fd);

	if (size > 0) {
		if (eOption) //e option
		{
			sprintf(tmp_e, "%s/%s", errorDir, id); //errorDir/id�� tmp_e�� ����
			if (access(tmp_e, F_OK) < 0) //tmp_e ������ �������� �ʴ´ٸ� ��� 755�� mkdir
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname); //errorDir/id/qname_error.txt�� tmp_e�� ����
			rename(tmp_f, tmp_e); //tmp_f���� tmp_e�� �̸� ����

			result = check_error_warning(tmp_e); //error warning üũ
		}
		else { //e option ���� ��
			result = check_error_warning(tmp_f); //error warning üũ
			unlink(tmp_f); //tmp_f ���� ����
		}

		return result;
	}

	unlink(tmp_f); //tmp_f ���� ����
	return true;
}

double check_error_warning(char* filename)
{
	FILE* fp;
	char tmp[BUFLEN];
	double warning = 0;

	if ((fp = fopen(filename, "r")) == NULL) { //filename ���� , ���ϵ�ũ���� fp
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while (fscanf(fp, "%s", tmp) > 0) { //tmp�� �Է¹޾Ƽ� fp�� �Է���
		if (!strcmp(tmp, "error:")) //tmp�� "error:" �� ������ ���� ����
			return ERROR; //0
		else if (!strcmp(tmp, "warning:")) //tmp�� "warning:" �̶� ������ 
			warning += WARNING; //warning�� ���� WARNING(-0.1) ���� 
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

	memset(qname, 0, sizeof(qname)); //qname�� 0���� ä��
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //filename�� ���̸�ŭ �����ؼ� qname�� ���� �ٵ� ���̰� filename (ó�� ~ . ���� ������) => Ȯ���ڸ� ����

	sprintf(ans_fname, "%s/%s.stdout", ansDir,qname); // ansDir/qname.stdout�� ans_fname�� ����
	fd = creat(ans_fname, 0666); //ans_fname���� ��� 666���� ����

	sprintf(tmp, "%s/%s.exe", ansDir, qname); // ansDir/qname.exe�� tmp�� ����
	redirection(tmp, fd, STDOUT); // tmp�����ϰ� ǥ������Ϸ��� �� fd�� �����ϴ� ��(ans_fname)�� �� 
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname); // stuDir/id/qname.stdout�� std_fname�� ����
	fd = creat(std_fname, 0666); //std_fname���� ��� 666���� ����

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); //stuDir/id/qname.stdexe & �� tmp�� ����

	start = time(NULL);
	redirection(tmp, fd, STDOUT); //tmp�����ϰ� ǥ������Ϸ��� �� fd�� �����ϴ� �� (std_fname)�� ��

	sprintf(tmp, "%s.stdexe", qname); //qname.stdexe�� tmp�� ����
	while ((pid = inBackground(tmp)) > 0) { //���μ�����ȣ�� 0���� ũ��
		end = time(NULL);

		if (difftime(end, start) > OVER) { //����ð��� 5�� �ʰ���
			kill(pid, SIGKILL); //���μ��� ������ ����
			close(fd);
			return false;
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname); //stuDir/id/qname.stdout , ansDir/qname.stdout ������ true �ٸ��� false
}

pid_t inBackground(char* name) // .stdexe ����
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;

	memset(tmp, 0, sizeof(tmp)); //tmp 0���� ä��
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); //background.txt ���� �б⾲��,���0666���� ����

	sprintf(command, "ps | grep %s", name); //ps | grep �� ��׶���� ����Ǵ� ���α׷��� ���� ĳġ�ϰ��� ���
	redirection(command, fd, STDOUT); //ps | grep ����, ������(ǥ������Ϸ��� ��) fd�� ��

	lseek(fd, 0, SEEK_SET); //offset ��ó���� ��ġ
	read(fd, tmp, sizeof(tmp)); //fd�� ����Ű�� txt������ tmp�����ŭ �о tmp�� �־����

	if (!strcmp(tmp, "")) { //tmp�� ����ٸ�
		unlink("background.txt"); //���ϻ���, �������� 1 ����
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " ")); //tmp���ڿ��� ������ �������� �ڸ� �װ� ����� ��ȯ�ؼ� pid�� ����
	close(fd);

	unlink("background.txt"); //���� ����
	return pid; //ps | grep �� ��� �� ���μ��� ��ȣ ����
}

int compare_resultfile(char* file1, char* file2) //stuDir/id/qname.stdout , ansDir/qname.stdout (���, ���� ������ ����)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;
	//�б� �������� ���� ���� ���� fd1, fd2�� ����Ŵ
	fd1 = open(file1, O_RDONLY); //�л� �� ���� ���
	fd2 = open(file2, O_RDONLY); //��� ���� ���

	while (1)
	{ //size 1�� ����
		while ((len1 = read(fd1, &c1, 1)) > 0) { //fd1�� ����Ű�� ���� 1��ŭ �а� c1�� ���� , ���� ����Ʈ ���� len1
			if (c1 == ' ') //�����̸� �ٽ�
				continue;
			else
				break;
		}
		while ((len2 = read(fd2, &c2, 1)) > 0) { //fd2�� ����Ű�� ���� 1��ŭ �а� c2�� ���� , ���� ����Ʈ ���� len2
			if (c2 == ' ') //�����̸� �ٽ�
				continue;
			else
				break;
		}

		if (len1 == 0 && len2 == 0) //�� ���� ��� read�� ������ ���� ������ ��� break
			break;
		//c1, c2 ��� �ҹ��ڷ�
		to_lower_case(&c1);
		to_lower_case(&c2);

		if (c1 != c2) { //�л����̶� ����̶� �ٸ��� return false
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

	saved = dup(old); //old�� saved�� �����ؼ� ����
	dup2(new, old); //old�� fd�� new�� fd�� �ٲ�

	system(command); //command����

	dup2(saved, old); //saved�� �����ϴ� ���� old�� ������
	close(saved);
}

int get_file_type(char* filename) //������ Ÿ�� (c or txt)�� ��������
{
	char* extension = strrchr(filename, '.'); //filename�� ���������� .�� ���� ù��° ���ڿ� �����͸� ����

	if (!strcmp(extension, ".txt")) //ã�� ù��° .�� .txt�� TEXTFILE�� ����
		return TEXTFILE;
	else if (!strcmp(extension, ".c")) //ã�� ù��° .�� .c�� CFILE�� ����
		return CFILE;
	else
		return -1;
}

void rmdirs(const char* path) //���丮, ���丮 �Ʒ��� ���ϵ� ��� ����
{
	struct dirent* dirp;
	struct stat statbuf;
	DIR* dp;
	char tmp[257];

	if ((dp = opendir(path)) == NULL) //���ڷ� ���� ���丮 ����
		return;

	while ((dirp = readdir(dp)) != NULL) //������ ���丮 read
	{
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))//dirp->d_name�� "." �̰ų� dirp->d_name�� ".."�̸� �ݺ��� �ٽ�
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name); ////path/dirp->d_name ���� tmp �� ����

		if (lstat(tmp, &statbuf) == -1) //�ɺ��� ��ũ�� ������ tmp�� �ѱ�
			continue;

		if (S_ISDIR(statbuf.st_mode)) //���丮�̸� ���丮 ���� , ���丮�� �ƴϸ� unlink���ش�
			rmdirs(tmp);
		else
			unlink(tmp);
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char* c) //�ҹ��ڷ� �ٲ�
{
	if (*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage() //h�ɼ� �޾��� �� ���� ���
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify score before scoring\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -i                print ID's wrong questions\n");
	printf(" -h                print usage\n");
}

