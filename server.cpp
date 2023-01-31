#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "cam.h"
#include "server.h"
#include "convert.h"
static int cam_flag = CAM_OFF;
void* cam_send(void* arg){
	int fd = (int)arg;
	int ret;
	unsigned int size;
	unsigned int index;
	int width= 640;
	int height=480;
	char *pic_data;
	convert_rgb_to_jpg_init();
	char *rgb = malloc(width * height * 3);
	char picbuf[1024*1024];
	while(1){
	if(CAM_ON == cam_flag){
		ret = camera_dqbuf((void **)&pic_data, &size, &index);
		if (ret == -1)
			break;
		//printf("pic size is %d\n",size);
		convert_yuv_to_rgb(pic_data,rgb, width, height, 24);
		int len=convert_rgb_to_jpg_work(rgb, picbuf, width, height, 24, 80);
		printf("len=%d\n",len);
		//先发送长度
		write(fd,&len,4);
		//再发送数据
		int all_len = 0;
	while(all_len<len){
		int cur_len = write(fd,picbuf+all_len, len-all_len);
		if(0 == cur_len)
		pthread_exit(NULL); //退出
			all_len = all_len + cur_len;
		}
	ret = camera_eqbuf(index);
	if (ret == -1)
	break;
	}
	else{
	sleep(1);
}
}
}
int socket_init(char* ip, int port){
	//创建监听套接字
	int fd= socket(PF_INET, SOCK_STREAM, 0);
	int opt=1;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(const void *)&opt,sizeof(opt));
	//绑定地址
	struct sockaddr_in myaddr;
	memset(&myaddr,0,sizeof(myaddr));
	myaddr.sin_family=PF_INET;
	myaddr.sin_port=htons(port);
	myaddr.sin_addr.s_addr=inet_addr(ip);
	bind(fd,(struct sockaddr*)(&myaddr),sizeof(myaddr));
	listen(fd,10);
	return fd;
}
#if 0
void* client_done(void* arg){
	int fd = (int)arg;
	struct sockaddr_in c_addr;
	int len = 0;
	while(1){
	s_connfd = accept(fd, (struct sockaddr *) &c_addr,&len);
	}
}	
#endif
int tcp_server_send_exact_nbytes(int connfd, const void *buf, int count){
	ssize_t ret;
	ssize_t total = 0;
	while (total != count) {
		ret = write(connfd, buf + total, count - total);
		if (ret == -1) {
		perror("server->write");
		return total;
		} else
			total += ret;
	}
	return total;
}
void* qt_done(void* arg){
	while(1){
	int fd = (int)arg;
	struct sockaddr_in c_addr;
	int len = 0;
	int cmd = -1;
	printf("qt accept begin\n");
	int a_fd = accept(fd, (struct sockaddr *) &c_addr,&len);
	printf("qt accept ok\n");
	//发送图片
	pthread_t id;
	pthread_create(&id,NULL,cam_send,(void*)a_fd);
	pthread_detach(id);
	//接收命令
	while(1){
	int ret = -1;
	ret = read(a_fd,&cmd,4);
	if(0==ret)
	break;
	switch(cmd){
	case CAM_ON:
	cam_flag = CAM_ON;
	break;
	case CAM_OFF:
	cam_flag = CAM_OFF;
	break;
	default:
	continue;
	}
	}
	close(a_fd);
	}
}
void* arm_done(void* arg){
while(1){
		int fd = (int)arg;
		struct sockaddr_in c_addr;
		int len = 0;
		int cmd = -1;
		printf("arm accept begin\n");
		int a_fd = accept(fd, (struct sockaddr *) &c_addr,&len);
		printf("arm accept ok\n");
		//接收命令
		while(1){
		int ret = -1;
		ret = read(a_fd,&cmd,4);
		if(0==ret)
		break;
		switch(cmd){
		case TEMP_SEND:{
		int temp = ds18b20();
		if(write(a_fd, &temp, 4)<0)
		perror("wirte");
		printf("temp=%d\n", temp);
		break;
		}
		case BEZZER_ON:
		pwm("pwm_on");
		break;
		case BEZZER_OFF:
		pwm("pwm_off");
		break;
		default:
		continue;
		}
		}
		close(a_fd);
		}
}
