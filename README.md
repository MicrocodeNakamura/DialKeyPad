# DialKeyPad
ダイアルとキーを持つデバイスのファームウェア

# 追加方法
pico-examplesフォルダに回答したdevelopフォルダを格納する。<BR>
pico-examplesフォルダのCMakeLists.txt 末尾に以下の行を追加する。<BR>
add_subdirectory(develop)<BR>

pico-examplesフォルダでcmake -S. -B build を実行する。<BR>
pico-examples/developフォルダでmake を実行する。<BR>

