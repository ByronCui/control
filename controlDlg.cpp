// controlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "control.h"
#include "controlDlg.h"
#include "connection.h"
#include "sockdata.h"
#include "minilzo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IN_LEN      (128*1024ul)
#define OUT_LEN     (IN_LEN + IN_LEN / 16 + 64 + 3)

static unsigned char __LZO_MMODEL in  [ IN_LEN ];
static unsigned char __LZO_MMODEL out [ OUT_LEN ];

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

BMP_SEND_LEN	bmpSendLen,rec;

int ctlcnt,ctldcnt;  
bool flag; //����ͷ�Ƿ����

unsigned char *pBMP = NULL;
unsigned char *pBMP2 = NULL;
LPRECT lprect;
HBITMAP hTempBitmap;
HBITMAP GetHBmp(HDC hDC, unsigned char *pBuf);
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControlDlg dialog

CControlDlg::CControlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CControlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CControlDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CControlDlg)
	DDX_Control(pDX, IDC_PIC, m_pic);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CControlDlg, CDialog)
	//{{AFX_MSG_MAP(CControlDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_exit, Onexit)
	ON_COMMAND(ID_connect, Onconnect)
	ON_WM_TIMER()
	ON_COMMAND(ID_min, Onmin)
	ON_COMMAND(ID_max, Onmax)
	ON_COMMAND(ID_restore, Onrestore)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControlDlg message handlers

BOOL CControlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	lzo_init();

	flag = 0;
	ctlcnt = 0;
	ctldcnt = 0;

	pBMP = (unsigned char *)malloc(5*1024*1024);
	pBMP2 = (unsigned char *)malloc(5*1024*1024);
	if ((pBMP == NULL) ||(pBMP2 == NULL))
	{
		AfxMessageBox("�ڴ����ʧ��");
	}
	//Ҫ��ȡλͼ�Ĵ�С
	lprect = new RECT();
	GetDesktopWindow()->GetWindowRect(lprect);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CControlDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CControlDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
