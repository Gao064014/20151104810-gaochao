#include <reg51.h>						 //ͷ�ļ�
#define uchar unsigned char				 //�궨��
#define uint unsigned int				 //�궨��
bit Adjust;			  					 //���ڱ�־λ��=1��ʾ�������磬=0����ʾ��λ
bit Alarm_flag;							 //���Ͷ��ű�־λ��=1��ʾ�Ѿ����ͣ�=0��ʾû�з���
sbit IN=P1^4;						 	 //���
sbit OUT=P1^3;						 	 //����
sbit BEEP=P2^2;							 //�������˿�
sbit rs=P1^0;							 //LCD1602λ����
sbit rw=P1^1; 							 //LCD1602λ����
sbit e=P1^2;							 //LCD1602λ����
sbit STALL1=P3^2;                        //��λָʾ��
sbit STALL2=P3^4;                        //��λָʾ��
sbit STALL3=P3^5;                        //��λָʾ��
sbit STALL4=P2^1;                        //��λָʾ��
sbit STALL5=P3^3;                        //��λָʾ��
sbit STALL6=P3^6;                        //��λָʾ��
sbit STALL7=P3^7;                        //��λָʾ��
sbit STALL8=P2^0;                        //��λָʾ��

uchar code table1[]="Empty Stall:    ";	 //�ճ�λ
uchar code table2[]="Guide:          ";	 //��ʾ��λ���
uchar code table3[]="                ";			 	//��������
uchar code table4[]="GSM Stall Alarm!";				//��ʼ����ʾ����
uchar code table5[]="SIM Initialize!!";				//��ʼ����ʾ����
uchar code table7[]="  Calling Up... ";

uchar Count=8;                           //��λ��

uchar xdata UART0_Data[256];							//����ֻ�ģ�������
uchar UART0_Len=0;										//���峤��
uchar Len=0;											//���峤��
uchar UART0_flg=0;

uchar code Command_AT[]="AT\r\n";						//����AT����������
uchar code Command_AT_CMGF[]="AT+CMGF=1\r\n";			//�Ѹ�ʽ����Ϊtext��ʽ

uchar code Command_AT_CMGS[]="AT+CMGS=";				//���͵�ָ������ָ��
uchar code Command_AT_YIN[]={0x22};						//����ָ������˫����
uchar code Command_AT_ENTER[]="\r\n";					//����ָ������˫����
uchar Number[]="                ";						//���͵�ָ������  �������洢
uchar code Command_AT_NUM[]="18847138287";				//���͵�ָ������  �����������
uchar code Command_AT_IPR[]="AT+IPR=9600\r\n";			//���ò�����
uchar code Command_AT_CNMI[]="AT+CNMI=1,1,2\r\n";		//���ö���Ϣ��ʾ
uchar code Command_AT_CLIP[]="AT+CLIP=1\r\n";			//������ʾ
uchar code Command_AT_CRC[]="AT+CRC=0\r\n";				//�������翪ͷΪRING���е绰�����յ�RING����
uchar code Command_ATH[]="ATH\r\n";						//�ҵ绰
uchar code Command_ATE[]="ATE1\r\n";					//���û���
uchar code Command_AT_End[]={0x1A};						//���Ͷ������ݽ�����
uchar code Command_NUM[]="0123456789+-";				//�������¶�ֵ��ȡ������
uchar Command_AT_Stall[33]="The number of parking space is   ";	  //ͣ��λ������*
uchar code Command_AT_Full[43]="Please note that the parking space is full!";    //��ע�⳵λ����

void Send_Message1(void);
void Send_Message2(void);
void LCD_dsiplay(void);
/**************************************LCD1602**********************************************************/ 
void delay_lcd(uint z)			  					//��ʾ����ʱ0.2ms����
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=20;y>0;y--);
}

void write_com(uchar com)			 				//LCD1602дָ���
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

void write_date(uchar date)							//LCD1602д���ݺ���
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

void init()						    //LCD1602��ʼ������
{	
	uchar num;
	e=0;						    // ʱ���e��ʼΪ0
	write_com(0x38);		        //����16*2��ʾ��5*7����8λ���ݽӿ�
	write_com(0x0c);				//���ù��
	write_com(0x06);				//����Զ���1,������뷽ʽ
	write_com(0x01);				//����
	write_com(0x80);				//���ó�ʼ��ʾλ��
	for(num=0;num<16;num++)
	{										  
		write_date(table4[num]);	//��ʾ����table1������
		delay_lcd(1);
	}
	write_com(0x80+0x40);
	for(num=0;num<16;num++)
	{
		write_date(table5[num]);	//��ʾ����table2������
		delay_lcd(1);
	}					  
}
void Dispaly(char add,char dat)	 	//��ʾ�ճ�λ��
{
	write_com(0x80+add);			   	 			//Ҫд�ĵ�ַ
	write_date(0x30+dat);				   	 		//����   1602�ַ���
}
void LCD_Display_String(unsigned char line,unsigned char *string)
{ 										//Һ������ʾ���ݣ���Ҫ��ʾ������д����Ӧ��λ��
	unsigned char i;
	unsigned char address=0;
	if(line==1)
	{
		address=0x80;	 				//0X80�ǵ�1�еĵ�1��λ��  ,0x81��2λ
	}
	else if(line==2)
	{
		address=0x80+0x40;  			//0X80+0x40�ǵ�2�еĵ�1��λ��  ,0X80+0x40+1�ǵ�2�е�2λ
	}
	
	for(i=0;i<16;i++)
	{
		write_com(address);
		write_date(string[i]);
		address++;
	} 
}
/*************************************��ʱ1ms����**********************************************************/
void delay(uint z)			  		
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}

