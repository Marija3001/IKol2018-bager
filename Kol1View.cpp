
// Kol2018View.cpp : implementation of the CKol2018View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Kol2018.h"
#endif

#include "Kol2018Doc.h"
#include "Kol2018View.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BODY_ARM_X 60.0
#define BODY_ARM_Y 170.0f

#define STEP_ANGLE 10.0
#define STEP_MOVE 10.0
#define SCALE_VILJUSKA 2.5
// CKol2018View

IMPLEMENT_DYNCREATE(CKol2018View, CView)

BEGIN_MESSAGE_MAP(CKol2018View, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_ERASEBKGND() //obavezno dodati!
	ON_WM_KEYDOWN() //obavezno dodati!
END_MESSAGE_MAP()

// CKol2018View construction/destruction

CKol2018View::CKol2018View() noexcept
{
	m_pBager = new DImage();
	m_pArm1 = new DImage();
	m_pArm2 = new DImage();
	m_pPozadina = new DImage();

	m_pBager->Load(CString("bager.png"));
	m_pArm1->Load(CString("arm1.png"));
	m_pArm2->Load(CString("arm2.png"));
	m_pPozadina->Load(CString("pozadina.png"));

	m_hViljuska = GetEnhMetaFile(CString("viljuska.emf"));
	ENHMETAHEADER header;
	GetEnhMetaFileHeader(m_hViljuska, sizeof(ENHMETAHEADER), &header);
	
	m_rectViljuska = CRect(header.rclBounds.left, header.rclBounds.top, header.rclBounds.right, header.rclBounds.bottom);
	m_angleArm1 = -90.0;
	m_angleArm2 = -90.0;
	m_angleFork = -90;
	m_moveX = 0.0;
}

CKol2018View::~CKol2018View()
{
	delete m_pBager;
	delete m_pArm1;
	delete m_pArm2;
	delete m_pPozadina;
	DeleteEnhMetaFile(m_hViljuska);
}

BOOL CKol2018View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CKol2018View drawing

void CKol2018View::OnDraw(CDC* pDC)
{
	CKol2018Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rcClient;
	GetClientRect(&rcClient);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);
	memDC.FillSolidRect(rcClient, RGB(255, 255, 255));

	int oldMode = memDC.SetGraphicsMode(GM_ADVANCED);
	XFORM oldForm;
	memDC.GetWorldTransform(&oldForm);

	DrawBackground(&memDC);
	/*DrawBody(&memDC);
	DrawArm1(&memDC);
	DrawArm2(&memDC);*/
	DrawExcavator(&memDC);
	memDC.SetWorldTransform(&oldForm);
	memDC.SetGraphicsMode(oldMode);

	pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);
	bmp.DeleteObject();
	memDC.DeleteDC();
}
void CKol2018View::Translate(CDC* pDC, double dX, double dY, bool rightMultiply)
{
	XFORM xForm = { 1.0, 0.0, 0.0, 1.0, dX, dY };
	pDC->ModifyWorldTransform(&xForm, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol2018View::Rotate(CDC* pDC, float angle, bool rightMultiply)
{
	float rad = (float)(angle * 3.14159265f / 180);
	XFORM xForm = {cos(rad), sin(rad), -sin(rad), cos(rad), 0, 0};
	pDC->ModifyWorldTransform(&xForm, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol2018View::Scale(CDC* pDC, float scaleX, float scaleY, bool rightMultiply)
{
	XFORM xForm = {scaleX, 0, 0, scaleY, 0, 0};
	pDC->ModifyWorldTransform(&xForm, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}
// CKol2018View printing

BOOL CKol2018View::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CKol2018View::DrawBackground(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	int w = m_pPozadina->Width();
	int h = m_pPozadina->Height();

	int x = (rect.Width() - w) / 2; //uvek centriran po x osi
	int y = (rect.Height() - h); //donja ivica na dnu

	m_pPozadina->Draw(pDC, CRect(0,0,w ,h), CRect(x, y,x+w, y+h));
}

void CKol2018View::DrawImgTransparent(CDC* pDC, DImage* pImage)
{
	unsigned char* bits = pImage->GetDIBBits();
	COLORREF bgColor = RGB(bits[2], bits[1], bits[0]); // uzimamo boju gornjeg levog piksela

	CRect rc(0, 0, pImage->Width(), pImage->Height());
	pImage->DrawTransparent(pDC, rc, rc, bgColor);
}

void CKol2018View::DrawBody(CDC* pDC) //telo bagera, pomeriti koord sistem, tako sa se ruka, koja ce se nacrtati, nalazi ispred kabine
{
	DrawImgTransparent(pDC, m_pBager);
	Translate(pDC, BODY_ARM_X, BODY_ARM_Y, false);
	/*Translate(pDC, BODY_ARM_X, BODY_ARM_Y, false);
	DrawImgTransparent(pDC, m_pBager);*/
}

void CKol2018View::DrawArm1(CDC* pDC)
{
	Rotate(pDC, (float)m_angleArm1, false); //okrece ga oko tog kruga
	Translate(pDC, -58.0, -61.0, false);//pomera ga tako da se tacka rotacije poklopi sa tackom rotacije tela
	DrawImgTransparent(pDC, m_pArm1);
	Translate(pDC, 309.0f, 61.0f, false); //ostavlja koord sistem u centru manjeg kruga
}

void CKol2018View::DrawArm2(CDC* pDC)
{
	Rotate(pDC, (float)m_angleArm2, false);
	Translate(pDC, -36.0f, -40.0f, false);
	DrawImgTransparent(pDC, m_pArm2);
	Translate(pDC, 272, 40, false);
}

void CKol2018View::DrawFork(CDC* pDC)
{
	Rotate(pDC, (float)m_angleFork, false);
	Scale(pDC, SCALE_VILJUSKA, SCALE_VILJUSKA, false);
	Translate(pDC, -14.0, -20.0, false);

	CRect rc(0, 0, m_rectViljuska.Width(), m_rectViljuska.Height());
	PlayEnhMetaFile(pDC->m_hDC, m_hViljuska, rc);
}

void CKol2018View::DrawExcavator(CDC* pDC) //bager se nalazi u inicijalno donje desnom uglu prozora
{
	CRect rcClient;
	GetClientRect(&rcClient);

	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Translate(pDC, (rcClient.Width() - m_pBager->Width() +m_moveX), (float)(rcClient.Height() - m_pBager->Height()), false);

	DrawBody(pDC);
	DrawArm1(pDC);
	DrawArm2(pDC);
	DrawFork(pDC);

	pDC->SetWorldTransform(&oldForm);

}

void CKol2018View::OnKeyDown(UINT nChar, UINT nRepCnt,UINT nFlags)
{
	switch (nChar)
	{
	case '1': case VK_NUMPAD1:m_angleArm1 -= STEP_ANGLE; break;
	case '2': case VK_NUMPAD2:m_angleArm1 += STEP_ANGLE; break;
	case '3': case VK_NUMPAD3:m_angleArm2 -= STEP_ANGLE; break;
	case '4': case VK_NUMPAD4:m_angleArm2 += STEP_ANGLE; break;
	case '5': case VK_NUMPAD5:m_angleFork += STEP_ANGLE; break;
	case '6': case VK_NUMPAD6:m_angleFork -= STEP_ANGLE; break;
	case VK_LEFT: m_moveX -= STEP_MOVE; break;
	case VK_RIGHT: m_moveX += STEP_MOVE; break;
	default:
		return;
	}
	Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CKol2018View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CKol2018View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CKol2018View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CKol2018View diagnostics

#ifdef _DEBUG
void CKol2018View::AssertValid() const
{
	CView::AssertValid();
}

void CKol2018View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CKol2018Doc* CKol2018View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CKol2018Doc)));
	return (CKol2018Doc*)m_pDocument;
}
#endif //_DEBUG


// CKol2018View message handlers
