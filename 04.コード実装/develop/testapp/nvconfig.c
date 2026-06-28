#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nvconfig.h"
#include "assert.h"
#include "pico/flash.h"
#include "hardware/flash.h"

typedef unsigned long uint32_t;
typedef unsigned char uint8_t;

#define TRUE 1
#define FALSE 0
/** NVメモリへのアクセステスト */
uint8_t testReadBuffer[1024];
uint8_t testWriteBuffer[1024];

/**
 * ブロック
 * 0-1 コード領域（128KB)
 * 2 ユーザー設定データ（A面）
 * 3 ユーザー設定データ（B面）
 */


uint32_t *flash_crc;
action_list_type *flash_actdb;


/** 書き換え用コードをRAMに転送する処理を実装する。 __not_in_flash_funcマクロ でRAMに転送する関数を定義する。
 * （sdkで実装されている機能）
 */

/** 初期値データのチェックを行い、counter値の新しいデータを設定値に転送する */
 void FLASH_RAM_FUNC(nvram_mid_init_g)(void) {
    int flag [MAX_ACTION_PAGES];
    int i;
    flash_crc   = (uint32_t *)FLASH_OFFSET_CRC;
    flash_actdb = (action_list_type *)FLASH_OFFSET_ACTDB;

    /** 各ブロックごとに初期値の確認を行い、不揮発領域の整合性が取れなかった場合は初期値の書き込みを行う。 */
    for ( i = 0 ; i < MAX_ACTION_PAGES ; i++ ) {
        flag[i] = 0;
        /** CRC値が設定されている場合（0xFFFF以外かつ、CRC値がヒットしない場合はそのセクションを初期化する） */
        if ( *((uint32_t *)((FLASH_SECTOR_SIZE * i) - 4)) != 0xffffffff ) {
//        if ( flash_crc != nvram_mid_crc(flash_actdb[i], (ACTION_ASSIGN_MAX * sizeof(action_type)) ) ) {
            /** ブロックの消去フラグセット */
            flag[i] = 1;
        }
    }
    /** CRC異常が発生したセクターのフォーマット */
    nvram_drv_init_g(flag);
}

/** 初期化フラグが設定されたセクターをフォーマット（消去）する */
void FLASH_RAM_FUNC(nvram_drv_init_g)(int * flag) {
    int i;
  
    for ( i = 0 ; i < MAX_ACTION_PAGES ; i++ ){
        if ( flag[i] != 0) {
            /** section erase */
            nvram_drv_erase_g ( (uint32_t)( FLASH_SECTOR_SIZE * i ), 1);
        }
    }
}

/** 指定 */
void FLASH_RAM_FUNC(nvram_drv_erase_g)(uint32_t offset, uint32_t pages) {
    int rc,i;
    flash_safe_cmd_t flash_cmd;
  
    /** section erase */
    for ( i = 0 ; i < pages ; i++ ){
        flash_cmd.cmd_erase.addr = offset;
        flash_cmd.cmd_erase.size = FLASH_SECTOR_SIZE;
        rc = flash_safe_execute(call_flash_range_erase, &flash_cmd.cmd_erase, FLASH_TIMEOUT);
        hard_assert(rc == PICO_OK);
    }
}

/** 書き込み効率が悪い。 細かくアドレス計算をしても良いが、ファイルシステムをあてがった方が現実的。 */
int FLASH_RAM_FUNC(nvram_mid_write_g)(uint32_t dest, const void *src, uint32_t len){
    int rc;
    int result = TRUE;
    flash_safe_cmd_t erase_cmd;

    /** フラッシュ書き換えのパラメータ指定内容の確認 */
    /** 書き込み先アドレスが、データフラッシュ領域の外側ならエラー */
    if  ( (dest + len) >= FLASH_DATA_AREA_SIZE) { return FALSE; }

    /** 書き換え対象セクションのイレース */
    dest += FLASH_TARGET_OFFSET; /** 書き込みアドレスのオフセット加算 */
    /** コマンドの削除用域はセクタサイズ（16K） */
    erase_cmd.cmd_erase.addr = dest;
    erase_cmd.cmd_erase.size = FLASH_SECTOR_SIZE;    
    rc = flash_safe_execute(call_flash_range_erase, &erase_cmd.cmd_erase,
        FLASH_TIMEOUT);
    hard_assert(rc == PICO_OK);

    /** フラッシュ書き換え。 書き換えサイズに合わせてページの書き換えを行う。
     * 書き換え対象のデータ格納アドレス（キャッシュメモリ）をsectorとpageに変換する処理が必要。
     */
    rc = nvram_drv_write_g( dest, src, FLASH_SECTOR_SIZE);
    return result;
}