/*************************************��λ���㲢��ʾ**********************************************************/
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

	Command_AT_Stall[31]=Command_NUM[Count];	//�浽Command_AT_Tem[31]
	Dispaly(14,Count);			 //��ʾ��λ��
	LCD_dsiplay();						//��ʾ���
	
	if(Count==0)				 //�޳�λ����
	{
		BEEP=1;
		if(Alarm_flag==0)			//������־λΪ1ʱ���ͱ�����Ϣ
		{
			Send_Message2();		//���ͱ�������
		}
		Alarm_flag=1;				//��־λ��һ����ʾ�����Ѿ����ͣ������ظ����Ͷ���
	}		
	else
	{
		BEEP=0;
		Alarm_flag=0;				//�����־λ���´μ������Ͷ���һ��
	}		
}

void LCD_dsiplay(void)
{	
	write_com(0x80+0x40+7);		 //д�׵�ַ
	switch(Count)				 //��ʾ�ճ�λ���
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
void delay1(uint z)			  						//GSM������ʱ1ms����
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}

void Sys_Init(void)
{
	TMOD=0x21;						//���ö�ʱ��1�Ĺ�����ʽΪ��ʽ2
	TH1=0xfd;
	TL1=0xfd;						//���ô��пڲ�����Ϊ9600
	TR1=1;							//������ʱ��1
	SCON=0x50;						//���ô��пڵĹ�����ʽΪ��ʽһ���������
	PCON=0x00;
	ES=1;							//�����п��ж�	
	EA=1;							//�����ж�
}
void Delay_0_5s(uint y)				 //��ʱ1s
{
	uint a;
	for(a=0;a<10*y;a++)			 	//20Ϊ1s
	{
		TH0=(65536-50000)/256;
		TL0=(65536-50000)%256;
		TR0=1;
		while(!TF0);
		TF0=0;
	}
}

void Send_Command(uchar Value[],uchar Len)	//����ָ�Value[]��ʾ�����ĸ����飬Len��ʾ���͸�������ļ�λ
{
	uchar i;
	for(i=0;i<Len;i++)
	{
		SBUF=Value[i];
		while(!TI);
		TI=0;
	}
}

void SIM_Init(void)					  	  //SIMģ���ʼ��
{
	Send_Command(Command_ATE,5);		  //���û���
	delay1(1000);
	Send_Command(Command_AT,3);			  //��������
	delay1(10);
	Send_Command(Command_AT_CMGF,10);	  //����Ϊtext��ʽ
	delay1(2000);
	Send_Command(Command_AT_CNMI,14);	  //����Ϊ����Ϊ�жϷ�ʽ���ն��ţ���TC35���ն��ź�����λ�����ڷ��� +CMTI: "SM",1 ��
	delay1(3000);
	Send_Command(Command_AT_CLIP,10);	  //����������ʾ  RING   +CLIP:
	delay1(1000);
	Send_Command(Command_AT_CRC,9);	  	  //����������ʾ  RING   +CLIP:
	delay1(1000);
	Send_Command(Command_AT_IPR,12);	  //���ò�����=9600
	delay1(1000);
}

