#ifndef __SOCKDATA_H__
#define __SOCKDATA_H__


#define MSG_CTRL    16			//ctrl��
#define MSG_SHIFT    17			//shift��
#define MSG_LBUTTONDBLCLK 1		//���˫��
#define MSG_LBUTTONDOWN 2		//�������
#define MSG_RBUTTONDOWN 3		//�Ҽ�����

struct BMP_SEND_LEN
{
	unsigned long oldlen;	//���δѹ��ǰ�ĳ���
	unsigned long newlen;	//ѹ���󳤶�
};

struct msg_data
{
	CPoint point;	//��������
	int mouse;		//����3��״̬
	int flag;		//�Ƿ�����ϼ�
	int ch;			//key
	msg_data():point(0,0),mouse(-1),flag(-1),ch(-1){}
};

#endif