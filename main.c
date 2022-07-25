#include <stdio.h>
#include <cjson/cJSON.h>

#include "keyboard_termios.h"
#include "request.h"
#include "html.h"

#define MAX_LIST_BOARDS 30
#define MAX_LIST_THREADS 5
#define MAX_THREAD_PREVIEW_CHARS 300

#define KEY_NAVIGATE_UP 0x41
#define KEY_NAVIGATE_DOWN 0x42
#define KEY_NAVIGATE_RIGHT 0x43
#define KEY_NAVIGATE_LEFT 0x44

#define KEY_ENTER 0xa

#define STATE_SELECTING 0
#define STATE_VIEWING 1

#define UNSELECTED_BOARD_ESCAPE "\x1b[36m"
#define SELECTED_BOARD_ESCAPE "\x1b[31m"
#define BOARD_TITLE_ESCAPE "\x1b[34m"
#define END_ESCAPE "\033[0m"

int selected_board = 0;
int selected_thread = 0;
int selected_page = 1;
int boards_size = 0;

int key;
int state = 0;

cJSON *json;
cJSON *boards;

cJSON *board_json;
cJSON *threads;
int threads_size;

struct BoardInfo {
    char *name;
    char *title;
};

struct BoardInfo get_board_from_boards(int i) {
    cJSON *board = cJSON_GetArrayItem(boards, i);
    cJSON *name = cJSON_GetObjectItemCaseSensitive(board,
        "board");
    cJSON *title = cJSON_GetObjectItemCaseSensitive(board, "title");
    struct BoardInfo result;
    result.name = name->valuestring;
    result.title = title->valuestring;
    return result;
}

void get_board_threads() {
    clear_screen();

    struct BoardInfo board_info = get_board_from_boards(selected_board);
    printf("Loading page %i on /%s/ ...\n", selected_page, board_info.name);
    char *url;
    asprintf(&url, "https://a.4cdn.org/%s/%i.json", board_info.name, selected_page);
    struct MemoryStruct chunk = request(url);
    board_json = cJSON_ParseWithLength(chunk.memory, chunk.size);
    threads = cJSON_GetObjectItemCaseSensitive(board_json, "threads");
    threads_size = cJSON_GetArraySize(threads);
    free(chunk.memory);
    selected_thread = 0;
}

void get_boards() {
    clear_screen();
    printf("Getting boards...\n");
    struct MemoryStruct chunk = request("https://a.4cdn.org/boards.json");

    json = cJSON_ParseWithLength(chunk.memory, chunk.size);
    boards = cJSON_GetObjectItemCaseSensitive(json, "boards");  
    boards_size = cJSON_GetArraySize(boards);
    
    free(chunk.memory);
}

void clear_screen() {
    printf("\e[1;1H\e[2J");
}

void print_boards() {
    clear_screen();
    for(int i = selected_board; i < (selected_board + MAX_LIST_BOARDS) % boards_size + 1; i++) {
        struct BoardInfo board_info = get_board_from_boards(i);
        printf("%i. /%s/ - %s\n", i+1, board_info.name, board_info.title);
    }
}

int check_cjson_string(cJSON *cj) {
    return cJSON_IsString(cj) && (cj->valuestring != NULL);
}

void print_thread(int i) {
    cJSON *thread = cJSON_GetArrayItem(threads, i);
    cJSON *posts = cJSON_GetObjectItemCaseSensitive(thread, "posts");
    int posts_size = cJSON_GetArraySize(posts);
    cJSON *op = cJSON_GetArrayItem(posts, 0);
    cJSON *opCom = cJSON_GetObjectItemCaseSensitive(op, "com");
    cJSON *opSub = cJSON_GetObjectItemCaseSensitive(op, "sub");
    char *opCharSub;
    if(check_cjson_string(opSub)) {
        opCharSub = opSub->valuestring;
    } else opCharSub = "";
    if(check_cjson_string(opCom)) {
        
        printf("%sposts:%s %i | %s%s%s\n", i == selected_thread ? SELECTED_BOARD_ESCAPE : UNSELECTED_BOARD_ESCAPE, END_ESCAPE, posts_size, BOARD_TITLE_ESCAPE, opCharSub, END_ESCAPE);
        char *opCharCom = opCom->valuestring;
        char *innerText = htmlInnerText(opCharCom);
        if(i != selected_thread) {
            char preview[MAX_THREAD_PREVIEW_CHARS];
            strncpy(preview, innerText, MAX_THREAD_PREVIEW_CHARS);
            preview[MAX_THREAD_PREVIEW_CHARS] = 0;
            printf("%s%s\n\n", preview, strlen(innerText) > MAX_THREAD_PREVIEW_CHARS ? "..." : "");
        } else printf("%s\n\n", innerText);
        free(innerText);
        return;
    }

    printf("\n\n\n");
    return;
    
}

void print_threads() {
    clear_screen();
    
    for(int i = selected_thread; i < (selected_thread + MAX_LIST_THREADS); i++) {
        print_thread(i);
    }
}

void stateSelecting() {
    switch(key) {
        case KEY_NAVIGATE_UP:
            selected_board--;
            print_boards();
            break;
        case KEY_NAVIGATE_DOWN:
            selected_board++;
            print_boards();
            break;
        case KEY_ENTER:
            get_board_threads();
            stateViewing();
            state = STATE_VIEWING;
            break;
        case KEY_NAVIGATE_RIGHT:
            break;
        case KEY_NAVIGATE_LEFT:
            break;
    }
}

void stateViewing() {
    printf("got %i threads\n", threads_size);
    print_threads();
    switch(key) {
        case KEY_NAVIGATE_UP:
            selected_thread--;
            print_threads();
            break;
        case KEY_NAVIGATE_DOWN:
            selected_thread++;
            print_threads();
            break;
        case KEY_ENTER:
            break;
        case KEY_NAVIGATE_RIGHT:
            selected_page++;
            get_board_threads();
            print_threads();
            break;
        case KEY_NAVIGATE_LEFT:
            selected_page--;
            get_board_threads();
            print_threads();
            break;
    };
};

int main(void) {
    get_boards();
    print_boards();
    printf("got %i boards\n", boards_size);
    while(1) {
        key = getkey();
//      printf("%x ", key);
        switch(state) {
            case STATE_SELECTING:
                stateSelecting();
                break;
            case STATE_VIEWING:
                stateViewing();
                break;
        }
    }
    curl_global_cleanup();
    return 0;
}
