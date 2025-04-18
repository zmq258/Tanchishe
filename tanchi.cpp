#include<stdio.h>
#include<time.h>
#include<windows.h>
#include<stdlib.h>
#include<string.h>

#define U 1
#define D 2
#define L 3 
#define R 4       //蛇的状态，U：上 ；D：下；L:左 R：右
typedef struct {
    char username[50];
    char password[50];
    time_t registrationTime; // 新增：记录用户注册时间
} User;

typedef struct {
    char username[50];
    time_t startTime;
    time_t endTime;
    int score;
} GameLog;

typedef struct SNAKE //蛇身的一个节点
{
    int x;
    int y;
    struct SNAKE* next;
}snake;

//全局变量//
int score = 0, add = 10;//总得分与每次吃食物得分。
int status, sleeptime = 200;//每次运行的时间间隔
snake* head, * food;//蛇头指针，食物指针
snake* q;//遍历蛇的时候用到的指针
int endgamestatus = 0; //游戏结束的情况，1：撞到墙；2：咬到自己；3：主动退出游戏。
char currentUser[50];
const char *userDataFile = "users.dat";
const char *gameLogFile = "game_log.dat";

//声明全部函数//
void Pos(int x, int y);
void creatMap();
void initsnake();
int biteself();
void createfood();
void cantcrosswall();
void snakemove();
void pause();
void gamecircle();
void welcometogame();
void endgame();
void gamestart();
int userExists(const char *username);
int loginUser();
void registerUser();
void saveGameLog();
void showGameLog();

