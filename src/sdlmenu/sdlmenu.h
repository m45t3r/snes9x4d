#ifndef ___MENU_h_
#define ___MENU_h_

#define MAX_MENU_ITEMS 16
#define MAX_SAVE_STATE_SLOTS 5
#define MENU_HEIGHT 224
#define MENU_WIDTH 256
#define SBUFFER PATH_MAX

void menu_loop(void);
char *menu_romselector();
void menu_disptext(void);
void save_screenshot(char *fname);
void load_screenshot(char *fname);
void show_screenshot(void);
void capt_screenshot(void);
int batt_level(void);

#define strfmt(str, format, args...)                                                                                   \
	do {                                                                                                           \
		char tmp[SBUFFER];                                                                                     \
		snprintf(tmp, SBUFFER, format, args);                                                                  \
		memcpy(str, tmp, SBUFFER);                                                                             \
	} while (0)

#endif // ___MENU_h_