int FLASH_RAM_FUNC(nvram_drv_write_g)(uint32_t offset, const void *src, uint32_t size){
    int rc;
    int result = TRUE;

    flash_safe_cmd_t write_cmd;
    write_cmd.cmd_write.src_addr = (uint8_t *)src;
    write_cmd.cmd_write.dst_addr = offset;
    write_cmd.cmd_write.size = size;
    /** アドレス補正 */
    write_cmd.cmd_write.dst_addr = write_cmd.cmd_write.dst_addr & (uint32_t)(FLASH_ERASE_MASK);

    /** flash_range_program */
    /* 書き換え領域の先端と終端は非更新領域の既存データと重ね合わせたうえで書き込みデータを作る */
    rc = flash_safe_execute(call_flash_range_write, &write_cmd, FLASH_TIMEOUT);
    hard_assert(rc == PICO_OK);

    return result;
}

void FLASH_RAM_FUNC(call_flash_range_erase)( void *pcmd ) {
    flash_safe_cmd_t *cmd = pcmd;
    /** アドレス補正。 セクタの境界以外のアドレス情報は除去する */
    cmd->cmd_erase.addr = cmd->cmd_erase.addr & (uint32_t)(FLASH_ERASE_MASK);

    /** FLASHメモリの削除 */
    flash_range_erase(cmd->cmd_erase.addr , cmd->cmd_erase.size );
}

void FLASH_RAM_FUNC(call_flash_range_write)( void *pcmd) {
    flash_safe_cmd_t *cmd = pcmd;
    uint32_t offset;
    offset = cmd->cmd_write.dst_addr & (FLASH_WRITE_MASK);
    flash_range_program(offset, cmd->cmd_write.src_addr, cmd->cmd_write.size);
}

#ifdef FEATURE_TESTCODE_ENABLE
/**------------------------------ Test code */
int nvram_test_check_memory( void ) {
    int i;
    int result = TRUE;
    for ( i = 0 ; i < 256 ; i++ ) {
        if ( testReadBuffer[i] != i ) {
            result = FALSE;
            break;
        }
    }
    return result;
}

int nvram_test_check_memory_reverse( void ) {
    int i;
    int result = TRUE;
    for ( i = 0 ; i < 256 ; i++ ) {
        if ( testReadBuffer[i] != 256-i ) {
            result = FALSE;
            break;
        }
    }
    return result;
}

int nvram_test_g(void){
    int testno = 1;
    int result = 0;
    int i;
    printf("### NV RAM testing.\n");
    memcpy( testReadBuffer, (void *)(FLASH_READ_ADDR), 256 );
    if (nvram_test_check_memory() != TRUE) {
            printf ("Read NV ram data not match estimate values.\n" );
        result = 2;
    }
    if ( result != 0 ) {
        printf ("Error occured. Test no %d, error code %d\n", testno, result );
    }
    testno++;
    
//    if ( result == 0 ){
        for ( i = 0 ; i < 256 ; i++ ){ testWriteBuffer[i] = 255-i; }
        if ( nvram_mid_write_g(0,testWriteBuffer, FLASH_SECTOR_SIZE) != TRUE ) {
            printf ("NV ram data could not write.\n" );
            result = 3;
            for ( i = 0 ; i < 256 ; i++ ){ testReadBuffer[i] = 0; }
            if (nvram_test_check_memory_reverse() != TRUE) {
                printf ("Read NV ram data not match estimate values.\n" );
                result = 2;
            }
        }
        if ( result != 0 ) {
            printf ("Error occured. Test no %d, error code %d\n", testno, result );
        }
//    }
    testno++;

    return result;
}
#endif