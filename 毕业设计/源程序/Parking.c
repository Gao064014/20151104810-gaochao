#include <reg51.h>						 //头文件
#define uchar unsigned char				 //宏定义
#define uint unsigned int				 //宏定义
bit Adjust;			  					 //调节标志位，=1表示进入来电，=0是显示车位
bit Alarm_flag;							 //发送短信标志位，=1表示已经发送，=0表示没有发送
sbit IN=P1^4;						 	 //入口
sbit OUT=P1^3;						 	 //出口
sbit BEEP=P2^2;							 //蜂鸣器端口
sbit rs=P1^0;							 //LCD1602位定义
sbit rw=P1^1; 							 //LCD1602位定义
sbit e=P1^2;							 //LCD1602位定义
sbit STALL1=P3^2;                        //车位指示灯
sbit STALL2=P3^4;                        //车位指示灯
sbit STALL3=P3^5;                        //车位指示灯
sbit STALL4=P2^1;                        //车位指示灯
sbit STALL5=P3^3;                        //车位指示灯
sbit STALL6=P3^6;                        //车位指示灯
sbit STALL7=P3^7;                        //车位指示灯
sbit STALL8=P2^0;                        //车位指示灯

uchar code table1[]="Empty Stall:    ";	 //空车位
uchar code table2[]="Guide:          ";	 //显示车位序号
uchar code table3[]="                ";			 	//清屏内容
uchar code table4[]="GSM Stall Alarm!";				//初始化显示内容
uchar code table5[]="SIM Initialize!!";				//初始化显示内容
uchar code table7[]="  Calling Up... ";

uchar Count=8;                           //车位数

uchar xdata UART0_Data[256];							//存放手机模块的数组
uchar UART0_Len=0;										//定义长度
uchar Len=0;											//定义长度
uchar UART0_flg=0;

uchar code Command_AT[]="AT\r\n";						//发送AT，建立连接
uchar code Command_AT_CMGF[]="AT+CMGF=1\r\n";			//把格式设置为text格式

uchar code Command_AT_CMGS[]="AT+CMGS=";				//发送到指定号码指令
uchar code Command_AT_YIN[]={0x22};						//发送指定号码双引号
uchar code Command_AT_ENTER[]="\r\n";					//发送指定号码双引号
uchar Number[]="                ";						//发送到指定号码  来电号码存储
uchar code Command_AT_NUM[]="18847138287";				//发送到指定号码  报警号码对象
uchar code Command_AT_IPR[]="AT+IPR=9600\r\n";			//设置波特率
uchar code Command_AT_CNMI[]="AT+CNMI=1,1,2\r\n";		//设置短消息提示
uchar code Command_AT_CLIP[]="AT+CLIP=1\r\n";			//来电显示
uchar code Command_AT_CRC[]="AT+CRC=0\r\n";				//设置来电开头为RING，有电话打入收到RING提醒
uchar code Command_ATH[]="ATH\r\n";						//挂电话
uchar code Command_ATE[]="ATE1\r\n";					//设置回显
uchar code Command_AT_End[]={0x1A};						//发送短信内容结束符
uchar code Command_NUM[]="0123456789+-";				//短信里温度值提取的数字
uchar Command_AT_Stall[33]="The number of parking space is   ";	  //停车位数量是*
uchar code Command_AT_Full[43]="Please note that the parking space is full!";    //请注意车位满了

void Send_Message1(void);
void Send_Message2(void);
void LCD_dsiplay(void);
/**************************************LCD1602**********************************************************/ 
void delay_lcd(uint z)			  					//显示屏延时0.2ms函数
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=20;y>0;y--);
}

void write_com(uchar com)			 				//LCD1602写指令函数
{
	rw=0;
	delay_lcd(5);
	rs=0;
	delay_lcd(5);
	e=1;
	delay_lcd(5);
	P0=com;
	delay_lcd(5);
	e=0;
	delay_lcd(5);	
}

void write_date(uchar date)							//LCD1602写数据函数
{									
	rw=0;
	delay_lcd(5);
	rs=1;
	delay_lcd(5);
	e=1;
	delay_lcd(5);
	P0=date;
	delay_lcd(5);
	e=0;
	delay_lcd(5);	
}

