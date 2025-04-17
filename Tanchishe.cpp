#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

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
int endgamestatus = 0;
User currentUser;
time_t game_start_time;

// 函数声明
void Pos(int x, int y);
void creatMap();
void initsnake();
int biteself();
void createfood();
void cantcrosswall();
void snakemove();
void pause_game();
void gamecircle();
void welcometogame();
void endgame();
void gamestart();
void loadOrRegisterUser();
// int authenticateUser();
void showUserLogs();
void writeUserLog();
int authenticateUser();  // 登录函数
void registerUser();     // 注册函数
void initWorkingDirectory();

// —— 控制台工具 —— //
void Pos(int x, int y) {
    COORD pos = { short(x), short(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// —— 用户管理 —— //
void loadOrRegisterUser() {
    FILE *fp = fopen("users.dat", "rb+");
    if (!fp) {
        // 首次启动，创建文件并注册第一个用户
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

    // 登录流程
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
    printf("\n按任意键返回游戏…");
    getch();
    system("cls");
    gamestart();
}

// 在游戏结束前写日志
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

void creatMap()//创建地图
{
    int i;
    for (i = 0; i < 58; i += 2)//打印上下边框
    {
        Pos(i, 0);
        printf("■");
        Pos(i, 26);
        printf("■");
    }
    for (i = 1; i < 26; i++)//打印左右边框
    {
        Pos(0, i);
        printf("■");
        Pos(56, i);
        printf("■");
    }
}

int biteself()//判断是否咬到了自己
{
    snake* self;
    self = head->next;
    while (self != NULL)
    {
        if (self->x == head->x && self->y == head->y)
        {
            return 1;
        }
        self = self->next;
    }
    return 0;
}

void createfood()//随机出现食物
{
    snake* food_1;
    srand((unsigned)time(NULL));
    food_1 = (snake*)malloc(sizeof(snake));
    while ((food_1->x % 2) != 0)    //保证其为偶数，使得食物能与蛇头对其
    {
        food_1->x = rand() % 52 + 2;
    }
    food_1->y = rand() % 24 + 1;
    q = head;
    while (q->next == NULL)
    {
        if (q->x == food_1->x && q->y == food_1->y) //判断蛇身是否与食物重合
        {
            free(food_1);
            createfood();
        }
        q = q->next;
    }
    Pos(food_1->x, food_1->y);
    food = food_1;
    printf("■");
}

void cantcrosswall()//不能穿墙
{
    if (head->x == 0 || head->x == 56 || head->y == 0 || head->y == 26)
    {
        endgamestatus = 1;
        endgame();
    }
}

void snakemove()//蛇前进,上U,下D,左L,右R
{
    snake* nexthead;
    cantcrosswall();

    nexthead = (snake*)malloc(sizeof(snake));
    if (status == U)
    {
        nexthead->x = head->x;
        nexthead->y = head->y - 1;
        if (nexthead->x == food->x && nexthead->y == food->y)//如果下一个有食物//
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                                               //如果没有食物//
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == D)
    {
        nexthead->x = head->x;
        nexthead->y = head->y + 1;
        if (nexthead->x == food->x && nexthead->y == food->y)  //有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                               //没有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == L)
    {
        nexthead->x = head->x - 2;
        nexthead->y = head->y;
        if (nexthead->x == food->x && nexthead->y == food->y)//有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                                //没有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == R)
    {
        nexthead->x = head->x + 2;
        nexthead->y = head->y;
        if (nexthead->x == food->x && nexthead->y == food->y)//有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                                         //没有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (biteself() == 1)       //判断是否会咬到自己
    {
        endgamestatus = 2;
        endgame();
    }
}

void pause_game()//暂停
{
    while (1)
    {
        Sleep(300);
        if (GetAsyncKeyState(VK_SPACE))
        {
            break;
        }

    }
}

void initsnake()//初始化蛇身
{
    snake* tail;
    int i;
    tail = (snake*)malloc(sizeof(snake));//从蛇尾开始，头插法，以x,y设定开始的位置//
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
    while (tail != NULL)//从头到为，输出蛇身
    {
        Pos(tail->x, tail->y);
        printf("■");
        tail = tail->next;
    }
}

// 游戏主循环，增加 F5 处理 //
void gamecircle() {
    Pos(64, 15); printf("不能穿墙，不能咬到自己");
    Pos(64, 16); printf("用↑.↓.←.→分别控制蛇的移动.");
    Pos(64, 17); printf("F1 加速 F2 减速 F5 日志 ESC 退出 SPACE 暂停");
    status = R;

    while (1) {
        // 显示分数与用户状态
        Pos(64, 10); printf("得分：%d  ", score);
        // 显示当前用户
        Pos(2, 28); printf("***%s正在游戏中***", currentUser.username);
        // 检测按键
        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            showUserLogs();
        }
        
        if (GetAsyncKeyState(VK_UP)    & 0x8000 && status != D) status = U;
        else if (GetAsyncKeyState(VK_DOWN)  & 0x8000 && status != U) status = D;
        else if (GetAsyncKeyState(VK_LEFT)  & 0x8000 && status != R) status = L;
        else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && status != L) status = R;

        if (GetAsyncKeyState(VK_ESCAPE)) {
            endgamestatus = 3; break;
        }
        // 控制加速、减速同原
        Sleep(sleeptime);
        snakemove();
    }
}

void welcometogame()//开始界面
{
    Pos(40, 12);
    printf("欢迎来到贪食蛇游戏！");
    Pos(40, 25);
    system("pause");
    system("cls");
    Pos(25, 12);
    printf("用↑.↓.←.→分别控制蛇的移动， F1 为加速，2 为减速\n");
    Pos(25, 13);
    printf("加速将能得到更高的分数。\n");
    system("pause");
    system("cls");
}

// 结束游戏，写日志后退出
void endgame() {
    writeUserLog();
    system("cls");
    Pos(24, 12);
    if (endgamestatus == 1)   printf("撞墙了。游戏结束!\n");
    else if (endgamestatus == 2) printf("咬到自己了。游戏结束!\n");
    else if (endgamestatus == 3) printf("主动退出。再见!\n");
    Pos(24, 13); printf("您的得分是%d\n", score);
    exit(0);
}

// 界面初始化同原
void gamestart() {
    system("mode con cols=100 lines=30");
    welcometogame();
    creatMap();
    initsnake();
    createfood();
}

void loginMenu() {
    int choice;
    while (1) {
        system("cls");
        printf("===== 贪吃蛇游戏登录系统 =====\n");
        printf("1. 登录\n");
        printf("2. 注册新用户\n");
        printf("3. 退出\n");
        printf("请选择操作：");
        scanf("%d", &choice);
        getchar(); // 吸收换行

        if (choice == 1) {
            if (authenticateUser()) {
                printf("登录成功！欢迎，%s。\n", currentUser.username);
                Sleep(1000);
                break;
            } else {
                printf("登录失败。按任意键继续。\n");
                getchar();
            }
        } else if (choice == 2) {
            registerUser();
        } else if (choice == 3) {
            printf("感谢使用，再见！\n");
            exit(0);
        } else {
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
    while (fread(&u, sizeof(User), 1, fp) == 1) {
        if (strcmp(u.username, uname) == 0 && strcmp(u.password, pwd) == 0) {
            currentUser = u;
            fclose(fp);
            return 1;  // 登录成功
        }
    }
    fclose(fp);
    return 0;
}

void registerUser() {
    char uname[32], pwd[32];
    printf("请输入新用户名: "); scanf("%31s", uname);
    printf("请输入密码: "); scanf("%31s", pwd);

    FILE *fp = fopen("users.dat", "ab+");
    if (!fp) {
        printf("无法打开用户文件！\n");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    int new_id = (int)(size / sizeof(User)) + 1;

    User newUser;
    newUser.id = new_id;
    strcpy(newUser.username, uname);
    strcpy(newUser.password, pwd);
    fwrite(&newUser, sizeof(User), 1, fp);
    fclose(fp);

    printf("注册成功！请使用新账户登录。\n");
    Sleep(1000);
}

void initWorkingDirectory() {
    char path[MAX_PATH];
    // 取出当前 exe 的完整路径
    GetModuleFileNameA(NULL, path, MAX_PATH);
    // 找到最后一个 '\'，把它之后的 exe 文件名截掉
    char *p = strrchr(path, '\\');
    if (p) *p = '\0';
    // 把当前工作目录切换到 exe 所在的文件夹
    SetCurrentDirectoryA(path);
}

int main() {
    // 切到 exe 同目录（ .cpp 所在目录）
    initWorkingDirectory();
    loginMenu();
    game_start_time = time(NULL);
    gamestart();
    gamecircle();
    endgame();
    return 0;
}