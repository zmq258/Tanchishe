#include <iostream>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
using namespace std;

#define U 1
#define D 2
#define L 3 
#define R 4       // U���� ��D���£�L:�� R����

typedef struct SNAKE {
    int x, y;
    struct SNAKE* next;
} snake;

// �û���Ϣ
typedef struct {
    int id;
    char username[32];
    char password[32];
} User;

// ȫ�ֱ���
int score = 0, add = 10;
int status, sleeptime = 200;
snake *head, *food, *q;
User currentUser;
time_t game_start_time;

// ������Ϸ״̬
enum GameState { MENU, PLAY, EXIT };

// ��������
void Pos(int x, int y);
void creatMap();
void initsnake();
int biteself();
void createfood();
int snakemove();  // ���ڷ�����ײԭ��0=������1=ײǽ��2=ҧ���Լ�
void welcometogame();
enum GameState endgame(int reason, int score);
void gamestart();
void loadOrRegisterUser();
void showUserLogs();
void writeUserLog();
enum GameState loginMenu();
int authenticateUser();  // ��¼����
void registerUser();     // ע�ắ��
void initWorkingDirectory();

void Pos(int x, int y) {
    COORD pos = { short(x), short(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void loadOrRegisterUser() {
    FILE *fp = fopen("users.dat", "rb+");
    if (!fp) {
        fp = fopen("users.dat", "wb+");
        printf("=== �״�ʹ�ã���ע�����û� ===\n");
        currentUser.id = 1;
        printf("�û���: "); scanf("%31s", currentUser.username);
        printf("����: "); scanf("%31s", currentUser.password);
        fwrite(&currentUser, sizeof(User), 1, fp);
        fclose(fp);
        printf("ע��ɹ��������������¼��\n");
        exit(0);
    }
    fclose(fp);

    while (!authenticateUser()) {
        printf("��¼ʧ�ܣ������ԡ�\n");
    }
}

void showUserLogs() {
    system("cls");
    printf("=== �û���־ (ID, �û���, ��ʼʱ��, ʱ��(s), �÷�) ===\n\n");
    FILE *lf = fopen("userlog.txt", "r");
    if (!lf) {
        printf("������־��\n");
    } else {
        char line[256];
        while (fgets(line, sizeof(line), lf)) {
            printf("%s", line);
        }
        fclose(lf);
    }
    printf("\n�������������Ϸ��");
    getch();
    system("cls");
}

void writeUserLog() {
    time_t end = time(NULL);
    double duration = difftime(end, game_start_time);
    struct tm *tm_start = gmtime(&game_start_time);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm_start);

    FILE *lf = fopen("userlog.txt", "a");
    fprintf(lf, "%d, %s, %s, %.0f, %d\n",
            currentUser.id, currentUser.username,
            buf, duration, score);
    fclose(lf);
}

void creatMap() {
    for (int i = 0; i < 58; i += 2) {
        Pos(i, 0);  printf("��");
        Pos(i, 26); printf("��");
    }
    for (int i = 1; i < 26; i++) {
        Pos(0, i);  printf("��");
        Pos(56, i); printf("��");
    }
}

void initsnake()//��ʼ������
{
    snake* tail;
    int i;
    tail = (snake*)malloc(sizeof(snake));//����β��ʼ��ͷ�巨����x,y�趨��ʼ��λ��//
    tail->x = 24;
    tail->y = 5;
    tail->next = NULL;
    for (i = 1; i <= 4; i++)
    {
        head = (snake*)malloc(sizeof(snake));
        head->next = tail;
        head->x = 24 + 2 * i;
        head->y = 5;
        tail = head;
    }
    while (tail != NULL)//��ͷ��Ϊ���������
    {
        Pos(tail->x, tail->y);
        printf("��");
        tail = tail->next;
    }
}

int biteself() {
    for (snake* s = head->next; s; s = s->next) {
        if (s->x == head->x && s->y == head->y) {
            return 1;
        }
    }
    return 0;
}

void createfood() {
    snake* food_1 = (snake*)malloc(sizeof(snake));
    srand((unsigned)time(NULL));
    do {
        food_1->x = (rand() % 26) * 2 + 2;  // ��֤ż��
        food_1->y = rand() % 24 + 1;
    } while ( /* �������ͻ */ ([&] {
        for (q = head; q; q = q->next)
            if (q->x == food_1->x && q->y == food_1->y)
                return true;
        return false;
    }()));
    food = food_1;
    Pos(food->x, food->y);
    printf("��");
}

int snakemove() {
    // ������һ��ͷ������
    int nx = head->x, ny = head->y;
    if (status == U)    ny--;
    else if (status == D) ny++;
    else if (status == L) nx -= 2;
    else if (status == R) nx += 2;

    // ײǽ���
    if (nx <= 0 || nx >= 56 || ny <= 0 || ny >= 26) {
        return 1;  // ײǽ
    }

    // ��ͷ�ڵ�
    snake* nexthead = (snake*)malloc(sizeof(snake));
    nexthead->x = nx;
    nexthead->y = ny;
    nexthead->next = head;
    head = nexthead;

    // �Ե�ʳ�
    if (nx == food->x && ny == food->y) {
        score += add;
        createfood();
    } else {
        // �Ƴ�β�ڵ�
        q = head;
        while (q->next->next) q = q->next;
        Pos(q->next->x, q->next->y);
        printf("  ");
        free(q->next);
        q->next = NULL;
    }

    // ��ҧ���
    if (biteself()) {
        return 2;
    }

    // �ػ�����
    for (q = head; q; q = q->next) {
        Pos(q->x, q->y);
        printf("��");
    }
    return 0;  // һ������
}

void welcometogame()//��ʼ����
{
    Pos(40, 12);
    printf("��ӭ����̰ʳ����Ϸ��");
    Pos(40, 25);
    system("pause");
    system("cls");
    Pos(25, 12);
    printf("�á�.��.��.���ֱ�����ߵ��ƶ��� F1 Ϊ���٣�2 Ϊ����\n");
    Pos(25, 13);
    printf("���ٽ��ܵõ����ߵķ�����\n");
    system("pause");
    system("cls");
}

enum GameState endgame(int reason, int sc) {
    writeUserLog();
    system("cls");
    Pos(24, 12);
    if (reason == 1)      printf("ײǽ�ˡ���Ϸ����!\n");
    else if (reason == 2) printf("ҧ���Լ��ˡ���Ϸ����!\n");
    else if (reason == 3) printf("�����˳����ټ�!\n");
    Pos(24, 13);
    cout << "���ĵ÷���" << sc << endl;
    system("pause");
    return MENU;
}

void gamestart() {
    system("mode con cols=100 lines=30");
    // ��ӭ�����ԡ�
    system("cls");
    creatMap();
    initsnake();
    createfood();
    game_start_time = time(NULL);
}

enum GameState gamecircle() {
    Pos(64, 15); printf("���ܴ�ǽ������ҧ���Լ�");
    Pos(64, 16); printf("�������� �����ƶ���F1 ���� F2 ���� F5 ��־ ESC �˳� SPACE ��ͣ");
    status = R;

    while (true) {
        Pos(64, 10); printf("�÷֣�%d  ", score);
        Pos(2, 28);   printf("***%s ������Ϸ��***", currentUser.username);

        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            showUserLogs();
            system("cls");
            creatMap(); // ���»���ͼ����
            for (q = head; q; q = q->next) { Pos(q->x,q->y); printf("��"); }
            Pos(food->x, food->y); printf("��");
        }
        if (GetAsyncKeyState(VK_UP)    & 0x8000 && status != D) status = U;
        if (GetAsyncKeyState(VK_DOWN)  & 0x8000 && status != U) status = D;
        if (GetAsyncKeyState(VK_LEFT)  & 0x8000 && status != R) status = L;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && status != L) status = R;

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return endgame(3, score);
        }
        Sleep(sleeptime);
        int reason = snakemove();
        if (reason != 0) {
            return endgame(reason, score);
        }
    }
}

