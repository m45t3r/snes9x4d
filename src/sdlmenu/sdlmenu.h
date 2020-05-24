#ifndef ___MENU_h_
#define ___MENU_h_

#define MAX_MENU_ITEMS 16
#define MAX_SAVE_STATE_SLOTS 5
#define MENU_HEIGHT 224
#define MENU_WIDTH 256
#define SBUFFER 256

void menu_loop(void);
char *menu_romselector();
void menu_disptext(void);
void save_screenshot(char *fname);
void load_screenshot(char *fname);
void show_screenshot(void);
void capt_screenshot(void);
int batt_level(void);

#define strfmt(str, format, ...)                                                                                       \
	do {                                                                                                           \
		char tmp[SBUFFER];                                                                                     \
		snprintf(tmp, SBUFFER, format, __VA_ARGS__);                                                           \
		memcpy(str, tmp, SBUFFER);                                                                             \
	} while (0)

// ##__VA_ARGS__ is a GNU C++ extension
#define fmt_disptxt_item(item, format, ...) strfmt(item, format, item, ##__VA_ARGS__)

#define save_state_fname(fname, ext_format, args...)                                                                   \
	do {                                                                                                           \
		char ext[8];                                                                                           \
		snprintf(ext, PATH_MAX, ext_format, args);                                                             \
		memcpy(fname, S9xGetFilename(ext), PATH_MAX);                                                          \
	} while (0)

#endif // ___MENU_h_
