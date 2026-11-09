// Kol1View.cpp : implementation of the CKol1View class  (I kolokvijum 2023 - Robot)
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

#define STEP_ANGLE 5.0    // korak ugla za ruku (stepeni)
#define STEP_ROBOT 10.0   // korak rotacije celog robota (stepeni)
#define STEP_SCALE 0.9    // faktor smanjenja / povecanja
#define STEP_MOVE  10.0   // pomeraj levo-desno (pikseli)

IMPLEMENT_DYNCREATE(CKol1View, CView)

BEGIN_MESSAGE_MAP(CKol1View, CView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// ---------------------------------------------------------------
// 1. Ucitavanje slika u konstruktoru, oslobadjanje u destruktoru
// ---------------------------------------------------------------
CKol1View::CKol1View() noexcept
{
	m_pGlava = new DImage();        m_pGlava->Load(CString("glava.png"));
	m_pNadkolenica = new DImage();  m_pNadkolenica->Load(CString("nadkolenica.png"));
	m_pNadlaktica = new DImage();   m_pNadlaktica->Load(CString("nadlaktica.png"));
	m_pPodkolenica = new DImage();  m_pPodkolenica->Load(CString("podkolenica.png"));
	m_pPodlaktica = new DImage();   m_pPodlaktica->Load(CString("podlaktica.png"));
	m_pSaka = new DImage();         m_pSaka->Load(CString("saka.png"));
	m_pStopalo = new DImage();      m_pStopalo->Load(CString("stopalo.png"));
	m_pTelo = new DImage();         m_pTelo->Load(CString("telo.png"));
	m_pPozadina = new DImage();     m_pPozadina->Load(CString("pozadina.jpg"));

	// 6.
	m_angleNadlaktica = 0.0;
	m_anglePodlaktica = 0.0;
	m_angleSaka = 0.0;
	m_angleRobot = 0.0;
	m_scaleRobot = 1.0;
	m_moveX = 0.0;
}

CKol1View::~CKol1View()
{
	delete m_pGlava;
	delete m_pNadkolenica;
	delete m_pNadlaktica;
	delete m_pPodkolenica;
	delete m_pPodlaktica;
	delete m_pSaka;
	delete m_pStopalo;
	delete m_pTelo;
	delete m_pPozadina;
}

BOOL CKol1View::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ---------------------------------------------------------------
// 2. Crtanje bez treperenja (memorijski DC)
// ---------------------------------------------------------------
BOOL CKol1View::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;   // pozadinu brise memorijski DC, ne Windows
}

// ---------------------------------------------------------------
// 11. OnDraw - cela scena
// ---------------------------------------------------------------
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

	// 10. centar robota se postavlja u centar prozora; oko te tacke robot se
	//     rotira i skalira (transformacije se primenjuju odozdo nagore)
	Translate(&memDC, rcClient.Width() / 2.0f + (float)m_moveX, rcClient.Height() / 2.0f, false);
	Rotate(&memDC, (float)m_angleRobot, false);
	Scale(&memDC, (float)m_scaleRobot, (float)m_scaleRobot, false);
	// centar robota = tacka na osi simetrije (desna ivica tela), na pola visine tela
	Translate(&memDC, -(float)m_pTelo->Width(), -m_pTelo->Height() / 2.0f, false);

	DrawRobot(&memDC);

	memDC.SetWorldTransform(&oldForm);
	memDC.SetGraphicsMode(oldMode);

	pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);
	bmp.DeleteObject();
	memDC.DeleteDC();
}