//�˳�����
void CControlDlg::Onexit() 
{
	OnOK();
}
//������Ϣ��Ӧ��������
BOOL CControlDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN   &&   pMsg->wParam==VK_ESCAPE)     
	{
		CWnd::ShowWindow(SW_RESTORE);	//SW_SHOWMINIMIZED  	 SW_RESTORE
		ModifyStyle( 0, WS_CAPTION );
		return   TRUE;  //��Esc����ʹ���˳�ȫ��
	}
	if(pMsg->message==WM_KEYDOWN   &&   pMsg->wParam==VK_RETURN)     return   TRUE;  //����Enter��

	if( m_set_dlg.m_IsControl )  //�������ض˵���Ϣ
	{
		msg_data _msg;
		_msg.point = pt;

		if (pMsg->message==WM_KEYDOWN)//ģ������¼�
		{
			if(pMsg->wParam!=16 && pMsg->wParam!=17)//������ϼ��ĵ�һ����
			{
				if (::GetKeyState(VK_CONTROL)<0)//��ȡָ���������״̬
				{
					_msg.flag=16;
				}
				else if(::GetKeyState(VK_SHIFT)<0)
				{
					_msg.flag=17;
				}
				else
				{
					_msg.flag=-1;
				}
				_msg.ch=pMsg->wParam;

				send(&_msg);
			}
		}
		else if(pMsg->message == WM_LBUTTONDBLCLK) //���˫��
		{
			_msg.flag=1;
			send(&_msg);
			//MessageBox("doubleclick");
		}
		else if(pMsg->message == WM_LBUTTONDOWN) //�������
		{
			_msg.flag=2;
			send(&_msg);
			//MessageBox("leftclick");
		}
		else if(pMsg->message == WM_RBUTTONDOWN) //�Ҽ�����
		{
			_msg.flag=3;
			send(&_msg);
			//MessageBox("rightclick");
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
//׼������
void CControlDlg::Onconnect() 
{
	if( IDOK == m_set_dlg.DoModal() )
	{
		start_connection( m_set_dlg.m_IsControl );
	}
}
//��ʼ����
void CControlDlg::start_connection(bool iscontrol)
{
	if( iscontrol ) //���ض�
	{
		SetPos();
		m_con.Create( atoi( m_set_dlg.m_port ) );
		m_con.Listen();
	}
	else   //���ض�
	{
		CWnd::ShowWindow(SW_SHOWMINIMIZED);
		m_sock.Create();
		m_sock.Connect( m_set_dlg.m_ControlIP, atoi( m_set_dlg.m_port ) );
		SetTimer(1,200,0); //���ö�ʱ��
	}
}
//���ض����ӳɹ�
void CControlDlg::accept()
{
	m_con.Accept(m_sock);
	m_is_con = 1;
	SetWindowText("Control");
}
//���ض����ӳɹ�
void CControlDlg::connection()
{
	m_is_con = 1;
	SetWindowText("Controlled");
}

//��������
DWORD nDataReceived;
void CControlDlg::receive()
{
	int ret;
	//MessageBox("receive");
	if( m_set_dlg.m_IsControl ) //���ض˽���λͼ��Ϣ
	{
		//��ȡ��������ԭʼ��ѹ�������ݵĴ�С
		if( !flag )
		{
			//MessageBox("kaishi");

			ret = m_sock.Receive((char *)pBMP, sizeof(BMP_SEND_LEN));
			if (ret == sizeof(BMP_SEND_LEN))
			{
				memcpy(&bmpSendLen, pBMP, sizeof(BMP_SEND_LEN));
				flag = 1;
			}
			nDataReceived = 0;
			return ;
		}

		//��ʼ����ѹ��λͼ������

		if (nDataReceived < bmpSendLen.newlen)
		{
			if((ctlcnt = m_sock.Receive((char *)(pBMP + nDataReceived), 
				bmpSendLen.newlen - nDataReceived))==SOCKET_ERROR)
			{
				return ;
			}
			else nDataReceived += ctlcnt;
		}
		CString str;
		str.Format("%d %d",nDataReceived,bmpSendLen.newlen);
		//MessageBox(str);

		if( nDataReceived == bmpSendLen.newlen )
		{
			flag = 0;
			//��ѹ��λͼ����
			int ret2 = lzo1x_decompress(pBMP, bmpSendLen.newlen, pBMP2, 
				&bmpSendLen.oldlen, NULL);
			if (ret2 != LZO_E_OK)
			{
				AfxMessageBox("��ѹ������");
				return ;
			}
			//MessageBox("�������");
			//��ʾ��������
			CDC *pDC = GetDC();
			HBITMAP hBMP = GetHBmp( pDC->m_hDC, pBMP2);
			m_pic.SetBitmap(hBMP);
			DeleteObject(hBMP);
			ReleaseDC(pDC);
			//m_sock.OnReceive(0);
		}
	}
	else   //���ض˽�����Ϣ��Ӧ
	{
		msg_data _msg;
		if(m_sock.Receive((LPVOID)&_msg,sizeof(msg_data)) == SOCKET_ERROR)
		{
			MessageBox(_T("��������ʱ�������������������ӡ�"),_T("����"),MB_ICONSTOP);
			return ;
		}
		//MessageBox( msg.str );
		SetCursorPos( _msg.point.x, _msg.point.y );
		if( _msg.ch > 0 ) //���������Ӧ
		{
			if( _msg.flag < 0 )  //�������̰���
			{
				keybd_event(_msg.ch,0,0,0);
				keybd_event(_msg.ch,0,KEYEVENTF_KEYUP,0);
			}
			else if( _msg.flag == MSG_CTRL ) //��ϼ�ctr
			{
				keybd_event(16,0,0,0);
				keybd_event(_msg.ch,0,0,0);
				keybd_event(_msg.ch,0,KEYEVENTF_KEYUP,0);
				keybd_event(16,0,KEYEVENTF_KEYUP,0);
			}
			else if( _msg.flag == MSG_SHIFT ) //��ϼ�shift
			{
				keybd_event(17,0,0,0);
				keybd_event(_msg.ch,0,0,0);
				keybd_event(_msg.ch,0,KEYEVENTF_KEYUP,0);
				keybd_event(17,0,KEYEVENTF_KEYUP,0);
			}
		}
		else  //���������Ӧ
		{
			if( _msg.flag == MSG_LBUTTONDOWN ) mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,0,0,0,0);
			else if( _msg.flag == MSG_LBUTTONDBLCLK ) 
			{
				mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,0,0,0,0);
				mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,0,0,0,0);
			}
			else if( _msg.flag == MSG_RBUTTONDOWN ) mouse_event(MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP,0,0,0,0);//*/
		}
	}
}
//���ö�ʱ��
void CControlDlg::OnTimer(UINT nIDEvent) 
{
	screen();
	CDialog::OnTimer(nIDEvent);
}
//������Ϣ
void CControlDlg::send(msg_data *m_msg_data)
{
	m_sock.Send((LPVOID)m_msg_data,sizeof(msg_data));
}
//λͼ���ݴ���
int ToMemory(HBITMAP hbitmap , unsigned char *pBuf, int ibits)
{
	int ret = 0;
	HDC hdc;			//�豸������
	WORD wbitcount; 	//��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ���
	
	//λͼ��ÿ��������ռ�ֽ����������ɫ���С��λͼ�������ֽڴ�С��λͼ�ļ���С ��д���ļ��ֽ���
	DWORD dwpalettesize=0, dwbmbitssize, dwdibsize;

	BITMAP bitmap;				//λͼ���Խṹ
	BITMAPFILEHEADER bmfhdr;	//λͼ�ļ�ͷ�ṹ
	BITMAPINFOHEADER bi;		//λͼ��Ϣͷ�ṹ
	LPBITMAPINFOHEADER lpbi;	//ָ��λͼ��Ϣͷ�ṹ
	
	//�����ļ��������ڴ�������ɫ����
	HANDLE hdib, hpal, holdpal=NULL;

	if (ibits <= 1)
			wbitcount = 1;
	else if (ibits <= 4)
			wbitcount = 4;
	else if (ibits <= 8)
			wbitcount = 8;
	else if (ibits <= 16)
			wbitcount = 16;
	else if (ibits <= 24)
			wbitcount = 24;
	else 
			wbitcount = 32;

	//�����ɫ���С
	if (wbitcount <= 8)
		dwpalettesize = (1 << wbitcount) * sizeof(RGBQUAD);

	//����λͼ��Ϣͷ�ṹ
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmap.bmWidth;
	bi.biHeight = bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wbitcount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwbmbitssize = ((bitmap.bmWidth * wbitcount+31)/32)* 4 * bitmap.bmHeight ;
	//Ϊλͼ���ݷ����ڴ�
	hdib = GlobalAlloc(GHND, dwbmbitssize + dwpalettesize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	*lpbi = bi;
	
	// �����ɫ�� 
	hpal = GetStockObject(DEFAULT_PALETTE);
	if (hpal)
	{
		hdc = ::GetDC(NULL);
		holdpal = SelectPalette(hdc, (HPALETTE)hpal, false);
		RealizePalette(hdc);
	}
	
	// ��ȡ�õ�ɫ�����µ�����ֵ
	GetDIBits(hdc, hbitmap, 0, (UINT) bitmap.bmHeight,(LPSTR)lpbi + 
				sizeof(BITMAPINFOHEADER)+dwpalettesize,(BITMAPINFO*)lpbi, DIB_RGB_COLORS);
	
	//�ָ���ɫ�� 
	if (holdpal)
	{
		SelectPalette(hdc, (HPALETTE)holdpal, true);
		RealizePalette(hdc);
		::ReleaseDC(NULL, hdc);
	}

	
	// ����λͼ�ļ�ͷ
	bmfhdr.bfType = 0x4d42; // "BM"
	dwdibsize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ dwpalettesize + dwbmbitssize; 
	bmfhdr.bfSize = dwdibsize;	//����BMP�ļ��Ĵ�С
	bmfhdr.bfReserved1 = 0;
	bmfhdr.bfReserved2 = 0;
	bmfhdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + 
		(DWORD)sizeof(BITMAPINFOHEADER)+ dwpalettesize; //λͼ���ݵ�λ��

	// д��λͼ�ļ�ͷ
	memcpy(pBuf, (LPSTR)&bmfhdr, sizeof(BITMAPFILEHEADER));
	ret += sizeof(BITMAPFILEHEADER);

	// д��λͼ�ļ���������
	memcpy(pBuf+ret, (LPSTR)lpbi, dwdibsize);
	ret += dwdibsize;

	//��� 
	GlobalUnlock(hdib);
	GlobalFree(hdib);
	return ret;
}

//�ͻ��˽������������ݸ����ض�
void CControlDlg::screen()
{
	if( !m_is_con )
	{
		KillTimer(1);
		return ;
	}
	if( !flag )
	{
		flag = 1;
		unsigned long l = 0;

		hTempBitmap = getbitmap();

		int ret;
		ret= ToMemory(hTempBitmap, pBMP, 8);

		//ѹ��
		int r = lzo1x_1_compress(pBMP, ret, pBMP2, &l, wrkmem);
		if (r != LZO_E_OK)
		{
			AfxMessageBox("ѹ������");
			return;
		}

		bmpSendLen.oldlen = ret;
		bmpSendLen.newlen = l;

		//�ȷ�λͼѹ��ǰ������ݵĴ�С
		m_sock.Send((const char *)&bmpSendLen, sizeof(bmpSendLen));

		ret = l;
		unsigned long nPos = 0;
		int cnt,i=0;
		while ( nPos < bmpSendLen.newlen )
		{
			i++;
			if((cnt = m_sock.Send((const char *)(pBMP2 + nPos),bmpSendLen.newlen-nPos))==SOCKET_ERROR)
			{
				if(GetLastError() == WSAEWOULDBLOCK) break;
			}
			else nPos += cnt;
		}

		DeleteObject(hTempBitmap);

		flag = 0;
	}
}

//�ͻ��˽���
HBITMAP CControlDlg::getbitmap()
{
	HDC hscrdc, hmemdc; 
	// ��Ļ���ڴ��豸������
	HBITMAP hbitmap, holdbitmap; 
	// λͼ���
	int nx, ny, nx2, ny2; 
	// ѡ����������
	int nwidth, nheight; 
	// λͼ��Ⱥ͸߶�
	int xscrn, yscrn; 
	// ��Ļ�ֱ���

	// ȷ��ѡ������Ϊ�վ���
	if (IsRectEmpty(lprect))
		return NULL;

	//Ϊ��Ļ�����豸������
	hscrdc = CreateDC("display", NULL, NULL, NULL);
	//Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
	hmemdc = CreateCompatibleDC(hscrdc);
	// ���ѡ����������
	nx = lprect->left;
	ny = lprect->top;
	nx2 = lprect->right;
	ny2 = lprect->bottom;
	// �����Ļ�ֱ���
	xscrn = GetDeviceCaps(hscrdc, HORZRES);
	yscrn = GetDeviceCaps(hscrdc, VERTRES);
	//ȷ��ѡ�������ǿɼ���
	if (nx < 0)
		nx = 0;
	if (ny < 0)
		ny = 0;
	if (nx2 > xscrn)
		nx2 = xscrn;
	if (ny2 > yscrn)
		ny2 = yscrn;

	nwidth = nx2 - nx;
	nheight = ny2 - ny;

	// ����һ������Ļ�豸��������ݵ�λͼ
	hbitmap = CreateCompatibleBitmap(hscrdc, nwidth, nheight);

	// ����λͼѡ���ڴ��豸��������
	holdbitmap = (HBITMAP)SelectObject(hmemdc, hbitmap);
	// ����Ļ�豸�����������ڴ��豸��������
	BitBlt(hmemdc, 0, 0, nwidth, nheight,hscrdc, nx, ny, SRCCOPY);
	//�õ���Ļλͼ�ľ��
	hbitmap = (HBITMAP)SelectObject(hmemdc, holdbitmap);
	
	//��� 
	DeleteDC(hscrdc);
	DeleteDC(hmemdc);
	
	// ����λͼ���
	return hbitmap;
}
//���ض˴���λͼ����
HBITMAP GetHBmp(HDC hDC, unsigned char *pBuf)
{
	BITMAPINFO *pBmpInfo = NULL;
	unsigned char *pBits = NULL;
	BITMAPFILEHEADER *pBmpFileHeader = NULL;
	pBmpFileHeader = (BITMAPFILEHEADER *)pBuf;
	
	//����bmpλͼ��Ϣ��BITMAPINFO
	pBmpInfo = (BITMAPINFO *)(pBuf + sizeof(BITMAPFILEHEADER));
	
	//һ��ɨ������ռ�õ��ֽ���
	int DataSizePerLine= (pBmpInfo->bmiHeader.biWidth * 
		pBmpInfo->bmiHeader.biBitCount+31)/8; 
	DataSizePerLine= DataSizePerLine/4*4;	//����
	
	//λͼ���ݴ�С
	int DataSize= DataSizePerLine * pBmpInfo->bmiHeader.biHeight;
	
	//ָ��ʵ�ʵ�������
	pBits = pBuf+pBmpFileHeader->bfOffBits;
	
	//�õ�һ�� HBITMAP����ʹ��һ��ͼƬ�ؼ���ʾ�ڽ�����
	HBITMAP hBmp = ::CreateDIBitmap(hDC, 
		&(pBmpInfo->bmiHeader), 
		CBM_INIT, 		
		pBuf+pBmpFileHeader->bfOffBits, 
		pBmpInfo, 
		DIB_RGB_COLORS);
	
	return hBmp;
} 

void CControlDlg::SetPos() //���öԻ����С
{
	ModifyStyle(WS_CAPTION, 0); // ȥ��������  
	ModifyStyleEx(WS_EX_DLGMODALFRAME, 0); // ȥ���߿�
	CWnd::ShowWindow(SW_SHOWMAXIMIZED );
}

void CControlDlg::Onmin() 
{
	CWnd::ShowWindow(SW_SHOWMINIMIZED ); //��С��
}

void CControlDlg::Onmax() 
{
	ModifyStyle(WS_CAPTION, 0); // ȥ��������
	CWnd::ShowWindow(SW_SHOWMAXIMIZED ); //���
}

void CControlDlg::Onrestore() 
{	 
	CWnd::ShowWindow(SW_RESTORE );  //��ԭ
	ModifyStyle( 0, WS_CAPTION );   //��ʾ������
}

void CControlDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CControlDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CControlDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CControlDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CControlDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CControlDlg::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnRButtonDblClk(nFlags, point);
}

void CControlDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnRButtonDown(nFlags, point);
}

void CControlDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnRButtonUp(nFlags, point);
}


void CControlDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	pt = point;
	CDialog::OnMouseMove(nFlags, point);
}
