# 計網Final Project

Client連上Server時會是Logout狀態

## Compile
輸入`make`以編譯Server.cpp和Client.cpp，分別會產生server和client的執行檔，Server可以輸入`./server`創建server，port為8081，client可以輸入`./client`以連上server。輸入`make clean`以清除server和client的執行檔。


## Client Logout Status
Client在logout的status時，可以輸入三種指令 **(registration/login/logout)**

### registration
註冊新帳號的指令為`registration`，輸入後會需要再次輸入想要註冊的username。Username會被存在user.txt裡面，每一行都是一個user。當此username已存在時，會跳出`registration failed`的錯誤訊息。當註冊成功時，server會在`home/`創建一個新的directory`home/${username}`，這個directory會被用來當作這個user唯一可以access的目錄。

### login
登入帳號的指令為`login`，輸入後會需要再次輸入想要登入的username。login指令只能在logout status時使用。Server接著會去找這個username是否已被註冊，如果是，則登入成功，進入login status。反之則會登入時會跳`login failed`，繼續保持logout status。

### logout
登出帳號得指令為`logout`，可以分為在login status或是logout status。如果在login status輸入`logout`，則client會登出目前的user，進入到logout status。如果再logout status輸入`logout`，則這個client會exit，斷開與server當前thread的連線，並讓thread回歸thread pool。

## Client Login Status
Client在logout的status時，可以輸入四種指令 **(logout/message/ls/transfer)**

### message
Client想傳遞message給其他user的指令為`message ${target_username}`，輸入後會再次輸入想要傳遞的message。如果target_user不存在，則會跳出錯誤訊息，反之則會跳出success的訊息。target_user不會馬上收到訊息，必須要等當前指令結束後或idle狀態輸入任意指令後才會收到訊息。

### ls
Client想要查看自己家目錄底下有什麼檔案的指令為`ls`，輸入後會逐行顯示檔案名稱。每個Client剛註冊時會有自己的家目錄`home/${username}`，`ls`指令會列出這個資料夾的檔案。

### transfer
Client想傳遞檔案的指令為`transfer ${target_user} --file ${filename}`，client會從自己的家目錄傳file給server的暫存資料夾(relay mode)，接著server會再傳給另一個user，儲存在該user的家目錄。傳遞完成之後server會把該暫存檔刪除。與`message`指令相同，需先等target_user結束當前指令後或在idle狀態輸入任意指令後才會成功收到檔案，接著該檔案可以在另一個user輸入`ls`找到。