enum GameState loginMenu() {
    int choice;
    while (true) {
        system("cls");
        printf("===== ̰������Ϸ��¼ϵͳ =====\n");
        printf("1. ��¼\n2. ע�����û�\n3. �˳�\n");
        printf("��ѡ�������");
        if (scanf("%d", &choice) != 1) {
            while (getchar()!='\n');
            continue;
        }
        getchar();
        switch (choice) {
        case 1:
            if (authenticateUser()) {
                printf("��¼�ɹ�����ӭ��%s��\n", currentUser.username);
                Sleep(1000);
                return PLAY;
            } else {
                printf("��¼ʧ�ܡ��������������\n");
                getchar();
            }
            break;
        case 2:
            registerUser();
            break;
        case 3:
            return EXIT;
        default:
            printf("��Чѡ������ԡ�\n");
            Sleep(1000);
        }
    }
}

int authenticateUser() {
    char uname[32], pwd[32];
    printf("�û���: "); scanf("%31s", uname);
    printf("����: "); scanf("%31s", pwd);

    FILE *fp = fopen("users.dat", "rb");
    if (!fp) return 0;
    User u;
    while (fread(&u, sizeof(u), 1, fp)) {
        if (strcmp(u.username, uname)==0 && strcmp(u.password, pwd)==0) {
            currentUser = u;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void registerUser() {
    char uname[32], pwd[32];
    printf("���������û���: "); scanf("%31s", uname);
    printf("����������: "); scanf("%31s", pwd);

    FILE *fp = fopen("users.dat", "ab+");
    if (!fp) {
        printf("�޷����û��ļ���\n");
        return;
    }
    fseek(fp, 0, SEEK_END);
    int new_id = ftell(fp)/sizeof(User) + 1;
    User nu = { new_id, "", "" };
    strcpy(nu.username, uname);
    strcpy(nu.password, pwd);
    fwrite(&nu, sizeof(nu), 1, fp);
    fclose(fp);
    printf("ע��ɹ�����ʹ�����˻���¼��\n");
    Sleep(1000);
}

void initWorkingDirectory() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) *p = '\0';
    SetCurrentDirectoryA(path);
}

int main() {
    initWorkingDirectory();
    GameState state = MENU;
    while (state != EXIT) {
        if (state == MENU) {
            state = loginMenu();
        } else if (state == PLAY) {
            gamestart();
            state = gamecircle();
        }
    }
    return 0;
}