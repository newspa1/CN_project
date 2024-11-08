# 計網Project B11902167 鐘文駿

## Phase 1

`code/`資料夾內有3個file: `Makefile`, `Server.cpp`和Client.cpp。
- Makefile負責編譯Server.cpp和Client.cpp成server和client。
- server會不停的接收來自client的message，並判斷client當時正在執行的operation，給予正確的response。收到user Registration會把user的資訊存在`user.txt`
- client分成Login和Logout狀態，剛開始為Logout，可以選擇要Registration還是Login。想要以一個username Login需先用Registration註冊那個username(否則會跳`Login failed`，重複Registration同個username也會跳`Registration failed`)，Registration後才可以正常以這個username Login，進入Login的狀態。Login之後可以任意傳遞訊息給server，若輸入Logout即可正常登出，回到Logout的狀態。