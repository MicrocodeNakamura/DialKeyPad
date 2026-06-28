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

void main(void) {
    while (true) {
        sleep_ms(1000);
    }
}