// 检查用户是否已存在
int userExists(const char *username) {
    FILE *file = fopen(userDataFile, "r");
    if (!file) {
        return 0;
    }
    User user;
    while (fread(&user, sizeof(User), 1, file)) {
        if (strcmp(user.username, username) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void Pos(int x, int y)//设置光标位置
{
    COORD pos;
    HANDLE hOutput;
    pos.X = x;
    pos.Y = y;
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOutput, pos);
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
    food_1 = (snake*)malloc(sizeof(snake));
    do {
        food_1->x = (rand() % 26) * 2 + 2; // 确保 x 为偶数且在地图范围内
        food_1->y = rand() % 24 + 1;       // 确保 y 在地图范围内
        q = head;
        while (q != NULL)
        {
            if (q->x == food_1->x && q->y == food_1->y) //判断蛇身是否与食物重合
            {
                break;
            }
            q = q->next;
        }
    } while (q != NULL);

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
    }
    else if (status == D)
    {
        nexthead->x = head->x;
        nexthead->y = head->y + 1;
    }
    else if (status == L)
    {
        nexthead->x = head->x - 2;
        nexthead->y = head->y;
    }
    else if (status == R)
    {
        nexthead->x = head->x + 2;
        nexthead->y = head->y;
    }

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

    if (biteself() == 1)       //判断是否会咬到自己
    {
        endgamestatus = 2;
        endgame();
    }
}

void pause()//暂停
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

// 定义颜色设置函数
void setTextColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
void saveGameLog(time_t startTime, time_t endTime, int score) 
{
    GameLog log;
    strcpy(log.username, currentUser);
    log.startTime = startTime;
    log.endTime = endTime;
    log.score = score;
    FILE *logFile = fopen(gameLogFile, "a");
    if (logFile) {
        fwrite(&log, sizeof(GameLog), 1, logFile);
        fclose(logFile);
    }
}

void gamecircle()//控制游戏        
{
    time_t startTime = time(NULL);
    Pos(64, 10);
    printf("***%s正在游戏中***", currentUser);
    Pos(64, 11);
    printf("按F5显示游戏用户日志");
    Pos(64, 15);
    printf("不能穿墙，不能咬到自己\n");
    Pos(64, 16);
    printf("用↑.↓.←.→分别控制蛇的移动.");
    Pos(64, 17);
    printf("F1 为加速，F2 为减速\n");
    Pos(64, 18);
    printf("ESC ：退出游戏.space：暂停游戏.");
    Pos(64, 20);
    status = R;
    while (1)
    {
        Pos(64, 12);
        printf("得分：%d  ", score);
        Pos(64, 13);
        printf("每个食物得分：%d分", add);
        if (GetAsyncKeyState(VK_UP) && status != D)
        {
            status = U;
        }
        else if (GetAsyncKeyState(VK_DOWN) && status != U)
        {
            status = D;
        }
        else if (GetAsyncKeyState(VK_LEFT) && status != R)
        {
            status = L;
        }
        else if (GetAsyncKeyState(VK_RIGHT) && status != L)
        {
            status = R;
        }
        else if (GetAsyncKeyState(VK_SPACE))
        {
            pause();
        }
        else if (GetAsyncKeyState(VK_ESCAPE))
        {
            endgamestatus = 3;
            break;
        }
        else if (GetAsyncKeyState(VK_F1))
        {
            if (sleeptime >= 50)
            {
                sleeptime = sleeptime - 30;
                add = add + 2;
                if (sleeptime == 320)
                {
                    add = 2;//防止减到1之后再加回来有错
                }
            }
        }
        else if (GetAsyncKeyState(VK_F2))
        {
            if (sleeptime < 350)
            {
                sleeptime = sleeptime + 30;
                add = add - 2;
                if (sleeptime == 350)
                {
                    add = 1;  //保证最低分为1
                }
            }
        }
        else if (GetAsyncKeyState(VK_F5))
        {
            showGameLog();
        }
        Sleep(sleeptime);
        snakemove();
    }
    time_t endTime = time(NULL);
    saveGameLog(startTime, endTime, score);
}

// 保存游戏日志
// 修改函数定义，使其接收三个参数

int compareScore(const void *a, const void *b) {
    return ((GameLog *)b)->score - ((GameLog *)a)->score;
}
// 显示游戏日志
void showGameLog() {
    FILE *logFile = fopen(gameLogFile, "r");
    if (!logFile) {
        printf("无法打开游戏日志文件。\n");
        return;
    }

    // 动态分配日志数组
    GameLog *logs = NULL;
    int logCount = 0;
    int capacity = 10;
    logs = (GameLog *)malloc(capacity * sizeof(GameLog));
    if (!logs) {
        printf("内存分配失败。\n");
        fclose(logFile);
        return;
    }

    while (fread(&logs[logCount], sizeof(GameLog), 1, logFile)) {
        logCount++;
        if (logCount >= capacity) {
            capacity *= 2;
            GameLog *temp = (GameLog *)realloc(logs, capacity * sizeof(GameLog));
            if (!temp) {
                printf("内存分配失败。\n");
                free(logs);
                fclose(logFile);
                return;
            }
            logs = temp;
        }
    }
    fclose(logFile);

    // 按得分从高到低排序
    
    qsort(logs, logCount, sizeof(GameLog), compareScore);

    // 显示排序后的日志
    system("cls");
    Pos(20, 5);
    printf("所有用户游戏日志（按得分从高到低排序）\n");
    Pos(20, 6);
    printf("用户名\t注册时间\t开始时间\t结束时间\t得分\n");
    int y = 7;
    for (int i = 0; i < logCount; i++) {
        char startTimeStr[26];
        char endTimeStr[26];
        char registrationTimeStr[26];
        // 获取用户注册时间
        User user;
        FILE *userFile = fopen(userDataFile, "r");
        if (userFile) {
            while (fread(&user, sizeof(User), 1, userFile)) {
                if (strcmp(user.username, logs[i].username) == 0) {
                    ctime_s(registrationTimeStr, sizeof(registrationTimeStr), &user.registrationTime);
                    registrationTimeStr[strcspn(registrationTimeStr, "\n")] = 0; // 去除换行符
                    break;
                }
            }
            fclose(userFile);
        }
        ctime_s(startTimeStr, sizeof(startTimeStr), &logs[i].startTime);
        ctime_s(endTimeStr, sizeof(endTimeStr), &logs[i].endTime);
        startTimeStr[strcspn(startTimeStr, "\n")] = 0; // 去除换行符
        endTimeStr[strcspn(endTimeStr, "\n")] = 0; // 去除换行符
        Pos(20, y);
        printf("%s\t%s\t%s\t%s\t%d\n", logs[i].username, registrationTimeStr, startTimeStr, endTimeStr, logs[i].score);
        y++;
    }

    free(logs);
    system("pause");
    system("cls");
    endgame();
}

void welcometogame()//开始界面
{
    Pos(40, 12);
    printf("欢迎来到贪食蛇游戏！");
    Pos(40, 25);
    system("pause");
    system("cls");
    Pos(25, 12);
    printf("用↑.↓.←.→分别控制蛇的移动， F1 为加速，F2 为减速\n");
    Pos(25, 13);
    printf("加速将能得到更高的分数。\n");
    system("pause");
    system("cls");
}

void endgame()//结束游戏
{
    system("cls");
    Pos(24, 12);
    if (endgamestatus == 1)
    {
        printf("对不起，您撞到墙了。游戏结束!");
    }
    else if (endgamestatus == 2)
    {
        printf("对不起，您咬到自己了。游戏结束!");
    }
    else if (endgamestatus == 3)
    {
        printf("您已经结束了游戏。");
    }
    Pos(24, 13);
    printf("您的得分是%d\n", score);

    Pos(24, 15);
    printf("1. 再来一把");
    Pos(24, 16);
    printf("2. 退出游戏");
    Pos(24, 17);
    Pos(24, 18);
    printf("请输入你的选择 (1/2/F5): ");

    while (1) {
        if (GetAsyncKeyState(VK_F5)) {
            showGameLog();
            break;
        }
        if (GetAsyncKeyState(VK_RETURN)) {
            int choice;
            
            scanf("%d", &choice);
            switch (choice) {
                case 1:
                    // 重置游戏状态
                    score = 0;
                    add = 10;
                    sleeptime = 200;
                    endgamestatus = 0;
                    // 重新初始化蛇和食物
                    free(head);
                    free(food);
                    gamestart();
                    gamecircle();
                    break;
                case 2:
                    exit(0);
                default:
                    Pos(24, 20);
                    printf("无效的选择，请重新输入。");
                    Sleep(1500);
                    endgame();
            }
            break;
        }
    }
}

// 显

void gamestart()//游戏初始化
{
    system("mode con cols=100 lines=30");
    welcometogame();
    creatMap();
    initsnake();
    createfood();
}
void showStartMenu() {
    system("cls");
    Pos(40, 12);
    printf("欢迎来到贪食蛇游戏！");
    Pos(40, 14);
    printf("1. 注册");
    Pos(40, 15);
    printf("2. 登录");
    Pos(40, 16);
    printf("请输入你的选择 (1/2): ");
    int choice=0;
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            registerUser();
            break;
        case 2:
            if (!loginUser()) {
                showStartMenu(); // 登录失败，重新显示菜单
            }
            break;
        default:
            Pos(40, 18);
            printf("无效的选择，请重新输入。");
            Sleep(1500);
            showStartMenu();
    }
}