void init()						    //LCD1602初始化函数
{	
	uchar num;
	e=0;						    // 时序表e初始为0
	write_com(0x38);		        //设置16*2显示，5*7点阵，8位数据接口
	write_com(0x0c);				//设置光标
	write_com(0x06);				//光标自动加1,光标输入方式
	write_com(0x01);				//清屏
	write_com(0x80);				//设置初始显示位置
	for(num=0;num<16;num++)
	{										  
		write_date(table4[num]);	//显示数组table1的内容
		delay_lcd(1);
	}
	write_com(0x80+0x40);
	for(num=0;num<16;num++)
	{
		write_date(table5[num]);	//显示数组table2的内容
		delay_lcd(1);
	}					  
}
void Dispaly(char add,char dat)	 	//显示空车位数
{
	write_com(0x80+add);			   	 			//要写的地址
	write_date(0x30+dat);				   	 		//内容   1602字符库
}
void LCD_Display_String(unsigned char line,unsigned char *string)
{ 										//液晶屏显示内容，把要显示的内容写到对应的位置
	unsigned char i;
	unsigned char address=0;
	if(line==1)
	{
		address=0x80;	 				//0X80是第1行的第1个位置  ,0x81第2位
	}
	else if(line==2)
	{
		address=0x80+0x40;  			//0X80+0x40是第2行的第1个位置  ,0X80+0x40+1是第2行第2位
	}
	
	for(i=0;i<16;i++)
	{
		write_com(address);
		write_date(string[i]);
		address++;
	} 
}
/*************************************延时1ms函数**********************************************************/
void delay(uint z)			  		
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}

/*************************************车位计算并显示**********************************************************/
void stall_count(void)						
{
	if(IN==0)
	{
		delay(5);
		if(IN==0)
		{
			while(!IN);
			if(Count>0)
			Count--;
		}
	}
	if(OUT==0)
	{
		delay(5);
		if(OUT==0)
		{
			while(!OUT);
			if(Count<8)
			Count++;
		}
	}
	switch(Count)
	{
		case 0:STALL1=1;STALL2=1;STALL3=1;STALL4=1;STALL5=1;STALL6=1;STALL7=1;STALL8=1;break;
		case 1:STALL1=0;STALL2=1;STALL3=1;STALL4=1;STALL5=1;STALL6=1;STALL7=1;STALL8=1;break;
		case 2:STALL1=0;STALL2=1;STALL3=1;STALL4=1;STALL5=0;STALL6=1;STALL7=1;STALL8=1;break;
		case 3:STALL1=0;STALL2=0;STALL3=1;STALL4=1;STALL5=0;STALL6=1;STALL7=1;STALL8=1;break;
		case 4:STALL1=0;STALL2=0;STALL3=1;STALL4=1;STALL5=0;STALL6=0;STALL7=1;STALL8=1;break;
		case 5:STALL1=0;STALL2=0;STALL3=0;STALL4=1;STALL5=0;STALL6=0;STALL7=1;STALL8=1;break;
		case 6:STALL1=0;STALL2=0;STALL3=0;STALL4=1;STALL5=0;STALL6=0;STALL7=0;STALL8=1;break;
		case 7:STALL1=0;STALL2=0;STALL3=0;STALL4=0;STALL5=0;STALL6=0;STALL7=0;STALL8=1;break;
		case 8:STALL1=0;STALL2=0;STALL3=0;STALL4=0;STALL5=0;STALL6=0;STALL7=0;STALL8=0;break;
		default:break;
	}

	Command_AT_Stall[31]=Command_NUM[Count];	//存到Command_AT_Tem[31]
	Dispaly(14,Count);			 //显示车位数
	LCD_dsiplay();						//显示序号
	
	if(Count==0)				 //无车位报警
	{
		BEEP=1;
		if(Alarm_flag==0)			//警报标志位为1时发送报警信息
		{
			Send_Message2();		//发送报警短信
		}
		Alarm_flag=1;				//标志位置一，表示短信已经发送，避免重复发送短信
	}		
	else
	{
		BEEP=0;
		Alarm_flag=0;				//清除标志位，下次继续发送短信一次
	}		
}