void Transform(void)						       		//����������ȡʶ���ж�
{
	uchar i;
	if(UART0_flg==1)
	{	
		delay1(2000);
		Adjust=1;
		if((UART0_Data[Len+8]>=0x30)&&(UART0_Data[Len+8]<0x40))//��һ��������������֣��������
		{
			for(i=0;i<16;i++)
			{
				Number[i]=UART0_Data[Len+8+i];			//��ȡ����
			}	   
			if((Number[0]<0x30)||(Number[0]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[0]=0x20;Number[1]=0x20;Number[2]=0x20;Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[1]<0x30)||(Number[1]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[1]=0x20;Number[2]=0x20;Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[2]<0x30)||(Number[2]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[2]=0x20;Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[3]<0x30)||(Number[3]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[3]=0x20;Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[4]<0x30)||(Number[4]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[4]=0x20;Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[5]<0x30)||(Number[5]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[5]=0x20;Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[6]<0x30)||(Number[6]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[6]=0x20;Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[7]<0x30)||(Number[7]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[7]=0x20;Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[8]<0x30)||(Number[8]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[8]=0x20;Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[9]<0x30)||(Number[9]>=0x40))		//������Ч���֣���ʾ�ո�
			{Number[9]=0x20;Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[10]<0x30)||(Number[10]>=0x40))	//������Ч���֣���ʾ�ո�
			{Number[10]=0x20;Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[11]<0x30)||(Number[11]>=0x40))	//������Ч���֣���ʾ�ո�
			{Number[11]=0x20;Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[12]<0x30)||(Number[12]>=0x40))	//������Ч���֣���ʾ�ո�
			{Number[12]=0x20;Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[13]<0x30)||(Number[13]>=0x40))	//������Ч���֣���ʾ�ո�
			{Number[13]=0x20;Number[14]=0x20;Number[15]=0x20;}
			if((Number[14]<0x30)||(Number[14]>=0x40))	//������Ч���֣���ʾ�ո�
			{Number[14]=0x20;Number[15]=0x20;}
			if((Number[15]<0x30)||(Number[15]>=0x40))	//������Ч���֣���ʾ�ո�
			{Number[15]=0x20;}

			LCD_Display_String(1,table7);				//��ʾ����
			LCD_Display_String(2,Number);
			delay1(2000);
			Send_Command(Command_ATH,4);				//���͹һ�����
			delay1(2000);
			Send_Message1();							//���͵�ǰ��λ��
			Adjust=0;
			LCD_Display_String(1,table1);				//��ʾ����
			LCD_Display_String(2,table2);				//��ʾ����
		}
		UART0_Len = 0;
		UART0_flg=0;
	}	  
}
void Send_Message(void)					  	  //���͵�ָ������  �������
{
	Send_Command(Command_AT_CMGS,8);		  //����ָ��
	delay1(500);							  
	Send_Command(Command_AT_YIN,1);		      //��������
	delay1(100);
	Send_Command(Number,11);		  		  //���ͺ���
	delay1(100);
	Send_Command(Command_AT_YIN,1);		      //��������
	delay1(100);
	Send_Command(Command_AT_ENTER,1);		  //���ͻس�
	delay1(100);
}	
void Send_Message1(void)					  //���ͳ�λ��
{
	Send_Message();							  //���͵�ָ������
	delay1(1000);							  //�ȴ��ظ��������
	Send_Command(Command_AT_Stall,33);		  //���Ͷ�������
	delay1(1000);
	Send_Command(Command_AT_End,1);			  //���Ͷ��Ž�����
	delay1(100);
}
void Send_Message0(void)					  //���͵�ָ������  �����������
{
	Send_Command(Command_AT_CMGS,8);		  //����ָ��
	delay1(500);							  
	Send_Command(Command_AT_YIN,1);		      //��������
	delay1(100);
	Send_Command(Command_AT_NUM,11);		  //���ͺ���
	delay1(100);
	Send_Command(Command_AT_YIN,1);		      //��������
	delay1(100);
	Send_Command(Command_AT_ENTER,1);		  //���ͻس�
	delay1(100);
}
void Send_Message2(void)					  //���ͳ�λ������Ϣ
{
	Send_Message0();						  //���͵�ָ������
	delay1(1000);							  //�ȴ��ظ��������
	Send_Command(Command_AT_Full,43);		  //���Ͷ�������
	delay1(1000);
	Send_Command(Command_AT_End,1);			  //���Ͷ��Ž�����
	delay1(100);
}
/*************************************������**********************************************************/
void main()
{	
	BEEP=0;
	Sys_Init();									//��Ƭ��ϵͳ��ʼ��
	init();										//Һ������ʼ��
	Delay_0_5s(50);								//�ȴ�ģ���ʼ��
	SIM_Init();									//ģ���ʼ��
	LCD_Display_String(1,table1);				//��ʾ����
	LCD_Display_String(2,table2);				//��ʾ����
	while(1)
	{
		if(Adjust==0)			 				//û������ʱ��ʾ��λ
		{
			
			stall_count();						//��λ������			
		}
		Transform();							//ͨѶ����
	}	
}
/***********************************�����жϷ����ӳ��� ������� �����ݾ�ֱ�ӽ���***********************************************************/
void Serial() interrupt 4
{	
	RI=0;										//�������
	UART0_Data[UART0_Len] = SBUF;	    		//���뵽��Ӧ����
   	
	if(SBUF==0x2B)								//+�� ��ʾ�ж��Ż��ߵ绰����
	{
		Len=UART0_Len;							//��¼��� 
	}
	if((UART0_Data[Len+1]==0x43)&&(UART0_Data[Len+2]==0x4C)&&(UART0_Data[Len+3]==0x49)&&(UART0_Data[Len+4]==0x50)&&(UART0_Data[Len+5]==0x3A))
	{											//���յ��������ǵ绰��ʾUART0_Data����="+CLIP:"���Ƚ�ǰ6λ
		UART0_flg = 1;							//�һ���־λ��һ
	}
					
	UART0_Len++;								//��ŵ���	 
}
/***********************************************************************************************/