server: main.cpp  HttpConnection/HttpConnection.cpp ThreadPool/ThreadPool.cpp TimeList/TimeList.cpp TimeList/TimeListNode.cpp Utility/Utility.cpp WebServer.cpp Config/Config.cpp
	g++ -o server  $^ 

clean:
	rm  -r server