void LCD_dsiplay(void)
{	
	write_com(0x80+0x40+7);		 //写首地址
	switch(Count)				 //显示空车位序号
	{
		case 8:	write_date(0x30+1);		   	 			
				write_date(0x30+2);
				write_date(0x30+3);
				write_date(0x30+4);
				write_date(0x30+5);
				write_date(0x30+6);
				write_date(0x30+7);
				write_date(0x30+8);
				break;
		case 7:	write_date(0x30+1);		   	 			
				write_date(0x30+2);
				write_date(0x30+3);
				write_date(0x30+4);
				write_date(0x30+5);
				write_date(0x30+6);
				write_date(0x30+7);
				write_date(0x20);
				break;
		case 6:	write_date(0x30+1);		   	 			
				write_date(0x30+2);
				write_date(0x30+3);
				write_date(0x30+5);
				write_date(0x30+6);
				write_date(0x30+7);
				write_date(0x20);
				write_date(0x20);
				break;
		case 5:	write_date(0x30+1);		   	 			
				write_date(0x30+2);
				write_date(0x30+3);
				write_date(0x30+5);
				write_date(0x30+6);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				break;
		case 4:	write_date(0x30+1);		   	 			
				write_date(0x30+2);
				write_date(0x30+5);
				write_date(0x30+6);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				break;
		case 3:	write_date(0x30+1);		   	 			
				write_date(0x30+2);
				write_date(0x30+5);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				break;
		case 2:	write_date(0x30+1);		   	 			
				write_date(0x30+5);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				break;
		case 1:	write_date(0x30+1);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);   	 			
				break;
		case 0:	write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				write_date(0x20);
				break;
		default:break;		   	 			
	}
}
/************************************SIM800***********************************************************/
void delay1(uint z)			  						//GSM短信延时1ms函数
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}

void Sys_Init(void)
{
	TMOD=0x21;						//设置定时器1的工作方式为方式2
	TH1=0xfd;
	TL1=0xfd;						//设置串行口波特率为9600
	TR1=1;							//开启定时器1
	SCON=0x50;						//设置串行口的工作方式为方式一，允许接收
	PCON=0x00;
	ES=1;							//开串行口中断	
	EA=1;							//开总中断
}
void Delay_0_5s(uint y)				 //延时1s
{
	uint a;
	for(a=0;a<10*y;a++)			 	//20为1s
	{
		TH0=(65536-50000)/256;
		TL0=(65536-50000)%256;
		TR0=1;
		while(!TF0);
		TF0=0;
	}
}

void Send_Command(uchar Value[],uchar Len)	//发送指令，Value[]表示发送哪个数组，Len表示发送该数组里的几位
{
	uchar i;
	for(i=0;i<Len;i++)
	{
		SBUF=Value[i];
		while(!TI);
		TI=0;
	}
}

void SIM_Init(void)					  	  //SIM模块初始化
{
	Send_Command(Command_ATE,5);		  //设置回显
	delay1(1000);
	Send_Command(Command_AT,3);			  //建立连接
	delay1(10);
	Send_Command(Command_AT_CMGF,10);	  //设置为text格式
	delay1(2000);
	Send_Command(Command_AT_CNMI,14);	  //设置为设置为中断方式接收短信，即TC35接收短信后向下位机串口发送 +CMTI: "SM",1 。
	delay1(3000);
	Send_Command(Command_AT_CLIP,10);	  //设置来电提示  RING   +CLIP:
	delay1(1000);
	Send_Command(Command_AT_CRC,9);	  	  //设置来电提示  RING   +CLIP:
	delay1(1000);
	Send_Command(Command_AT_IPR,12);	  //设置波特率=9600
	delay1(1000);
}

