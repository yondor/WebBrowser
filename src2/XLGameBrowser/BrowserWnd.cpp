#include "StdAfx.h"
#include "BrowserWnd.h"
#include "BrowserComunication.h"

const UINT BrowserWnd::WM_BROWSER_QUIT = WM_USER+3333;
const UINT BrowserWnd::WM_BROWSER_NAVIGATE = WM_USER+3334;

BrowserWnd::BrowserWnd(void)
{
	m_dwMark = 0;
}

BrowserWnd::~BrowserWnd(void)
{
}

void BrowserWnd::SetMark(DWORD dwMark)
{
	m_dwMark = dwMark;
}

HWND BrowserWnd::CreateBrowserWnd(HWND hOwner)
{
	CRect rect(200, 200, 800, 600);
	ATLASSERT(::IsWindow(hOwner));
	HWND hNewWnd = Create(hOwner, &rect, WebBrowser_CLSID, WS_CLIPSIBLINGS|WS_CHILD, WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE|WS_EX_NOPARENTNOTIFY);
	return hNewWnd;
}

void BrowserWnd::DestroyBrowserWnd()
{
	ATLASSERT(::IsWindow(m_hWnd));
	PostMessage(WM_BROWSER_QUIT, 0, 0);
}

//这个方法不是线程安全的。。。post到浏览器线程去
HRESULT BrowserWnd::Navigate(LPCTSTR pstrUrl,int nFlag ,LPCTSTR pTargetName,LPCTSTR pPostData,LPCTSTR pHeader)
{
	m_strNavigateUrl = pstrUrl;
 	PostMessage(WM_BROWSER_NAVIGATE, 0, 0);

	return 0L;
}

HRESULT BrowserWnd::GetBrowser(IWebBrowser2** ppWebBrowser)
{
	ATLASSERT( ppWebBrowser ); 
	if ( ppWebBrowser == NULL ) 
	{
		return E_POINTER; 
	}

	*ppWebBrowser = NULL;

	CAxWindow wnd = m_hWnd;
	CComPtr<IWebBrowser2> spWebBrowser;
	HRESULT hRet = this->QueryControl(IID_IWebBrowser2, (void**)&spWebBrowser);
	ATLASSERT(SUCCEEDED(hRet));

	return spWebBrowser.CopyTo(ppWebBrowser);
}

LRESULT BrowserWnd::OnBrowserNavigate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CComPtr<IWebBrowser2> spWebBrowser2;
	HRESULT hRet = GetBrowser(&spWebBrowser2);
	if(SUCCEEDED(hRet) && spWebBrowser2)
	{
		CComVariant vtURL(m_strNavigateUrl);
		CComVariant vtFlag(0);
		CComVariant vtTargetName;
// 		if(pTargetName != NULL)
// 		{
// 			vtTargetName = pTargetName;
// 		}
 		CComVariant vtPostData;
// 		if(pPostData != NULL)
// 		{
// 			vtPostData = pPostData;
// 		}
 		CComVariant vtHeader;
// 		if(pHeader != NULL)
// 		{
// 			vtHeader = pHeader;
// 		}

		hRet = spWebBrowser2->Navigate2(&vtURL, &vtFlag, &vtTargetName, &vtPostData, &vtHeader);
	}

	return 0L;
}

LRESULT BrowserWnd::OnBrowserQuit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//use destroy window without 'WS_EX_NOPARENTNOTIFY' will bring dead lock, for its parent is in other process.
	//If the window being destroyed is a child window that does not have the WS_EX_NOPARENTNOTIFY style, a WM_PARENTNOTIFY message is sent to the parent.
	//sendmessage will block..  so specify 'WS_EX_NOPARENTNOTIFY' is important
	//注：因为DestroyWindow如果未指定WS_EX_NOPARENTNOTIFY的话，会SendMessage到父窗口
	//而父窗口有可能也在反初始化中，根本处理不了此消息。那么发送就会阻塞。。造成死锁
	DestroyWindow();
	SetMsgHandled(TRUE);
	return 0L;
}

void BrowserWnd::OnDestroy()
{
	::PostQuitMessage(0);
}

int BrowserWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//操！这条还是必须的
	SetMsgHandled(FALSE);

	BrowserComunication::Instance()->NotifyHostBrowserCreated(m_dwMark);

	SetTimer(12345, 10, NULL);
	return 0;
}

void BrowserWnd::OnTimer(UINT_PTR nIDEvent)
{
	RECT rc;
	CWindow& wnd = GetParent();
	if(::IsWindow(wnd))
	{
		wnd.GetClientRect(&rc);
		rc.bottom -= 80;
		MoveWindow(&rc);

		if(::IsWindowVisible(wnd))
		{
			ShowWindow(SW_SHOW);
		}
		else
		{
			ShowWindow(SW_HIDE);
		}
	}
}