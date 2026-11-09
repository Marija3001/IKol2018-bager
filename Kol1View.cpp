// Kol1View.cpp : implementation of the CKol1View class  (I kolokvijum 2018 - Bager)
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "Kol1.h"
#endif

#include "Kol1Doc.h"
#include "Kol1View.h"
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define STEP_ANGLE 10.0   // korak rotacije (stepeni)
#define STEP_MOVE  10.0   // pomeraj bagera po X (pikseli)
#define FORK_SCALE 2.5f   // uvecanje viljuske

// tacka na slici bager.png gde se kaci ruka (ispred kabine).
// PROVERITI na slici bager.png i po potrebi korigovati.
#define BODY_ARM_X 5.0f
#define BODY_ARM_Y 105.0f

IMPLEMENT_DYNCREATE(CKol1View, CView)

BEGIN_MESSAGE_MAP(CKol1View, CView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// ---------------------------------------------------------------
// 1. Ucitavanje slika i metafajla, oslobadjanje u destruktoru
// ---------------------------------------------------------------
CKol1View::CKol1View() noexcept
{
	m_pBager = new DImage();     m_pBager->Load(CString("bager.png"));
	m_pArm1 = new DImage();      m_pArm1->Load(CString("arm1.png"));
	m_pArm2 = new DImage();      m_pArm2->Load(CString("arm2.png"));
	m_pPozadina = new DImage();  m_pPozadina->Load(CString("pozadina.png"));

	// metafajl + njegov okvirni pravougaonik (iz zaglavlja)
	m_hFork = GetEnhMetaFile(_T("viljuska.emf"));
	ENHMETAHEADER header;
	GetEnhMetaFileHeader(m_hFork, sizeof(ENHMETAHEADER), &header);
	m_rcFork = CRect(header.rclBounds.left, header.rclBounds.top,
	                 header.rclBounds.right, header.rclBounds.bottom);

	// 10. svaka komponenta je inicijalno zarotirana za -90 u odnosu na prethodnu
	m_angleArm1 = -90.0;
	m_angleArm2 = -90.0;
	m_angleFork = -90.0;
	m_moveX = 0.0;
}

CKol1View::~CKol1View()
{
	delete m_pBager;
	delete m_pArm1;
	delete m_pArm2;
	delete m_pPozadina;
	DeleteEnhMetaFile(m_hFork);
}

BOOL CKol1View::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ---------------------------------------------------------------
// 2. Transformacije
//    rightMultiply = false -> MWT_LEFTMULTIPLY (lokalna transformacija)
//    rightMultiply = true  -> MWT_RIGHTMULTIPLY (globalna transformacija)
// ---------------------------------------------------------------
void CKol1View::Translate(CDC* pDC, float dX, float dY, bool rightMultiply)
{
	XFORM form = { 1.0f, 0.0f, 0.0f, 1.0f, dX, dY };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Rotate(CDC* pDC, float angle, bool rightMultiply)
{
	float rad = (float)(angle * M_PI / 180.0);
	XFORM form = { cosf(rad), sinf(rad), -sinf(rad), cosf(rad), 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Scale(CDC* pDC, float sX, float sY, bool rightMultiply)
{
	XFORM form = { sX, 0.0f, 0.0f, sY, 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

// ---------------------------------------------------------------
// 3. Pozadina: centrirana po X, donja ivica = donja ivica prozora
// ---------------------------------------------------------------
void CKol1View::DrawBackground(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int w = m_pPozadina->Width();
	int h = m_pPozadina->Height();

	int x = (rcClient.Width() - w) / 2;
	int y = rcClient.Height() - h;

	m_pPozadina->Draw(pDC, CRect(0, 0, w, h), CRect(x, y, x + w, y + h));
}

// ---------------------------------------------------------------
// 4. Transparentno crtanje - transparentna je boja prvog piksela
// ---------------------------------------------------------------
void CKol1View::DrawImgTransparent(CDC* pDC, DImage* pImage)
{
	unsigned char* bits = pImage->GetDIBBits();
	COLORREF bgColor = RGB(bits[2], bits[1], bits[0]);   // u memoriji je BGR(A)

	CRect rc(0, 0, pImage->Width(), pImage->Height());
	pImage->DrawTransparent(pDC, rc, rc, bgColor);
}

// ---------------------------------------------------------------
// 5. Telo bagera; posle crtanja lokalni k.s. se pomera u tacku
//    kacenja ruke (ispred kabine) - NE vraca se transformacija
// ---------------------------------------------------------------
void CKol1View::DrawBody(CDC* pDC)
{
	Translate(pDC, BODY_ARM_X, BODY_ARM_Y, false);
	DrawImgTransparent(pDC, m_pBager);
}

// ---------------------------------------------------------------
// 6. Prvi deo ruke: rotacija oko veceg kruga (58,61);
//    na kraju k.s. ostaje u centru manjeg kruga (309,61)
// ---------------------------------------------------------------
void CKol1View::DrawArm1(CDC* pDC)
{
	Rotate(pDC, (float)m_angleArm1, false);
	Translate(pDC, -58.0f, -61.0f, false);
	DrawImgTransparent(pDC, m_pArm1);
	Translate(pDC, 309.0f, 61.0f, false);
}

// ---------------------------------------------------------------
// 7. Drugi deo ruke: rotacija oko veceg kruga (36,40);
//    na kraju k.s. ostaje u centru manjeg kruga (272,40)
// ---------------------------------------------------------------
void CKol1View::DrawArm2(CDC* pDC)
{
	Rotate(pDC, (float)m_angleArm2, false);
	Translate(pDC, -36.0f, -40.0f, false);
	DrawImgTransparent(pDC, m_pArm2);
	Translate(pDC, 272.0f, 40.0f, false);
}

// ---------------------------------------------------------------
// 8. Viljuska: rotacija oko (14,20), uvecanje 2.5
// ---------------------------------------------------------------
void CKol1View::DrawFork(CDC* pDC)
{
	Rotate(pDC, (float)m_angleFork, false);
	Scale(pDC, FORK_SCALE, FORK_SCALE, false);
	Translate(pDC, -14.0f, -20.0f, false);

	CRect rc(0, 0, m_rcFork.Width(), m_rcFork.Height());
	PlayEnhMetaFile(pDC->m_hDC, m_hFork, rc);
}

// ---------------------------------------------------------------
// 9. Ceo bager, inicijalno u donjem desnom uglu prozora
// ---------------------------------------------------------------
void CKol1View::DrawExcavator(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Translate(pDC,
		(float)(rcClient.Width() - m_pBager->Width() + m_moveX),
		(float)(rcClient.Height() - m_pBager->Height()), false);

	DrawBody(pDC);    // k.s. -> tacka kacenja ruke
	DrawArm1(pDC);    // k.s. -> kraj arm1
	DrawArm2(pDC);    // k.s. -> kraj arm2
	DrawFork(pDC);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 11. Crtanje scene bez flicker-a (memorijski DC)
// ---------------------------------------------------------------
BOOL CKol1View::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;   // pozadinu brise memorijski DC, ne Windows
}

void CKol1View::OnDraw(CDC* pDC)
{
	CKol1Doc* pDoc = GetDocument();
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
	DrawExcavator(&memDC);

	memDC.SetWorldTransform(&oldForm);
	memDC.SetGraphicsMode(oldMode);

	pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);
	bmp.DeleteObject();
	memDC.DeleteDC();
}

// ---------------------------------------------------------------
// 10. Tasteri: 1/2 arm1, 3/4 arm2, 5/6 viljuska, <- / -> pomeranje
// ---------------------------------------------------------------
void CKol1View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case '1': case VK_NUMPAD1: m_angleArm1 -= STEP_ANGLE; break;
	case '2': case VK_NUMPAD2: m_angleArm1 += STEP_ANGLE; break;
	case '3': case VK_NUMPAD3: m_angleArm2 -= STEP_ANGLE; break;
	case '4': case VK_NUMPAD4: m_angleArm2 += STEP_ANGLE; break;
	case '5': case VK_NUMPAD5: m_angleFork -= STEP_ANGLE; break;
	case '6': case VK_NUMPAD6: m_angleFork += STEP_ANGLE; break;
	case VK_LEFT:  m_moveX -= STEP_MOVE; break;
	case VK_RIGHT: m_moveX += STEP_MOVE; break;
	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
