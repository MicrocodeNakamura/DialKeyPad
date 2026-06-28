/* examplesに自作のプロジェクトを追加。 このコードをベースに新規プロジェクトを作成する */

/* 【プロジェクトの追加手順】
 * 1.追加するプロジェクトのフォルダを、pico-examples/developフォルダの配下に追加する
 *   pico-examples/develop/myapp
 * 2.pico-examples/develop/CMakeLists.txtの末尾に以下の行を追加する
 *   add_subdirectory_exclude_platforms(myapp)
 * 3.pico-examples/develop/myappにCMakeLists.txtを追加する
 * 4.追加したCMakelistsに以下の内容を記述する。
 * 
 * add_executable(myapp myapp.c)
 * target_sources(myapp PRIVATE myapp.c)
 * target_link_libraries(myapp PRIVATE pico_stdlib)
 * pico_add_extra_outputs(myapp)
 * # add url via pico_set_program_url
 * example_auto_set_url(myapp)
 * 
 * 【ビルド確認】
 * 1.pico-examplesフォルダでcmake -S . -B buildを実行して、
 *   pico-examples/buildフォルダにビルドファイルを生成出来ていることを確認する
 * 2.pico-examples/buildフォルダでmakeを実行して、myapp.uf2ファイルが出来ていることを確認する
 * 
 * 【注意点】
 * ・myapp以外の名称を使う場合は、読み替えを行うこと
 * ・ライブラリの追加が必要な場合は、参照したpico-examplesのCMあけLists.txtを手掛かりに、
 *   target_link_librariesの行に追加すること
 */
#include "pico/stdlib.h"
#include "hardware/timer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "nvconfig.h"

/**
* examples drivers 
* あとでマージする用のドライバコード。 とりあえずの実装検証用サンプル
*/
/* Interrupt handler use dial only. Last 2 io ports are used by extend dials */
#define MAX_GPIO_IRQS 4
/* GPIO割り込みの端子番号 */
#define IRQ_DIAL_PIN_1 12
#define IRQ_DIAL_PIN_2 13
#define DEBUG_LED_PIN 25
#define SW_PIN_1 26
#define SW_PIN_2 27
#define SW_PIN_3 28
#define PHASE_DIFFERENT_THRETHOULD 30000 /* us */

/* GPIO割り込みのコールバック関数型 */
typedef void (*callback_gpio_t)(uint32_t events);

typedef struct {
    callback_gpio_t callback;
    int gpio_no;
    int attr; /* edge or level interrupt attribute. */
} irq_register_gpio_t;

irq_register_gpio_t irq_register_gpio[MAX_GPIO_IRQS];

static uint8_t last_irq = 0;
static uint8_t debug_right = 0;
static uint8_t debug_left = 0;

void init_systime_mid_hwtimer_g(void);
uint32_t get_systime_mid_hwtimer_g(void);
int main(void);
int microrl_main(void);

extern void usb_init(void);
extern void usb_loop(void);

/* GPIO割り込みコールバック関数  カウンタの値を取得して、ダイアル左右の操作を判定して終了する */
void dial_callback_pin1_gpio_drv_i( uint32_t events ){
    uint32_t dt;
    dt = get_systime_mid_hwtimer_g();
    if ( last_irq == IRQ_DIAL_PIN_2 ) {
        if ( dt < PHASE_DIFFERENT_THRETHOULD ) {
            // Dial right
            debug_left++;
        } 
    }
    last_irq = IRQ_DIAL_PIN_1;
}
void dial_callback_pin2_gpio_drv_i( uint32_t events ){
    uint32_t dt;
    dt = get_systime_mid_hwtimer_g();
    if ( last_irq == IRQ_DIAL_PIN_1 ) {
        if ( dt < PHASE_DIFFERENT_THRETHOULD ) {
            // Dial left
            debug_right++;
        }
    }
    last_irq = IRQ_DIAL_PIN_2;
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == IRQ_DIAL_PIN_1) {
        dial_callback_pin1_gpio_drv_i(events);
    } else if (gpio == IRQ_DIAL_PIN_2) {
        dial_callback_pin2_gpio_drv_i(events);
    }
}

