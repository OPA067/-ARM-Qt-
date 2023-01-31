#include "widget.h"
#include "ui_widget.h"
#include<QDebug>
#include<QByteArray>
#include<QString>
Widget::Widget(QWidget *parent)
 	: QWidget(parent)
 	, ui(new Ui::Widget){
 		ui->setupUi(this);
 		socket = new QTcpSocket();
 		arm_socket = new QTcpSocket();
 		temp_time = new QTimer();
 		connect(socket,&QTcpSocket::connected,this,&Widget::connection);
 		connect(temp_time,SIGNAL(timeout()),this,SLOT(get_temp_data()));
 		connect(socket,SIGNAL(readyRead()),this,SLOT(pic_show()));
 		pic_flag=true;
 		ui->progressBar->setValue(0);
 		this->setWindowTitle("智能酿酒系统");
}
Widget::~Widget(){
 	delete ui;
}
void Widget::on_pushButton_clicked(){
 	QString ip="192.168.2.32";
 	int port=8888;
 	QString armip="192.168.2.32";
 	int armport=7777;
 	socket->connectToHost(ip,port);
 	arm_socket->connectToHost(armip,armport);
}
void Widget::connection(){
 	ui->lineEdit->setText("连接成功");
}
void Widget::pic_show(){
 	if(pic_flag){
 		if(sizeof(pic_size)>socket->bytesAvailable())
 			return ;
 		socket->read((char*)&pic_size,sizeof(pic_size));
 		qDebug()<<"pic_size"<<pic_size;
 		pic_flag=false;
 	}
 	else{
 		unsigned char pic_data[480*640*3];
 		memset(pic_data,0,sizeof(pic_data));
 		if(pic_size>socket->bytesAvailable())
 			return;
 		int l = socket->read((char*)pic_data,pic_size);
 		QPixmap pix;
 		pix.loadFromData((const uchar*)pic_data,pic_size,"JPEG");
 		ui->cam_label->setPixmap(pix);
 		pic_flag = true;
 	}
}
void Widget::get_temp_data(){
 	int cam=2;
 	int temp=20;//初始化为 20
 	arm_socket->write((char*)&cam,sizeof(cam));
 	while(arm_socket->bytesAvailable()>4){
 	arm_socket->waitForReadyRead(10);
 	}
 	arm_socket->read((char*)&temp,sizeof(temp));
 	if(temp>40||temp<10){
 	cam=3;
 	arm_socket->write((char*)&cam,sizeof (cam));
 	}
 	else{
 	cam=4;
 	arm_socket->write((char*)&cam,sizeof (cam));
 	}
 	QString S = QString::number(temp);
 	ui->progressBar->setValue(temp);
 	ui->lineEdit_2->setText(S);
}
void Widget::on_pushButton_2_clicked(){
 	if(ui->pushButton_2->text() == "开始"){
 		int cam=0;
 		socket->write((char*)&cam,sizeof(cam));
 		ui->pushButton_2->setText("停止");
 	}
 	else{
 		int cam=1;
 		socket->write((char*)&cam,sizeof(cam));
 		ui->pushButton_2->setText("开始");
	}
}
void Widget::on_pushButton_3_clicked(){
 	if(ui->pushButton_3->text()=="开始"){
 		temp_time->start(2000);
 		ui->pushButton_3->setText("停止");
 	}
 	else{
 		temp_time->stop();
 		ui->pushButton_3->setText("开始");
 	}
}

