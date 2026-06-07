# DialKeyPad
ダイアルとキーを持つデバイスのファームウェア

# 追加方法
pico-examplesフォルダに回答したdevelopフォルダを格納する。
pico-examplesフォルダのCMakeLists.txt 末尾に以下の行を追加する。
add_subdirectory(develop)

pico-examplesフォルダでcmake -S. -B build を実行する。
pico-examples/developフォルダでmake を実行する。

