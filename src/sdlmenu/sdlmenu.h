#ifndef ___MENU_h_
#define ___MENU_h_

#define MAX_MENU_ITEMS 15
#define MAX_SAVE_STATE_SLOTS 3
#define MENU_HEIGHT 224
#define MENU_WIDTH 256
#define SBUFFER 256

void menu_loop(void);
char* menu_romselector();
void menu_disptext(void);
void save_screenshot(char *fname);
void load_screenshot(char *fname);
void show_screenshot(void);
void capt_screenshot(void);
int batt_level(void);

#endif // ___MENU_h_