void Transform(void)						       		//接收内容提取识别判断
{
	uchar i;
	if(UART0_flg==1)
	{	
		delay1(2000);
		Adjust=1;
		if((UART0_Data[Len+8]>=0x30)&&(UART0_Data[Len+8]<0x40))//第一个号码必须是数字，避免错乱
		{
			for(i=0;i<16;i++)
			{
				Number[i]=UART0_Data[Len+8+i];			//提取号码
			}	   
			if((Number[0]<0x30)||(Number[0]>=0x40))		//不是有效数字，显示空格
			{Number[0]=0x20;Number[1]=0x20;Number[2]=0x20;Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[1]<0x30)||(Number[1]>=0x40))		//不是有效数字，显示空格
			{Number[1]=0x20;Number[2]=0x20;Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[2]<0x30)||(Number[2]>=0x40))		//不是有效数字，显示空格
			{Number[2]=0x20;Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[3]<0x30)||(Number[3]>=0x40))		//不是有效数字，显示空格
			{Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[4]<0x30)||(Number[4]>=0x40))		//不是有效数字，显示空格
			{Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[5]<0x30)||(Number[5]>=0x40))		//不是有效数字，显示空格
			{Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[6]<0x30)||(Number[6]>=0x40))		//不是有效数字，显示空格
			{Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[7]<0x30)||(Number[7]>=0x40))		//不是有效数字，显示空格
			{Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[8]<0x30)||(Number[8]>=0x40))		//不是有效数字，显示空格
			{Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[9]<0x30)||(Number[9]>=0x40))		//不是有效数字，显示空格
			{Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[10]<0x30)||(Number[10]>=0x40))	//不是有效数字，显示空格
			{Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[11]<0x30)||(Number[11]>=0x40))	//不是有效数字，显示空格
			{Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[12]<0x30)||(Number[12]>=0x40))	//不是有效数字，显示空格
			{Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[13]<0x30)||(Number[13]>=0x40))	//不是有效数字，显示空格
			{Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[14]<0x30)||(Number[14]>=0x40))	//不是有效数字，显示空格
			{Number[14]=0x20;Number[15]=0x20;}
			if((Number[15]<0x30)||(Number[15]>=0x40))	//不是有效数字，显示空格
			{Number[15]=0x20;}

			LCD_Display_String(1,table7);				//显示号码
			LCD_Display_String(2,Number);
			delay1(2000);
			Send_Command(Command_ATH,4);				//发送挂机命令
			delay1(2000);
			Send_Message1();							//发送当前车位数
			Adjust=0;
			LCD_Display_String(1,table1);				//显示内容
			LCD_Display_String(2,table2);				//显示内容
		}
		UART0_Len = 0;
		UART0_flg=0;
	}	  
}
void Send_Message(void)					  	  //发送到指定号码  来电对象
{
	Send_Command(Command_AT_CMGS,8);		  //发送指令
	delay1(500);							  
	Send_Command(Command_AT_YIN,1);		      //发送引号
	delay1(100);
	Send_Command(Number,11);		  		  //发送号码
	delay1(100);
	Send_Command(Command_AT_YIN,1);		      //发送引号
	delay1(100);
	Send_Command(Command_AT_ENTER,1);		  //发送回车
	delay1(100);
}	
void Send_Message1(void)					  //发送车位数
{
	Send_Message();							  //发送到指定号码
	delay1(1000);							  //等待回复输入符号
	Send_Command(Command_AT_Stall,33);		  //发送短信内容
	delay1(1000);
	Send_Command(Command_AT_End,1);			  //发送短信结束符
	delay1(100);
}
void Send_Message0(void)					  //发送到指定号码  报警号码对象
{
	Send_Command(Command_AT_CMGS,8);		  //发送指令
	delay1(500);							  
	Send_Command(Command_AT_YIN,1);		      //发送引号
	delay1(100);
	Send_Command(Command_AT_NUM,11);		  //发送号码
	delay1(100);
	Send_Command(Command_AT_YIN,1);		      //发送引号
	delay1(100);
	Send_Command(Command_AT_ENTER,1);		  //发送回车
	delay1(100);
}
void Send_Message2(void)					  //发送车位满短信息
{
	Send_Message0();						  //发送到指定号码
	delay1(1000);							  //等待回复输入符号
	Send_Command(Command_AT_Full,43);		  //发送短信内容
	delay1(1000);
	Send_Command(Command_AT_End,1);			  //发送短信结束符
	delay1(100);
}
/*************************************主函数**********************************************************/
void main()
{	
	BEEP=0;
	Sys_Init();									//单片机系统初始化
	init();										//液晶屏初始化
	Delay_0_5s(50);								//等待模块初始化
	SIM_Init();									//模块初始化
	LCD_Display_String(1,table1);				//显示内容
	LCD_Display_String(2,table2);				//显示内容
	while(1)
	{
		if(Adjust==0)			 				//没有来电时显示车位
		{
			
			stall_count();						//车位检测计算			
		}
		Transform();							//通讯处理
	}	
}
/***********************************串口中断服务子程序 无需调用 有数据就直接进入***********************************************************/
void Serial() interrupt 4
{	
	RI=0;										//接收完毕
	UART0_Data[UART0_Len] = SBUF;	    		//存入到对应数组
   	
	if(SBUF==0x2B)								//+号 表示有短信或者电话打入
	{
		Len=UART0_Len;							//记录序号 
	}
	if((UART0_Data[Len+1]==0x43)&&(UART0_Data[Len+2]==0x4C)&&(UART0_Data[Len+3]==0x49)&&(UART0_Data[Len+4]==0x50)&&(UART0_Data[Len+5]==0x3A))
	{											//接收到的内容是电话提示UART0_Data【】="+CLIP:"，比较前6位
		UART0_flg = 1;							//挂机标志位置一
	}
					
	UART0_Len++;								//序号递增	 
}
/***********************************************************************************************/