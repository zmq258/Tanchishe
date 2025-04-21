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
#define R 4       // U：上 ；D：下；L:左 R：右

typedef struct SNAKE {
    int x, y;
    struct SNAKE* next;
} snake;

// 用户信息
typedef struct {
    int id;
    char username[32];
    char password[32];
} User;

// 全局变量
int score = 0, add = 10;
int status, sleeptime = 200;
snake *head, *food, *q;
User currentUser;
time_t game_start_time;

// 定义游戏状态
enum GameState { MENU, PLAY, EXIT };

// 函数声明
void Pos(int x, int y);
void creatMap();
void initsnake();
int biteself();
void createfood();
int snakemove();  // 现在返回碰撞原因：0=正常，1=撞墙，2=咬到自己
void welcometogame();
enum GameState endgame(int reason, int sc);
void gamestart();
enum GameState gamecircle();
void loadOrRegisterUser();
enum GameState loginMenu();
int authenticateUser();  // 登录函数
void registerUser();     // 注册函数
void initWorkingDirectory();
void showUserLogs();
void writeUserLog();

void Pos(int x, int y) {
    COORD pos = { short(x), short(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void loadOrRegisterUser() {
    FILE *fp = fopen("users.dat", "rb+");
    if (!fp) {
        fp = fopen("users.dat", "wb+");
        printf("=== 首次使用，请注册新用户 ===\n");
        currentUser.id = 1;
        printf("用户名: "); scanf("%31s", currentUser.username);
        printf("密码: "); scanf("%31s", currentUser.password);
        fwrite(&currentUser, sizeof(User), 1, fp);
        fclose(fp);
        printf("注册成功！请重启程序登录。\n");
        exit(0);
    }
    fclose(fp);

    while (!authenticateUser()) {
        printf("登录失败，请重试。\n");
    }
}

void showUserLogs() {
    system("cls");
    printf("=== 用户日志 (ID, 用户名, 开始时间, 时长(s), 得分) ===\n\n");
    FILE *lf = fopen("userlog.txt", "r");
    if (!lf) {
        printf("暂无日志。\n");
    } else {
        char line[256];
        while (fgets(line, sizeof(line), lf)) {
            printf("%s", line);
        }
        fclose(lf);
    }
    system("pause");
}

void writeUserLog() {
    // 获取结束时刻
    time_t end_time = time(NULL);

    // 计算时长（秒）
    double duration = difftime(end_time, game_start_time);

    // 本地时间结构
    struct tm *tm_start = localtime(&game_start_time);
    struct tm *tm_end   = localtime(&end_time);

    // 格式化成 "YYYY-MM-DD HH:MM:SS"
    char buf_start[32], buf_end[32];
    strftime(buf_start, sizeof(buf_start), "%Y-%m-%d %H:%M:%S", tm_start);
    strftime(buf_end,   sizeof(buf_end),   "%Y-%m-%d %H:%M:%S", tm_end);

    // 追加写入日志
    FILE *lf = fopen("userlog.txt", "a");
    if (lf) {
        // 格式：ID, 用户名, 开始时间, 结束时间, 时长(s), 得分
        fprintf(lf, "%d, %s, %s, %s, %.0f, %d\n",
                currentUser.id,
                currentUser.username,
                buf_start,
                buf_end,
                duration,
                score);
        fclose(lf);
    }
}


void creatMap() {
    for (int i = 0; i < 58; i += 2) {
        Pos(i, 0);  printf("■");
        Pos(i, 26); printf("■");
    }
    for (int i = 1; i < 26; i++) {
        Pos(0, i);  printf("■");
        Pos(56, i); printf("■");
    }
}

void initsnake() {
    snake* tail;
    tail = (snake*)malloc(sizeof(snake));
    tail->x = 24;
    tail->y = 5;
    tail->next = NULL;
    for (int i = 1; i <= 4; i++) {
        head = (snake*)malloc(sizeof(snake));
        head->next = tail;
        head->x = 24 + 2 * i;
        head->y = 5;
        tail = head;
    }
    for (snake* p = tail; p; p = p->next) {
        Pos(p->x, p->y);
        printf("■");
    }
}

int biteself() {
    for (snake* s = head->next; s; s = s->next) {
        if (s->x == head->x && s->y == head->y) return 1;
    }
    return 0;
}

void createfood() {
    snake* food_1 = (snake*)malloc(sizeof(snake));
    srand((unsigned)time(NULL));
    do {
        food_1->x = (rand() % 26) * 2 + 2;
        food_1->y = rand() % 24 + 1;
    } while ([&] {
        for (q = head; q; q = q->next)
            if (q->x == food_1->x && q->y == food_1->y)
                return true;
        return false;
    }());
    food = food_1;
    Pos(food->x, food->y);
    printf("■");
}

int snakemove() {
    int nx = head->x, ny = head->y;
    if (status == U)    ny--;
    else if (status == D) ny++;
    else if (status == L) nx -= 2;
    else if (status == R) nx += 2;

    if (nx <= 0 || nx >= 56 || ny <= 0 || ny >= 26) return 1;

    snake* nexthead = (snake*)malloc(sizeof(snake));
    nexthead->x = nx;
    nexthead->y = ny;
    nexthead->next = head;
    head = nexthead;

    if (nx == food->x && ny == food->y) {
        score += add;
        createfood();
    } else {
        q = head;
        while (q->next->next) q = q->next;
        Pos(q->next->x, q->next->y);
        printf("  ");
        free(q->next);
        q->next = NULL;
    }

    if (biteself()) return 2;

    for (q = head; q; q = q->next) {
        Pos(q->x, q->y);
        printf("■");
    }
    return 0;
}

void welcometogame() {
    Pos(40, 12);
    printf("欢迎来到贪食蛇游戏！");
    Pos(40, 25);
    system("pause");
    system("cls");
    Pos(25, 12);
    printf("↑ ↓ ← → 分别控制蛇的移动，F1 加速，F2 减速\n");
    Pos(25, 13);
    printf("加速将能得到更高的分数。\n");
    system("pause");
    system("cls");
}

// 修改：结束游戏后选择重新登录、再来一局或退出
enum GameState endgame(int reason, int sc) {
    writeUserLog();
    system("cls");
    Pos(24, 12);
    if (reason == 1)      printf("撞墙了。游戏结束!\n");
    else if (reason == 2) printf("咬到自己了。游戏结束!\n");
    else if (reason == 3) printf("主动退出。\n");
    Pos(24, 13);
    cout << "您的得分是 " << sc << endl;
    Pos(24, 15);
    printf("1. 重新登录  2. 再来一局  3. 退出\n");
    printf("请选择: ");
    int choice;
    while (true) {
        if (scanf("%d", &choice) != 1) {
            while (getchar()!='\n');
            continue;
        }
        switch (choice) {
            case 1: return MENU;
            case 2: return PLAY;
            case 3: return EXIT;
            default:
                printf("无效选择，请重新输入: ");
        }
    }
}

void gamestart() {
    system("mode con cols=100 lines=30");
    system("cls");
    creatMap();
    initsnake();
    createfood();
    game_start_time = time(NULL);
}

enum GameState gamecircle() {
    Pos(64, 15); printf("不能穿墙，不能咬到自己");
    Pos(64, 16); printf("↑↓←→ 控制移动，F1 加速 F2 减速 F5 日志 ESC 退出 SPACE 暂停");
    status = R;

    while (true) {
        Pos(64, 10); printf("得分：%d  ", score);
        Pos(2, 28);   printf("***%s 正在游戏中***", currentUser.username);

        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            showUserLogs();
            system("cls");
            creatMap();
            for (q = head; q; q = q->next) { Pos(q->x,q->y); printf("■"); }
            Pos(food->x, food->y); printf("■");
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

// 修改：登录界面增加查看日志选项
enum GameState loginMenu() {
    int choice;
    while (true) {
        system("cls");
        printf("===== 贪吃蛇游戏登录系统 =====\n");
        printf("1. 登录\n2. 注册新用户\n3. 查看日志\n4. 退出\n");
        printf("请选择操作：");
        if (scanf("%d", &choice) != 1) {
            while (getchar()!='\n');
            continue;
        }
        getchar();
        switch (choice) {
        case 1:
            if (authenticateUser()) {
                printf("登录成功！欢迎，%s。\n", currentUser.username);
                Sleep(1000);
                return PLAY;
            } else {
                printf("登录失败。按任意键继续。\n");
                getchar();
            }
            break;
        case 2:
            registerUser();
            break;
        case 3:
            showUserLogs();
            break;
        case 4:
            return EXIT;
        default:
            printf("无效选项，请重试。\n");
            Sleep(1000);
        }
    }
}

int authenticateUser() {
    char uname[32], pwd[32];
    printf("用户名: "); scanf("%31s", uname);
    printf("密码: "); scanf("%31s", pwd);

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
    // —— 第一步：确保 users.dat 和 userlog.txt 存在 —— 
    // 如果 users.dat 不存在，就创建一个空文件
    FILE* fp = fopen("users.dat", "rb");
    if (!fp) {
        fp = fopen("users.dat", "wb");
        if (!fp) {
            printf("无法创建 users.dat！请检查权限。\n");
            system("pause");
            return;
        }
        fclose(fp);
    } else {
        fclose(fp);
    }
    // 如果 userlog.txt 不存在，也创建一个空文件
    fp = fopen("userlog.txt", "r");
    if (!fp) {
        fp = fopen("userlog.txt", "w");
        if (!fp) {
            printf("无法创建 userlog.txt！请检查权限。\n");
            system("pause");
            return;
        }
        fclose(fp);
    } else {
        fclose(fp);
    }

    char uname[32], pwd[32];
    while (true) {
        // —— 第二步：输入用户名并检测重复 —— 
        printf("请输入新用户名: ");
        scanf("%31s", uname);

        // 打开 users.dat，检查是否已有同名用户
        fp = fopen("users.dat", "rb");
        if (!fp) {
            printf("无法打开 users.dat 进行检查。\n");
            system("pause");
            return;
        }
        User u;
        bool exists = false;
        while (fread(&u, sizeof(u), 1, fp)) {
            if (strcmp(u.username, uname) == 0) {
                exists = true;
                break;
            }
        }
        fclose(fp);

        if (exists) {
            printf("用户名 \"%s\" 已被注册，请换一个用户名。\n\n", uname);
            system("pause");
            continue;
        }

        // —— 第三步：输入密码，追加写入 users.dat —— 
        printf("请输入密码: ");
        scanf("%31s", pwd);

        fp = fopen("users.dat", "ab+");
        if (!fp) {
            printf("无法打开 users.dat 进行写入！\n");
            system("pause");
            return;
        }
        // 计算新用户 ID
        fseek(fp, 0, SEEK_END);
        int new_id = ftell(fp) / sizeof(User) + 1;

        User nu;
        nu.id = new_id;
        strncpy(nu.username, uname, sizeof(nu.username)-1);
        nu.username[sizeof(nu.username)-1] = '\0';
        strncpy(nu.password, pwd, sizeof(nu.password)-1);
        nu.password[sizeof(nu.password)-1] = '\0';

        fwrite(&nu, sizeof(nu), 1, fp);
        fclose(fp);

        printf("注册成功！请使用新账户登录。\n");
        Sleep(1000);
        break;
    }
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
            score = 0; // 重置分数
            gamestart();
            state = gamecircle();
        }
    }
    return 0;
}