// ---------------------------------------------------------------
// 3. Pozadina u prirodnoj velicini
// ---------------------------------------------------------------
void CKol1View::DrawBackground(CDC* pDC)
{
	int w = m_pPozadina->Width();
	int h = m_pPozadina->Height();
	m_pPozadina->Draw(pDC, CRect(0, 0, w, h), CRect(0, 0, w, h));
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
// 5. Transformacije
//    rightMultiply = false -> MWT_LEFTMULTIPLY (lokalna, "unutrasnja" transformacija)
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
// 7. Leva polovina robota (bez glave); koordinatni pocetak = gornji levi ugao tela
//
//    Tabela 1:
//    Telo        (25,65)  -> Nadlaktica  (35,35)
//    Nadlaktica  (22,167) -> Podlaktica  (30,33)
//    Podlaktica  (30,140) -> Saka        (25,3)
//    Telo        (61,262) -> Nadkolenica (29,20)
//    Nadkolenica (30,184) -> Podkolenica (25,37)
//    Podkolenica (25,248) -> Stopalo     (20,16)
//
//    Nadovezivanje: Translate(tacka na roditelju), Rotate(ugao),
//    Translate(-tacka na detetu) -> dete se rotira oko svoje tacke kacenja.
// ---------------------------------------------------------------
void CKol1View::DrawHalf(CDC* pDC)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	// noga (crta se prva, da bi telo bilo preko nje)
	Translate(pDC, 61.0f, 262.0f, false);
	Translate(pDC, -29.0f, -20.0f, false);
	DrawImgTransparent(pDC, m_pNadkolenica);

	Translate(pDC, 30.0f, 184.0f, false);
	Translate(pDC, -25.0f, -37.0f, false);
	DrawImgTransparent(pDC, m_pPodkolenica);

	Translate(pDC, 25.0f, 248.0f, false);
	Translate(pDC, -20.0f, -16.0f, false);
	DrawImgTransparent(pDC, m_pStopalo);

	pDC->SetWorldTransform(&oldForm);

	// telo
	DrawImgTransparent(pDC, m_pTelo);

	// ruka (crta se preko tela)
	Translate(pDC, 25.0f, 65.0f, false);
	Rotate(pDC, (float)m_angleNadlaktica, false);
	Translate(pDC, -35.0f, -35.0f, false);
	DrawImgTransparent(pDC, m_pNadlaktica);

	Translate(pDC, 22.0f, 167.0f, false);
	Rotate(pDC, (float)m_anglePodlaktica, false);
	Translate(pDC, -30.0f, -33.0f, false);
	DrawImgTransparent(pDC, m_pPodlaktica);

	Translate(pDC, 30.0f, 140.0f, false);
	Rotate(pDC, (float)m_angleSaka, false);
	Translate(pDC, -25.0f, -3.0f, false);
	DrawImgTransparent(pDC, m_pSaka);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 8. Glava - centrirana na osi simetrije (x = sirina tela), iznad tela
// ---------------------------------------------------------------
void CKol1View::DrawHead(CDC* pDC)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Translate(pDC, m_pTelo->Width() - m_pGlava->Width() / 2.0f, -(float)m_pGlava->Height(), false);
	DrawImgTransparent(pDC, m_pGlava);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 9. Ceo robot: leva polovina + refleksija oko ose x = sirina tela + glava
// ---------------------------------------------------------------
void CKol1View::DrawRobot(CDC* pDC)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	// leva polovina
	DrawHalf(pDC);

	// desna polovina: x' = 2W - x  (prvo Scale(-1,1), pa Translate(2W,0))
	Translate(pDC, 2.0f * m_pTelo->Width(), 0.0f, false);
	Scale(pDC, -1.0f, 1.0f, false);
	DrawHalf(pDC);
	pDC->SetWorldTransform(&oldForm);

	// glava
	DrawHead(pDC);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 7. i 10. Tasteri
//    A/S saka, D/F podlaktica, G/H nadlaktica (sa ogranicenjima)
//    1 - rotacija u jednu stranu + smanjenje, 2 - rotacija u drugu + povecanje
// ---------------------------------------------------------------
void CKol1View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case 'A': m_angleSaka = max(m_angleSaka - STEP_ANGLE, -10.0); break;
	case 'S': m_angleSaka = min(m_angleSaka + STEP_ANGLE, 30.0); break;
	case 'D': m_anglePodlaktica = max(m_anglePodlaktica - STEP_ANGLE, -10.0); break;
	case 'F': m_anglePodlaktica = min(m_anglePodlaktica + STEP_ANGLE, 80.0); break;
	case 'G': m_angleNadlaktica = max(m_angleNadlaktica - STEP_ANGLE, -10.0); break;
	case 'H': m_angleNadlaktica = min(m_angleNadlaktica + STEP_ANGLE, 90.0); break;
	case '1':
	case VK_NUMPAD1:
		m_angleRobot -= STEP_ROBOT;
		m_scaleRobot *= STEP_SCALE;
		break;
	case '2':
	case VK_NUMPAD2:
		m_angleRobot += STEP_ROBOT;
		m_scaleRobot /= STEP_SCALE;
		break;
	case VK_LEFT:  m_moveX -= STEP_MOVE; break;
	case VK_RIGHT: m_moveX += STEP_MOVE; break;
	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