/* GPIO初期化関数 他のスイッチの初期化処理も追加する。*/
void io_init_gpio_drv_g() {
    stdio_init_all();

//    gpio_init(DEBUG_LED_PIN);
//    gpio_set_dir(DEBUG_LED_PIN, GPIO_OUT);

    gpio_init(IRQ_DIAL_PIN_2);
    gpio_set_dir(IRQ_DIAL_PIN_2, GPIO_IN);
    gpio_pull_down(IRQ_DIAL_PIN_2);

    gpio_init(IRQ_DIAL_PIN_1);
    gpio_set_dir(IRQ_DIAL_PIN_1, GPIO_IN);
    gpio_pull_down(IRQ_DIAL_PIN_1);

    gpio_init(IRQ_DIAL_PIN_2);
    gpio_set_dir(IRQ_DIAL_PIN_2, GPIO_IN);
    gpio_pull_down(IRQ_DIAL_PIN_2);

    gpio_init(SW_PIN_1);
    gpio_set_dir(SW_PIN_1, GPIO_IN);
//    gpio_pull_up(SW_PIN_1);

    gpio_init(SW_PIN_2);
    gpio_set_dir(SW_PIN_2, GPIO_IN);
//    gpio_pull_down(SW_PIN_2);

    gpio_init(SW_PIN_3);
    gpio_set_dir(SW_PIN_3, GPIO_IN);
//    gpio_pull_up(SW_PIN_3);
    
    gpio_set_irq_enabled_with_callback(IRQ_DIAL_PIN_1, GPIO_IRQ_EDGE_FALL , true, &gpio_callback);
    gpio_set_irq_enabled(IRQ_DIAL_PIN_1, 0x4u, true);    /** Fall detect(0x04) */
    gpio_set_irq_enabled(IRQ_DIAL_PIN_2, 0x4u, true);    
}

/* GPIO IRQ initialize function */
void ireqinit_drv_gpio_g(void) {
    irq_register_gpio[0].gpio_no = IRQ_DIAL_PIN_1;
    irq_register_gpio[0].attr = GPIO_IRQ_EDGE_FALL;
    irq_register_gpio[0].callback = &dial_callback_pin1_gpio_drv_i;
    irq_register_gpio[1].gpio_no = IRQ_DIAL_PIN_2;
    irq_register_gpio[1].attr = GPIO_IRQ_EDGE_FALL;
    irq_register_gpio[1].callback = &dial_callback_pin2_gpio_drv_i;
    /* irq function regist to gpio service on sdk. */

}

static absolute_time_t app_systime_previous;

void init_systime_mid_hwtimer_g(void) {
    app_systime_previous = get_absolute_time();
}

uint32_t get_systime_mid_hwtimer_g(void) {
    int64_t dt;
    absolute_time_t app_systime;

    app_systime = get_absolute_time();
    dt = absolute_time_diff_us(app_systime_previous, app_systime );
    app_systime_previous = app_systime;
    if (dt > 0xffffffff ) {
        // dt disable overlap.
        dt = 0xffffffff;
    }
    return (uint32_t)dt;
}

static absolute_time_t io_systime_previous;
/** 50msごとにスイッチ入力のポーリングを行う。 
 *  スイッチのチェックタイミングは厳密ではないのでシステム時間が50ms以上経過していれば
 *  スイッチのチェック処理を呼び出す */
void ioLoop_App(void) {
    int64_t dt;
    bool swstate1 = true;
    bool swstate2 = true;
    bool swstate3 = true;
    absolute_time_t io_systime;

    io_systime = get_absolute_time();
    dt = absolute_time_diff_us(io_systime_previous, io_systime );
    if (dt > 50000) {
        io_systime_previous = io_systime;
    }

    swstate1 = gpio_get(SW_PIN_1);
    if (!swstate1) {
//        gpio_put(DEBUG_LED_PIN, 0);
    } else {
//        gpio_put(DEBUG_LED_PIN, 1);
    }
    swstate2 = gpio_get(SW_PIN_2);
    if (!swstate2) {
    } else {
    }
    swstate3 = gpio_get(SW_PIN_3);
    if (!swstate3) {
    } else {
    }
}

int main(void) {
    /** USB initialize. */
    usb_init();

    /** GPIO Initialize section. */
    init_systime_mid_hwtimer_g();
    io_init_gpio_drv_g();
    printf("\n### Title \n");

    nvram_test_g();

//    nvram_mid_init_g();

    io_systime_previous = get_absolute_time();

    /** 各関数ごとでシステム時間を参照し、ポーリング周期に合わせて処理を実施する */
    while (1) {
        /** スイッチ（IO）入力ポーリング */
        ioLoop_App();
        
        /** USBの応答生成(10msおき)　*/
        usb_loop();

        /** Command line interface */
        (void)microrl_main();
    }
    return 0;
}