int loginUser() {
    User user, userInput;
    int loginFailed = 0;
    while (1) {
        setTextColor(11); // 设置文本颜色为浅蓝色
        printf("+----------------------+\n");
        printf("|      用户登录页面      |\n");
        printf("+----------------------+\n");
        setTextColor(7); // 设置文本颜色为默认白色
        if (loginFailed) {
            setTextColor(4); // 设置文本颜色为红色
            printf("用户名或密码错误，请重新输入。\n");
            setTextColor(7); // 设置文本颜色为默认白色
        }
        printf("请输入用户名: ");
        scanf("%s", userInput.username);
        printf("请输入密码: ");
        scanf("%s", userInput.password);

        FILE *file = fopen(userDataFile, "r");
        if (!file) {
            setTextColor(4); // 设置文本颜色为红色
            printf("无法打开用户数据文件。\n");
            setTextColor(7); // 设置文本颜色为默认白色
            return 0;
        }
        int found = 0;
        while (fread(&user, sizeof(User), 1, file)) {
            if (strcmp(user.username, userInput.username) == 0) {
                found = 1;
                if (strcmp(user.password, userInput.password) == 0) {
                    fclose(file);
                    strcpy(currentUser, user.username);
                    setTextColor(2); // 设置文本颜色为绿色
                    printf("登录成功！\n");
                    setTextColor(7); // 设置文本颜色为默认白色
                    return 1;
                }
            }
        }
        fclose(file);
        loginFailed = 1;
    }
}

void registerUser() {
    User user;
    setTextColor(11); // 设置文本颜色为浅蓝色
    printf("+----------------------+\n");
    printf("|      用户注册页面      |\n");
    printf("+----------------------+\n");
    setTextColor(7); // 设置文本颜色为默认白色
    printf("请输入用户名: ");
    scanf("%s", user.username);
    if (userExists(user.username)) {
        setTextColor(4); // 设置文本颜色为红色
        printf("用户名已存在。\n");
        setTextColor(7); // 设置文本颜色为默认白色
        return;
    }
    printf("请输入密码: ");
    scanf("%s", user.password);
    user.registrationTime = time(NULL); // 记录注册时间

    FILE *file = fopen(userDataFile, "a");
    if (!file) {
        setTextColor(4); // 设置文本颜色为红色
        printf("无法打开用户数据文件。\n");
        setTextColor(7); // 设置文本颜色为默认白色
        return;
    }
    fwrite(&user, sizeof(User), 1, file);
    fclose(file);
    setTextColor(2); // 设置文本颜色为绿色
    printf("注册成功！将跳转到登录页面。\n");
    setTextColor(7); // 设置文本颜色为默认白色
    Sleep(1500); // 等待1.5秒
    loginUser();
}

// 新增函数：显示开始菜单


int main()
{
    srand((unsigned)time(NULL)); // 初始化随机数种子
    showStartMenu();

    while (1) {
        gamestart();
        gamecircle();
        endgame();
    }

    return 0;
}