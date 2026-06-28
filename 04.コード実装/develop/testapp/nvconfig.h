#ifndef __NVCONFIG_H__
#define __NVCONFIG_H__
#include <stdint.h>
#include "pico/stdlib.h"

/** NV領域制御コードをRAMに展開するためのマクロ */
#define FLASH_RAM_FUNC(x) __not_in_flash_func(x)

/** データFLASH領域の先頭オフセットの定義 */
extern uint8_t __flash_data_start;
#define FLASH_TARGET_OFFSET ((uint32_t)&__flash_data_start - XIP_BASE)
#define FLASH_READ_ADDR ((uint32_t)&__flash_data_start)

/** NVデータブロック */
#define FLASH_OFFSET_CRC     0x00000000
#define FLASH_OFFSET_ACTDB   0x00000100
#define FLASH_ERASE_MASK (uint32_t)(~(4096-1)) /** Clear sector per 4096 bytes */
#define FLASH_WRITE_MASK (uint32_t)(~(256-1)) /** Clear sector per 4096 bytes */
#define FLASH_SECTOR_SIZE (uint32_t)(4096 * 4) /** 1 sector has 16384 Byte */
#define FLASH_SECTOR_PAGES 8 /** total flash area 128KB */
#define FLASH_DATA_AREA_SIZE (FLASH_SECTOR_SIZE * FLASH_SECTOR_PAGES) /** total flash area 128KB */
#define FLASH_TIMEOUT 1000 /** タイムアウト時間 */
/** 記憶することのできる最大ページ数 */
#define MAX_ACTION_PAGES 8 

typedef enum {
    CLCK_NONE = 0,
    CLICK_LEFT_SINGLE, 
    CLICK_LEFT_DOUBLE,
    CLICK_LEFT_TRIPLE,
    CLICK_LEFT_DRAG,
    CLICK_RIGHT_SINGLE, 
    CLICK_RIGHT_DOUBLE,
    CLICK_RIGHT_TRIPLE,
    CLICK_RIGHT_DRAG,
    CLICK_CENTER_SINGLE, 
    CLICK_CENTER_DOUBLE,
    CLICK_CENTER_TRIPLE,
    CLICK_CENTER_DRAG,
    CLICK_MAX_ID
} click_type;

/** 操作されたデバイスを示すID。 設定情報とリンクしている */
typedef enum {
    DIAL1_LEFT = 0,
    DIAL1_RIGHT,
    DIAL2_LEFT,
    DIAL2_RIGHT,
    ACTION_BTN1,
    ACTION_BTN2,
    ACTION_BTN3,
    ACTION_BTN4,
    ACTION_BTN5,
    ACTION_ASSIGN_MAX
} deviceAssign_type;

/** リポートを返すデバイスカテゴリ */
typedef enum {
    CATEGORY_KEYBOARD = 0,
    CATEGORY_MOUSE,
    DEVICE_CATEGORY_MAX
} deviceCategory_type;

typedef struct {
    uint32_t length;
    /** 同時に押せるキーの数 */
    uint32_t keycode[4];
} keycombo_type;

typedef struct {
    uint32_t length;
    /** 同時に送れるキーコンビネーションの数 */
    keycombo_type keycombo[32];
} key_type;

typedef struct {
    click_type click;
    uint32_t delta_x;
    uint32_t delta_y;
    uint32_t presskeycode;
} mouse_type;

typedef union {
    key_type act_key;
    mouse_type act_mouse;
} action_union;

typedef struct {
    deviceCategory_type category;
    action_union action;
} action_type;

typedef struct {
    /** DIALとボタンに割り当てたアクションデータ（設定値） */
    action_type ActionConfig[MAX_ACTION_PAGES][ACTION_ASSIGN_MAX];
} action_list_type;

typedef struct {
    uint32_t addr;
    uint32_t size; 
} flash_cmd_erase_t;

typedef struct {
    uint8_t *src_addr;
    uint32_t dst_addr;
    uint32_t size; 
} flash_cmd_write_t;

typedef union {
    flash_cmd_erase_t cmd_erase;
    flash_cmd_write_t cmd_write;
} flash_safe_cmd_t;

/** 不揮発メモリドライバ */
void FLASH_RAM_FUNC(nvram_mid_init_g)(void);
void FLASH_RAM_FUNC(nvram_drv_init_g)(int * flag);
void FLASH_RAM_FUNC(nvram_drv_erase_g)(uint32_t offset, uint32_t pages);
int FLASH_RAM_FUNC(nvram_mid_write_g)(uint32_t offset, const void *src, uint32_t len);
int FLASH_RAM_FUNC(nvram_drv_write_g)(uint32_t offset, const void *src, uint32_t len);
void FLASH_RAM_FUNC(call_flash_range_erase)(void *param);
void FLASH_RAM_FUNC(call_flash_range_write)(void *param);

#endif /* __NVRAM_H__ */
