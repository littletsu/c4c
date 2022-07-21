#include <stdio.h>
#include <cjson/cJSON.h>

#include "keyboard_termios.h"
#include "request.h"
#include "html.h"

#define COM_PREVIEW_LENGTH 50
#define MAX_LIST_BOARDS 30

#define KEY_NAVIGATE_UP 0x41
#define KEY_NAVIGATE_DOWN 0x42
#define KEY_NAVIGATE_RIGHT 0x43
#define KEY_NAVIGATE_LEFT 0x44

#define KEY_ENTER 0xa

#define STATE_SELECTING 0
#define STATE_VIEWING 1

int selected_board = 0;
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
	struct BoardInfo board_info = get_board_from_boards(selected_board);
	char *url;
	asprintf(&url, "https://a.4cdn.org/%s/%i.json", board_info.name, 1);
	printf("%s", url);
	struct MemoryStruct chunk = request(url);
	board_json = cJSON_ParseWithLength(chunk.memory, chunk.size);
	threads = cJSON_GetObjectItemCaseSensitive(board_json, "threads");
	threads_size = cJSON_GetArraySize(threads);
	free(chunk.memory);
}

void get_boards() {
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

void print_thread(int i) {
	cJSON *thread = cJSON_GetArrayItem(threads, i);
	cJSON *posts = cJSON_GetObjectItemCaseSensitive(thread, "posts");
	int posts_size = cJSON_GetArraySize(posts);
	printf("posts: %i\n", posts_size);
	cJSON *op = cJSON_GetArrayItem(posts, 0);
	cJSON *opCom = cJSON_GetObjectItemCaseSensitive(op, "com");

	if(cJSON_IsString(opCom) && (opCom->valuestring != NULL)) {
		char *opCharCom = opCom->valuestring;
		char *innerText = htmlInnerText(opCharCom);
		printf("%s\n\n", innerText);
		free(innerText);
		return;
	}

	printf("empty\n\n");
	return;
	
}

void print_threads() {
	clear_screen();
	printf("got %i threads\n", threads_size);
	for(int i = 0; i < threads_size; i++) {
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
	print_threads();
};

int main(void) {
	get_boards();
	print_boards();
	printf("got %i boards\n", boards_size);
	while(1) {
		key = getkey();
//		printf("%x ", key);
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
