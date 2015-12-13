// HttpServer.cpp
// 监听端口提供http服务 on Win
// limingwei
// 2015-12-12 11:53:27

#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <io.h>
#include <windows.h>
#include <time.h>

// 常量
#define HTTP_PORT "88" // 监听端口
#define MAX_CONNECTION 10 // 最大连接数

// WinSock使用的库函数
#pragma comment(lib, "ws2_32.lib")


// 声明函数
char* sub_string(char* src, int pos, int length);
char * string_contact(const char *str1, const char *str2);
int index_of(char *str1, char *str2);
char* get_parameter(char* request_body_buf, char* parameter_key);
char* now_time_string();


// 入口
int main() {
    printf("INFO %s starting on port %s\n", now_time_string(), HTTP_PORT);

    WSADATA wsa={0};
    WSAStartup(MAKEWORD(2,2),&wsa);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    printf("INFO %s server_socket %d\n", now_time_string(), server_socket);

    // server_address
    struct sockaddr_in server_address;
    memset(&server_address,0,sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(HTTP_PORT));  
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);  

    // bind
    int bind_socket = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    printf("INFO %s bind socket %d return %d\n", now_time_string(), server_socket, bind_socket);

    // listen
    int listen_socket = listen(server_socket,MAX_CONNECTION);
    printf("INFO %s listening on port %s, listen_socket=%d\n", now_time_string(), HTTP_PORT, listen_socket);

    while(true) {
        struct sockaddr_in client_address;

        // accept
        int client_address_size = sizeof(client_address);
        int client_socket = accept(server_socket,(struct sockaddr*)&client_address,&client_address_size);
        printf("INFO %s #%d accepted ... ", now_time_string(), client_socket);
 
        // read_request
        char request_body_buf[20000];
 
        int client_socket_recv = recv(client_socket,request_body_buf,20000,0); // 返回应是数据长度
        printf(" recved recv_len=%d\n", client_socket_recv);

        char* callback_parameter_value;
        if(client_socket_recv > 0) {
            request_body_buf[client_socket_recv] = 0x00;
            callback_parameter_value = get_parameter(request_body_buf,"callback");
        }

        // write response
        char response_header[20000];
        char response_body[20000];
 
        memset(&response_header,0,sizeof(response_header));
        memset(&response_body,0,sizeof(response_body));

        sprintf(response_body,"%s({\"message\":\"hello world %d\"})",callback_parameter_value,rand()); 
        sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Length:%d\r\nConnection:close\r\n\r\n",strlen(response_body));

        send(client_socket, response_header, strlen(response_header), 0);
        send(client_socket, response_body, strlen(response_body), 0);

        printf("INFO %s #%d responsed, body_len=%d ... ", now_time_string(), client_socket, strlen(response_body));

        close(client_socket);

        printf("closed\n", client_socket);
    }

    return 0;
}


// sub_string
char* sub_string(char* src,int pos,int length) {  
    char* pch=src;
    char* dest=(char*)calloc(sizeof(char),length+1);    
    int i;
    pch=pch+pos;
    for(i=0; i<length; i++) {  
        dest[i]=*(pch++);
    }  
    dest[length]='\0';// 加上字符串结束符。  
    return dest; // 返回分配的字符数组地址。  
}


// 字符串拼接
char* string_contact(const char *str1,const char *str2) {
     char * result;
     result = (char*)malloc(strlen(str1) + strlen(str2) + 1); //str1的长度 + str2的长度 + \0;
     if(!result){ // 如果内存动态分配失败
        printf("Error: malloc failed in concat! \n");
        exit(EXIT_FAILURE);
     }
     strcpy(result,str1); 
     strcat(result,str2); // 字符串拼接
    return result;
}


// index_of
int index_of(char *str1, char *str2) {  
    char *p=str1;
    int i=0;
    p=strstr(str1,str2);
    if(p==NULL) {
        return -1;
    } else {
        while(str1!=p) {  
            str1++;  
            i++;
        }
    }
    return i;  
}

// now_time_string
char* now_time_string() {
    // 多次调用这段方法会出错
	/*
    time_t now;
    time(&now);

	char *now_buf="12345678901234567890123456789012";

    strftime(now_buf, 32, "%Y-%m-%d %H:%M:%S", localtime(&now));

    return now_buf;
	*/
	return NULL;
}

// get_parameter
char* get_parameter(char* request_body_buf,char* parameter_key) {
    char* parameter_from = string_contact("?", string_contact(parameter_key,"="));
    int parameter_from_index = index_of(request_body_buf, parameter_from);

    if(parameter_from_index < 0) {
        parameter_from = string_contact("&", string_contact(parameter_key,"="));
        parameter_from_index = index_of(request_body_buf, parameter_from);
    }
    if(parameter_from_index < 0) {
        return "";
    }

    char* string_from_parameter = sub_string(request_body_buf, parameter_from_index+1, strlen(request_body_buf));

    char* parameter_end = "&";
    int parameter_end_index = index_of(string_from_parameter, parameter_end);

    if(parameter_end_index < 0) {
        parameter_end_index = index_of(string_from_parameter, "HTTP/1.1");
    }
    if(parameter_end_index < 0) {
        int parameter_len = strlen(request_body_buf) - parameter_from_index - 10;
        return sub_string(request_body_buf, parameter_from_index + 10, parameter_len);
    }

    int parameter_len = parameter_end_index - strlen(parameter_key) - 1;
    return sub_string(request_body_buf, parameter_from_index + 10, parameter_len);
}